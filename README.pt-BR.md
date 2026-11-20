<div align="center">

[English](README.md) | **Português (Brasil)** | [简体中文](README.zh-CN.md)

# Wrecked Metal

**Um port nativo para Android de um clássico de corrida e destruição do N64, reconstruído a partir de C descompilado, e o começo de algo maior.**

*por Costelinha Games*

[![Build noturna](https://img.shields.io/badge/download-APK%20noturno-orange)](https://github.com/dedecidao/Wrecked-Metal/releases)
![Android 10+](https://img.shields.io/badge/Android-10%2B-3DDC84?logo=android&logoColor=white)
![Vulkan](https://img.shields.io/badge/renderer-Vulkan-AC162C?logo=vulkan&logoColor=white)
![Traga sua própria ROM](https://img.shields.io/badge/ROM-traga%20a%20sua-lightgrey)
[![Ko-fi](https://img.shields.io/badge/Ko--fi-apoie%20o%20projeto-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/costelinha_games)

</div>

---

## Por que isto existe

Cresci nos anos 90 com um controle na mão e um carinho especial por um tipo bem específico de jogo: carros, metal e uma arena onde vence o último que ainda estiver se movendo. Décadas depois, já como desenvolvedor sênior, quis jogar de novo, do jeito certo, no aparelho que carrego todo dia.

Não por um emulador. De forma nativa.

**Wrecked Metal** pega a descompilação feita pela comunidade, a lógica original reescrita como C legível, e compila direto para ARM no Android. Sem camada de emulação, sem adivinhar timing: o próprio código do jogo roda no seu celular, desenhado via Vulkan, com controles touch de verdade e suporte real a gamepads.

Essa é a base. O que eu quero mesmo construir fica em cima dela: uma nova camada de melhorias nativas, opções de qualidade de vida e, com o tempo, novos modos e mecânicas, sempre mantendo um jeito fiel de jogar o jogo exatamente como ele era.

## Dois jeitos de jogar

| Modo | O que é |
|---|---|
| **PLAY WRECKED METAL** | O jogo aprimorado. Todas as melhorias que o projeto adiciona, com configurações próprias. |
| **Play Fidelity** | O jogo como era no N64: HUD e filtragem originais, áudio em 22 kHz, o layout do controle do N64 (analógico e quatro botões C), nenhuma das melhorias. |

Cada modo tem seu próprio arquivo de configurações, então ajustar um nunca mexe no outro. O save é compartilhado entre os dois.

## Recursos

**Desempenho e gráficos**
- Código ARM nativo, sem emulador
- Renderer Fast3D sobre SDL_gpu / Vulkan
- 60 fps estáveis (testado em um Galaxy S23 Ultra), com vsync
- Renderização em 720p / 1080p, MSAA 2x, filtragem anisotrópica
- 3D em widescreen
- Distância de desenho até 16x a original

**Áudio**
- Saída em 44,1 kHz (o Fidelity mantém os 22 kHz originais)
- Substitua qualquer faixa sequenciada por uma gravação sua em `.ogg` ou `.mp3`
- O áudio volta sozinho depois de ligações e outras interrupções

**Controles**
- Controles touch desenhados sobre o jogo: um analógico flutuante (ou setas, se preferir), com botões entre os quais você pode deslizar o dedo
- Gamepads Bluetooth e USB funcionam direto (Xbox, DualShock / DualSense, 8BitDo e outros), e o overlay touch se esconde sozinho
- Acelerador e freio analógicos nos gatilhos, câmera no analógico direito
- Vibração háptica adaptativa e rumble

**No jogo**
- Menus de opções nativos (vídeo, som, controles, jogo, avançado) com a opção "restaurar padrões"
- Uma página de cheats para os códigos embutidos no jogo

Não há telemetria, conta, anúncios nem necessidade de acesso à rede para jogar.

## Requisitos

- Android 10 ou mais recente
- Uma GPU com suporte a Vulkan
- **Por enquanto, um aparelho que rode apps de 32 bits.** A build atual é `armeabi-v7a`. Alguns celulares recentes (Pixel 7 e mais novos, por exemplo) são só 64 bits e ainda não conseguem instalá-la. A build nativa de 64 bits (`arm64-v8a`) está em andamento.
- **Sua própria ROM, obtida legalmente** (veja abaixo)

## Instalar e jogar

1. Baixe o APK mais recente na página de [Releases](https://github.com/dedecidao/Wrecked-Metal/releases).
2. Abra no seu aparelho e permita a instalação de fontes desconhecidas se o Android pedir.
3. Abra o **Wrecked Metal** e toque em **Pick ROM**. Escolha a sua própria cópia do jogo.
4. O app verifica o arquivo (SHA1), converte para o formato certo se precisar e guarda na pasta privada dele. O arquivo nunca sai do seu aparelho.
5. Toque em **PLAY WRECKED METAL** ou **Play Fidelity**. Pronto.

### A ROM

O Wrecked Metal precisa da sua própria cópia de **Destruction Derby 64 (USA)**.

| | |
|---|---|
| Formatos aceitos | `.z64`, `.n64`, `.v64` (convertidos automaticamente) |
| SHA1 (`.z64`) | `a2c0799e13566d1b723299592cfcac9387f25fa7` |

Por favor, não peça ROMs em issues ou discussions. Esses pedidos serão fechados.

## Arquivos no seu aparelho

Em `Android/data/com.costelinhagames.wreckedmetal/files/`:

| Caminho | Conteúdo |
|---|---|
| `dd64.ini` / `dd64-fidelity.ini` | Configurações de cada modo |
| `Data/Saves/` | Save do Controller Pak (compartilhado pelos dois modos) |
| `Data/Music/` | Suas faixas de substituição opcionais |
| `Data/Logs/dd64.log` | Log da última sessão. Anexe quando for reportar um bug. |

## Roadmap

- [x] Build nativa para Android, 60 fps, controles touch e gamepad
- [x] Modos Aprimorado e Fidelity
- [ ] APK de release assinado
- [ ] Build nativa de 64 bits (`arm64-v8a`)
- [ ] Mais melhorias nativas e opções de qualidade de vida
- [ ] Novos modos e mecânicas de corrida construídos sobre o jogo original

Ideias e relatos de bugs são bem-vindos em [Discussions](https://github.com/dedecidao/Wrecked-Metal/discussions) e [Issues](https://github.com/dedecidao/Wrecked-Metal/issues).

## Apoie o projeto

O Wrecked Metal é, e vai continuar sendo, **gratuito**. Sem builds pagas, sem acesso antecipado, sem recursos atrás de paywall.

Se ele trouxe de volta boas lembranças e você quiser me ajudar a continuar trabalhando nele, pode me pagar um café no Ko-fi. É totalmente voluntário, e cada contribuição vira mais horas dedicadas a este projeto.

[![Apoie no Ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/costelinha_games)

## Compilando a partir do código-fonte

JDK 17, Gradle 8.14 (ou Android Studio), Android SDK com NDK 27 e CMake 3.22, Python 3 e `binutils-arm-linux-gnueabihf`.

```sh
git clone --recursive https://github.com/dedecidao/Wrecked-Metal
cd Wrecked-Metal
gradle assembleDebug
# APK em app/build/outputs/apk/debug/
```

A build é linkada com `symbols/dd64.syms`, que contém só nomes e endereços de símbolos, sem nenhum dado da ROM (veja [symbols/README.md](symbols/README.md)). As notas técnicas ficam em [DOCS_ANDROID_PORT.md](DOCS_ANDROID_PORT.md) (em inglês).

| Parte | Onde |
|---|---|
| Código do jogo (C descompilado) e o runtime de PC | `external/dd64` (submódulo) |
| SDL3: janela, Vulkan, áudio, gamepads | `external/SDL` (submódulo) |
| Ponto de entrada Android, logging, crash handler, háptico | `app/src/main/cpp/android/` |
| Launcher e controles touch | `app/src/main/java/io/github/dedecidao/dd64/` |

## Créditos

- A descompilação e o port de PC feitos pela comunidade, por [murkantor](https://github.com/murkantor/Destruction-Decomp-64), e o fork da engine deste projeto, [dedecidao/Destruction-Decomp-64](https://github.com/dedecidao/Destruction-Decomp-64)
- [SDL](https://libsdl.org) e Fast3D (Project Midas, veja `port/fast3d/LICENSE.txt` no submódulo da engine)
- Ports que mostraram o caminho no Android: 2ship2harkinian-Android, sm64coopdx-android, Zelda64Recomp-Android, dk64-recomp-android, BanjoRecomp-Android

## Aviso legal

O Wrecked Metal é um projeto de fã não oficial, feito para preservação e diversão pessoal. **Este repositório e o APK não contêm ROM, nem assets do jogo, nem qualquer material protegido por direitos autorais do jogo original.** O código do jogo é compilado a partir de uma descompilação da comunidade, e todos os dados do jogo são lidos em tempo de execução de uma ROM que você mesmo deve fornecer, extraída de um cartucho que você possui.

Destruction Derby e todos os nomes e marcas relacionados pertencem aos seus respectivos donos. Este projeto não é afiliado, endossado nem patrocinado por eles. As doações no Ko-fi são contribuições voluntárias pelo tempo do desenvolvedor e não compram nenhum software, conteúdo ou acesso.
