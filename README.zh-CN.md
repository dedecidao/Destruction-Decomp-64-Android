<div align="center">

[English](README.md) | [Português (Brasil)](README.pt-BR.md) | **简体中文**

# Wrecked Metal

**一款经典 N64 撞车竞速游戏的 Android 原生移植版，基于反编译的 C 代码重建，也是一个更大计划的起点。**

*Costelinha Games 出品*

[![每夜构建](https://img.shields.io/badge/%E4%B8%8B%E8%BD%BD-%E6%AF%8F%E5%A4%9C%E6%9E%84%E5%BB%BA%20APK-orange)](https://github.com/dedecidao/Wrecked-Metal/releases)
![Android 10+](https://img.shields.io/badge/Android-10%2B-3DDC84?logo=android&logoColor=white)
![Vulkan](https://img.shields.io/badge/renderer-Vulkan-AC162C?logo=vulkan&logoColor=white)
![自备 ROM](https://img.shields.io/badge/ROM-%E8%87%AA%E5%A4%87-lightgrey)
[![Ko-fi](https://img.shields.io/badge/Ko--fi-%E6%94%AF%E6%8C%81%E6%9C%AC%E9%A1%B9%E7%9B%AE-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/costelinha_games)

</div>

---

## 为什么做这个项目

我在 90 年代手握手柄长大，对一类非常特别的游戏情有独钟：汽车、钢铁，以及一座只有最后还能动的人才能获胜的竞技场。几十年后，作为一名资深开发者，我想在每天随身携带的设备上，好好地再玩一次。

不是通过模拟器，而是原生运行。

**Wrecked Metal** 采用社区对该游戏的反编译成果（原始逻辑被重写为可读的 C 代码），直接编译为 Android 上的 ARM 代码。没有模拟层，也不用去猜时序：游戏自己的代码就运行在你的手机上，通过 Vulkan 绘制，配有真正的触屏操控和真正的手柄支持。

这只是基础。我真正想做的是在它之上构建的东西：一层新的原生改进、便利性选项，以及将来的新模式和新玩法，同时始终保留一种完全忠实于原版的游玩方式。

## 两种游玩方式

| 模式 | 说明 |
|---|---|
| **PLAY WRECKED METAL** | 增强版游戏。包含本项目加入的所有改进，并有独立的设置。 |
| **Play Fidelity** | 与 N64 上一模一样的游戏：原版 HUD 和纹理过滤、22 kHz 音频、N64 手柄布局（摇杆加四个 C 键），不含任何增强。 |

每个模式都有自己的设置文件，调整其中一个永远不会影响另一个。两个模式共用同一份存档。

## 功能

**性能与画面**
- 原生 ARM 代码，无需模拟器
- 基于 SDL_gpu / Vulkan 的 Fast3D 渲染器
- 稳定 60 fps（在 Galaxy S23 Ultra 上测试），支持垂直同步
- 720p / 1080p 渲染、2x MSAA、各向异性过滤
- 宽屏 3D
- 可视距离最高可达原版的 16 倍

**音频**
- 44.1 kHz 输出（Fidelity 模式保留原版 22 kHz）
- 可用你自己录制的 `.ogg` 或 `.mp3` 替换任意音序曲目
- 来电或其他中断后音频会自动恢复

**操控**
- 叠加在游戏画面上的触屏控件：浮动虚拟摇杆（也可以换成方向键），按键之间可以滑动切换
- 蓝牙和 USB 手柄即插即用（Xbox、DualShock / DualSense、8BitDo 等），触屏控件会自动隐藏
- 扳机键支持模拟油门和刹车，右摇杆控制视角
- 自适应触感反馈与震动

**游戏内**
- 原生选项菜单（视频、声音、操控、游戏、高级），带"恢复默认"选项
- 作弊页面，可使用游戏内置的作弊码

没有遥测，不需要账号，没有广告，游玩无需联网。

## 系统要求

- Android 10 或更高版本
- 支持 Vulkan 的 GPU
- **目前需要能运行 32 位应用的设备。** 当前构建为 `armeabi-v7a`。部分新款手机（例如 Pixel 7 及更新机型）仅支持 64 位，暂时无法安装。原生 64 位（`arm64-v8a`）构建正在开发中。
- **你自己合法获得的 ROM**（见下文）

## 安装与游玩

1. 从 [Releases](https://github.com/dedecidao/Wrecked-Metal/releases) 页面下载最新的 APK。
2. 在设备上打开它，如果 Android 提示，请允许安装未知来源的应用。
3. 启动 **Wrecked Metal**，点击 **Pick ROM**，选择你自己的游戏副本。
4. 应用会校验文件（SHA1），必要时转换为正确的格式，并保存在应用的私有目录中。文件永远不会离开你的设备。
5. 点击 **PLAY WRECKED METAL** 或 **Play Fidelity**，完成。

### 关于 ROM

Wrecked Metal 需要你自己的 **Destruction Derby 64 (USA)** 副本。

| | |
|---|---|
| 支持的格式 | `.z64`、`.n64`、`.v64`（自动转换） |
| SHA1（`.z64`） | `a2c0799e13566d1b723299592cfcac9387f25fa7` |

请不要在 Issues 或 Discussions 中索要 ROM，此类请求会被关闭。

## 设备上的文件

位于 `Android/data/com.costelinhagames.wreckedmetal/files/`：

| 路径 | 内容 |
|---|---|
| `dd64.ini` / `dd64-fidelity.ini` | 各模式的设置 |
| `Data/Saves/` | Controller Pak 存档（两个模式共用） |
| `Data/Music/` | 你可选的替换曲目 |
| `Data/Logs/dd64.log` | 上一次运行的日志。报告 bug 时请附上它。 |

## 路线图

- [x] Android 原生构建、60 fps、触屏与手柄操控
- [x] 增强模式与 Fidelity 模式
- [ ] 签名的正式版 APK
- [ ] 原生 64 位（`arm64-v8a`）构建
- [ ] 更多原生改进和便利性选项
- [ ] 基于原版游戏打造的新模式和新竞速玩法

欢迎在 [Discussions](https://github.com/dedecidao/Wrecked-Metal/discussions) 和 [Issues](https://github.com/dedecidao/Wrecked-Metal/issues) 中提出想法和报告 bug。

## 支持本项目

Wrecked Metal 现在免费，将来也会一直**免费**。没有付费版本，没有抢先体验，没有付费墙后的功能。

如果它唤起了你美好的回忆，而你也想帮助我继续开发，可以在 Ko-fi 上请我喝杯咖啡。这完全出于自愿，每一笔打赏都会变成投入到这个项目中的更多时间。

[![在 Ko-fi 上支持我](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/costelinha_games)

## 从源码构建

JDK 17、Gradle 8.14（或 Android Studio）、带 NDK 27 和 CMake 3.22 的 Android SDK、Python 3 以及 `binutils-arm-linux-gnueabihf`。

```sh
git clone --recursive https://github.com/dedecidao/Wrecked-Metal
cd Wrecked-Metal
gradle assembleDebug
# APK 位于 app/build/outputs/apk/debug/
```

构建时会链接 `symbols/dd64.syms`，其中只包含符号名称和地址，不含任何 ROM 数据（见 [symbols/README.md](symbols/README.md)）。技术说明见 [DOCS_ANDROID_PORT.md](DOCS_ANDROID_PORT.md)（英文）。

| 部分 | 位置 |
|---|---|
| 游戏代码（反编译的 C）和 PC 运行时 | `external/dd64`（子模块） |
| SDL3：窗口、Vulkan、音频、手柄 | `external/SDL`（子模块） |
| Android 入口、日志、崩溃处理、触感反馈 | `app/src/main/cpp/android/` |
| 启动器和触屏控件 | `app/src/main/java/io/github/dedecidao/dd64/` |

## 致谢

- 由 [murkantor](https://github.com/murkantor/Destruction-Decomp-64) 主导的社区反编译和 PC 移植，以及本项目的引擎分支 [dedecidao/Destruction-Decomp-64](https://github.com/dedecidao/Destruction-Decomp-64)
- [SDL](https://libsdl.org) 和 Fast3D（Project Midas，见引擎子模块中的 `port/fast3d/LICENSE.txt`）
- 为 Android 移植指明方向的项目：2ship2harkinian-Android、sm64coopdx-android、Zelda64Recomp-Android、dk64-recomp-android、BanjoRecomp-Android

## 法律声明

Wrecked Metal 是一个非官方的粉丝项目，旨在保存游戏并供个人娱乐。**本仓库和 APK 不包含任何 ROM、游戏资源或原版游戏中受版权保护的内容。** 游戏代码由社区反编译成果构建，所有游戏数据都在运行时从你自行提供的 ROM 中读取，该 ROM 须从你拥有的卡带中提取。

Destruction Derby 及所有相关名称和商标归其各自所有者所有。本项目与他们没有任何关联，也未获得其认可或赞助。Ko-fi 捐助是对开发者时间的自愿打赏，不购买任何软件、内容或使用权限。
