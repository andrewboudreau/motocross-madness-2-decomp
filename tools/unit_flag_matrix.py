#!/usr/bin/env python3
"""Score every known function of a unit under extra VC6 flag sets.

usage:
  PYTHONPATH=. python tools/unit_flag_matrix.py SRC [SRC ...] [--flags "/Oa" ...] [--json OUT]
  PYTHONPATH=. python tools/unit_flag_matrix.py --all [--flags "/Oa" ...] [--json OUT]

A per-unit compiler flag is only plausible when it keeps the unit's exact
functions exact.  This tool compiles each source once per flag set (always
plus the unmodified `vc6_o2_mt` baseline) and strictly scores, with
relocations resolved, every target of that source:

- calibration cases (tools/run_calibration.py CASES) naming the source;
- targets.json entries under src/ and samples/ naming it (physics trees are
  compiled with src/krusty2 on the include path, as run_physics_samples does);
- with explicit SRC arguments also every function symbol the source's
  `.bindings.json` binds and the object defines (near misses without a
  targets entry); the extent comes from a targets entry, else from
  tools/rank_near_misses.retail_extent.

A flag set is written space-separated (`--flags "/Oa /Ob1"`); the token
`/O2=/Ox` replaces `/O2` in the profile instead of appending.  Each cell is
`insn diffs/strict bytes` (aligned normalised instructions, as
tools/sweep_variants.py scores), `X` marks strict exact.  With --all only the
per-flag summary is printed: units whose exact functions all stay exact, units
that lose one (the flag is ruled out for that unit's source shape), gains.

Needs VC6_ROOT and MCM2_EXE (Windows-style paths).
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import tempfile
from collections import defaultdict
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))

DEFAULT_SETS = ['/Oa', '/Ow', '/Oa /Ow', '/Op']
PROFILE = 'vc6_o2_mt'
TEXT_START, TEXT_END = 0x00401000, 0x00550000   # retail .text; bindings beyond it name data


def known_targets() -> list[dict]:
    import run_calibration as C
    out = []
    for c in C.CASES:
        out.append({'source': (ROOT / c['source']).resolve(), 'symbol': c['symbol'],
                    'va': int(c['target_va'], 16), 'size': c['target_size'],
                    'bindings': (ROOT / c['bindings']).resolve() if c.get('bindings') else None,
                    'includes': list(c.get('includes', [])), 'expect': 'exact'})
    for tj in sorted(list((ROOT / 'samples').rglob('targets.json')) + list((ROOT / 'src').rglob('targets.json'))):
        try:
            entries = json.loads(tj.read_text(encoding='utf-8'))
        except ValueError:
            continue
        physics = 'physics' in tj.parts or 'krusty2' in tj.parts
        for e in entries if isinstance(entries, list) else []:
            if not isinstance(e, dict) or not e.get('source') or 'target_va' not in e:
                continue
            bindings = (tj.parent / e['bindings']).resolve() if e.get('bindings') else ({} if physics else None)
            out.append({'source': (tj.parent / e['source']).resolve(),
                        'symbol': e.get('candidate_symbol_contains') or e.get('symbol'),
                        'va': int(e['target_va'], 16), 'size': int(e['target_size']), 'bindings': bindings,
                        'includes': [str(ROOT / 'src/krusty2')] if physics else [],
                        'expect': e.get('expect', 'exact')})
    return out


def binding_targets(pe, src: Path, have: set, sizes: dict) -> list[dict]:
    import rank_near_misses as R
    path = src.with_name(src.name[:-4] + '.bindings.json')
    if not path.exists():
        return []
    starts = sorted(R.candidates(ROOT)[0])
    out = []
    for sym, va in json.loads(path.read_text(encoding='utf-8')).items():
        if not (isinstance(va, str) and sym.startswith('?') and '$' not in sym) or '@@3' in sym or '@@6B' in sym:
            continue
        va = int(va, 16)
        if va in have or sym.startswith('??_C@') or not TEXT_START <= va < TEXT_END:
            continue
        try:
            size = sizes.get(va) or R.retail_extent(pe, va, starts)
        except Exception:   # data outside the image's file-backed code
            continue
        out.append({'source': src, 'symbol': sym, 'va': va, 'size': size, 'bindings': path,
                    'includes': [], 'expect': 'bindings', 'optional': True})
    return out


def compile_source(src: Path, includes: list[str], flags: str, obj: Path, vc6_root: str) -> str | None:
    extra = flags.split()
    cmd = [sys.executable, str(ROOT / 'tools/compile.py'), '--compiler', 'vc6', '--vc6-root', vc6_root,
           str(src), '-o', str(obj)]
    profile = PROFILE
    if '/O2=/Ox' in extra:
        extra.remove('/O2=/Ox')
        profiles = json.loads((ROOT / 'config/compile_profiles.json').read_text(encoding='utf-8'))
        profiles['profiles']['ox'] = ['/Ox' if f == '/O2' else f for f in profiles['profiles'][PROFILE]]
        alt = obj.with_suffix('.profiles.json')
        alt.write_text(json.dumps(profiles), encoding='utf-8')
        cmd += ['--profiles', str(alt)]
        profile = 'ox'
    cmd += ['--profile', profile]
    for f in extra:
        cmd += ['--extra', f]
    for i in includes:
        cmd += ['--include', i]
    p = subprocess.run(cmd, capture_output=True, text=True, encoding='latin-1', cwd=ROOT,
                       env=dict(os.environ, PYTHONPATH=str(ROOT), MSYS_NO_PATHCONV='1'))
    if p.returncode or not obj.exists():
        return (p.stdout + p.stderr).strip()[-300:] or 'compile failed'
    return None


def score(pe, obj, t: dict) -> dict:
    import sweep_variants as S
    bindings = None
    if t['bindings'] == {}:
        bindings = {}
    elif t['bindings'] is not None:
        bindings = {k: int(v, 0) if isinstance(v, str) else v
                    for k, v in json.loads(Path(t['bindings']).read_text(encoding='utf-8')).items()}
    r = S.score_target(pe, obj, S.Target(t['va'], t['size'], t['symbol']), bindings)
    return {'exact': bool(r.get('exact')), 'matching': r['matching'], 'insn': r['insn_diffs']}


def run(sources: list[Path], sets: list[str], with_bindings: bool, jobs: int) -> list[dict]:
    from mcm2tool.coff import CoffObject
    from mcm2tool.pe import PEImage
    pe = PEImage(os.environ['MCM2_EXE'])
    every = known_targets()
    sizes = {t['va']: t['size'] for t in every}
    targets: dict[Path, dict] = defaultdict(dict)
    for t in every:
        if t['source'] in sources:
            targets[t['source']].setdefault((t['va'], t['symbol']), t)
    if with_bindings:
        for src in sources:
            for t in binding_targets(pe, src, {va for va, _ in targets[src]}, sizes):
                targets[src].setdefault((t['va'], t['symbol']), t)
    work = []
    tmp = Path(tempfile.mkdtemp(prefix='unitflags'))
    for si, src in enumerate(sources):
        includes = sorted({i for t in targets[src].values() for i in t['includes']})
        for fi, flags in enumerate([''] + sets):
            work.append((src, includes, flags, tmp / f's{si}_f{fi}.obj'))
    vc6_root = os.environ['VC6_ROOT']
    with ThreadPoolExecutor(jobs) as pool:
        errors = list(pool.map(lambda w: compile_source(*w, vc6_root), work))
    rows = []
    for (src, _, flags, obj), err in zip(work, errors):
        coff = None if err else CoffObject(obj)
        defined = set() if err else {s.name for s in coff.symbols if s.section_number > 0}
        for (va, sym), t in sorted(targets[src].items()):
            if t.get('optional') and not err and sym not in defined:
                continue
            row = {'source': src.relative_to(ROOT).as_posix(), 'va': f'0x{va:08x}', 'size': t['size'],
                   'symbol': sym, 'flags': flags or 'base', 'expect': t['expect']}
            if err:
                row['error'] = err
            else:
                try:
                    row.update(score(pe, coff, t))
                except Exception as exc:  # a symbol the flag set removed, an unbindable relocation
                    row['error'] = str(exc)[:200]
            rows.append(row)
    return rows


def print_tables(rows: list[dict], sets: list[str]) -> None:
    names = ['base'] + sets
    for src in dict.fromkeys(r['source'] for r in rows):
        idx = {(r['va'], r['symbol'], r['flags']): r for r in rows if r['source'] == src}
        keys = list(dict.fromkeys((r['va'], r['symbol']) for r in rows if r['source'] == src and r['flags'] == 'base'))
        print(f'== {src}  (cell: insn diffs/strict bytes, X exact)')
        print(f"{'va':10} {'size':>5} " + ' '.join(f'{n[:11]:>11}' for n in names))
        for va, sym in keys:
            cells = []
            for n in names:
                r = idx.get((va, sym, n))
                cells.append('ERR' if r is None or 'error' in r else f"{'X' if r['exact'] else ''}{r['insn']}/{r['matching']}")
            print(f"{va} {idx[(va, sym, 'base')]['size']:>5} " + ' '.join(f'{c:>11}' for c in cells))


def summary(rows: list[dict], sets: list[str]) -> None:
    base = {(r['source'], r['va'], r['symbol']): r for r in rows if r['flags'] == 'base'}
    for flags in sets:
        per = defaultdict(lambda: {'exact': 0, 'lost': 0, 'gained': [], 'better': []})
        for r in rows:
            if r['flags'] != flags:
                continue
            b = base.get((r['source'], r['va'], r['symbol']))
            if not b or 'error' in b:
                continue
            u = per[r['source']]
            ok = 'error' not in r
            if b['exact']:
                u['exact'] += 1
                u['lost'] += not (ok and r['exact'])
            elif ok and r['exact']:
                u['gained'].append(r['va'])
            elif ok and r['insn'] < b['insn']:
                u['better'].append(f"{r['va']} {b['insn']}->{r['insn']}")
        with_exact = [s for s, u in per.items() if u['exact']]
        ruled = [s for s in with_exact if per[s]['lost']]
        print(f'== {flags}: {len(per)} units, {len(with_exact)} with exact functions, '
              f'{len(ruled)} lose one (ruled out), {len(with_exact) - len(ruled)} keep all; '
              f"gains {sum(len(u['gained']) for u in per.values())}")
        for s, u in per.items():
            if u['gained'] or u['better']:
                print(f"  {s}: exact {u['exact']} lost {u['lost']} gained {u['gained']} better {u['better'][:6]}")


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('sources', nargs='*')
    ap.add_argument('--all', action='store_true', help='every source with a calibration case or targets.json entry')
    ap.add_argument('--flags', action='append', help=f'flag set (repeatable; default {DEFAULT_SETS})')
    ap.add_argument('--jobs', type=int, default=8)
    ap.add_argument('--json')
    a = ap.parse_args(argv)
    sets = a.flags or DEFAULT_SETS
    if a.all:
        sources = sorted({t['source'] for t in known_targets() if t['source'].is_file()})
    else:
        sources = [Path(s).resolve() for s in a.sources]
    if not sources:
        ap.error('name a source or pass --all')
    rows = run(sources, sets, not a.all, a.jobs)
    if a.json:
        Path(a.json).write_text(json.dumps(rows, indent=1), encoding='utf-8')
    if not a.all:
        print_tables(rows, sets)
    summary(rows, sets)
    return 0


if __name__ == '__main__':
    sys.exit(main())
