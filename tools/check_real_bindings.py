#!/usr/bin/env python3
"""Check that every `__real@` binding points at retail bytes holding the named value.

Usage:
    python tools/check_real_bindings.py [--exe work/game/mcm2.exe] [FILE.bindings.json ...]

A `__real@4@<x87 hex>` / `__real@8@<x87 hex>` symbol names its own value, so the
retail bytes at its bound address are independent evidence. A wrong address here
resolves the relocation to the wrong constant and can make a candidate that uses
the wrong literal compare strictly exact (KrustyBike 0x00492670 once bound the
100.0f key to the 255.0f at 0x00551420). With no files given, every tracked
`*.bindings.json` is checked. The exit status is 1 when any binding disagrees.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))

from mcm2tool.pe import PEImage  # noqa: E402
from propose_bindings import decode_real  # noqa: E402


def tracked_bindings() -> list[Path]:
    out = subprocess.run(['git', 'ls-files', '*.bindings.json'], cwd=ROOT,
                         capture_output=True, text=True, check=True).stdout
    return [ROOT / line for line in out.splitlines() if line]


def describe(enc: bytes) -> str:
    fmt = '<f' if len(enc) == 4 else '<d'
    return f'{struct.unpack(fmt, enc)[0]!r} ({enc.hex()})'


def check(files: list[Path], pe: PEImage) -> list[str]:
    problems = []
    for path in files:
        bindings = json.loads(path.read_text(encoding='utf-8'))
        for sym, va in bindings.items():
            enc = decode_real(sym)
            if enc is None or not isinstance(va, str):
                continue
            got = pe.bytes_at_va(int(va, 16), len(enc))
            if got != enc:
                rel = path.relative_to(ROOT) if path.is_relative_to(ROOT) else path
                problems.append(f'{rel}: {sym} -> {va} holds {describe(got)}, '
                                f'the key names {describe(enc)}')
    return problems


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--exe', default=str(ROOT / 'work/game/mcm2.exe'))
    ap.add_argument('files', nargs='*', type=Path)
    args = ap.parse_args()
    files = [p.resolve() for p in args.files] or tracked_bindings()
    problems = check(files, PEImage(args.exe))
    for line in problems:
        print(line)
    print(f'{len(problems)} mismatched __real@ bindings in {len(files)} files')
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
