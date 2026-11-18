#!/usr/bin/env python3
"""Export the symbol table of the matching N64 build for the Android build.

Run once, on the machine where the matching build was made (see symbols/README.md):

    python3 tools/export_symbols.py path/to/Destruction-Decomp-64/build/dd64.elf symbols/dd64.syms

The output has one line per symbol: name, value, type and binding. It holds only
names and addresses, no code or data from the ROM, so it can be committed; the
ELF itself must not be (it contains the game's code).
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from syms_format import write_syms  # noqa: E402


def read_elf_symbols(elf_path):
    import struct
    data = Path(elf_path).read_bytes()
    if data[:4] != b'\x7fELF' or data[4] != 1 or data[5] != 2:
        sys.exit(f'{elf_path}: not a 32-bit big-endian ELF (is it build/dd64.elf?)')
    e_shoff, = struct.unpack_from('>I', data, 0x20)
    e_shentsize, e_shnum = struct.unpack_from('>HH', data, 0x2E)
    sections = [struct.unpack_from('>IIIIIIIIII', data, e_shoff + i * e_shentsize) for i in range(e_shnum)]
    syms = []
    for sh in sections:
        if sh[1] != 2:  # SHT_SYMTAB
            continue
        strtab = sections[sh[6]]
        for off in range(sh[4], sh[4] + sh[5], 16):
            name_off, value, size, info, other, shndx = struct.unpack_from('>IIIBBH', data, off)
            end = data.index(b'\0', strtab[4] + name_off)
            syms.append((data[strtab[4] + name_off:end].decode(), value, info & 0xF, info >> 4))
    return syms


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    syms = read_elf_symbols(sys.argv[1])
    write_syms(sys.argv[2], syms)
    print(f'{sys.argv[2]}: {len(syms)} symbols')


if __name__ == '__main__':
    main()
