#!/usr/bin/env python3
"""Run tools/unit_layout.py over every src/ unit whose registered functions are all strict exact.

For each ``src/**/*.cpp`` (``src/bringup`` excluded) the sweep compiles the
file with the game-build profile and checks every calibration case and
``targets.json`` entry registered for it with the strict (relocation-resolved)
comparison. Units with at least one registered function and no failing one are
laid out with ``--start auto --text-x-start auto --xdata-start auto``.

Each laid-out unit gets a verdict (``identical`` when every placed group is
strict identical and every bound function sits at its bound VA) and a list of
findings, one per ``.text`` section that keeps the unit from being identical:

  static_deferred   a ``static`` function whose retail address precedes a
                    function our object emits before it (VC6 emits a static
                    after its first caller)
  order             an extern function whose retail address is out of the
                    object's section order (source order)
  gap_missing       retail has code between two of our consecutive functions:
                    a function missing from this source (owners named when the
                    repository registers them; ``other_src`` names src files)
  gap_extra         our object emits more bytes between two bound functions
                    than retail has there (an extra or unregistered function)
  unregistered      a function without a retail address
  pick_any_elsewhere  a pick-any COMDAT bound outside the unit (an earlier
                    object kept it)
  icf_elsewhere     a non-pick-any function bound outside the unit (folded by
                    /OPT:ICF or misbound)
  initializer_order a dynamic initializer ``_$E<n>`` (emitted where its object
                    is defined) out of retail order
  overlap           two bound functions closer in retail than our sizes allow
                    (misbinding)
  icf_duplicate     bound to the same VA as an earlier function of the object:
                    folded by /OPT:ICF, one copy kept
  bytes             at its bound VA but the laid-out bytes differ (EH/fill
                    groups differ, or a relocation resolves differently; with
                    ``after_placement_findings`` possibly only a consequence)

The diagnosis reads bound addresses, not the auto-anchored layout, so one
misplaced function does not cascade. Output (ignored): ``work/unit-layout-sweep/
report.json`` and ``report.md``.

    python tools/unit_layout_sweep.py --vc6-root "$VC6_ROOT" --exe "$MCM2_EXE"
    python tools/unit_layout_sweep.py --source src/reconstructed/AgeManager.cpp
"""
from __future__ import annotations

import argparse
import json
import os
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))

from mcm2tool.coff import CoffError, CoffObject  # noqa: E402
from mcm2tool.resolved_match import RelocationError, match_object  # noqa: E402
import unit_layout  # noqa: E402
from unit_layout import COMDAT_PICK_ANY, aligned_size, hx  # noqa: E402

DEFAULT_OUT = ROOT / 'work' / 'unit-layout-sweep'
STATIC = 3


def rel(path: Path) -> str:
    return path.resolve().relative_to(ROOT).as_posix()


def registered_rows(root: Path = ROOT) -> dict[str, list[dict]]:
    """src source -> [{symbol, va, size, bindings, origin}] from calibration
    cases and src targets.json entries."""
    from tools.rebuild_coverage import load_calibration_cases, load_targets
    rows: dict[str, list[dict]] = {}
    for case in load_calibration_cases(root):
        rows.setdefault(case['source'], []).append({
            'symbol': case['symbol'], 'va': int(case['target_va'], 16), 'size': case['target_size'],
            'bindings': case.get('bindings'), 'origin': 'calibration'})
    for manifest in sorted((root / 'src').rglob('*targets.json')):
        data = json.loads(manifest.read_text(encoding='utf-8'))
        for entry in (data.get('targets', []) if isinstance(data, dict) else data):
            if 'target_va' not in entry or 'source' not in entry:
                continue
            source = os.path.normpath(manifest.parent.relative_to(root) / entry['source']).replace(os.sep, '/')
            bindings = entry.get('bindings')
            rows.setdefault(source, []).append({
                'symbol': entry['candidate_symbol_contains'], 'va': int(str(entry['target_va']), 0),
                'size': int(entry['target_size']),
                'bindings': (manifest.parent / bindings).relative_to(root).as_posix() if bindings else None,
                'origin': manifest.relative_to(root).as_posix(), 'expect': entry.get('expect', 'exact')})
    return rows


def retail_owners(root: Path = ROOT) -> list[tuple[int, int, str, str]]:
    """(va, size, source, symbol) of every registered function in src/ and samples/."""
    from tools.rebuild_coverage import load_calibration_cases, load_targets
    out = {(int(c['target_va'], 16), c['source']): (c['target_size'], c['symbol']) for c in load_calibration_cases(root)}
    for t in load_targets(root):
        out.setdefault((t['va'], t['source']), (t['size'], t['symbol']))
    return sorted((va, size, source, symbol) for (va, source), (size, symbol) in out.items())


