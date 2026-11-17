#!/usr/bin/env python3
"""Compare two ``linked_accuracy.py`` reports: did a source move or rename change any function?

Rows are keyed by retail VA (the bound retail function), so renaming a symbol
or moving it to another object is reported as a move, not as a loss. A move
or rename is expected to leave every score unchanged; any score that drops, any
bound function that disappears and any new call mismatch is a regression, and
the exit status is 1.

    python tools/accuracy_diff.py BEFORE/report.json AFTER/report.json
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path


def load(path: Path) -> dict[int, dict]:
    rows = {}
    for row in json.loads(path.read_text()):
        rows[int(row['retail_va'], 16)] = row
    return rows


def compare(before: dict[int, dict], after: dict[int, dict]) -> dict[str, list]:
    out = {'lost': [], 'gained': [], 'worse': [], 'better': [], 'calls': [], 'moved': [], 'renamed': []}
    for va in sorted(before.keys() | after.keys()):
        b, a = before.get(va), after.get(va)
        if a is None:
            out['lost'].append(b)
            continue
        if b is None:
            out['gained'].append(a)
            continue
        if a['score'] < b['score']:
            out['worse'].append((b, a))
        elif a['score'] > b['score']:
            out['better'].append((b, a))
        new_calls = (set(map(str, a.get('unexpected_calls', []))) - set(map(str, b.get('unexpected_calls', []))) or
                     set(map(str, a.get('missing_calls', []))) - set(map(str, b.get('missing_calls', []))))
        if new_calls:
            out['calls'].append((b, a))
        if a['object'] != b['object']:
            out['moved'].append((b, a))
        if a['symbol'] != b['symbol']:
            out['renamed'].append((b, a))
    return out


def regressions(result: dict[str, list]) -> int:
    return len(result['lost']) + len(result['worse']) + len(result['calls'])


def render(result: dict[str, list]) -> str:
    lines = []
    def pair(title, rows, fmt):
        if rows:
            lines.append('%s (%d):' % (title, len(rows)))
            lines.extend('  ' + fmt(r) for r in rows)
    pair('LOST', result['lost'], lambda r: '%s %s (%s)' % (r['retail_va'], r['symbol'], r['object']))
    pair('WORSE', result['worse'], lambda p: '%s %s %.3f -> %.3f' % (p[0]['retail_va'], p[1]['symbol'],
                                                                     p[0]['score'], p[1]['score']))
    pair('NEW CALL MISMATCH', result['calls'], lambda p: '%s %s' % (p[0]['retail_va'], p[1]['symbol']))
    pair('better', result['better'], lambda p: '%s %s %.3f -> %.3f' % (p[0]['retail_va'], p[1]['symbol'],
                                                                      p[0]['score'], p[1]['score']))
    pair('gained', result['gained'], lambda r: '%s %s (%s)' % (r['retail_va'], r['symbol'], r['object']))
    pair('moved', result['moved'], lambda p: '%s %s: %s -> %s' % (p[0]['retail_va'], p[1]['symbol'],
                                                                 p[0]['object'], p[1]['object']))
    pair('renamed', result['renamed'], lambda p: '%s %s -> %s' % (p[0]['retail_va'], p[0]['symbol'],
                                                                 p[1]['symbol']))
    lines.append('%d regressions' % regressions(result))
    return '\n'.join(lines)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('before', type=Path)
    ap.add_argument('after', type=Path)
    args = ap.parse_args(argv)
    result = compare(load(args.before), load(args.after))
    print(render(result))
    return 1 if regressions(result) else 0


if __name__ == '__main__':
    sys.exit(main())
