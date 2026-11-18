# arm64-v8a: análise e caminho

Atualizado em 2026-11-18, depois do primeiro teste no Galaxy S23 Ultra (Android 17, SDK 37).

## Por que o build 32-bit não sobe no aparelho

O `:game` roda em `app_process32`. Antes de `SDL_main`, o ART já mapeou nesse processo:

```
79a00000-7ba00000 r--s /memfd:jit-cache
7ba00000-7da00000 r-xs /memfd:jit-cache
7da00000-81a00000 rw-s /memfd:jit-cache
81a00000-83a00000 ---p [anon:dalvik-linear-alloc shadow map]
83a52000-8bc52000 ---p [anon:libwebview reservation]
```

O port precisa de 0x80000000-0x82011000 para a RAM do N64 (os ponteiros nos dados da
ROM são 0x80xxxxxx), e o `mmap(MAP_FIXED_NOREPLACE)` falha. Num app normal (forkado do
zygote) não há como mapear antes do ART. Em builds *debuggable* existe uma saída para
testes: `wrap.sh` com uma biblioteca em `LD_PRELOAD`, que reserva o endereço antes do ART
iniciar. Ela está sendo testada no branch `fix/armv7-ram`. Para um APK de release, o
caminho é o arm64.

## O que impede o arm64 hoje

Números medidos compilando o C do jogo para `aarch64-linux-android`:

| Problema | Tamanho |
|---|---|
| `s32`/`u32` são `long` em `include/ultra64.h` (8 bytes em LP64) | 1 typedef. É o primeiro erro do CI (`5FC50.c`: array de tamanho negativo) |
| Membros ponteiro em structs que espelham memória/dados do N64 (ficam com 8 bytes) | ~1.616 membros em ~2.500 definições de struct, em 435 arquivos (muitas são structs locais repetidas do m2c) |
| `M2C_FIELD(p, void **, off)` e afins: leitura de ponteiro num campo de 4 bytes | ~180 usos |
| Casts inteiro↔ponteiro com o typedef corrigido | 17 `pointer-to-int`, 1 `int-to-pointer`, 45 `shorten-64-to-32`, 18 `int-conversion` |
| Ponteiros de função guardados em dados do jogo | Os stubs nos endereços do N64 já cobrem os que vêm da ROM; os `&funcao` do C precisam ficar abaixo de 4 GB |

## Atalhos avaliados e descartados

| Ideia | Resultado |
|---|---|
| ABI ILP32 do AArch64 (`aarch64-linux-gnu_ilp32`) | O backend do clang 18 não aceita (`data layout ... does not match`), e o bionic não tem ILP32 |
| `__ptr32 __uptr` (ponteiros de 32 bits do MSVC) | O clang ignora em AArch64: as structs continuam com 8 bytes por ponteiro |
| Mapear 0x80000000 em 32-bit sem `wrap.sh` | O ART ocupa o endereço antes do nosso código rodar |

## Caminho escolhido: conversão do C, no estilo dos ports de decompilação

Ship of Harkinian, sm64 e o port de PC de Perfect Dark (fgsfdsfgs/perfect_dark) fizeram
isso. O Perfect Dark é o caso mais parecido, porque também tem ponteiros dentro dos dados
carregados da ROM. Plano, no `port-pc` sob `TARGET_PC64`, sem mudar o build de PC 32-bit
nem o N64:

1. **Tipos**: `s32`/`u32` viram `int`/`unsigned int` (`<stdint.h>`).
2. **Código e globais abaixo de 4 GB**: o `libdd64.so` vira um carregador pequeno que
   reserva uma região baixa e carrega o jogo (`libdd64game.so`) dentro dela com
   `android_dlopen_ext(ANDROID_DLEXT_RESERVED_ADDRESS)`. Assim, `&funcao` e `&global`
   cabem em 32 bits, e os stubs nos endereços do N64 continuam valendo.
3. **RAM do N64 em 0x80000000** no processo 64-bit. Confirmar com o APK de teste
   `dd64-arm64-probe.apk` (release `nightly`).
4. **Pilhas das threads do jogo abaixo de 4 GB** (`pthread_attr_setstack` numa região
   baixa), porque o jogo guarda endereços de variáveis locais em campos de 4 bytes.
5. **Membros ponteiro** das structs que espelham dados do N64 viram um tipo de 4 bytes
   (`N64Ptr(T)`, um `u32` com macros para ler e escrever), struct por struct, começando
   pelos headers limpos (`car.h`, `heap.h`, `game.h`) e pelos tipos da libultra
   (`OSMesgQueue`, `OSThread`, `OSTask`, `Gfx`).
6. **`M2C_FIELD` de ponteiro** passa a ler 4 bytes e estender com zero.
7. **Runtime** (`port/src`, Fast3D): ler endereços de display list e de mensagens como
   `u32` e traduzir com `Port_N64ToHost`. Boa parte disso já é feito assim.
8. Cada passo é verificado com o job "arm64 compile check" do CI e, quando linkar, no S23.
