#!/usr/bin/env python3
"""Compile and match every physics reconstruction under samples/physics/.

Each subdirectory may hold a ``targets.json`` list. Every entry needs
``source`` (path relative to that directory), ``candidate_symbol_contains``,
``target_va`` and ``target_size``. Sources are compiled once each with the
chosen compiler profile; each target is matched with tools/match.py.

Exit status is nonzero when any entry with ``"expect": "exact"`` (the default)
fails to match. Entries marked ``"expect": "partial"`` are reported but do not
fail the run; use that for readable reconstructions whose codegen is still off.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--exe', default=str(ROOT / 'work/game/mcm2.exe'))
    ap.add_argument('--compiler', choices=['clang-cl', 'vc6'], default='vc6')
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--profile', default='vc6_o2_ml',
                    help='vc6_o2_ml (no /G6) matched 139/194 physics targets vs 76/194 with /G6, with no target exact only under /G6')
    ap.add_argument('--root', default=str(ROOT / 'samples/physics'), help='directory to scan')
    ap.add_argument('--out', default=str(ROOT / 'work/physics-objs'))
    ap.add_argument('--json', action='store_true', help='print full per-target JSON')
    a = ap.parse_args()
    if a.compiler == 'vc6' and not a.vc6_root:
        raise SystemExit('set VC6_ROOT or pass --vc6-root')
    os.environ['PYTHONPATH'] = str(ROOT) + os.pathsep + os.environ.get('PYTHONPATH', '')
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    results = []
    compiled: dict[Path, Path | None] = {}
    for tjson in sorted(Path(a.root).rglob('targets.json')):
        for t in json.loads(tjson.read_text()):
            if 'source' not in t:
                continue
            src = (tjson.parent / t['source']).resolve()
            if src not in compiled:
                rel = src.relative_to(ROOT)
                obj = out / (str(rel.with_suffix('')).replace('\\', '_').replace('/', '_') + '.obj')
                cmd = [sys.executable, str(ROOT / 'tools/compile.py'), str(src), '-o', str(obj),
                       '--compiler', a.compiler, '--profile', a.profile]
                if a.compiler == 'vc6':
                    cmd += ['--vc6-root', a.vc6_root]
                r = subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=ROOT)
                compiled[src] = obj if r.returncode == 0 and obj.exists() else None
                if compiled[src] is None:
                    print(f'COMPILE FAIL {rel}\n{r.stdout}', file=sys.stderr)
            obj = compiled[src]
            expect = t.get('expect', 'exact')
            if obj is None:
                payload = {'error': 'compile failed', 'exact_after_relocation_mask': False}
            else:
                m = [sys.executable, str(ROOT / 'tools/match.py'), '--exe', a.exe, '--target-va', t['target_va'],
                     '--target-size', str(t['target_size']), '--obj', str(obj),
                     '--symbol', t['candidate_symbol_contains'], '--json']
                r = subprocess.run(m, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=ROOT)
                try:
                    payload = json.loads(r.stdout)
                except ValueError:
                    payload = {'error': r.stdout.strip()[-400:], 'exact_after_relocation_mask': False}
            payload['source'] = str(src.relative_to(ROOT))
            payload['expected'] = t
            results.append(payload)
    if a.json:
        print(json.dumps(results, indent=2))
    exact = [r for r in results if r.get('exact_after_relocation_mask')]
    required = [r for r in results if r['expected'].get('expect', 'exact') == 'exact']
    failed = [r for r in required if not r.get('exact_after_relocation_mask')]
    for r in results:
        e = r['expected']
        tag = 'EXACT' if r.get('exact_after_relocation_mask') else ('FAIL ' if r in failed else 'part ')
        pct = r.get('match_percent', 0)
        sizes = f"{r.get('candidate_size', '?')}/{e['target_size']}"
        print(f"{tag} {e['target_va']} {pct:6.2f}% size {sizes:>9}  {e['candidate_symbol_contains']}  [{r['source']}]"
              + (f"  ERR {r['error'][:120]}" if 'error' in r else ''))
    print(f'\n{len(exact)}/{len(results)} exact; {len(failed)} required failures')
    return 1 if failed else 0


if __name__ == '__main__':
    raise SystemExit(main())
