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
    elif mode == 'widths':
        import gen_widths
        gen_widths.read_symbols = gen_data.read_symbols
        sys.argv = ['gen_widths.py', '--elf', syms_path] + argv
        gen_widths.main()
    else:
        sys.exit(f'unknown mode {mode}')


if __name__ == '__main__':
    main()
