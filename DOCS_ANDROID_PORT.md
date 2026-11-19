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
| game | announce every wreck (drones, set all drones to e cpu unlockables removidos: ficam em 1x / default / off) |
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
| interpolation | off | decisão do André após a fase 2 |
| clean cut-outs | on | decisão do André após a fase 2 |
| seamless sky | on | decisão do André após a fase 2 |
| drones / set all drones to / cpu unlockables | 1x / default / off, removidos | decisão do André: os drones faziam carros voarem na largada das arenas |
| raphnet | oculto | não é compilado no Android |
| keyboard drives, raphnet, controller paks, páginas keys/pad, display, frame limit | ocultos | não se aplicam ao celular |

## Vibração (fase 4)

Caminho do rumble do jogo até o motor:

| Etapa | Onde |
|---|---|
| O jogo pede rumble | `Pad_Rumble(canal, quadros, força)` (`src/auto/59D40.c`), chamado em batidas entre carros (`28C70.c`), destruição/aterrissagem (`1C624.c`, 30 quadros a 0,9 e contínuo pela força do impacto) e `2E9D0.c` (5 quadros a 0,75); zerado nos menus/pausa (`46894.c`, `476B8.c`) |
| Liga/desliga o motor (PWM: mais fraco = pulsos mais espaçados) | `Pad_Update` (`59B08.c`) → `__osMotorAccess` (`port/src/os_hw.c`) |
| Port | `Port_InputRumble`: no PC só gamepads (`port/src/input.c`, compilado no Android como `Port_InputRumbleHost`) |
| Android | `app/src/main/cpp/android/haptics.c`: gamepads como no PC (o motor do Rumble Pak via `Port_InputRumbleHost`); **sem gamepad, o vibrador do celular** (SDL haptic "VIBRATOR_SERVICE" → `VibrationEffect` com amplitude), com **perfil adaptativo**: `Pad_Rumble` do jogo é embrulhado (`59D40.c` compilado com `Pad_Rumble=Pad_RumbleGame`) |

Perfil do vibrador (pedido do André: sair da pista é comum e não pode ser forte/chato;
batidas continuam fortes):

| Evento | Como é detectado | Vibração (× rumble strength) |
|---|---|---|
| Terra/grama/zebra | `Pad_Rumble(ch, 1, 0.5/0.6)` a cada quadro (`1C624.c`) | **sutil**: 12%, pulsos de 150 ms renovados |
| Raspar na parede/carro | contatos leves (`28C70.c`, força < 0,85) repetidos em menos de 200 ms | **suave**: 25%, pulsos de 150 ms |
| Batida | contato isolado ou forte (`28C70.c`), gatilhos (`2E9D0.c`) | **forte**: 55% a 100% pela força, 35 ms por quadro pedido (90-650 ms) |
| Carro destruído | `Pad_Rumble(ch, 30, 0.9)` (`1C624.c`) | 100%, ~0,65 s |

Pulsos sutis nunca cortam uma batida em andamento.

Testado no S23 (45 s de corrida): 10 pulsos de raspão a 25%/150 ms; batidas a 55% (175 ms),
66%, 95% e 100% (350 ms).

## Áudio

Como o jogo toca som: sintetizador libultra (amostras ADPCM da ROM, mixadas pelo microcódigo
de áudio do RSP, emulado no host em `port/src/audio.c`). **A música é sequenciada** (notas
tipo MIDI tocadas pelos instrumentos dos bancos de som do jogo), não é áudio gravado.

| O quê | Onde |
|---|---|
| Taxa do sintetizador | `src/auto/5D430.c` `Audio_LoadBanks` (`cfg.x2C`): 22050 Hz no N64 |
| Início de cada música | `src/auto/90EC.c` `Res_LoadSoundBank`, recurso tipo 13: copia a sequência da ROM (`p->x4`..`p->x8`) e chama `Seq_StartWithBank`; handle em `D_8011EEE0` |
| Volume / fades da música | `Sound_SetVolume` (`66B24.c`) com a chave `D_8011EEE0`; `Sound_FadeOutAll` (`66960.c`) bit 2 = música (vozes com `sfxId == 0`) |
| Pitch das vozes | `AudioVoice_UpdatePitch` (`676B4.c`) → `alSynSetPitch` → `alResampleParam` (`7438C.c`, caso 7): razão pura, assume 22050 Hz |
| Reverb | `alFxNew` (`71F34.c`): atrasos em amostras |

