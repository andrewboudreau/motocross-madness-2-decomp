#!/usr/bin/env python3
"""Infer original translation-unit ownership from retail layout and compare it with the repo.

Evidence used (levels in the JSON output: anchor, between, class, run):

* Confirmed: ``analysis/source_xrefs.json`` lists every retail ``__FILE__``
  string (``D:\\aardvark\\VC\\krusty2\\<unit>.cpp``) and the code that loads it.
  The function holding such a load was compiled in that unit, unless the
  string comes from an inline function defined in a header (``.h`` paths).
* Strong inference: LINK places each object's code contiguously, so every
  function between two anchors of the same unit belongs to that unit too.
  Pick-any COMDATs (inline methods, ``??_G``/``??_E`` destructors, templates)
  are kept by whichever object the linker saw first and are reported, never
  used, as evidence.
* Provisional (``class``): a method in the gap between two different units
  goes to the neighbour that already holds methods of its class.
* Provisional (``run``): a function next to an attributed one, defined by the
  same repo object.

The defined functions come from a ``linked_accuracy.py`` report of a source
build (``symbol``, ``object``, ``retail_va``, ``retail_bytes``), so each
function is attributed to the object that defines it, not to files that only
reference it.

Output: per original unit, its retail span, the repo objects that define code
inside it, and the functions that sit inside the span but are defined in
another repo file (move candidates); per repo object, the units its code falls
in (split candidates).
"""
from __future__ import annotations

import argparse
import bisect
import json
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BS = chr(92)


def unit_name(path: str) -> str:
    return path.replace('/', BS).split(BS)[-1]


def is_pick_any(symbol: str) -> bool:
    """Symbols whose COMDAT may be kept by any object that references them."""
    return (symbol.startswith('??_G') or symbol.startswith('??_E') or '$' in symbol.split('@')[0]
            or '?$' in symbol)


def class_of(symbol: str) -> str | None:
    """The class of a mangled member function (``?Name@Class@@...``), else None."""
    if not symbol.startswith('?') or symbol.startswith('??_'):
        return None
    parts = symbol[1:].split('@@', 1)[0].split('@')
    if symbol.startswith('??'):  # constructor/destructor/operator: ??0Class@@, ??4Class@@
        return parts[0][2:] or None
    return parts[1] if len(parts) >= 2 else None


def load_functions(report: Path):
    rows = json.loads(report.read_text())
    out = []
    for r in rows:
        va = int(r['retail_va'], 16)
        out.append({'va': va, 'end': va + int(r['retail_bytes']), 'symbol': r['symbol'],
                    'object': r['object']})
    out.sort(key=lambda r: r['va'])
    return out


def load_anchors(xrefs: Path):
    anchors = []  # (va, unit)
    for e in json.loads(xrefs.read_text()):
        name = unit_name(e['path'])
        if not name.lower().endswith(('.cpp', '.c')):
            continue
        for t in e.get('text_xrefs', []):
            anchors.append((int(t, 16), name))
    anchors.sort()
    return anchors


def attribute(functions, anchors):
    """Return per-function evidence: 'anchor' unit, 'between' unit, or None."""
    starts = [f['va'] for f in functions]
    anchored = {}
    for va, unit in anchors:
        i = bisect.bisect_right(starts, va) - 1
        if i >= 0 and functions[i]['va'] <= va < functions[i]['end']:
            anchored.setdefault(i, set()).add(unit)
    # Ordered anchor list by function index, skipping pick-any functions.
    seq = [(i, next(iter(u))) for i, u in sorted(anchored.items())
           if len(u) == 1 and not is_pick_any(functions[i]['symbol'])]
    result = [None] * len(functions)
    for i, u in anchored.items():
        result[i] = ('anchor', ','.join(sorted(u)))
    for (i, a), (j, b) in zip(seq, seq[1:]):
        if a == b:
            for k in range(i + 1, j):
                if result[k] is None:
                    result[k] = ('between', a)
    # Provisional: a method in the gap between two different units goes to the
    # neighbouring unit that already holds (anchor/between) methods of its
    # class, when exactly one of the two does.
    members = defaultdict(set)
    for k, e in enumerate(result):
        if e and e[0] in ('anchor', 'between') and ',' not in e[1] and class_of(functions[k]['symbol']):
            members[class_of(functions[k]['symbol'])].add(e[1])
    for (i, a), (j, b) in zip(seq, seq[1:]):
        if a == b:
            continue
        for k in range(i + 1, j):
            owner = class_of(functions[k]['symbol'])
            if result[k] is None and owner and not is_pick_any(functions[k]['symbol']):
                hits = members[owner] & {a, b}
                if len(hits) == 1:
                    result[k] = ('class', hits.pop())
    # Weaker: extend each attributed function over the neighbouring functions
    # that the same repo object defines, up to the first function of another
    # object or another unit. Repo objects are not original units, so this is
    # provisional and reported separately.
    for i, e in enumerate(list(result)):
        if e is None or e[0] not in ('anchor', 'between') or ',' in e[1] or is_pick_any(functions[i]['symbol']):
            continue
        for step in (-1, 1):
            k = i + step
            while 0 <= k < len(functions) and functions[k]['object'] == functions[i]['object']:
                if result[k] is not None:
                    break
                result[k] = ('run', e[1])
                k += step
    return result


