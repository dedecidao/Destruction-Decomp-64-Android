/*
 * arm64 address-space probe (built instead of the game with -Pdd64.probe=true,
 * until the 64-bit conversion links). It answers one question on a real phone:
 * can a 64-bit app process map the N64 RAM image at 0x80000000-0x82011000, the
 * way the port needs? It logs what is mapped there, tries the reservation and
 * shows the answer. Nothing from the game runs.
 */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <android/log.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#ifndef MAP_FIXED_NOREPLACE
#define MAP_FIXED_NOREPLACE 0x100000
#endif

const char *DD64A_GetBasePath(void);

#define LO 0x80000000ul
#define HI 0x82011000ul

static FILE *sLog;
static char sReport[4096];

static void say(const char *fmt, ...) {
    char buf[512];
    va_list ap;

    va_start(ap, fmt);
    SDL_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    __android_log_write(ANDROID_LOG_INFO, "DD64", buf);
    if (sLog != NULL) {
        fprintf(sLog, "%s\n", buf);
        fflush(sLog);
    }
    SDL_strlcat(sReport, buf, sizeof(sReport));
    SDL_strlcat(sReport, "\n", sizeof(sReport));
}

int main(int argc, char **argv) {
    char path[1024];
    FILE *maps;
    char line[512];
    int inTheWay = 0;
    void *p;

    SDL_snprintf(path, sizeof(path), "%sData/Logs", DD64A_GetBasePath());
    SDL_CreateDirectory(path);
    SDL_strlcat(path, "/dd64.log", sizeof(path));
    sLog = fopen(path, "w");

    say("probe: %d-bit process, pointer %u bytes", (int)(sizeof(void *) * 8), (unsigned)sizeof(void *));
    maps = fopen("/proc/self/maps", "r");
    while (maps != NULL && fgets(line, sizeof(line), maps) != NULL) {
        unsigned long lo, hi;
        if (sscanf(line, "%lx-%lx", &lo, &hi) == 2 && hi > LO && lo < HI) {
            line[strcspn(line, "\n")] = 0;
            say("probe: in the way: %s", line);
            inTheWay++;
        }
    }
    if (maps != NULL) {
        fclose(maps);
    }
    p = mmap((void *)LO, HI - LO, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (p == (void *)LO) {
        say("probe: OK, 0x%lx-0x%lx mapped (%d mappings were near it)", LO, HI, inTheWay);
        ((volatile unsigned char *)p)[0] = 1;
        munmap(p, HI - LO);
    } else {
        say("probe: FAILED to map 0x%lx-0x%lx (got %p)", LO, HI, p);
    }
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "DD64 arm64 probe", sReport, NULL);
    if (sLog != NULL) {
        fclose(sLog);
    }
    return 0;
}