**Synth quality (`[audio] synth_rate`, página sound):** 22050 (N64) ou **44100 (padrão no
Android)**. A 44100 o resampler recebe metade da razão, os atrasos do reverb dobram e o
low-pass mantém o corte: mesmo tom e mesmo reverb, sem o teto de 11 kHz do N64 (agudos
mais limpos, sem aliasing nos instrumentos transpostos para cima). Medido no S23: mesma
fundamental (531 vs 564 cruzamentos/s), mesmo nível (RMS 4471 vs 4541). O heap de áudio do
N64 (0x35B60 em 0x800D3200) só tinha 6 KB livres; a 44100 o sintetizador usa um heap próprio
de 512 KB no topo da RAM do port (`Port_SynthHeapSize`, o heap principal termina abaixo,
`5C480.c`).

**Músicas substitutas (`port/src/music.c`):** se existir
`Android/data/io.github.dedecidao.dd64.debug/files/Data/Music/song_XXXXXXXX.ogg` (ou `.mp3`),
ela toca no lugar da música do jogo, em loop, num stream SDL próprio; a música original
continua rodando em silêncio (o fluxo do jogo espera por ela), e o slider de música e os
fades vão para o arquivo. Decoders: stb_vorbis 1.22 e dr_mp3 (domínio público,
`external/dd64/port/third_party`). Mono ou estéreo, qualquer taxa (o SDL converte).

As 13 músicas (id = onde a sequência começa na ROM; ordem do "playsong" das opções do jogo):

| id | Onde toca |
|---|---|
| `song_00D9A7B0` | tela de título / menus |
| `song_00DAF910`, `song_00D90F00`, `song_00DA6330`, `song_00D89010`, `song_00DB7650`, `song_00DAC320`, `song_00DB3880`, `song_00DBB960`, `song_00DA0180`, `song_00DBEF30`, `song_00D972A0`, `song_00DC4FB0` | playsong (corridas/arenas) |

**Dump para remasterizar:** `DD64_MUSICDUMP=1` no `Data/env.txt` grava cada música tocada em
`Data/MusicDump/song_XXXXXXXX.wav` (a saída do jogo enquanto aquela música é a atual; para a
música pura, use o "playsong" nas opções, onde não há efeitos). Fluxo: dump → remasterizar/
substituir → salvar como `.ogg` (Vorbis q6+, 44,1/48 kHz) com o mesmo nome em `Data/Music`.

## Menu Cheats (fase 5)

Página **cheats** no menu de PC (PC e Android), `external/dd64/port/src/options.c`
(`kCheats`, `applyCheats`), chaves `[cheats]` no dd64.ini. Usa os flags dos códigos
do próprio jogo (DOCS_INTERNALS_AND_OPTIONS.md):

| Linha | Flag | Código original | Quando vale |
|---|---|---|---|
| unlock all cars | `0x801253F8` | DUD | na hora (seleção de carro) |
| unlock all tracks | `0x801253F9` | BRAD | na hora (todas as dificuldades, e com elas as pistas do campeonato) |
| indestructible car | `0x8012540C + 0x18*j` `+0` | BAR | na próxima corrida |
| no time limit | `+1` | DAD | na próxima corrida |
| super ram | `+2` | LRLR | na próxima corrida (zonas de dano em 100: bate forte e sofre pouco) |
| heavy car | `+4` | BALD | na próxima corrida |

Os flags são reaplicados a cada leitura dos controles (`Port_InputNoteMenuPad`), porque
`Cheat_ResetAll` os zera ao trocar de modo. Desligar uma linha zera o flag uma vez; os
códigos digitados no jogo continuam funcionando. Valem para os 4 jogadores.

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
