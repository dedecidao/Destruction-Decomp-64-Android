# DOCS_ANDROID_PORT.md — memória do projeto (port Android)

Este arquivo guarda o que o André pediu para "memorizar", as descobertas técnicas do
port e as variáveis importantes do jogo. Atualizar a cada fase. As opções do menu e o
mapa de memória detalhado estão em [DOCS_INTERNALS_AND_OPTIONS.md](DOCS_INTERNALS_AND_OPTIONS.md).

## Tela de referência: Game Mode

O primeiro menu depois do "press start" (world championship / arcade / time trial /
multi-player, "press z-button for options menu", "press r-button for pc options").

| O quê | Onde |
|---|---|
| Modo de jogo | `MODE_MAIN` (`gGame->mode`) |
| Desenha a tela | `external/dd64/src/auto/40338.c` `MainMenu_Show` |
| Cima / baixo | `src/auto/3FD00.c` `MainMenu_Prev`, `src/auto/3FE04.c` `MainMenu_Next` |
| A (escolhe o modo) | `src/auto/3FF30.c` `MainMenu_Select` |
| Z abre as opções do jogo; entrada dos códigos de cheat | `src/auto/40244.c` `MainMenu_CheckOptions` (`Cheat_UpdateInput`) |
| Opções do jogo (music/voice/sfx/playsong/stereo), `MODE_OPTIONS` | `src/auto/47F6C.c` `OptionsMenu_Prev/Next`, `483D4.c` `_Left`, `48A0C.c` `_Right`, `4905C.c` `_Back`, `492A8.c` `_Update`, `49380.c` `_Init` |
| Menu de PC (R) | `external/dd64/port/game/pcmenu.c` (desenho e entrada), `external/dd64/port/src/options.c` (páginas e linhas), `port/src/config.c` (chaves e padrões do dd64.ini) |

## Descobertas (bugs corrigidos no Android)

| Sintoma | Causa | Correção |
|---|---|---|
| "Could not map the game's memory at 0x80000000" | Num processo 32-bit o ART põe o cache do JIT e o shadow map do linear-alloc em 0x7DA00000-0x83A00000 | `wrap.sh` (builds debug) pré-carrega `libdd64reserve.so`, que reserva a faixa antes do ART subir |
| Tela preta | `gen_widths.py` achava 20 valores estáticos em vez de 4842: o clang não gera DWARF para variáveis `extern` | `symbols/dd64_widths.c` vindo do build de PC |
| Tela preta | SDL_gpu com MSAA 4x fica preto na Adreno 740 | MSAA travado em 1x (`android/gpu_caps.c`) |
| Carregamento de minutos | TLS emulado pelo NDK abaixo do minSdk 29 (`__thread` em `port/src/shadow.c`, todo acesso à memória) | minSdk 29 |
| Tela de título nunca vinha | `inflate_codes` (`src/auto/51980.c`) compilado com clang -O2 para ARM entra em loop infinito | Esse arquivo em -O0 (causa no clang ainda aberta) |
| "Could not load ROM" depois de atualizar | Activity recriada sem o extra do intent | `GameActivity` usa `files/rom/dd64.z64` como padrão |
| **Crash com Z + baixo na tela Game Mode / opções** (fase 1) | `D_800BAD10` (tabela do menu de opções) é *definido* em `47F6C.c` e *também* posto em 0x800BAD10 pelo `dd64_syms.ld`. Com `-fvisibility=hidden` o clang acessava relativo ao PC, como se o .so estivesse carregado no endereço 0: `OptionsMenu_Next` lia 0x800BAD10 + endereço de carga (0x5AFC1D10) | Objetos do jogo com `-fvisibility=default`: os 5 globais definidos no jogo e fixados pelos syms (`D_800B7DFC`, `D_800B7F8C`, `D_800B8244`, `D_800BAD10`, `D_800BBD94`) passam pela GOT, que tem o endereço absoluto |

Regra geral: **um `.so` 32-bit (PIC) não é o exe de endereço fixo do PC**. Todo símbolo
do jogo num endereço N64 absoluto tem de ser acessado pela GOT, nunca relativo ao PC.
No código do port (visibilidade hidden), usar `Port_N64ToHost(endereço)` em vez de
`extern`.