def strict_verdict(pe, obj: CoffObject, row: dict) -> tuple[bool, str]:
    try:
        sym = obj.find_symbol(row['symbol'])
    except CoffError as exc:
        return False, str(exc)
    bindings = {}
    if row.get('bindings'):
        values = json.loads((ROOT / row['bindings']).read_text())
        bindings = {k: int(v, 0) if isinstance(v, str) else int(v) for k, v in values.items()}
    try:
        result = match_object(obj, sym.name, row['va'], pe.bytes_at_va(row['va'], row['size']), bindings)
    except RelocationError as exc:
        return False, str(exc)
    if result['strict_exact']:
        return True, ''
    return False, 'byte mismatch'


def function_symbols(obj: CoffObject) -> dict[str, int]:
    """Function symbol -> storage class (2 extern, 3 static)."""
    return {s.name: s.storage_class for s in obj.symbols
            if s.section_number > 0 and s.type == 0x20 and s.storage_class in (2, 3)}


def increasing_subsequence(values: list[int], weights: list[int] | None = None) -> set[int]:
    """Indices of one heaviest strictly increasing subsequence (weight 1 each by default)."""
    weights = weights or [1] * len(values)
    best: list[tuple[int, list[int]]] = []
    for i, v in enumerate(values):
        prior = [b for j, b in enumerate(best) if values[j] < v]
        weight, chain = max(prior, key=lambda b: b[0], default=(0, []))
        best.append((weight + weights[i], chain + [i]))
    return set(max(best, key=lambda b: b[0], default=(0, []))[1])


def diagnose(layout: unit_layout.Layout, report: dict, obj: CoffObject,
             owners: list[tuple[int, int, str, str]], source: str) -> list[dict]:
    """Findings for the .text sections that keep the unit from laying out exactly."""
    storage = function_symbols(obj)
    findings = []
    text = layout.group('.text')
    for sec in text:
        if sec.status == 'elsewhere':
            kind = 'pick_any_elsewhere' if sec.selection == COMDAT_PICK_ANY else 'icf_elsewhere'
            findings.append({'kind': kind, 'functions': [f[0] for f in sec.functions],
                             'bound_va': hx(layout.bound_va(sec)), 'detail': sec.reason})
        elif sec.status == 'folded':
            findings.append({'kind': 'icf_duplicate', 'functions': [f[0] for f in sec.functions],
                             'bound_va': hx(layout.bound_va(sec)), 'detail': sec.reason})
    placed = [s for s in text if s.status == 'placed']
    position = {id(s): i for i, s in enumerate(placed)}
    bound = [(s, layout.bound_va(s)) for s in placed]
    for s, va in bound:
        if va is None and s.functions:
            findings.append({'kind': 'unregistered', 'functions': [f[0] for f in s.functions],
                             'static': any(storage.get(f[0]) == STATIC for f in s.functions)})
    known = [(s, va) for s, va in bound if va is not None]
    # Order: one heaviest increasing run of bound VAs in section order; an
    # extern weighs more than a static, so a static (VC6 emits it after its
    # first caller, a dynamic initializer _$E<n> where its object is defined)
    # takes the blame when either could.
    is_static = lambda s: any(storage.get(f[0]) == STATIC for f in s.functions)
    keep = increasing_subsequence([va for _, va in known], [1 if is_static(s) else 2 for s, _ in known])
    in_order = [(s, va) for i, (s, va) in enumerate(known) if i in keep]
    for i, (s, va) in enumerate(known):
        if i in keep:
            continue
        name = s.functions[0][0]
        early = [x for x, v in in_order if position[id(x)] < position[id(s)] and v > va]
        late = [x for x, v in in_order if position[id(x)] > position[id(s)] and v < va]
        kind = ('initializer_order' if name.startswith('_$E') else
                'static_deferred' if is_static(s) and early else 'order')
        finding = {'kind': kind, 'functions': [f[0] for f in s.functions], 'bound_va': hx(va)}
        if early:
            finding['emitted_after'] = early[0].functions[0][0]
        elif late:
            finding['emitted_before'] = late[-1].functions[0][0]
        findings.append(finding)
    # Gaps between functions that are neighbours in retail order.
    chain = sorted(known, key=lambda p: p[1])
    for (a, va_a), (b, va_b) in zip(chain, chain[1:]):
        expected = va_a + aligned_size(a.size, a.align)
        between = []
        if position[id(b)] > position[id(a)]:
            middle = placed[position[id(a)] + 1:position[id(b)]]
            if all(layout.bound_va(x) is None for x in middle):
                between = middle
                expected += sum(aligned_size(x.size, x.align) for x in middle)
        if va_b > expected:
            gap = [{'va': hx(v), 'size': n, 'source': src, 'symbol': sym}
                   for v, n, src, sym in owners if expected <= v < va_b and src != source]
            other = sorted({g['source'] for g in gap if g['source'].startswith('src/')})
            findings.append({'kind': 'gap_missing', 'after': a.functions[0][0], 'before': b.functions[0][0],
                             'range': f'{hx(expected)}..{hx(va_b)}', 'bytes': va_b - expected,
                             'unbound_between': [x.functions[0][0] for x in between],
                             'retail_owners': gap, 'other_src': other})
        elif va_b < expected:
            findings.append({'kind': 'gap_extra' if between else 'overlap',
                             'after': a.functions[0][0], 'before': b.functions[0][0],
                             'bytes': expected - va_b,
                             'unbound_between': [x.functions[0][0] for x in between]})
    # Byte differences of functions at their bound VA. With any placement
    # finding they may only follow from it (a relocation to a moved function).
    placement = any(f['kind'] not in ('pick_any_elsewhere', 'icf_elsewhere', 'icf_duplicate') for f in findings)
    flagged = {f for item in findings for f in item.get('functions', [])}
    for row in report['sections']:
        if row['status'] != 'placed':
            continue
        for f in row['functions']:
            if f['offset_agrees'] and not f['bytes_equal'] and f['symbol'] not in flagged:
                findings.append({'kind': 'bytes', 'functions': [f['symbol']], 'section': row['name'],
                                 'after_placement_findings': placement,
                                 'first_differences': row['first_differences'][:4]})
    return findings


