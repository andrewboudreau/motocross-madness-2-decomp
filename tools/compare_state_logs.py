"""Compare two statelog.txt files from the runtime (MCM2_STATE_LOG=1).

    python tools/compare_state_logs.py RETAIL_LOG SOURCE_LOG [--json OUT]

The runtime writes one line per presented frame (runtime/d3drm/replay.c):
    F <frame> T <clock> M <menu> [R <minutes> <seconds> N <n> | racer ... ]
The logs are aligned at their first race frame (the first line with racers),
because the menus and loading may take a different number of frames. The
report gives the frames before the race in each log, how many race frames
agree, and the first field that differs, with floats decoded.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import struct
import sys

RACER_FIELDS = ('x', 'y', 'z', 'vx', 'vy', 'vz', 'speed', 'laps', 'gates', 'place', 'total_time')
FLOAT_FIELDS = {'x', 'y', 'z', 'vx', 'vy', 'vz', 'speed', 'total_time', 'minutes', 'seconds'}


def as_float(word: str) -> float:
    return struct.unpack('<f', struct.pack('<I', int(word, 16)))[0]


def parse(line: str) -> dict:
    head, _, racers = line.partition(' | ')
    words = head.split()
    row = {'frame': int(words[1]), 'clock': int(words[3]), 'menu': int(words[5]),
           'race': None, 'racers': []}
    if 'R' in words[6:]:
        rest = words[words.index('R', 6) + 1:]
        if rest and rest[0] not in ('blocked',) and len(rest) >= 4:
            row['race'] = {'minutes': rest[0], 'seconds': rest[1]}
            for chunk in (' | ' + racers).split(' | ')[1:]:
                values = chunk.split()
                row['racers'].append(None if values == ['none'] else dict(zip(RACER_FIELDS, values)))
        else:
            row['race'] = 'blocked' if rest[:1] == ['blocked'] else None
    if line.rstrip().endswith('fault'):
        row['fault'] = True
    return row


def load(path: Path) -> list[dict]:
    rows = []
    for line in path.read_text(errors='replace').splitlines():
        if line.startswith('F '):
            try:
                rows.append(parse(line))
            except (IndexError, ValueError):
                rows.append({'frame': None, 'unparsed': line})
    return rows


def show(field: str, value):
    if value is None:
        return None
    if field in FLOAT_FIELDS:
        return {'bits': value, 'float': as_float(value)}
    return value


def first_difference(a: dict, b: dict) -> dict | None:
    if a['race'] != b['race'] and not (isinstance(a['race'], dict) and isinstance(b['race'], dict)):
        return {'field': 'race', 'a': a['race'], 'b': b['race']}
    if isinstance(a['race'], dict):
        for field in ('minutes', 'seconds'):
            if a['race'][field] != b['race'][field]:
                return {'field': field, 'a': show(field, a['race'][field]),
                        'b': show(field, b['race'][field])}
    if len(a['racers']) != len(b['racers']):
        return {'field': 'racer count', 'a': len(a['racers']), 'b': len(b['racers'])}
    for index, (x, y) in enumerate(zip(a['racers'], b['racers'])):
        if x is None or y is None:
            if x != y:
                return {'racer': index, 'field': 'present', 'a': x is not None, 'b': y is not None}
            continue
        for field in RACER_FIELDS:
            if x.get(field) != y.get(field):
                return {'racer': index, 'field': field, 'a': show(field, x.get(field)),
                        'b': show(field, y.get(field))}
    return None


def compare(a_rows: list[dict], b_rows: list[dict]) -> dict:
    def race_start(rows):
        return next((i for i, row in enumerate(rows) if row.get('racers')), None)

    start_a, start_b = race_start(a_rows), race_start(b_rows)
    report = {'frames': [len(a_rows), len(b_rows)],
              'race_start_frame': [a_rows[start_a]['frame'] if start_a is not None else None,
                                   b_rows[start_b]['frame'] if start_b is not None else None]}
    if start_a is None or start_b is None:
        report['result'] = 'no race frames in ' + ' and '.join(
            name for name, start in (('a', start_a), ('b', start_b)) if start is None)
        return report
    race_a, race_b = a_rows[start_a:], b_rows[start_b:]
    agreed = 0
    for x, y in zip(race_a, race_b):
        if x.get('frame') is None or y.get('frame') is None:
            report['difference'] = {'field': 'unparsed line', 'a': x.get('unparsed'),
                                    'b': y.get('unparsed')}
            break
        difference = first_difference(x, y)
        if difference:
            difference['frames'] = [x['frame'], y['frame']]
            report['difference'] = difference
            break
        agreed += 1
    report['race_frames'] = [len(race_a), len(race_b)]
    report['race_frames_agreeing'] = agreed
    if 'difference' in report:
        report['result'] = 'differ'
    elif len(race_a) == len(race_b):
        report['result'] = 'identical race frames'
    else:
        report['result'] = 'agree while both run; one log is longer'
    return report


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('a', type=Path)
    parser.add_argument('b', type=Path)
    parser.add_argument('--json', type=Path)
    args = parser.parse_args(argv)
    report = compare(load(args.a), load(args.b))
    text = json.dumps(report, indent=2)
    if args.json:
        args.json.write_text(text + '\n')
    print(text)
    return 0 if report['result'] == 'identical race frames' else 1


if __name__ == '__main__':
    sys.exit(main())
