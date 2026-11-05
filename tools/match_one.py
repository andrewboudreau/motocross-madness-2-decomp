#!/usr/bin/env python3
"""Compile one source with VC6 and strictly compare one function with retail.

usage: PYTHONPATH=. python tools/match_one.py SRC VA SIZE [SYMBOL_SUBSTRING] [--bnd BINDINGS] [-q]

Needs VC6_ROOT and MCM2_EXE in the environment (Windows-style paths).  Bindings
default to SRC with a .bindings.json suffix.  Prints the strict byte score, the
normalised instruction alignment and a short instruction diff; with relocations
resolved it also lists the exact byte offsets that differ (offset, candidate,
retail), so a two-byte near miss shows which operand it is.
"""
import json
import os
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))

from tools import rank_near_misses as R  # noqa: E402
from tools import sweep_variants as S  # noqa: E402
from mcm2tool.coff import CoffObject  # noqa: E402
from mcm2tool.pe import PEImage  # noqa: E402
from mcm2tool.resolved_match import match_object  # noqa: E402


def main(argv):
    args = [a for a in argv if not a.startswith('-')]
    bnd = argv[argv.index('--bnd') + 1] if '--bnd' in argv else None
    if bnd:
        args.remove(bnd)
    if len(args) < 3:
        print(__doc__)
        return 2
    src, va, size = args[0], int(args[1], 16), int(args[2], 0)
    sub = args[3] if len(args) > 3 else None
    if bnd is None:
        p = Path(src).with_suffix('.bindings.json')
        bnd = str(p) if p.exists() else None
    obj = Path(tempfile.gettempdir()) / f'match_one_{os.getpid()}.obj'
    obj.unlink(missing_ok=True)
    err = R.compile_one(ROOT, os.environ['VC6_ROOT'], Path(src).resolve(), obj)
    if err:
        print('COMPILE', err)
        return 1
    o = CoffObject(obj)
    pe = PEImage(os.environ['MCM2_EXE'])
    b = None
    if bnd:
        b = {k: int(v, 0) if isinstance(v, str) else v for k, v in json.load(open(bnd)).items()}
    if sub is None:
        sub = S.symbol_for_va(b, va)
    names = [s.name for s in o.symbols if s.section_number > 0 and s.type == 0x20]
    sym = ([n for n in names if n == sub] or [n for n in names if sub in n])[0]
    row = S.score_target(pe, o, S.Target(va, size, sym), b, want_diff='-q' not in argv)
    print(sym, row.get('mode'), row['matching'], '/', row.get('compared'),
          'EXACT' if row.get('exact') else '', 'aligned', row['aligned'], '/', row['retail_insns'],
          'cand', row['candidate_size'], 'insn_diffs', row['insn_diffs'], row.get('strict_error', ''))
    if b is not None and not row.get('strict_error'):
        retail = pe.bytes_at_va(va, size)
        st = match_object(o, sym, va, retail, b)
        cand, _, _ = o.symbol_extent(o.find_symbol(sym))
        patched = bytearray(cand)
        for r in st['relocations_applied']:
            patched[r['offset']:r['offset'] + 4] = r['written_u32'].to_bytes(4, 'little')
        diffs = [(hex(i), patched[i], retail[i]) for i in range(min(len(retail), len(patched)))
                 if patched[i] != retail[i]]
        print('byte diffs (offset, candidate, retail):', diffs[:40], '...' if len(diffs) > 40 else '')
    if '-q' not in argv:
        print('\n'.join(row['diff']))
    obj.unlink(missing_ok=True)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
