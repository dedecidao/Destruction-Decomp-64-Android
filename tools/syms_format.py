"""symbols/dd64.syms: the N64 build's symbol table as text.

    # name value type bind
    boot 0x80000450 2 1

type and bind are the ELF st_info fields (type 2 = function, bind 1 = global),
which is all tools/port/gen_data.py and gen_widths.py read from the ELF.
"""
from pathlib import Path

HEADER = '# Destruction Derby 64: symbols of the matching N64 build (tools/export_symbols.py). No ROM data.\n'


def write_syms(path, syms):
    lines = [HEADER, '# name value type bind\n']
    for name, value, typ, bind in syms:
        if not name or ' ' in name:
            continue
        lines.append(f'{name} 0x{value:08X} {typ} {bind}\n')
    Path(path).write_text(''.join(lines))


def read_syms(path):
    syms = []
    for line in Path(path).read_text().splitlines():
        if not line or line.startswith('#'):
            continue
        name, value, typ, bind = line.split()
        syms.append((name, int(value, 16), int(typ), int(bind)))
    return syms
