#!/usr/bin/env python3
"""Scan a VC6 candidate's sensitivity to the number of declarations before it.

VC6 SP3 schedules two equal-priority independent loads (and so assigns their
registers) by a tie-break that depends on where the function's nodes fall in
the compiler's symbol arena (docs/VC6_OPERAND_ORDER.md). Prepending unrelated
declarations to the translation unit moves that position: every function
declaration with one parameter advances it by one unit, every typedef, extern
variable, forward struct declaration or parameter-less prototype by half a
unit, and the pattern repeats every 32 units (64 typedefs). A sensitive
function therefore changes at some prefix sizes and stays put at others.

This tool compiles SRC with k = 0..KMAX prepended typedef declarations (or
one-parameter prototypes with --kind fn) and reports, for every bound function
in SRC, whether it is strict exact at each k. It answers two questions:

1. Is a near miss a scheduling tie-break (some k makes it exact) or a source
   form difference (no k changes it)?
2. Which registered exact functions of a file would a header edit put at risk
   (their row is not all X)?

Usage:
  python3 tools/decl_shift_scan.py SRC BINDINGS [--include DIR]... [--fn 0xVA:SYMBOL-SUBSTRING]...
         [--va 0xVA]... [--kmax 63] [--kind td|fn] [--exe mcm2.exe] [--json OUT]

Without --fn/--va every function symbol of SRC that BINDINGS binds is scanned;
--fn adds a function whose VA the bindings do not hold (the symbol substring
is resolved against the object). The compile goes through tools/compile.py
with the calibrated vc6 profile; the prefix, summary and window parsing need
no VC6 (tests/test_decl_shift_scan.py).
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

UNITS_PER_PERIOD = 32          # measured period of the tie-break pattern, in units
UNIT_WEIGHT = {'td': 0.5, 'fn': 1.0}   # units per prepended declaration of each kind


def prefix_declarations(k: int, kind: str = 'td') -> str:
    """The k declarations prepended to the source for scan position k."""
    if kind == 'td':
        return ''.join(f'typedef int DeclShiftProbe{i};\n' for i in range(k))
    if kind == 'fn':
        return ''.join(f'int DeclShiftProbe{i}(int);\n' for i in range(k))
    raise ValueError(f'unknown declaration kind {kind!r}')


def units(k: int, kind: str = 'td') -> float:
    """Arena units the prefix of k declarations of `kind` occupies."""
    return k * UNIT_WEIGHT[kind]


def windows(flags: list[bool]) -> list[tuple[int, int]]:
    """Maximal runs of consecutive true positions as (first, last) pairs."""
    out: list[tuple[int, int]] = []
    start = None
    for i, f in enumerate(flags):
        if f and start is None:
            start = i
        elif not f and start is not None:
            out.append((start, i - 1))
            start = None
    if start is not None:
        out.append((start, len(flags) - 1))
    return out


def summarize(scores: dict[int, tuple[int, int, bool]]) -> dict:
    """Summary of one function's scan: scores[k] = (matching, compared, strict_exact)."""
    ks = sorted(scores)
    exact = [k for k in ks if scores[k][2]]
    distinct = sorted({scores[k][0] for k in ks})
    base = scores.get(0)
    flags = [scores[k][2] for k in ks]
    return {
        'k0': list(base[:2]) if base else None,
        'distinct_scores': distinct,
        'exact_k': exact,
        'exact_windows': windows(flags) if ks == list(range(len(ks))) else None,
        'sensitive': len(distinct) > 1,
        'verdict': ('exact at every k' if len(exact) == len(ks) and ks else
                    'never exact' if not exact else
                    'exact only at some k (scheduling tie-break)'),
    }


def is_function_name(name: str) -> bool:
    """A mangled or C function symbol, not a string literal, vtable or variable."""
    if not (name.startswith('?') or name.startswith('_')) or '$' in name[1:]:
        return False
    if name.startswith('??_C@') or name.startswith('??_7') or name.startswith('??_R'):
        return False
    return not (name.startswith('?') and '@@3' in name)


def function_symbols(obj) -> list[str]:
    return [s.name for s in obj.symbols
            if s.section_number > 0 and s.storage_class == 2 and is_function_name(s.name)]


def resolve_symbol(obj, substring: str) -> str | None:
    cands = [n for n in function_symbols(obj) if substring in n]
    exact = [c for c in cands if c == substring]
    return (exact or cands or [None])[0]


