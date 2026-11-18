#!/usr/bin/env python3
"""Run the PC port's table generators (external/dd64/tools/port) from symbols/dd64.syms
instead of build/dd64.elf, so the APK builds without the ROM.

    gen_from_syms.py data   --syms S --dd64 D --out DIR
    gen_from_syms.py widths --syms S --dd64 D --objdump OBJDUMP --out FILE objects...

The generators are used as they are; only their ELF reader is swapped for the
text file.
"""
import sys
from pathlib import Path


def pop_arg(argv, name):
    i = argv.index(name)
    value = argv[i + 1]
    del argv[i:i + 2]
    return value


# The -D renames the game is built with (app/src/main/cpp/CMakeLists.txt), so
# the game's own libc and libm keep their N64 behaviour without clashing with the host's
HOST_DEFINE_RENAMES = {n: 'dd64_' + n for n in (
    'acos asin atan atan2 log log10 pow exp frexp modf sinf cosf rand atoi ldiv lldiv').split()}


def relabel_fix(path):
    """dd64_fix.c names every game function as `extern char f[]`. With clang that
    clashes with libc names (memcpy, atoi...) and with the -D renames, so each one
    gets a private C name and its real link name as an asm label."""
    import re
    decl = re.compile(r'^extern char (\w+)\[\] __attribute__\(\(weak\)\);$')
    names = set()
    lines = []
    for line in path.read_text().splitlines():
        m = decl.match(line)
        if m:
            n = m.group(1)
            names.add(n)
            link = HOST_DEFINE_RENAMES.get(n, n)
            line = f'extern char dd64fix_{n}[] __asm__("{link}") __attribute__((weak));'
        lines.append(line)
    entry = re.compile(r'^(    \{ 0x[0-9A-F]+, )(\w+)( \},)$')
    out = []
    for line in lines:
        m = entry.match(line)
        if m and m.group(2) in names:
            line = f'{m.group(1)}dd64fix_{m.group(2)}{m.group(3)}'
        out.append(line)
    path.write_text('\n'.join(out) + '\n')


def main():
    argv = sys.argv[1:]
    mode = argv.pop(0)
    syms_path = pop_arg(argv, '--syms')
    dd64 = Path(pop_arg(argv, '--dd64'))

    sys.path.insert(0, str(Path(__file__).resolve().parent))
    sys.path.insert(0, str(dd64 / 'tools' / 'port'))
    from syms_format import read_syms
    import gen_data

    syms = read_syms(syms_path)
    gen_data.read_symbols = lambda _elf: syms

    if mode == 'data':
        sys.argv = ['gen_data.py', '--elf', syms_path] + argv
        gen_data.main()
        out = Path(argv[argv.index('--out') + 1])
        relabel_fix(out / 'dd64_fix.c')
    elif mode == 'widths':
        import gen_widths
        gen_widths.read_symbols = gen_data.read_symbols
        sys.argv = ['gen_widths.py', '--elf', syms_path] + argv
        gen_widths.main()
    else:
        sys.exit(f'unknown mode {mode}')


if __name__ == '__main__':
    main()
