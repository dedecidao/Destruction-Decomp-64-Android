/*
 * libdd64reserve.so: preloaded by wrap.sh (debug builds, armeabi-v7a) so that it
 * runs before ART starts in the app's process.
 *
 * In a 32-bit process forked from the zygote, ART puts its JIT code cache and the
 * GC's linear-alloc shadow map right on 0x80000000-0x83A00000 (seen on a Galaxy
 * S23 Ultra, Android 17), before any app code runs, and the game's RAM image has
 * to live at its N64 addresses (android/ram_arm.c). wrap.sh starts the process
 * with exec instead of a zygote fork, and this constructor reserves the range
 * (PROT_NONE) before the runtime maps anything. Port_RamReserve then maps the
 * RAM image over this reservation, which it finds with dlsym.
 */
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/prctl.h>

#ifndef MAP_FIXED_NOREPLACE
#define MAP_FIXED_NOREPLACE 0x100000
#endif
#ifndef PR_SET_VMA
#define PR_SET_VMA 0x53564d41
#define PR_SET_VMA_ANON_NAME 0
#endif

#define RESERVE_BASE 0x80000000u
#define RESERVE_SIZE 0x02400000u /* 0x80000000-0x82400000: RAM, scratch, guard and room */

__attribute__((visibility("default"))) uintptr_t dd64_low_reserve_base;
__attribute__((visibility("default"))) size_t dd64_low_reserve_size;

__attribute__((constructor(101))) static void dd64Reserve(void) {
    void *p;

    if (sizeof(void *) != 4) {
        return; /* arm64 maps 0x80000000 without help */
    }
    p = mmap((void *)(uintptr_t)RESERVE_BASE, RESERVE_SIZE, PROT_NONE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE | MAP_FIXED_NOREPLACE, -1, 0);
    if (p == MAP_FAILED) {
        return;
    }
    if (p != (void *)(uintptr_t)RESERVE_BASE) {
        munmap(p, RESERVE_SIZE); /* kernel without MAP_FIXED_NOREPLACE: took it as a hint */
        return;
    }
    prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, (unsigned long)p, RESERVE_SIZE, "dd64 RAM reserve");
    dd64_low_reserve_base = RESERVE_BASE;
    dd64_low_reserve_size = RESERVE_SIZE;
}