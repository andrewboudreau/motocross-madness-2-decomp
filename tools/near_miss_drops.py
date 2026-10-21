"""Report rows whose strict matching bytes dropped between two gate runs.

usage:
  python tools/near_miss_drops.py BASE.json NEW.json

Both files are outputs of tools/run_physics_samples.py --json-out (a list of
results) or tools/run_calibration.py (a list of cases, each with "result");
the two formats may be mixed. Lost strict-exact entries are what the gates
already guard; this catches the rest: a near miss that loses positional
strict bytes, or a row missing from NEW (its source stopped compiling).
Rows are keyed by target VA; a VA with several cases keeps its best count.
Exit status 1 when any row dropped.
"""
import json
import sys


def strict_bytes(path):
    rows = {}
    for entry in json.load(open(path)):
        result = entry.get("result", entry) or {}
        matched = (result.get("strict_match") or {}).get("matching_positions", 0)
        rows[entry["target_va"]] = max(rows.get(entry["target_va"], -1), matched)
    return rows


def drops(base, new):
    """(va, before, after) for every row of base that lost bytes; after is None when missing."""
    return [(va, before, new.get(va)) for va, before in sorted(base.items())
            if new.get(va) is None or new[va] < before]


def main(argv):
    if len(argv) != 3:
        sys.exit(__doc__)
    found = drops(strict_bytes(argv[1]), strict_bytes(argv[2]))
    for va, before, after in found:
        print("near-miss drop", va, before, "->", "missing" if after is None else after)
    print(len(found), "near-miss drops")
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
