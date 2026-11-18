/*
 * Touch controls and player 1's pad on Android.
 *
 * The on-screen controls are drawn and tracked in Java (TouchControlsView),
 * above the game's surface, so they work with any renderer. The view sends the
 * pressed N64 buttons and the stick (N64 units, -80..80) here; they are merged
 * into player 1's pad.
 *
 * port/src/input.c is built with Port_InputGetPad renamed to
 * Port_InputGetPadHost (CMakeLists.txt): this file is the Port_InputGetPad the
 * rest of the port calls, wrapping the PC one (gamepads, scripted input).
 */
#include <SDL3/SDL.h>
#include <jni.h>

#include "port.h"

void Port_InputGetPadHost(int channel, OSContPad *out);
int Port_InputConnectedHost(int channel);

static SDL_AtomicInt sTouchButtons;
static SDL_AtomicInt sTouchStick; /* x in the low 16 bits, y in the high, signed */
static SDL_AtomicInt sTouchActive;
/* Buttons pressed since the game last read the pad: a tap shorter than the game's
   polling interval still counts once */
static SDL_AtomicInt sTouchLatched;

JNIEXPORT void JNICALL Java_io_github_dedecidao_dd64_TouchControlsView_nativeSetState(JNIEnv *env, jclass cls,
                                                                                     jint buttons, jint x, jint y) {
    int old = SDL_GetAtomicInt(&sTouchButtons), latched;
    do {
        latched = SDL_GetAtomicInt(&sTouchLatched);
    } while (!SDL_CompareAndSwapAtomicInt(&sTouchLatched, latched, latched | (buttons & ~old & 0xFFFF)));
    SDL_SetAtomicInt(&sTouchButtons, buttons & 0xFFFF);
    SDL_SetAtomicInt(&sTouchStick, (int)((u32)(u16)(s16)x | ((u32)(u16)(s16)y << 16)));
    SDL_SetAtomicInt(&sTouchActive, 1);
}

void Port_InputGetPad(int channel, OSContPad *out) {
    Port_InputGetPadHost(channel, out);
    if (channel == 0 && SDL_GetAtomicInt(&sTouchActive)) {
        u32 stick = (u32)SDL_GetAtomicInt(&sTouchStick);
        s8 x = (s8)(s16)(stick & 0xFFFF), y = (s8)(s16)(stick >> 16);
        out->button |= (u16)(SDL_GetAtomicInt(&sTouchButtons) | SDL_SetAtomicInt(&sTouchLatched, 0));
        /* The touch stick wins while it is held; a gamepad's stick otherwise */
        if (x != 0 || y != 0) {
            out->stick_x = x;
            out->stick_y = y;
        }
    }
}

int Port_InputConnected(int channel) {
    return Port_InputConnectedHost(channel);
}
