/*
 * Entry point on Android, in place of external/dd64/port/src/main.c.
 *
 * SDLActivity (GameActivity.java) loads libSDL3.so and libdd64.so and calls
 * SDL_main on its own thread. The arguments are the path of the ROM the
 * launcher copied into the app's storage. The rest is the PC main loop as it is.
 */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <android/log.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unwind.h>
#include <dlfcn.h>

#include "port.h"
#include "port_config.h"
#include "port_raphnet.h"

void boot(void);
const char *DD64A_GetBasePath(void);

volatile int gPortQuit;
static FILE *sLogFile;

void Port_Log(const char *fmt, ...) {
    char buf[1024];
    va_list ap;

    va_start(ap, fmt);
    SDL_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    __android_log_write(ANDROID_LOG_INFO, "DD64", buf);
    if (sLogFile != NULL) {
        fprintf(sLogFile, "%s\n", buf);
        fflush(sLogFile);
    }
}

void Port_Fatal(const char *fmt, ...) {
    char buf[1024];
    va_list ap;

    va_start(ap, fmt);
    SDL_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    Port_Log("fatal: %s", buf);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Destruction Derby 64", buf, NULL);
    exit(1);
}

void Port_RequestQuit(void) {
    gPortQuit = 1;
}

/* ---- Crash log: registers and call chain, for llvm-addr2line on libdd64.so ---- */

typedef struct {
    int n;
} UnwindState;

static _Unwind_Reason_Code unwindOne(struct _Unwind_Context *ctx, void *arg) {
    UnwindState *st = arg;
    uintptr_t pc = _Unwind_GetIP(ctx);
    Dl_info d;

    if (pc == 0 || st->n >= 32) {
        return _URC_END_OF_STACK;
    }
    if (dladdr((void *)pc, &d) && d.dli_fname != NULL) {
        Port_Log("crash: #%02d %08X %s+%X (%s)", st->n, (unsigned)pc, d.dli_fname,
                 (unsigned)(pc - (uintptr_t)d.dli_fbase), d.dli_sname != NULL ? d.dli_sname : "?");
    } else {
        Port_Log("crash: #%02d %08X", st->n, (unsigned)pc);
    }
    st->n++;
    return _URC_NO_REASON;
}

static void crashHandler(int sig, siginfo_t *si, void *uc) {
    const mcontext_t *m = &((const ucontext_t *)uc)->uc_mcontext;
    UnwindState st = { 0 };
    Dl_info d;

    Port_Log("crash: signal %d at pc %08lX, accessing %p", sig, (unsigned long)m->arm_pc, si->si_addr);
    if (dladdr((void *)m->arm_pc, &d) && d.dli_fname != NULL) {
        Port_Log("crash: pc = %s+%lX", d.dli_fname, (unsigned long)(m->arm_pc - (uintptr_t)d.dli_fbase));
    }
    if (dladdr((void *)m->arm_lr, &d) && d.dli_fname != NULL) {
        Port_Log("crash: lr = %s+%lX", d.dli_fname, (unsigned long)(m->arm_lr - (uintptr_t)d.dli_fbase));
    }
    Port_Log("crash: r0 %08lX r1 %08lX r2 %08lX r3 %08lX r4 %08lX r5 %08lX r6 %08lX r7 %08lX",
             m->arm_r0, m->arm_r1, m->arm_r2, m->arm_r3, m->arm_r4, m->arm_r5, m->arm_r6, m->arm_r7);
    Port_Log("crash: r8 %08lX r9 %08lX r10 %08lX fp %08lX ip %08lX sp %08lX lr %08lX",
             m->arm_r8, m->arm_r9, m->arm_r10, m->arm_fp, m->arm_ip, m->arm_sp, m->arm_lr);
    _Unwind_Backtrace(unwindOne, &st);
    signal(sig, SIG_DFL);
    raise(sig);
}

static void installCrashHandler(void) {
    static const int sigs[] = { SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT };
    struct sigaction sa;
    size_t i;

    SDL_zero(sa);
    sa.sa_sigaction = crashHandler;
    sa.sa_flags = SA_SIGINFO | SA_RESETHAND | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    for (i = 0; i < SDL_arraysize(sigs); i++) {
        sigaction(sigs[i], &sa, NULL);
    }
}

static void openLog(void) {
    const char *base = DD64A_GetBasePath();
    char path[1024];

    SDL_snprintf(path, sizeof(path), "%sData/Logs", base);
    SDL_CreateDirectory(path);
    SDL_snprintf(path, sizeof(path), "%sData/Logs/dd64.log", base);
    sLogFile = fopen(path, "w");
}

int main(int argc, char **argv) {
    Port_RamReserve(); /* 0x80000000 up, before anything else can take it */
    openLog();
    installCrashHandler();
    if (argc > 1 && argv[1] != NULL && argv[1][0] != '\0') {
        SDL_setenv_unsafe("DD64_ROM", argv[1], 1); /* rom.c tries DD64_ROM first */
    }
    /* Landscape, full screen, no on-screen keyboard; the back button is the game's Start */
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
    Port_ConfigLoad();
    gPortConfig.fullscreen = 1;
    SDL_SetAppMetadata("Destruction Derby 64", "0.1", "io.github.dedecidao.dd64");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
        Port_Fatal("SDL_Init: %s", SDL_GetError());
    }
    if (!Port_RomLoad()) {
        Port_Fatal("Could not load the Destruction Derby 64 (USA) ROM.\n\n"
                   "Pick your cart image (.z64, .n64 or .v64) again on the start screen.");
    }
    Port_RamInit();
    Port_OsInit();
    Port_RcpInit();
    Port_VideoInit();
    Port_AudioInit();
    Port_InputInit();

    boot();
    Port_ViStart();

    while (!gPortQuit) {
        Port_PumpEvents();
        Port_RcpRunPending();
        Port_ViRetrace();
        Port_VideoPresentInterp();
        Port_OptionsApplyPending();
        Port_RcpWait(SDL_GetTicksNS() + 1000000);
    }

    SDL_Quit();
    /* The game's threads have no way to stop cleanly: end the process, so that
       the next launch starts with the RAM image mapped afresh */
    exit(0);
}
