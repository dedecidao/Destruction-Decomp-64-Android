/*
 * The N64 RAM image at its own N64 addresses, as external/dd64/port/src/ram.c
 * does on PC, with ARM code in the jump stubs instead of x86.
 *
 *   0x80000400-0x8007ADD0  at each game function's N64 address, a stub that jumps
 *                          to the native function (N64 function pointers in the
 *                          game's data stay callable)
 *   0x8007ADD0-0x82000000  data, rodata, bss, boot blocks and heap
 *   0x82000000-0x82010000  scratch for PORT_DMA_BUF
 *   then a guard page
 *
 * A stub is 8 bytes, in ARM state (the N64 addresses are even):
 *     ldr pc, [pc, #-4]
 *     .word target          (bit 0 set for Thumb targets: ldr pc interworks)
 * Every N64 function is at least 8 bytes (jr ra and its delay slot), and the
 * overlay functions at 0x80134280 and 0x80134288 are 8 apart, so stubs never
 * overlap. Everything else in the code range is a permanently undefined
 * instruction, so a call to a function the port lacks stops right there.
 */
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "port.h"
#include "port_ram.h"
#include "port_shadow.h"

#define MAP_BASE 0x80000000u
#define RAM_VRAM PORT_RAM_VRAM
#define RAM_END PORT_RAM_END
#define MAP_END (RAM_END + PORT_SCRATCH_SIZE)
#define CODE_END 0x8007ADD0u
#define PAGE 0x1000u
#define STUB_SIZE 8u

#define ARM_LDR_PC_PC_M4 0xE51FF004u /* ldr pc, [pc, #-4] */
#define ARM_UDF 0xE7F000F0u          /* udf #0 */

#ifndef MAP_FIXED_NOREPLACE
#define MAP_FIXED_NOREPLACE 0x100000
#endif

extern u8 gRam[]; /* 0x8007ADD0, from dd64_syms.ld */

void *Port_N64ToHost(uint32_t addr) {
    addr = 0x80000000u | (addr & 0x1FFFFFFF); /* KSEG1 is the same memory */
    return addr >= RAM_VRAM && addr < MAP_END ? (void *)addr : NULL;
}

uint32_t Port_HostToN64(const void *p) {
    u32 a = (u32)p;

    return a >= RAM_VRAM && a < MAP_END ? a : 0;
}

static void protect(u32 start, u32 end, int exec, int write) {
    int prot = (write || exec) ? PROT_READ : PROT_NONE;

    if (write) prot |= PROT_WRITE;
    if (exec) prot |= PROT_EXEC;
    if (mprotect((void *)start, end - start, prot) != 0) {
        Port_Log("ram: mprotect %08X-%08X failed", start, end);
    }
}

static void trapUnknownFunction(void) {
    Port_Fatal("The game called an N64 function the port does not have");
}

static void writeJump(u32 at, const void *target) {
    u32 *p = (u32 *)at;

    p[0] = ARM_LDR_PC_PC_M4;
    p[1] = (u32)target;
}

static void flushCode(u32 start, u32 end) {
    __builtin___clear_cache((char *)start, (char *)end);
}

static u8 *sMapped;

void Port_RamReserve(void) {
    void *base = mmap((void *)MAP_BASE, MAP_END + PAGE - MAP_BASE, PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);

    if (base == MAP_FAILED) {
        base = NULL;
    } else if (base != (void *)MAP_BASE) {
        /* An old kernel without MAP_FIXED_NOREPLACE took it as a hint */
        munmap(base, MAP_END + PAGE - MAP_BASE);
        base = NULL;
    }
    sMapped = base;
}

/* The mapping the process already has around 0x80000000, for the error message */
static void logMapsNear(void) {
    FILE *f = fopen("/proc/self/maps", "r");
    char line[512];

    while (f != NULL && fgets(line, sizeof(line), f) != NULL) {
        unsigned long lo, hi;
        if (sscanf(line, "%lx-%lx", &lo, &hi) == 2 && hi > MAP_BASE && lo < MAP_END + PAGE) {
            line[strcspn(line, "\n")] = 0;
            Port_Log("ram: in the way: %s", line);
        }
    }
    if (f != NULL) {
        fclose(f);
    }
}

void Port_RamInit(void) {
    u8 *base;
    u32 i, *w;

    if (sMapped == NULL) {
        Port_RamReserve();
    }
    base = sMapped;
    if (base != (u8 *)MAP_BASE || gRam != (u8 *)RAM_VRAM || gPortRamVram != RAM_VRAM) {
        logMapsNear();
        Port_Fatal("Could not map the game's memory at 0x80000000.\n\n"
                   "Something else in the app's process already uses that address. "
                   "Please send Data/Logs/dd64.log to the developers.");
    }

    for (w = (u32 *)MAP_BASE; w < (u32 *)CODE_END; w++) {
        *w = ARM_UDF;
    }
    for (i = 0; i < gPortAllFuncCount; i++) {
        const void *target = gPortAllFuncs[i].func;
        u32 a = gPortAllFuncs[i].addr;

        if (a < MAP_BASE || a + STUB_SIZE > CODE_END || (a & 3) != 0) {
            continue;
        }
        writeJump(a, target != NULL ? target : (const void *)trapUnknownFunction);
    }
    flushCode(MAP_BASE, CODE_END);

    memcpy(gRam, gPortRom + gPortRamRomStart, gPortRamRomEnd - gPortRamRomStart);
    memset(gRam + (gPortRamRomEnd - gPortRamRomStart), 0,
           MAP_END - RAM_VRAM - (gPortRamRomEnd - gPortRamRomStart));
    Port_MarkBE(gRam, gPortRamRomEnd - gPortRamRomStart);
    Port_ConvertStaticData(gRam, gPortRamRomEnd - gPortRamRomStart);

    protect(MAP_BASE, CODE_END & ~(PAGE - 1), 1, 0);
    protect(CODE_END & ~(PAGE - 1), (CODE_END & ~(PAGE - 1)) + PAGE, 1, 1);
    protect((CODE_END & ~(PAGE - 1)) + PAGE, MAP_END, 0, 1);
    protect(MAP_END, MAP_END + PAGE, 0, 0);
}

/* Boot_LoadData copied a block from ROM: stubs for the overlay's C functions */
void Port_BlockLoaded(void *dst, u32 size) {
    u32 start = (u32)dst, i;

    Port_ConvertStaticData(dst, size);

    for (i = 0; i < gPortOverlayFuncCount; i++) {
        u32 a = gPortOverlayFuncs[i].addr;
        if (a >= start && a + STUB_SIZE <= start + size) {
            u32 page = a & ~(PAGE - 1);
            protect(page, page + PAGE, 1, 1);
            writeJump(a, gPortOverlayFuncs[i].func);
            flushCode(a, a + STUB_SIZE);
            Port_MarkNative((void *)a, STUB_SIZE);
        }
    }
}
