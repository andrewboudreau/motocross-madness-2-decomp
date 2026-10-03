#!/usr/bin/env python3
"""Compile and match every physics reconstruction under samples/physics/.

Promoted translation units live under src/krusty2/ (retail file names, backed by
__FILE__ and link-order evidence); experimental reconstructions stay under
samples/physics/. Both trees are scanned by default, and src/krusty2 is on the
include path so shared headers are included as e.g. "core/GameObject.h".

Each subdirectory may hold a ``targets.json`` list. Every entry needs
``source`` (path relative to that directory), ``candidate_symbol_contains``,
``target_va`` and ``target_size``. Sources are compiled once each with the
chosen compiler profile; each target is matched with tools/match.py.

Exit status is nonzero when any entry with ``"expect": "exact"`` (the default)
fails the relocation-masked diagnostic comparison. This is not strict proof.
Use --strict for merge acceptance: every required target must match after all
relocations are resolved. An optional ``bindings`` path is relative to targets.json.
Entries marked ``"expect": "partial"`` are reported but do not fail the run.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.resolved_match import RelocationError, match_object
from tools.match import compare


def compare_target(pe, obj, target, manifest):
    bindings_path = manifest.parent / target['bindings'] if target.get('bindings') else None
    result = compare(pe, obj, target['candidate_symbol_contains'],
                     int(target['target_va'], 0), target['target_size'], bindings_path)
    if bindings_path is None:
        try:
            strict = match_object(obj, target['candidate_symbol_contains'],
                                  int(target['target_va'], 0),
                                  pe.bytes_at_va(int(target['target_va'], 0), target['target_size']), {})
        except RelocationError as exc:
            strict = {'strict_exact': False, 'ignored_bytes': 0, 'error': str(exc)}
        result.update(strict_exact=strict['strict_exact'], strict_match=strict)
    return result


def required_failures(results, strict=False):
    key = 'strict_exact' if strict else 'exact_after_relocation_mask'
    return [r for r in results if r['expected'].get('expect', 'exact') in ('exact', 'masked')
            and r.get(key) is not True]


def validation_status(result):
    """Missing address evidence is different from a completed byte mismatch."""
    if result.get('strict_exact') is True:
        return 'strict-exact'
    if result.get('error') == 'compile failed':
        return 'compile-failed'
    error = result.get('strict_match', {}).get('error', result.get('error', ''))
    if error.startswith('unresolved symbol:'):
        return 'unresolved-relocation'
    if error:
        return 'validation-error'
    return 'byte-mismatch'


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--exe', default=str(ROOT / 'work/game/mcm2.exe'))
    ap.add_argument('--compiler', choices=['clang-cl', 'vc6'], default='vc6')
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--profile', default='vc6_o2_ml',
                    help='vc6_o2_ml (no /G6) matched 139/194 physics targets vs 76/194 with /G6, with no target exact only under /G6')
    ap.add_argument('--root', action='append', default=None,
                    help='directory to scan (repeatable; default: samples/physics and src/krusty2)')
    ap.add_argument('--include', action='append', default=None,
                    help='project include directory (repeatable; default: src/krusty2)')
    ap.add_argument('--source', action='append', type=Path,
                    help='check only this source file (repeatable; relative to repository root)')
    ap.add_argument('--out', default=str(ROOT / 'work/physics-objs'))
    ap.add_argument('--json', action='store_true', help='print full per-target JSON')
    ap.add_argument('--json-out', type=Path, help='write the complete per-target report')
    ap.add_argument('--strict', action='store_true', help='require resolved relocations for every required match')
    a = ap.parse_args()
    roots = a.root or [str(ROOT / 'samples/physics'), str(ROOT / 'src/krusty2')]
    include_dirs = a.include or [str(ROOT / 'src/krusty2')]
    if a.compiler == 'vc6' and not a.vc6_root:
        raise SystemExit('set VC6_ROOT or pass --vc6-root')
    os.environ['PYTHONPATH'] = str(ROOT) + os.pathsep + os.environ.get('PYTHONPATH', '')
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    results = []
    pe = PEImage(a.exe)
    objects = {}
    compiled: dict[Path, Path | None] = {}
    tjsons = sorted({t for r in roots if Path(r).is_dir() for t in Path(r).rglob('targets.json')})
    selected_sources = {(ROOT / p).resolve() for p in a.source or []}
    seen_sources = set()
    for tjson in tjsons:
        for t in json.loads(tjson.read_text()):
            if 'source' not in t:
                continue
            src = (tjson.parent / t['source']).resolve()
            if selected_sources and src not in selected_sources:
                continue
            seen_sources.add(src)
            if t.get('expect', 'exact') not in ('exact', 'masked', 'partial'):
                raise SystemExit(f'unknown expectation in {tjson}: {t.get("expect")}')
            if src not in compiled:
                rel = src.relative_to(ROOT)
                obj = out / (str(rel.with_suffix('')).replace('\\', '_').replace('/', '_') + '.obj')
                cmd = [sys.executable, str(ROOT / 'tools/compile.py'), str(src), '-o', str(obj),
                       '--compiler', a.compiler, '--profile', a.profile]
                cmd += [f'--include={d}' for d in include_dirs]
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
                try:
                    if obj not in objects:
                        objects[obj] = CoffObject(obj)
                    payload = compare_target(pe, objects[obj], t, tjson)
                except (ValueError, OSError) as exc:
                    payload = {'error': str(exc), 'exact_after_relocation_mask': False, 'strict_exact': False}
            payload['source'] = str(src.relative_to(ROOT))
            payload['expected'] = t
            payload['validation_status'] = validation_status(payload)
            results.append(payload)
    if not results or selected_sources - seen_sources:
        raise SystemExit('no targets found for one or more requested sources/roots')
    if a.json:
        print(json.dumps(results, indent=2))
    if a.json_out:
        a.json_out.parent.mkdir(parents=True, exist_ok=True)
        a.json_out.write_text(json.dumps(results, indent=2) + '\n')
    exact = [r for r in results if r.get('exact_after_relocation_mask')]
    strict_exact = [r for r in results if r.get('strict_exact') is True]
    failed = required_failures(results, a.strict)
    for r in results:
        e = r['expected']
        tag = 'EXACT' if r.get('strict_exact') else ('MASK ' if r.get('exact_after_relocation_mask') else 'part ')
        if r in failed:
            tag = 'FAIL '
        pct = r.get('match_percent', 0)
        sizes = f"{r.get('candidate_size', '?')}/{e['target_size']}"
        print(f"{tag} {e['target_va']} {pct:6.2f}% size {sizes:>9}  {e['candidate_symbol_contains']}  [{r['source']}]"
              + (f"  ERR {r['error'][:120]}" if 'error' in r else ''))
    mode = 'strict' if a.strict else 'diagnostic'
    print(f'\n{len(exact)}/{len(results)} relocation-masked matches; '
          f'{len(strict_exact)}/{len(results)} strict exact; {len(failed)} required failures ({mode})')
    unresolved = sum(r['validation_status'] == 'unresolved-relocation' for r in failed)
    if unresolved:
        print(f'{unresolved} required targets lack relocation bindings; '
              'this is incomplete evidence, not a completed strict byte comparison.')
    return 1 if failed else 0


if __name__ == '__main__':
    raise SystemExit(main())
