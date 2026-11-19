# DOCS_ANDROID_PORT.md: project memory for the Android port

This file holds what Andre asked to "memorize", the technical findings of the port
and the game's important variables. Keep it up to date with every phase.

## Reference screen: Game Mode

The first menu after "press start" (world championship / arcade / time trial /
multi-player, "press z-button for options menu", "press r-button for pc options").

| What | Where |
|---|---|
| Game mode | `MODE_MAIN` (`gGame->mode`) |
| Draws the screen | `external/dd64/src/auto/40338.c` `MainMenu_Show` |
| Up / down | `src/auto/3FD00.c` `MainMenu_Prev`, `src/auto/3FE04.c` `MainMenu_Next` |
| A (choose a mode) | `src/auto/3FF30.c` `MainMenu_Select` |
| Z opens the game options, cheat input | `src/auto/40244.c` `MainMenu_CheckOptions` (`Cheat_UpdateInput`) |
| Game options (music/voice/sfx/playsong/stereo), `MODE_OPTIONS` | `src/auto/47F6C.c` `OptionsMenu_Prev/Next`, `483D4.c` `_Left`, `48A0C.c` `_Right`, `4905C.c` `_Back`, `492A8.c` `_Update`, `49380.c` `_Init` |
| PC options page (R) | `external/dd64/port/game/pcmenu.c` (drawing and input), `external/dd64/port/src/options.c` (pages and rows), `port/src/config.c` (dd64.ini keys and defaults) |

## Findings (bugs fixed on Android)

| Symptom | Cause | Fix |
|---|---|---|
| "Could not map the game's memory at 0x80000000" | In a 32-bit process ART puts its JIT cache and linear-alloc shadow map on 0x7DA00000-0x83A00000 | `wrap.sh` (debug builds) preloads `libdd64reserve.so`, which reserves the range before ART starts |
| Black screen | `gen_widths.py` found 20 static values instead of 4842: clang writes no DWARF for `extern` variables | `symbols/dd64_widths.c` from the PC build |
| Black screen | SDL_gpu with a 4x MSAA target is black on the Adreno 740 | MSAA capped at 1x (`android/gpu_caps.c`) |
| Minutes of loading | NDK emulated TLS below minSdk 29 (`__thread` in `port/src/shadow.c`, every memory access) | minSdk 29 |
| Title screen never comes | `inflate_codes` (`src/auto/51980.c`) built by clang -O2 for ARM loops forever | that file at -O0 (cause in clang still open) |
| "Could not load ROM" after an update | The activity recreated without the intent extra | `GameActivity` falls back to `files/rom/dd64.z64` |
| **Crash on Z + down in the Game Mode / options screen** (phase 1) | `D_800BAD10` (the options menu's table) is *defined* in `47F6C.c` and *also* placed at 0x800BAD10 by `dd64_syms.ld`. With `-fvisibility=hidden` clang reached it pc-relative, as if the .so were loaded at 0: `OptionsMenu_Next` read 0x800BAD10 + load address (0x5AFC1D10) | Game objects built with `-fvisibility=default`: the five game-defined globals the syms also place (`D_800B7DFC`, `D_800B7F8C`, `D_800B8244`, `D_800BAD10`, `D_800BBD94`) are reached through the GOT, filled with their absolute address |

General rule found: **a 32-bit `.so` (PIC) is not the PC's fixed-address exe**. Any
game symbol at an absolute N64 address must be reached through the GOT, never
pc-relative.

## Testing on the phone (adb)

- Script: `C:\Users\Admin\Documents\claudeProjects\DD64-android-logs\test-run.ps1`
  (`-Install`, `-Env "DD64_PRESS=...;DD64_GFXLOG=1"`, `-Wait`): opens `GameActivity`
  directly (exported in debug builds), screenshot and logs.
- `Data/env.txt` on the phone: `DD64_*` switches, as the PC shell sets them
  (`DD64_PRESS`, `DD64_BOOT`, `DD64_FAKEGAS`, `DD64_GFXLOG`, `DD64_STACKS=<ms>`,
  `DD64_PEEK=addr,...`, `DD64_TEXDUMP`, `DD64_SHOT`).
- SDL sends relative paths (dumps) to the internal `files/` folder: `adb shell run-as
  io.github.dedecidao.dd64.debug ls files`.

## Memorized (requested by Andre)

- (phase 3) `announce every wreck`: see the `TODO(FUTURE_MOD)` in the code.
