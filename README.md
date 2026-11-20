<div align="center">

# Wrecked Metal

**A native Android port of a classic N64 demolition racer, rebuilt from decompiled C, and the start of something bigger.**

*by Costelinha Games*

[![Nightly build](https://img.shields.io/badge/download-nightly%20APK-orange)](https://github.com/dedecidao/Wrecked-Metal/releases)
![Android 10+](https://img.shields.io/badge/Android-10%2B-3DDC84?logo=android&logoColor=white)
![Vulkan](https://img.shields.io/badge/renderer-Vulkan-AC162C?logo=vulkan&logoColor=white)
![Bring Your Own ROM](https://img.shields.io/badge/ROM-bring%20your%20own-lightgrey)
[![Ko-fi](https://img.shields.io/badge/Ko--fi-support%20the%20project-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/costelinha_games)

</div>

---

## Why this exists

I grew up in the 90s with a controller in my hands and a soft spot for one very specific kind of game: cars, metal, and an arena where the last one still moving wins. Decades later, as a senior developer, I wanted to play it again, properly, on the device I carry every day.

Not through an emulator. Natively.

**Wrecked Metal** takes the community decompilation of the game, the original logic rewritten as readable C, and compiles it straight to ARM for Android. No emulation layer, no guessing at timing: the game's own code runs on your phone, drawn through Vulkan, with real touch controls and real gamepad support.

That is the foundation. What I actually want to build sits on top of it: a new layer of native improvements, quality-of-life options, and eventually new modes and mechanics, while always keeping a faithful way to play the game exactly as it was.

## Two ways to play

| Mode | What it is |
|---|---|
| **PLAY WRECKED METAL** | The enhanced game. Every improvement the project adds, with its own settings. |
| **Play Fidelity** | The game as it was on the N64: original HUD and filtering, 22 kHz audio, the N64 controller layout (stick and four C buttons), none of the enhancements. |

Each mode keeps its own settings file, so tweaking one never touches the other. Your save is shared between them.

## Features

**Performance and graphics**
- Native ARM code, no emulator
- Fast3D renderer on SDL_gpu / Vulkan
- Steady 60 fps (tested on a Galaxy S23 Ultra), with vsync
- 720p / 1080p rendering, 2x MSAA, anisotropic filtering
- Widescreen 3D
- Draw distance up to 16x the original

**Audio**
- 44.1 kHz output (Fidelity keeps the original 22 kHz)
- Replace any sequenced track with your own recorded `.ogg` or `.mp3`
- Audio recovers on its own after phone calls and other interruptions

**Controls**
- Touch controls drawn over the game: a floating analog stick (or arrows, if you prefer), with buttons you can slide between
- Bluetooth and USB gamepads work out of the box (Xbox, DualShock / DualSense, 8BitDo and others), and the touch overlay hides itself
- Analog gas and brake on the triggers, camera on the right stick
- Adaptive haptics and rumble

**In-game**
- Native options menus (video, sound, controls, game, advanced) with a "restore defaults" option
- A cheats page for the game's built-in codes

There is no telemetry, no account, no ads and no network access needed to play.

## Requirements

- Android 10 or newer
- A GPU with Vulkan support
- **For now, a device that can run 32-bit apps.** The current build is `armeabi-v7a`. Some recent phones (Pixel 7 and newer, for example) are 64-bit only and cannot install it yet. The native 64-bit (`arm64-v8a`) build is in progress.
- **Your own legally obtained ROM** (see below)

## Install and play

1. Download the latest APK from the [Releases](https://github.com/dedecidao/Wrecked-Metal/releases) page.
2. Open it on your device and allow installing from unknown sources if Android asks.
3. Launch **Wrecked Metal** and tap **Pick ROM**. Choose your own copy of the game.
4. The app checks the file (SHA1), converts it to the right format if needed, and stores it in its private folder. It never leaves your device.
5. Tap **PLAY WRECKED METAL** or **Play Fidelity**. Done.

### The ROM

Wrecked Metal needs your own copy of **Destruction Derby 64 (USA)**.

| | |
|---|---|
| Accepted formats | `.z64`, `.n64`, `.v64` (converted automatically) |
| SHA1 (`.z64`) | `a2c0799e13566d1b723299592cfcac9387f25fa7` |

Please don't ask for ROMs in issues or discussions. Those requests will be closed.

## Files on your device

Under `Android/data/io.github.dedecidao.dd64/files/`:

| Path | Contents |
|---|---|
| `dd64.ini` / `dd64-fidelity.ini` | Settings for each mode |
| `Data/Saves/` | Controller Pak save (shared by both modes) |
| `Data/Music/` | Your optional replacement tracks |
| `Data/Logs/dd64.log` | Log of the last session. Attach it when you report a bug. |

## Roadmap

- [x] Native Android build, 60 fps, touch and gamepad controls
- [x] Enhanced and Fidelity modes
- [ ] Signed release APK
- [ ] Native 64-bit (`arm64-v8a`) build
- [ ] More native improvements and quality-of-life options
- [ ] New modes and racing mechanics built on top of the original game

Ideas and bug reports are welcome in [Discussions](https://github.com/dedecidao/Wrecked-Metal/discussions) and [Issues](https://github.com/dedecidao/Wrecked-Metal/issues).

## Support the project

Wrecked Metal is, and will stay, **free**. No paid builds, no early access, no paywalled features.

If it brought back some good memories and you want to help me keep working on it, you can buy me a coffee on Ko-fi. It is entirely voluntary, and every tip goes into more hours on this project.

[![Support me on Ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/costelinha_games)

## Building from source

JDK 17, Gradle 8.14 (or Android Studio), Android SDK with NDK 27 and CMake 3.22, Python 3 and `binutils-arm-linux-gnueabihf`.

```sh
git clone --recursive https://github.com/dedecidao/Wrecked-Metal
cd Wrecked-Metal
gradle assembleDebug
# APK in app/build/outputs/apk/debug/
```

The build links against `symbols/dd64.syms`, which holds symbol names and addresses only and no ROM data (see [symbols/README.md](symbols/README.md)). Technical notes live in [DOCS_ANDROID_PORT.md](DOCS_ANDROID_PORT.md).

| Part | Where |
|---|---|
| Game code (decompiled C) and the PC runtime | `external/dd64` (submodule) |
| SDL3: window, Vulkan, audio, gamepads | `external/SDL` (submodule) |
| Android entry point, logging, crash handler, haptics | `app/src/main/cpp/android/` |
| Launcher and touch controls | `app/src/main/java/io/github/dedecidao/dd64/` |

## Credits

- The community decompilation and PC port by [murkantor](https://github.com/murkantor/Destruction-Decomp-64), and this project's engine fork, [dedecidao/Destruction-Decomp-64](https://github.com/dedecidao/Destruction-Decomp-64)
- [SDL](https://libsdl.org) and Fast3D (Project Midas, see `port/fast3d/LICENSE.txt` in the engine submodule)
- Ports that showed the way on Android: 2ship2harkinian-Android, sm64coopdx-android, Zelda64Recomp-Android, dk64-recomp-android, BanjoRecomp-Android

## Legal

Wrecked Metal is an unofficial fan project made for preservation and personal enjoyment. **This repository and the APK contain no ROM, no game assets and no copyrighted material from the original game.** The game code is built from a community decompilation, and all game data is read at runtime from a ROM that you must supply yourself, dumped from a cartridge you own.

Destruction Derby and all related names and trademarks belong to their respective owners. This project is not affiliated with, endorsed by, or sponsored by them. Ko-fi donations are voluntary tips for the developer's time and do not buy any software, content or access.
