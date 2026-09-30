#!/usr/bin/env python3
"""Compare the vtables VC6 emitted into an object file with the retail RTTI vtables.

usage: PYTHONPATH=. python samples/physics/hierarchy/vtable_counts.py OBJ [OBJ...]

For every class that has ??_7 vtable symbols in the object, prints the slot count of
each emitted vtable next to the retail slot counts from analysis/vtables.json (one per
object offset).  Flags classes whose sorted slot counts differ.  Use this after
migrating an area onto the canonical hierarchy: an override whose parameter list does
not exactly match the canonical base declaration silently becomes a NEW virtual and
grows the primary vtable instead of failing to compile.
"""
import json
import re
import sys
from pathlib import Path

from mcm2tool.coff import CoffObject

ROOT = Path(__file__).resolve().parents[3]


def main() -> int:
    retail = {}
    for v in json.loads((ROOT / 'analysis/vtables.json').read_text()):
        retail.setdefault(v['class'], []).append((int(v['object_offset']), len(v['entries'])))
    bad = 0
    for path in sys.argv[1:]:
        emitted = {}
        obj = CoffObject(path)
        for s in obj.symbols:
            m = re.match(r'\?\?_7(\w+)@@6B(.*)@$', s.name)
            if not m or s.section_number <= 0:
                continue
            sec = obj.section(s.section_number)
            emitted.setdefault(m.group(1), []).append((m.group(2) or '0@', (sec.raw_size - s.value) // 4))
        for cls, vts in sorted(emitted.items()):
            want = sorted(retail.get(cls, []))
            ok = sorted(n for _, n in vts) == sorted(n for _, n in want)
            bad += not ok
            got = ', '.join(f'{b.rstrip("@") or "self"}:{n}' for b, n in vts)
            exp = ', '.join(f'@{o}:{n}' for o, n in want) or 'no retail RTTI'
            print(f'{"ok  " if ok else "DIFF"} {cls:28s} emitted [{got}]  retail [{exp}]')
    return 1 if bad else 0


if __name__ == '__main__':
    sys.exit(main())
