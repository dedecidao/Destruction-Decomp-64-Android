/*
 * Rumble on Android: the phone vibrates when the game rumbles player 1 and no
 * gamepad is connected.
 *
 * The game rumbles through Pad_Rumble(channel, frames, strength)
 * (src/auto/59D40.c): car hits (28C70.c), wrecks and landings (1C624.c), and
 * 2E9D0.c; Pad_Update (59B08.c) then switches the Rumble Pak's motor on and off
 * through __osMotorAccess (port/src/os_hw.c) -> Port_InputRumble, pulsing it every
 * few frames for a weaker rumble, as the N64 did. On PC Port_InputRumble
 * (port/src/input.c, built here as Port_InputRumbleHost) drives gamepads only.
 *
 * Here: gamepads as on PC; with none, the phone's vibrator, which SDL lists as a
 * haptic device ("VIBRATOR_SERVICE", SDLControllerManager.java, VibrationEffect
 * since Android 8; VIBRATE is in the manifest). Calling the vibrator at the motor's
 * pulse rate (60 Hz) would stutter, so each "motor on" starts or renews a short
 * vibration at [input] rumble_strength, which simply runs out once the game stops
 * switching the motor on.
 */
#include <SDL3/SDL.h>

#include "port.h"
#include "port_config.h"

void Port_InputRumbleHost(int channel, int on);

#define PULSE_MS 160 /* each renewal's length */
#define RENEW_MS 100 /* renewed at most this often while the motor keeps coming on */

static SDL_Haptic *sPhone;
static int sTried;
static Uint64 sLastPlay;

static SDL_Haptic *phoneVibrator(void) {
    SDL_HapticID *ids;
    int n = 0, i, pick = -1;

    if (sTried) {
        return sPhone;
    }
    sTried = 1;
    if (!SDL_InitSubSystem(SDL_INIT_HAPTIC)) {
        Port_Log("haptics: %s", SDL_GetError());
        return NULL;
    }
    ids = SDL_GetHaptics(&n);
    for (i = 0; ids != NULL && i < n; i++) {
        const char *name = SDL_GetHapticNameForID(ids[i]);
        if (name != NULL && SDL_strstr(name, "VIBRATOR") != NULL) {
            pick = i;
        }
    }
    if (pick < 0 && n > 0) {
        pick = 0;
    }
    if (pick >= 0) {
        sPhone = SDL_OpenHaptic(ids[pick]);
        if (sPhone != NULL && !SDL_InitHapticRumble(sPhone)) {
            Port_Log("haptics: no rumble on %s: %s", SDL_GetHapticName(sPhone), SDL_GetError());
            SDL_CloseHaptic(sPhone);
            sPhone = NULL;
        }
    }
    Port_Log("haptics: %s", sPhone != NULL ? SDL_GetHapticName(sPhone) : "the phone has no vibrator SDL can use");
    SDL_free(ids);
    return sPhone;
}

void Port_InputRumble(int channel, int on) {
    SDL_Haptic *h;
    Uint64 now;

    Port_InputRumbleHost(channel, on); /* gamepads */
    if (channel != 0 || !on || !gPortConfig.rumble || SDL_HasGamepad()) {
        return;
    }
    h = phoneVibrator();
    now = SDL_GetTicks();
    if (h != NULL && (sLastPlay == 0 || now - sLastPlay >= RENEW_MS)) {
        SDL_PlayHapticRumble(h, (float)gPortConfig.rumbleStrength / 100.0f, PULSE_MS);
        sLastPlay = now;
    }
}
