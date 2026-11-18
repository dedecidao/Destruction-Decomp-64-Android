# symbols/

`dd64.syms` is the symbol table of the matching N64 build: each function's and
global's name and N64 address, one per line. The Android build uses it to place
every game global at its N64 address and to build the jump table for N64
function pointers, the same tables the PC build makes from `build/dd64.elf`.

It holds **no code or data from the ROM**, only names and addresses, so it can
be committed. The ELF itself must never be committed.

## Making it (once, and again whenever the N64 build's layout changes)

On the machine with the matching build of
[Destruction-Decomp-64](https://github.com/dedecidao/Destruction-Decomp-64)
(`make extract && make` printing `build/dd64.z64: OK`, on the `port-pc` branch):

```sh
python3 tools/export_symbols.py /path/to/Destruction-Decomp-64/build/dd64.elf symbols/dd64.syms
git add symbols/dd64.syms
```

The `port-pc` commit the ELF was built from must be the one `external/dd64`
points at, or the addresses will not fit the code.

## dd64_widths.c

The table of static data the port converts to host byte order at boot (address and
width of every scalar the game's C declares), as `tools/port/gen_widths.py` writes it
in the PC build. It is taken from the PC build because clang, unlike GCC, writes no
debug info for `extern` variables, so the generator finds almost nothing in the ARM
objects. Like `dd64.syms` it holds only addresses and widths. Copy it again from the
PC build's `build-win/gen/dd64_widths.c` whenever `external/dd64` moves, and keep the
comment line at the top naming the commit.