def retail_order(layout: unit_layout.Layout) -> list[str]:
    """Bound .text functions of the object sorted by retail address."""
    rows = [(layout.bound_va(s), s.functions[0][0]) for s in layout.group('.text')
            if s.status == 'placed' and s.functions and layout.bound_va(s) is not None]
    return [name for _, name in sorted(rows)]


def unit_verdict(report: dict, findings: list[dict]) -> str:
    groups = report['groups']
    strict = groups and all(g['verdict'] == 'strict_identical' for g in groups.values())
    if strict and not report['functions_offset_disagree']:
        return 'identical' if not any(f['kind'] == 'unregistered' for f in findings) else 'identical_with_unbound'
    if all(g['differing_bytes'] == 0 for g in groups.values()) and not report['functions_offset_disagree']:
        return 'masked_identical'
    return 'differs'


def lay_out(obj: CoffObject, bindings: dict[str, int], fallback: dict[str, int], retail_bytes):
    """The --start/--text-x-start/--xdata-start auto layout and its comparison;
    None when no bound function anchors .text."""
    layout = unit_layout.Layout(obj, bindings, fallback)
    start = layout.auto_text_start()
    if start is None:
        return None
    layout.place('.text', start)
    for name, auto in (('.text$x', layout.auto_text_x_start),
                       ('.xdata$x', lambda: layout.auto_xdata_start(retail_bytes))):
        value = auto()
        if value is not None:
            layout.place(name, value)
    layout.unplaced_groups()
    return layout, unit_layout.compare_layout(layout, retail_bytes)


def analyse(source: Path, obj_path: Path, pe, owners, rows: list[dict], fallback: dict[str, int]) -> dict:
    obj = CoffObject(obj_path)
    checks = []
    for row in rows:
        ok, why = strict_verdict(pe, obj, row)
        checks.append({'symbol': row['symbol'], 'va': hx(row['va']), 'origin': row['origin'],
                       'strict_exact': ok, 'why': why})
    entry = {'source': rel(source), 'registered': len(rows),
             'strict': sum(c['strict_exact'] for c in checks)}
    failing = [c for c in checks if not c['strict_exact']]
    if not rows or failing:
        entry.update(status='skipped', reason='no registered functions' if not rows else
                     f"{len(failing)} registered function(s) not strict exact", failing=failing[:10])
        return entry
    bindings, files, conflicts = unit_layout.source_bindings(source, obj)
    laid = lay_out(obj, bindings, fallback, pe.bytes_at_va)
    if laid is None:
        entry.update(status='skipped', reason='no bound non-pick-any function to anchor .text')
        return entry
    layout, report = laid
    findings = diagnose(layout, report, obj, owners, rel(source))
    entry.update(status='laid_out', verdict=unit_verdict(report, findings),
                 groups={k: {x: g[x] for x in ('start', 'end', 'verdict', 'differing_bytes', 'masked_bytes',
                                                'anchor')} for k, g in report['groups'].items()},
                 functions_placed=report['functions_placed'],
                 functions_offset_agree=report['functions_offset_agree'],
                 functions_without_binding=report['functions_without_binding'],
                 offset_disagree=report['functions_offset_disagree'],
                 retail_order=retail_order(layout),
                 findings=findings, binding_conflicts=conflicts)
    return entry