## Menus no Android (fase 3)

O menu de PC (R no menu do jogo) no Android: `external/dd64/port/src/options.c` (páginas
sob `#ifdef __ANDROID__`; as do PC continuam iguais) e `port/src/config.c` (padrões
`DEF(pc, android)` e valores forçados depois de ler o dd64.ini). O submódulo
`external/dd64` agora segue a branch **`android-port`** do fork.

| Página | Linhas no Android |
|---|---|
| video | resolution (720p/1080p de saída), vsync, interpolation, 3d resolution (720p/1080p), anti-aliasing (off/2x), clean cut-outs, hd fonts, seamless sky, draw distance (x8/x16), fps counter, renderer (vulkan / opengl es) |
| sound | master volume, audio delay |
| controls | rumble, rumble strength, stick dead zone, stick range, camera dead zone, analog pedals |
| game | drones, set all drones to, announce every wreck, cpu unlockables |
| advanced (nova) | screen shape (4:3/widescreen), n64 aa blur, texture filter, hud boxes, anisotropy, skip intros |

Toda página termina em **restore defaults** (`Port_ConfigDefault`).

Padrões do Android e o porquê:

| Opção | Padrão | Motivo |
|---|---|---|
| resolution (saída) | 720p | `GameActivity.applyOutputResolution` dimensiona a surface pela altura (largura segue o formato da tela); 1080p para quem quiser nitidez |
| 3d resolution | 720p | maior custo de GPU/bateria; 1080p disponível |
| anti-aliasing | **2x** | GPUs de celular são *tile-based*: as amostras do MSAA ficam na memória do tile e 2x custa pouco. 4x fica **preto** com SDL_gpu na Adreno 740 (testado), então o máximo no Android é 2x (`android/gpu_caps.c`) |
| n64 aa blur | on (avançado) | o visual do N64; precisa de MSAA ≥ 2x |
| texture filter | n64 3-point (avançado) | o filtro real do N64 (3 amostras), custo quase igual ao bilinear |
| anisotropy | 16x (avançado) | barato em GPUs modernas, chão nítido |
| draw distance | x16 (só x8/x16) | geometria do N64 é leve |
| hud boxes | off (avançado) | pedido |
| screen shape | widescreen (avançado), sem "stretched" | pedido |
| skip intros | on (avançado) | pedido |
| renderer | vulkan | o OpenGL ES funciona (shim de `glDepthRangef` em `gfx_opengl.cpp`) mas roda a ~3 fps na Adreno 740 |
| right stick camera | on, oculta | pedido |
| keyboard drives, raphnet, controller paks, páginas keys/pad, display, frame limit | ocultos | não se aplicam ao celular |

## Testes no celular (adb)

- Script: `C:\Users\Admin\Documents\claudeProjects\DD64-android-logs\test-run.ps1`
  (`-Install`, `-Env "DD64_PRESS=...;DD64_GFXLOG=1"`, `-Wait`): abre o `GameActivity`
  direto (exportado nos builds debug), tira print e salva os logs.
- `Data/env.txt` no celular: chaves `DD64_*`, como o shell do PC define
  (`DD64_PRESS`, `DD64_BOOT`, `DD64_FAKEGAS`, `DD64_GFXLOG`, `DD64_STACKS=<ms>`,
  `DD64_PEEK=end,...`, `DD64_TEXDUMP`, `DD64_SHOT`).
- O SDL manda caminhos relativos (dumps) para a pasta interna `files/`: `adb shell run-as
  io.github.dedecidao.dd64.debug ls files`.

## Memorizado (pedido do André)

- **Announce Every Wreck (fase 3):** padrão `OFF` por enquanto. Futuramente será
  forçado para `ON`, com SFX customizados ou efeitos visuais injetados no gancho onde a
  batida é anunciada: `external/dd64/port/src/field.c`, `Port_FieldAnnounceWreck`,
  marcado com `TODO(FUTURE_MOD)`.