def compile_prefixed(src: Path, k: int, kind: str, includes: list[str], workdir: Path,
                     vc6_root: str, repo: Path) -> Path | None:
    workdir.mkdir(parents=True, exist_ok=True)
    probe = workdir / f'{src.stem}_shift{k}{src.suffix}'
    probe.write_text(prefix_declarations(k, kind) + src.read_text())
    obj = probe.with_suffix('.obj')
    cmd = [sys.executable, str(repo / 'tools' / 'compile.py'), '--compiler', 'vc6',
           '--vc6-root', vc6_root, '--include', str(src.parent)]
    for d in includes:
        cmd += ['--include', d]
    cmd += [str(probe), '-o', str(obj)]
    r = subprocess.run(cmd, capture_output=True, text=True, cwd=repo)
    if r.returncode:
        sys.stderr.write(r.stdout[-2000:] + r.stderr[-2000:])
        return None
    return obj


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('src')
    ap.add_argument('bindings')
    ap.add_argument('--include', action='append', default=[])
    ap.add_argument('--fn', action='append', default=[], help='0xVA:symbol-substring to scan as well')
    ap.add_argument('--va', action='append', default=[], help='restrict the bound functions to these VAs')
    ap.add_argument('--kmax', type=int, default=63)
    ap.add_argument('--kind', choices=sorted(UNIT_WEIGHT), default='td')
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE'))
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--workdir', default=None, help='scratch directory for the prefixed sources (default: work/decl_shift)')
    ap.add_argument('--json', default=None, help='write the per-function summaries here')
    a = ap.parse_args()

    from mcm2tool.coff import CoffObject
    from mcm2tool.pe import PEImage
    from mcm2tool.resolved_match import match_object

    if not a.exe or not a.vc6_root:
        ap.error('--exe and --vc6-root (or MCM2_EXE / VC6_ROOT) are required')
    repo = Path(__file__).resolve().parent.parent
    src = Path(a.src).resolve()
    workdir = Path(a.workdir) if a.workdir else repo / 'work' / 'decl_shift'
    pe = PEImage(a.exe)
    bindings = {k: int(v, 16) for k, v in json.load(open(a.bindings)).items()}
    rev: dict[int, str] = {}
    for name, va in bindings.items():
        if name.startswith('?') and '$' not in name[1:]:
            rev.setdefault(va, name)
    vas = [int(x, 16) for x in a.va] if a.va else sorted(rev)
    extra: dict[int, str] = {}
    for item in a.fn:
        va_text, sub = item.split(':', 1)
        va = int(va_text, 16)
        extra[va] = sub
        if va not in vas:
            vas.append(va)

    targets: dict[int, str] = {}
    scores: dict[int, dict[int, tuple[int, int, bool]]] = {}
    for k in range(0, a.kmax + 1):
        obj_path = compile_prefixed(src, k, a.kind, a.include, workdir, a.vc6_root, repo)
        if obj_path is None:
            print(f'k={k:2d} compile failed')
            continue
        obj = CoffObject(str(obj_path))
        if not targets:
            names = set(function_symbols(obj))
            for va in vas:
                name = rev.get(va)
                if va in extra:
                    name = resolve_symbol(obj, extra[va])
                if name and name in names:
                    targets[va] = name
            vas = [va for va in vas if va in targets]
            if not targets:
                print('no bound function symbol of the source is in the object')
                return 1
            print('functions: ' + ' '.join(f'{i}:{va:#x}' for i, va in enumerate(vas)))
        row = []
        for va in vas:
            name = targets[va]
            try:
                raw, _, _ = obj.symbol_extent(obj.find_symbol(name))
                res = match_object(obj, name, va, pe.bytes_at_va(va, len(raw)),
                                   dict(bindings, **{name: va}))
                scores.setdefault(va, {})[k] = (res['matching_positions'], res['compared_positions'],
                                                bool(res['strict_exact']))
                row.append('X' if res['strict_exact'] else '.')
            except Exception as ex:  # unresolved relocation, missing symbol
                row.append('E')
                scores.setdefault(va, {})
        print(f'k={k:2d} units={units(k, a.kind):5.1f} ' + ''.join(row), flush=True)

    summaries = {}
    for va in vas:
        s = summarize(scores.get(va, {}))
        summaries[f'{va:#x}'] = dict(s, symbol=targets[va])
        print(f'{va:#x} {targets[va][:60]}: {s["verdict"]}; scores {s["distinct_scores"]}'
              + (f'; exact windows {s["exact_windows"]}' if s['exact_k'] and len(s['exact_k']) < len(scores.get(va, {})) else ''))
    if a.json:
        Path(a.json).write_text(json.dumps(summaries, indent=2) + '\n')
    return 0


if __name__ == '__main__':
    sys.exit(main())