def discover_sources(root: Path = ROOT) -> list[Path]:
    return sorted(p for p in (root / 'src').rglob('*.cpp') if 'bringup' not in p.relative_to(root / 'src').parts)


def render_markdown(entries: list[dict]) -> str:
    laid = [e for e in entries if e['status'] == 'laid_out']
    lines = ['# Unit layout sweep', '',
             f"{len(entries)} src units; {len(laid)} with every registered function strict exact were laid out; "
             f"{sum(e['verdict'] == 'identical' for e in laid)} identical, "
             f"{sum(e['verdict'] == 'identical_with_unbound' for e in laid)} identical with unbound functions.", '',
             '| Unit | Verdict | .text | Findings |', '|---|---|---|---|']
    for e in sorted(laid, key=lambda e: e['groups'].get('.text', {}).get('start', '')):
        text = e['groups'].get('.text', {})
        kinds: dict[str, int] = {}
        for f in e['findings']:
            kinds[f['kind']] = kinds.get(f['kind'], 0) + 1
        summary = ', '.join(f'{k} {n}' for k, n in sorted(kinds.items())) or '-'
        lines.append(f"| `{e['source']}` | {e['verdict']} | `{text.get('start')}..{text.get('end')}` | {summary} |")
    lines += ['', '## Findings', '']
    for e in laid:
        if not e['findings']:
            continue
        lines.append(f"### {e['source']} ({e['verdict']})")
        for f in e['findings']:
            detail = {k: v for k, v in f.items() if k != 'kind'}
            lines.append(f"- **{f['kind']}** {json.dumps(detail)}")
        lines.append('')
    skipped = [e for e in entries if e['status'] != 'laid_out']
    lines += ['## Not laid out', '', '| Unit | Reason |', '|---|---|']
    lines += [f"| `{e['source']}` | {e.get('reason', e['status'])} |" for e in skipped]
    return '\n'.join(lines) + '\n'


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE', str(ROOT / 'work/game/mcm2.exe')))
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--source', action='append', type=Path, help='sweep only these sources')
    ap.add_argument('--out', type=Path, default=DEFAULT_OUT)
    ap.add_argument('--jobs', type=int, default=8)
    args = ap.parse_args(argv)
    if not args.vc6_root:
        ap.error('set VC6_ROOT or pass --vc6-root')
    from mcm2tool.pe import PEImage
    pe = PEImage(args.exe)
    sources = [(ROOT / s).resolve() if not s.is_absolute() else s for s in args.source] if args.source \
        else discover_sources()
    objs = args.out / 'objs'
    objs.mkdir(parents=True, exist_ok=True)

    def build(source: Path):
        out = objs / (rel(source).replace('/', '_')[:-4] + '.obj')
        try:
            unit_layout.compile_source(source, out, args.vc6_root)
            return source, out, None
        except SystemExit as exc:
            return source, None, str(exc)[-2000:]

    with ThreadPoolExecutor(max_workers=max(1, args.jobs)) as pool:
        built = list(pool.map(build, sources))
    rows, owners, fallback = registered_rows(), retail_owners(), unit_layout.repository_bindings()
    entries = []
    for source, obj, error in built:
        if obj is None:
            entries.append({'source': rel(source), 'status': 'compile_failed', 'reason': 'VC6 compile failed',
                            'error': error})
            continue
        entries.append(analyse(source, obj, pe, owners, rows.get(rel(source), []), fallback))
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / 'report.json').write_text(json.dumps(entries, indent=2) + '\n')
    (args.out / 'report.md').write_text(render_markdown(entries))
    laid = [e for e in entries if e['status'] == 'laid_out']
    for e in laid:
        print(f"{e['verdict']:<24} {e['source']}  " + ', '.join(sorted({f['kind'] for f in e['findings']})))
    print(f"{len(entries)} units, {len(laid)} laid out, "
          f"{sum(e['verdict'] == 'identical' for e in laid)} identical; report: {args.out / 'report.md'}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
