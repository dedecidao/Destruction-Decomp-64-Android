# Wrecked Metal 64 DX

**English** · [Português](#português) · [中文](#中文)

> An ambitious customization project built on the best racing and destruction physics the 64 ever delivered.

Wrecked Metal 64 DX runs the game's decompiled C code natively on Android: compiled for ARM,
no emulator, Vulkan graphics (SDL_gpu), touch controls and gamepads. On top of it comes a
growing layer of customizations: higher resolution and anti-aliasing, longer draw distance,
44.1 kHz audio, recorded-music replacement, adaptive haptics, a free camera, analog pedals and more.

> **Status: beta in progress.** Tested on a Galaxy S23 Ultra (armeabi-v7a build).
> 64-bit (`arm64-v8a`) is being converted. See the [plan](docs/PLANO.md).

[💬 Community: ideas, bugs, setups → Discussions](https://github.com/dedecidao/Wrecked-Metal-64-DX/discussions)

## Two ways to play

| Button | What it is |
|---|---|
| **PLAY WRECKED METAL** | The customized game: every improvement of the project, its own settings (`dd64.ini`). |
| **Play Fidelity** | The game as it was on the N64: original HUD boxes and filtering, 22 kHz audio, the N64 controller (stick and four C buttons) without the port's customizations. Its own settings (`dd64-fidelity.ini`), so neither mode changes the other. |

## ROM (required)

You need **your own ROM** of **Destruction Derby 64 (USA)**. Nothing from the ROM is in this
repository, and nothing from it may be committed.

| ROM | Value |
|---|---|
| Game | Destruction Derby 64 (USA) |
| Formats | .z64, .n64, .v64 (16 MB) |
| SHA1 (.z64) | `a2c0799e13566d1b723299592cfcac9387f25fa7` |

Destruction Derby is a trademark of its respective owners. This project is not affiliated
with them and distributes no game content.

## Install and play

1. Download the APK from **Releases → Nightly** (the latest build of `main`, no sign-in needed).
2. Install it (allow installing from unknown sources).
3. Open the app, tap **Pick ROM** and choose your file. It is checked and copied into the
   app's private storage, and never leaves the device.
4. Tap **PLAY WRECKED METAL** (or **Play Fidelity**).

Requirements: Android 10 or newer with Vulkan.

## Controls

**Touch** (drawn over the game): the left half is the analog stick (it appears where the thumb
lands, or a d-pad with the "arrows" option); A (gas), B (brake), Z, R, L, Start; C-Up for the
camera (all four C buttons in Fidelity). A finger can slide from one button to another.

**Gamepads** (Bluetooth or USB: Xbox, DualShock/DualSense, 8BitDo...) work out of the box and hide
the touch controls. In Wrecked Metal the triggers are analog gas and brake and the right stick
turns the camera; in Fidelity the pad maps like an N64 controller. Rumble on both.

In-game, **R** on the title screen opens the options (video, sound, controls, game, advanced, cheats).

## Files on the device

In `Android/data/io.github.dedecidao.dd64/files/`:

| Path | Contents |
|---|---|
| `dd64.ini` / `dd64-fidelity.ini` | Settings of each mode |
| `Data/Saves/pak1.mpk` | Controller Pak save (shared by both modes) |
| `Data/Logs/dd64.log` | Log of the last session: attach it when reporting a bug |
| `Data/Music/` | Optional recorded tracks (.ogg/.mp3) that replace the sequenced songs |

## Build

JDK 17, Gradle 8.14 (or Android Studio), Android SDK with NDK 27 and CMake 3.22, Python 3 and
`binutils-arm-linux-gnueabihf`.

```sh
git clone --recursive https://github.com/dedecidao/Wrecked-Metal-64-DX
cd Wrecked-Metal-64-DX
gradle assembleDebug
# APK in app/build/outputs/apk/debug/
```

The game link uses `symbols/dd64.syms` (names and addresses only, no ROM data); see
[symbols/README.md](symbols/README.md). Technical notes: [DOCS_ANDROID_PORT.md](DOCS_ANDROID_PORT.md).

| Part | Where |
|---|---|
| Game code (decompiled C) and the PC runtime | `external/dd64` (submodule, the engine) |
| SDL3 (window, Vulkan, audio, gamepads) | `external/SDL` (submodule, 3.4.20) |
| Android entry, log, crash handler, haptics | `app/src/main/cpp/android/` |
| Launcher, touch controls | `app/src/main/java/io/github/dedecidao/dd64/` |

## Credits

- The community decompilation and PC port by [murkantor](https://github.com/murkantor/Destruction-Decomp-64),
  and this project's engine fork, [dedecidao](https://github.com/dedecidao/Destruction-Decomp-64)
- [SDL](https://libsdl.org), Fast3D (Project Midas, see `port/fast3d/LICENSE.txt` in the submodule)
- Android port references: dk64-recomp-android, Zelda64Recomp-Android, BanjoRecomp-Android,
  sm64coopdx-android, 2ship2harkinian-Android

---

## Português

> Um ambicioso projeto de customização em cima da melhor física de corridas e destruição que o 64 proporcionou.

Wrecked Metal 64 DX roda o código C decompilado do jogo nativamente no Android: compilado para
ARM, sem emulador, com gráficos Vulkan (SDL_gpu), controles touch e gamepads. Por cima disso vem
uma camada crescente de customizações: resolução e anti-aliasing maiores, distância de visão
maior, áudio a 44,1 kHz, troca da música por faixas gravadas, vibração adaptativa, câmera livre,
pedais analógicos e mais.

> **Status: beta em andamento.** Testado num Galaxy S23 Ultra (build armeabi-v7a). A versão
> 64-bit (`arm64-v8a`) está em conversão. Veja o [plano](docs/PLANO.md).

[💬 Comunidade: ideias, bugs, configurações → Discussions](https://github.com/dedecidao/Wrecked-Metal-64-DX/discussions)

### Dois jeitos de jogar

| Botão | O que é |
|---|---|
| **PLAY WRECKED METAL** | O jogo customizado: todas as melhorias do projeto, com configurações próprias (`dd64.ini`). |
| **Play Fidelity** | O jogo como era no N64: caixas do HUD e filtragem originais, áudio a 22 kHz, o controle do N64 (analógico e os quatro C) sem as customizações do port. Configurações próprias (`dd64-fidelity.ini`): um modo não muda o outro. |

### ROM (obrigatória)

Você precisa da **sua própria ROM** de **Destruction Derby 64 (USA)**: .z64, .n64 ou .v64
(16 MB), SHA1 (.z64) `a2c0799e13566d1b723299592cfcac9387f25fa7`. Nenhum dado da ROM está neste
repositório. Destruction Derby é marca dos seus respectivos donos; este projeto não é afiliado
a eles e não distribui nenhum conteúdo do jogo.

### Instalar e jogar

1. Baixe o APK em **Releases → Nightly** (o build mais novo da `main`, sem login).
2. Instale (permita fontes desconhecidas).
3. Abra o app, toque em **Escolher ROM** e escolha o arquivo. Ela é verificada e copiada para a
   área privada do app, e não sai do aparelho.
4. Toque em **PLAY WRECKED METAL** (ou **Play Fidelity**).

Requisitos: Android 10 ou mais novo, com Vulkan.

### Controles

**Touch**: a metade esquerda é o analógico (aparece onde o dedo toca, ou uma cruzeta com a opção
de setas); A (acelerar), B (frear), Z, R, L, Start; C-Up para a câmera (os quatro botões C no
Fidelity). O dedo pode deslizar de um botão para outro.

**Gamepads** (Bluetooth ou USB) funcionam direto e escondem o touch. No Wrecked Metal os gatilhos
são acelerador e freio analógicos e o analógico direito gira a câmera; no Fidelity o controle
funciona como um do N64. Vibração nos dois.

No jogo, **R** na tela de título abre as opções. Arquivos, build e créditos: veja as seções em
inglês acima (os caminhos são os mesmos).

---

## 中文

> 一个雄心勃勃的定制项目，建立在 64 平台上最出色的赛车与破坏物理之上。

Wrecked Metal 64 DX 在 Android 上原生运行游戏反编译后的 C 代码：编译为 ARM 程序，无需模拟器，
使用 Vulkan 图形（SDL_gpu），支持触屏与手柄。在此之上是不断扩展的定制内容：更高的分辨率与抗锯齿、
更远的可视距离、44.1 kHz 音频、录制音乐替换、自适应震动、自由视角、模拟踏板等。

> **状态：Beta 开发中。** 已在 Galaxy S23 Ultra 上测试（armeabi-v7a 版本）。64 位
> （`arm64-v8a`）正在移植中。

[💬 社区：建议、问题反馈、设置分享 → Discussions](https://github.com/dedecidao/Wrecked-Metal-64-DX/discussions)

### 两种游玩方式

| 按钮 | 说明 |
|---|---|
| **PLAY WRECKED METAL** | 定制版游戏：包含本项目的全部改进，使用独立设置（`dd64.ini`）。 |
| **Play Fidelity** | 原汁原味的 N64 版：原版 HUD 底框与纹理过滤、22 kHz 音频、N64 手柄布局（摇杆与四个 C 键），不含任何定制。使用独立设置（`dd64-fidelity.ini`），两种模式互不影响。 |

### ROM（必需）

你需要**自备** **Destruction Derby 64（美版）** 的 ROM：.z64、.n64 或 .v64（16 MB），
SHA1（.z64）`a2c0799e13566d1b723299592cfcac9387f25fa7`。本仓库不包含任何 ROM 数据。
Destruction Derby 是其各自所有者的商标；本项目与其无关，也不分发任何游戏内容。

### 安装与游玩

1. 在 **Releases → Nightly** 下载 APK（`main` 分支的最新构建，无需登录）。
2. 安装（允许安装未知来源应用）。
3. 打开应用，点击 **选择 ROM** 并选择文件。ROM 会被校验并复制到应用私有存储，不会离开本设备。
4. 点击 **PLAY WRECKED METAL**（或 **Play Fidelity**）。

系统要求：Android 10 或更高版本，支持 Vulkan。

### 操作

**触屏**：屏幕左半边为摇杆（手指落点处出现，也可在选项中改为方向键）；A（油门）、B（刹车）、
Z、R、L、Start；C-Up 切换视角（Fidelity 模式下为四个 C 键）。手指可在按键之间滑动。

**手柄**（蓝牙或 USB）即插即用，并会隐藏触屏按键。Wrecked Metal 模式下扳机为模拟油门与刹车，
右摇杆旋转视角；Fidelity 模式下按 N64 手柄映射。两种模式均支持震动。

游戏中在标题画面按 **R** 打开选项。文件位置、编译与致谢请参阅上方英文部分。
