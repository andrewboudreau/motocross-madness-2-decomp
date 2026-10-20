#!/usr/bin/env python3
"""Compare VC6 C++ exception-handling tables of retail functions and candidates.

A VC6 function with destructible locals, `new` expressions or try blocks
under /GX starts with `push -1; push handler` (after `push ebp; mov ebp, esp`
when it keeps a frame pointer). The handler is `mov eax, funcinfo; jmp
__CxxFrameHandler`, and the FuncInfo (magic 0x19930520) records:

  +4  maxState     number of unwind states (destructible objects / `new`s)
  +8  pUnwindMap   maxState entries of (toState, action funclet)
  +12 nTryBlocks
  +16 pTryBlockMap nTryBlocks entries of (low, high, catchHigh, nCatches, pHandlers)

These counts are confirmed-level evidence of how many try blocks and
destructible locals the source had. This tool parses them from the retail
image and from a candidate COFF object (following relocations) and reports
where the shapes differ.

    python tools/eh_funcinfo.py 0x4e9030
    python tools/eh_funcinfo.py 0x4e9030 --obj X.obj --symbol '?F@C@@QAEXXZ'
    python tools/eh_funcinfo.py --scan work/s14/rank.json
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from mcm2tool.coff import CoffObject  # noqa: E402
from mcm2tool.pe import PEImage  # noqa: E402

MAGIC = 0x19930520
PROLOGUES = (b'\x6a\xff\x68', b'\x55\x8b\xec\x6a\xff\x68')


def u32(b: bytes, o: int) -> int:
    return struct.unpack_from('<I', b, o)[0]


def i32(b: bytes, o: int) -> int:
    return struct.unpack_from('<i', b, o)[0]


def handler_operand_offset(code: bytes) -> int | None:
    """Offset of the handler imm32 in an EH prologue, or None."""
    for p in PROLOGUES:
        if code.startswith(p):
            return len(p)
    return None


def shape(info: dict | None) -> dict | None:
    """The source-level shape: state count, unwind chain and try/catch counts."""
    if info is None:
        return None
    return dict(max_state=info['max_state'],
                to_states=[t for t, _ in info['unwind']],
                null_actions=[a in (0, '0x0') for _, a in info['unwind']],
                try_blocks=[(t['low'], t['high'], t['catch_high'], t['catches']) for t in info['try']])


def retail(va: int, pe: PEImage) -> dict | None:
    code = pe.bytes_at_va(va, 16)
    off = handler_operand_offset(code)
    if off is None:
        return None
    handler = u32(code, off)
    h = pe.bytes_at_va(handler, 5)
    if h[0] != 0xb8:  # SEH (__except_handler3) frames are not C++ FuncInfo
        return None
    fi = u32(h, 1)
    f = pe.bytes_at_va(fi, 28)
    if u32(f, 0) != MAGIC:
        return None
    out = dict(funcinfo=hex(fi), max_state=u32(f, 4), try_blocks=u32(f, 12))
    n = out['max_state']
    um = pe.bytes_at_va(u32(f, 8), 8 * n) if n else b''
    out['unwind'] = [(i32(um, 8 * k), hex(u32(um, 8 * k + 4))) for k in range(n)]
    out['try'] = []
    nt = out['try_blocks']
    tb = pe.bytes_at_va(u32(f, 16), 20 * nt) if nt else b''
    for k in range(nt):
        lo, hi, ch, nc, _ = struct.unpack_from('<iiiiI', tb, 20 * k)
        out['try'].append(dict(low=lo, high=hi, catch_high=ch, catches=nc))
    return out


def find_symbol(o: CoffObject, name: str):
    exact = [s for s in o.symbols if s.name == name and s.section_number > 0]
    if exact:
        return exact[0]
    loose = [s for s in o.symbols if name in s.name and s.section_number > 0 and s.type == 0x20]
    return loose[0] if loose else None


def candidate(obj: Path, symbol: str) -> dict | None:
    o = CoffObject(obj)
    sym = find_symbol(o, symbol)
    if sym is None:
        raise KeyError(symbol)

    def raw(secnum: int) -> bytes:
        sec = o.section(secnum)
        return o.data[sec.raw_ptr:sec.raw_ptr + sec.raw_size]

    def rel_at(secnum: int, off: int):
        sec = o.section(secnum)
        for r in o.relocations:
            if r.section_number == sec.index and r.virtual_address == off:
                return o.symbol_by_index[r.symbol_index]
        return None

    def target(secnum: int, off: int):
        """(section, offset) a relocated dword at secnum:off points to."""
        s = rel_at(secnum, off)
        if s is None:
            return None
        return s.section_number, s.value + u32(raw(secnum), off)

    text = raw(sym.section_number)
    off = handler_operand_offset(text[sym.value:sym.value + 16])
    if off is None:
        return None
    h = target(sym.section_number, sym.value + off)
    if h is None or raw(h[0])[h[1]] != 0xb8:
        return None
    f = target(h[0], h[1] + 1)
    x = raw(f[0])
    if u32(x, f[1]) != MAGIC:
        return None
    out = dict(funcinfo=rel_at(h[0], h[1] + 1).name, max_state=u32(x, f[1] + 4),
               try_blocks=u32(x, f[1] + 12))
    out['unwind'] = []
    um = target(f[0], f[1] + 8)
    for k in range(out['max_state']):
        data = raw(um[0])
        act = rel_at(um[0], um[1] + 8 * k + 4)
        out['unwind'].append((i32(data, um[1] + 8 * k), act.name if act else 0))
    out['try'] = []
    if out['try_blocks']:
        tb = target(f[0], f[1] + 16)
        data = raw(tb[0])
        for k in range(out['try_blocks']):
            lo, hi, ch, nc, _ = struct.unpack_from('<iiiiI', data, tb[1] + 20 * k)
            out['try'].append(dict(low=lo, high=hi, catch_high=ch, catches=nc))
    return out


def row_obj(row: dict) -> Path:
    return ROOT / 'work/rank_near_misses' / (
        Path(row['source']).with_suffix('').as_posix().replace('/', '__') + '.obj')


def scan(rows: list, pe: PEImage) -> list:
    report = []
    for row in rows:
        r = retail(row['va'], pe)
        obj = row_obj(row)
        try:
            c = candidate(obj, row['symbol']) if obj.exists() else 'no object'
        except KeyError:
            c = 'no symbol'
        if r is None and not isinstance(c, dict):
            continue
        if isinstance(c, str):
            verdict = c
        else:
            verdict = 'same' if shape(r) == shape(c) else 'DIFFERENT'
        report.append(dict(va=hex(row['va']), lane=row.get('lane'), symbol=row['symbol'],
                           diffs=row.get('diffs'), verdict=verdict, retail=shape(r),
                           candidate=shape(c) if isinstance(c, dict) else None))
    return report


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('va', nargs='?')
    ap.add_argument('--obj')
    ap.add_argument('--symbol')
    ap.add_argument('--scan', help='rank_near_misses JSON: compare every EH-bearing row')
    ap.add_argument('--exe', default=str(ROOT / 'work/game/mcm2.exe'))
    a = ap.parse_args(argv)
    pe = PEImage(a.exe)
    if a.scan:
        rep = scan(json.load(open(a.scan)), pe)
        for e in rep:
            print(f"{e['va']} {e['lane']:7} {e['verdict']:10} diffs {e['diffs']} {e['symbol']}")
            if e['verdict'] == 'DIFFERENT':
                print(f"    retail    {e['retail']}\n    candidate {e['candidate']}")
        return 0
    va = int(a.va, 16)
    print('retail   ', retail(va, pe))
    if a.obj:
        print('candidate', candidate(Path(a.obj), a.symbol))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
