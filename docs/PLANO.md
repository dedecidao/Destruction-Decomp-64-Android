# Plano do port Android

Objetivo: um APK nativo de Destruction Derby 64 para Android, com controles touch e
suporte a gamepads, construído a partir do port de PC em
[dedecidao/Destruction-Decomp-64](https://github.com/dedecidao/Destruction-Decomp-64)
(branch `port-pc`). Aparelho de teste: Samsung Galaxy S23 Ultra (Snapdragon 8 Gen 2,
Adreno 740, Android 14+).

**Alvo final: `arm64-v8a`.**

## Ponto de partida: o que o port de PC já tem

O `port-pc` não é uma recompilação estática (N64Recomp/RT64). É a decompilação do jogo
compilada direto para o PC, com um runtime próprio no lugar da libultra:

| Peça | No `port-pc` | No Android |
|---|---|---|
| Janela, entrada, áudio, threads | SDL3 | SDL3 também roda no Android (`SDLActivity`) |
| Gráficos | Fast3D, com backend OpenGL e **SDL_gpu (Vulkan)** | SDL_gpu → Vulkan na Adreno 740, sem mudar o renderizador |
| Ordem de bytes | Hooks de `-fsanitize=kernel-address` (`port/src/shadow.c`) | O clang do NDK gera os mesmos hooks (`__asan_load4_noabort`...) |
| RAM do N64 | Mapeada nos endereços do N64 (0x80000000) | Igual, com `mmap(MAP_FIXED_NOREPLACE)` |
| Ponteiros de funções N64 | Stubs `jmp` x86 nos endereços do N64 | Stubs ARM (`ldr pc, [pc, #-4]`) em 32-bit; tabela de saltos em 64-bit |
| Tamanho de ponteiro | **32-bit** (`-m32`) | **Precisa virar 64-bit** (fase 2) |

Os repositórios de referência (dk64-recomp-android, Zelda64Recomp-Android, BanjoRecomp-Android,
sm64coopdx-android, 2ship2harkinian-Android) servem de guia para Gradle/NDK, seletor de ROM
com SAF, layout dos controles touch e escala da interface. RT64, Turnip e libadrenotools
não são necessários de início: o renderizador é o Fast3D sobre Vulkan, e o driver da
Adreno 740 já basta. libadrenotools pode entrar depois, se algum driver der problema.

## Estrutura do repositório

```
external/dd64        submódulo: Destruction-Decomp-64, branch port-pc (o jogo e o runtime de PC)
external/SDL         submódulo: SDL 3.4.20 (biblioteca nativa e o lado Java)
symbols/dd64.syms    nomes e endereços do build N64 (sem dados da ROM)
tools/               export_symbols.py e o adaptador dos geradores de tabela do port-pc
app/src/main/cpp     CMake do NDK e o código só do Android (android/)
app/src/main/java    LauncherActivity (ROM), GameActivity (SDL), TouchControlsView
.github/workflows    build do APK no GitHub Actions
```

O código do `port-pc` é usado como está. O que muda no Android fica em
`app/src/main/cpp/android/`, ou entra por `#define` no CMake (por exemplo, `input.c` é
compilado com `Port_InputGetPad` renomeado, e `touch.c` mistura o touch no controle 1).
Mudanças que precisarem ir no C do jogo (a conversão 64-bit) vão para o `port-pc` sob
`#ifdef`, sem quebrar o build de PC nem o build N64 que precisa bater byte a byte.

## Fases

### Fase 1: app Android e build (em andamento)

- [x] Gradle + NDK (CMake), SDL3 como submódulo, `GameActivity` herdando `SDLActivity`
- [x] Seletor de ROM com Storage Access Framework: aceita .z64/.n64/.v64, converte para
      .z64, confere o SHA1 de Destruction Derby 64 (USA) e guarda na área privada do app
- [x] Controles touch (Java, por cima da tela do jogo): analógico que aparece onde o dedo
      toca, A/B/Z/R/L/Start e os quatro C, deslizar entre botões, vibração, opacidade e
      tamanho ajustáveis, some quando um gamepad é conectado
- [x] Gamepads: o `input.c` do PC já usa `SDL_Gamepad` (Bluetooth e USB), incluindo
      gatilhos analógicos para acelerador e freio e rumble
- [x] Pasta de dados no armazenamento do app (`Android/data/<app>/files/Data`): saves do
      Controller Pak, `dd64.ini`, logs e fontes HD, acessível pelo gerenciador de arquivos
- [x] Build do APK no GitHub Actions
- [ ] `symbols/dd64.syms` exportado do `build/dd64.elf` do Andre (destrava o link do jogo)
- [ ] Build 32-bit (`armeabi-v7a`) completo como referência, se algum aparelho de teste
      aceitar 32-bit

### Fase 2: código nativo em 64-bit (`arm64-v8a`)

O jogo guarda ponteiros em campos de 4 bytes: nas structs, nos dados da ROM (os .o das
pistas dentro do `zippeddata.zip` são relocados com endereços de 32 bits) e nas listas
gráficas (`Gfx.w1`). A estratégia é a do "baixo 4 GB":

1. **Toda memória que o jogo enxerga fica abaixo de 4 GB.** A RAM do N64 continua em
   0x80000000 (um processo arm64 consegue mapear ali). As pilhas das threads do jogo
   passam a ser alocadas dentro dessa região, e o heap do jogo já fica nela.
2. **Ponteiros do jogo armazenados em campos de 32 bits viram deslocamentos `u32`**
   (endereços N64), convertidos em ponteiro nativo na hora do uso, com zero-extend
   (e nunca sign-extend: 0x80xxxxxx com sinal vira endereço de kernel no arm64).
3. **Ponteiros de função**: as funções do jogo ganham uma tabela de saltos em baixa memória
   (`ldr x16, literal; br x16`, 8 bytes por função, com os literais logo abaixo de
   0x80000000), então `&funcao` guardado num campo de 32 bits vira o endereço N64 do stub.
4. **Structs com ponteiros que vêm da ROM** mantêm o layout de 32 bits (tipos `N64Ptr`).
   As structs só do host podem usar ponteiros nativos.
5. O runtime (`port/src`, Fast3D) lê endereços de display list como `u32` e traduz com
   `Port_N64ToHost`, o que já acontece em boa parte.

Ferramentas para achar os pontos a mudar: os avisos do clang em arm64
(`-Wint-to-pointer-cast`, `-Wpointer-to-int-cast`, `-Wshorten-64-to-32`), o job
"arm64 compile check" do CI, e os checadores de `tools/port/` do `port-pc`.

Cada rodada é testada no S23 Ultra via o APK do CI. O log fica em
`Android/data/com.costelinhagames.wreckedmetal/files/Data/Logs/dd64.log`.

### Fase 3: acabamento

- Editor de layout dos controles touch (arrastar e redimensionar botões)
- Escala de UI, proporção 16:9/20:9 (o port de PC já tem `widescreen`)
- Giroscópio como direção (opcional, via sensores do SDL)
- Exportar e importar saves pelo SAF
- Chave de assinatura de release e GitHub Releases

## Testes no S23 Ultra

1. Baixe o APK do último build em *Actions → Android APK → Artifacts*.
2. Instale, abra, toque em **Escolher ROM** e escolha a sua ROM.
3. **Jogar.** Se algo falhar, mande o arquivo `Data/Logs/dd64.log` (pasta mostrada na tela
   inicial) ou a saída de `adb logcat -s DD64 SDL`.
