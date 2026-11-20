/*
 * Rumble on Android: gamepads as on PC, and with none, the phone's vibrator, with
 * a profile per kind of rumble.
 *
 * The game asks for rumble with Pad_Rumble(channel, frames, strength)
 * (src/auto/59D40.c), and Pad_Update (59B08.c) then pulses the Rumble Pak motor
 * through __osMotorAccess -> Port_InputRumble. The callers tell the kinds apart:
 *
 *   1C624.c  Pad_Rumble(ch, 1, 0.5 or 0.6)  every frame the wheels are on a rough
 *            surface (dirt, grass, kerbs): continuous
 *   28C70.c  Pad_Rumble(ch, 5..10, 0.5..0.9) a contact, by its force (another car;
 *            a wall too): repeated every frame while scraping
 *   2E9D0.c  Pad_Rumble(ch, 5, 0.75)          a trigger or pickup
 *   1C624.c  Pad_Rumble(ch, 30, 0.9)          the player's car is wrecked
 *   46894.c, 476B8.c  Pad_Rumble(ch, 0, 0)    stop (menus, pause)
 *
 * A Rumble Pak only has on and off, so on the N64 (and on gamepads, through
 * Port_InputRumbleHost) all of them feel alike: going off the road buzzes as hard
 * as a crash. The phone's vibrator has an amplitude, so here (Pad_Rumble wraps the
 * game's, built as Pad_RumbleGame, CMakeLists.txt):
 *
 *   rough surface   subtle (12%), short pulses renewed while it lasts
 *   wall scrape     soft (25%): light contacts coming again within 200 ms
 *   impact          strong, 55% to 100% by the hit's force, longer the harder
 *   wreck           100%, about 0.6 s
 *
 * all times [input] rumble_strength. Surface and scrape pulses never cut into an
 * impact still running. The vibrator is SDL's haptic device "VIBRATOR_SERVICE"
 * (SDLControllerManager.java, VibrationEffect with amplitude); VIBRATE is in the
 * manifest.
 */
#include <SDL3/SDL.h>

#include "port.h"
#include "port_config.h"

void Port_InputRumbleHost(int channel, int on);
void Pad_RumbleGame(s32 channel, s32 frames, f32 strength);

#define SURFACE_AMP 0.12f
#define SCRAPE_AMP 0.25f
#define SOFT_PULSE_MS 150  /* a surface or scrape pulse */
#define SOFT_RENEW_MS 110  /* renewed at most this often while it lasts */
#define SCRAPE_WINDOW_MS 200
#define IMPACT_MS_PER_FRAME 35
#define IMPACT_MIN_MS 90
#define IMPACT_MAX_MS 650

static SDL_Haptic *sPhone;
static int sTried;
static Uint64 sImpactUntil, sLastSoft, sLastContact;

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

static void play(SDL_Haptic *h, float amp, Uint32 ms) {
    amp *= (float)gPortConfig.rumbleStrength / 100.0f;
    SDL_PlayHapticRumble(h, SDL_clamp(amp, 0.0f, 1.0f), ms);
}

/* A soft pulse (surface, scrape), unless an impact is still running */
static void soft(SDL_Haptic *h, float amp, Uint64 now) {
    if (now < sImpactUntil || (sLastSoft != 0 && now - sLastSoft < SOFT_RENEW_MS)) {
        return;
    }
    play(h, amp, SOFT_PULSE_MS);
    sLastSoft = now;
}

static void phoneRumble(s32 frames, f32 strength) {
    SDL_Haptic *h;
    Uint64 now;
    Uint32 ms;
    float amp;

    if (strength <= 0.1f || frames <= 0) {
        return; /* a stop: the pulses run out by themselves */
    }
    h = phoneVibrator();
    if (h == NULL) {
        return;
    }
    now = SDL_GetTicks();
    if (gPortConfig.fidelity) {
        /* the Fidelity profile: as the Rumble Pak, every call the game's own strength and
           length (no subtle surface or scrape pulses) */
        play(h, strength, (Uint32)SDL_clamp(frames * IMPACT_MS_PER_FRAME, IMPACT_MIN_MS, IMPACT_MAX_MS));
        return;
    }
    if (frames == 1) {
        soft(h, SURFACE_AMP * strength / 0.5f, now); /* rough surface, every frame */
        return;
    }
    if (strength < 0.85f && sLastContact != 0 && now - sLastContact < SCRAPE_WINDOW_MS) {
        sLastContact = now;
        soft(h, SCRAPE_AMP, now); /* light contacts again and again: scraping a wall or a car */
        return;
    }
    sLastContact = now;
    /* an impact: 0.5 -> 55%, 0.9 -> 100%; the wreck (30 frames) about 0.65 s */
    amp = 0.55f + (strength - 0.5f) * 1.125f;
    ms = (Uint32)SDL_clamp(frames * IMPACT_MS_PER_FRAME, IMPACT_MIN_MS, IMPACT_MAX_MS);
    play(h, amp, ms);
    sImpactUntil = now + ms;
}

/* The game's Pad_Rumble (every caller): the Rumble Pak (gamepads) as before, and
   the phone's own profile when no gamepad is connected */
void Pad_Rumble(s32 channel, s32 frames, f32 strength) {
    Pad_RumbleGame(channel, frames, strength);
    if (channel == 0 && gPortConfig.rumble && !SDL_HasGamepad()) {
        phoneRumble(frames, strength);
    }
}

/* The Rumble Pak motor (Pad_Update's pulses): gamepads only; the phone follows
   Pad_Rumble above */
void Port_InputRumble(int channel, int on) {
    Port_InputRumbleHost(channel, on);
}
