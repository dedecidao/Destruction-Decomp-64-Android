# Destruction Derby 64 para Android

Port nativo de **Destruction Derby 64** para Android (APK), feito a partir do port de PC
da decompilação [dedecidao/Destruction-Decomp-64](https://github.com/dedecidao/Destruction-Decomp-64)
(branch `port-pc`). Ele roda o código do jogo compilado para ARM, sem emulador, com
gráficos por Vulkan (SDL_gpu), controles touch e suporte a gamepads.

> **Status: em desenvolvimento.** O app (seletor de ROM, controles touch, gamepads, build
> do APK) já está montado. O jogo ainda está sendo levado para 64-bit (`arm64-v8a`).
> Veja o [plano](docs/PLANO.md).

**Você precisa da sua própria ROM** de Destruction Derby 64 (USA). Nenhum dado da ROM
está neste repositório, e nada dela deve ser commitado.

| ROM | Valor |
|---|---|
| Jogo | Destruction Derby 64 (USA) |
| Formatos aceitos | .z64, .n64, .v64 (16 MB) |
| SHA1 (.z64) | `a2c0799e13566d1b723299592cfcac9387f25fa7` |

## Instalar e jogar

1. Baixe o APK em **Actions → Android APK → Artifacts** (ou em Releases, quando houver).
2. Instale no aparelho (permita instalar de fontes desconhecidas).
3. Abra o app, toque em **Escolher ROM** e escolha o arquivo. A ROM é verificada e
   copiada para a área privada do app. Ela não sai do aparelho.
4. Toque em **Jogar**.

Requisitos: Android 8.0 ou mais novo, com Vulkan. Testado no Galaxy S23 Ultra.

## Controles

**Touch** (aparecem por cima do jogo):

| Na tela | Controle N64 | No jogo |
|---|---|---|
| Metade esquerda | Analógico (aparece onde o dedo toca) | Direção |
| A (azul, grande) | A | Acelerar |
| B (verde) | B | Frear / ré |
| Z, R, L | Z, R, L | |
| C ▲▼◀▶ (amarelos) | Botões C | Câmera |
| START | Start | Pausa / menus |

Dá para deslizar o dedo de um botão para outro (de A para B, por exemplo). Na tela
inicial você ajusta opacidade e tamanho, ou desliga os controles.

**Gamepad** (Bluetooth ou USB, como Xbox, DualShock/DualSense, 8BitDo): funciona direto.
Os controles touch somem quando um gamepad é conectado. O gatilho direito é o acelerador e
o esquerdo o freio, ambos analógicos. O mapeamento pode ser mudado no `dd64.ini`
(`[input]`), como no PC, e há vibração quando o controle tem motor.

## Arquivos do jogo no aparelho

Ficam em `Android/data/io.github.dedecidao.dd64/files/Data/`, acessível pelo gerenciador
de arquivos ou por USB:

| Caminho | Conteúdo |
|---|---|
| `Data/Saves/pak1.mpk` | Save do Controller Pak |
| `dd64.ini` | Configurações (vídeo, áudio, entrada), as mesmas do PC |
| `Data/Logs/dd64.log` | Log da última sessão. Mande este arquivo quando reportar um problema |
| `Data/Textures/fonts_hd/` | Fontes HD geradas no PC (opcional) |

## Compilar

Precisa de JDK 17, Gradle 8.14 (ou o Android Studio), Android SDK com NDK 27 e CMake 3.22, além de Python 3 e
`binutils-arm-linux-gnueabihf` (para o `objdump` que lê o DWARF dos objetos ARM).

```sh
git clone --recursive https://github.com/dedecidao/Destruction-Decomp-64-Android
cd Destruction-Decomp-64-Android
gradle assembleDebug
# APK em app/build/outputs/apk/debug/
```

O link do jogo usa `symbols/dd64.syms`, que guarda só nomes e endereços do build N64,
sem nenhum dado da ROM. Como gerar está em [symbols/README.md](symbols/README.md). Sem
esse arquivo, o build só compila o código, como verificação.

| Propriedade Gradle | Padrão | Uso |
|---|---|---|
| `dd64.abis` | `armeabi-v7a` | ABIs empacotadas no APK |
| `dd64.arm64Link` | `false` | Liga o link do jogo em arm64 quando a conversão 64-bit estiver pronta |

## Como funciona

| Parte | Onde |
|---|---|
| Jogo (C decompilado) e runtime de PC | `external/dd64` (submódulo, `port-pc`) |
| SDL3 (janela, Vulkan, áudio, gamepads, `SDLActivity`) | `external/SDL` (submódulo, 3.2.30) |
| Entrada nativa, log, crash handler | `app/src/main/cpp/android/android_main.c` |
| RAM do N64 em 0x80000000 e stubs ARM para ponteiros de função | `app/src/main/cpp/android/ram_arm.c` |
| Touch → controle 1 | `app/src/main/cpp/android/touch.c` e `TouchControlsView.java` |
| Seletor de ROM (SAF) e opções | `LauncherActivity.java` |
| Tabelas de símbolos sem a ROM | `tools/export_symbols.py`, `tools/gen_from_syms.py` |

Detalhes e próximos passos em [docs/PLANO.md](docs/PLANO.md).

## Créditos

- Decompilação e port de PC: [murkantor/Destruction-Decomp-64](https://github.com/murkantor/Destruction-Decomp-64)
  e o fork [dedecidao/Destruction-Decomp-64](https://github.com/dedecidao/Destruction-Decomp-64)
- [SDL](https://libsdl.org), Fast3D (Project Midas, ver `port/fast3d/LICENSE.txt` no submódulo)
- Referências de ports Android: dk64-recomp-android, Zelda64Recomp-Android,
  BanjoRecomp-Android, sm64coopdx-android, 2ship2harkinian-Android e o catálogo native-ports

Destruction Derby é marca dos seus respectivos donos. Este projeto não é afiliado a eles e
não distribui nenhum conteúdo do jogo.