def analyse(functions, anchors):
    ev = attribute(functions, anchors)
    units = defaultdict(lambda: {'objects': defaultdict(int), 'functions': []})
    objects = defaultdict(lambda: defaultdict(int))
    for f, e in zip(functions, ev):
        if e is None:
            if not is_pick_any(f['symbol']):
                objects[f['object']]['(unattributed)'] += 1
            continue
        kind, unit = e
        if ',' in unit:
            continue
        rec = dict(f, evidence=kind, pick_any=is_pick_any(f['symbol']))
        units[unit]['functions'].append(rec)
        if not rec['pick_any']:
            units[unit]['objects'][f['object']] += 1
            objects[f['object']][unit] += 1
    return units, objects


def stem(name: str) -> str:
    return name.rsplit('.', 1)[0].lower()


def report_text(units, objects) -> str:
    lines = ['# Translation-unit ownership', '']
    lines.append('| Original unit | Span | Functions | Repo objects (functions) |')
    lines.append('|---|---|---|---|')
    moves = []
    for unit in sorted(units, key=lambda u: min(f['va'] for f in units[u]['functions'])):
        info = units[unit]
        fs = [f for f in info['functions'] if not f['pick_any']]
        if not fs:
            continue
        lo = min(f['va'] for f in fs)
        hi = max(f['end'] for f in fs)
        objs = sorted(info['objects'].items(), key=lambda kv: -kv[1])
        lines.append('| `%s` | 0x%08x-0x%08x | %d | %s |' % (
            unit, lo, hi, len(fs), ', '.join('`%s` (%d)' % kv for kv in objs)))
        home = objs[0][0] if objs else None
        for f in fs:
            if stem(f['object']) != stem(unit) and (home is None or stem(home) != stem(unit) or f['object'] != home):
                moves.append((unit, f))
    lines += ['', '## Functions defined outside their original unit', '']
    lines.append('| Function | Retail | Evidence | Defined in | Original unit |')
    lines.append('|---|---|---|---|---|')
    for unit, f in moves:
        lines.append('| `%s` | 0x%08x | %s | `%s` | `%s` |' % (f['symbol'], f['va'], f['evidence'], f['object'], unit))
    lines += ['', '## Repo objects spanning several original units', '']
    for obj in sorted(objects):
        us = objects[obj]
        if len(us) > 1:
            lines.append('- `%s`: %s' % (obj, ', '.join('`%s` (%d)' % kv for kv in sorted(us.items(), key=lambda kv: -kv[1]))))
    return '\n'.join(lines) + '\n'


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--report', type=Path, default=ROOT / 'work/linked-accuracy/report.json',
                    help='linked_accuracy.py report.json of a source build')
    ap.add_argument('--xrefs', type=Path, default=ROOT / 'analysis/source_xrefs.json')
    ap.add_argument('--out', type=Path, default=None, help='write the markdown report here')
    ap.add_argument('--json', type=Path, default=None, help='write per-function evidence as JSON')
    args = ap.parse_args(argv)
    functions = load_functions(args.report)
    anchors = load_anchors(args.xrefs)
    units, objects = analyse(functions, anchors)
    text = report_text(units, objects)
    if args.out:
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(text, encoding='utf-8')
    else:
        print(text)
    if args.json:
        rows = []
        for unit, info in units.items():
            for f in info['functions']:
                rows.append({'symbol': f['symbol'], 'retail_va': '0x%08x' % f['va'], 'object': f['object'],
                             'unit': unit, 'evidence': f['evidence'], 'pick_any': f['pick_any']})
        args.json.write_text(json.dumps(sorted(rows, key=lambda r: r['retail_va']), indent=1), encoding='utf-8')


if __name__ == '__main__':
    main()
