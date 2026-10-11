#!/usr/bin/env python3
"""Check that `__real@` and `??_C@` bindings point at retail bytes holding the named literal.

Usage:
    python tools/check_literal_bindings.py [--exe work/game/mcm2.exe] [FILE.bindings.json ...]

Both symbol kinds name their own contents, so the retail bytes at the bound
address are independent evidence:

    __real@4@<x87 hex> / __real@8@<x87 hex>   the float or double value
    ??_C@_0<length><hash>@<mangled prefix>@   a narrow string's byte length and
                                              its first (up to 32) bytes

An instruction's bytes do not include the value it loads, so a binding to the
wrong address lets a wrong literal compare as exact (KrustyBike 0x00492670
once bound the 100.0f key to the 255.0f at 0x00551420). With no files given,
every tracked `*.bindings.json` is checked. The exit status is 1 when any
binding disagrees.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))

from mcm2tool.pe import PEImage  # noqa: E402
from propose_bindings import decode_real  # noqa: E402

# `?0`..`?9` in a mangled string literal.
SPECIAL = b',/\\:. \n\t\'-'
DRIVE_PATH = re.compile(rb'[A-Za-z]:[\\/]')


def _mangled_number(text: str) -> tuple[int, str] | None:
    """MSVC encoded number: a digit d means d + 1, else hex digits A-P ending in '@'."""
    if not text:
        return None
    if text[0].isdigit():
        return int(text[0]) + 1, text[1:]
    m = re.match(r'([A-P]*)@', text)
    if not m:
        return None
    value = 0
    for ch in m.group(1):
        value = value * 16 + (ord(ch) - ord('A'))
    return value, text[m.end():]


def decode_string(name: str) -> tuple[int, bytes] | None:
    """`??_C@_0...` -> (byte length including the terminator, known leading bytes)."""
    if not name.startswith('??_C@_0') or not name.endswith('@'):
        return None
    parsed = _mangled_number(name[len('??_C@_0'):])
    if parsed is None:
        return None
    length, rest = parsed
    m = re.match(r'[A-P]*@', rest)  # the checksum
    if not m:
        return None
    body = rest[m.end():-1]
    out = bytearray()
    i = 0
    while i < len(body):
        ch = body[i]
        if ch != '?':
            out.append(ord(ch))
            i += 1
        elif body[i + 1:i + 2] == '$':
            hi, lo = body[i + 2:i + 4]
            out.append((ord(hi) - ord('A')) * 16 + ord(lo) - ord('A'))
            i += 4
        elif body[i + 1].isdigit():
            out.append(SPECIAL[int(body[i + 1])])
            i += 2
        elif 'a' <= body[i + 1] <= 'z':
            out.append(0xE1 + ord(body[i + 1]) - ord('a'))
            i += 2
        elif 'A' <= body[i + 1] <= 'Z':
            out.append(0xC1 + ord(body[i + 1]) - ord('A'))
            i += 2
        else:
            return None
    return length, bytes(out)


def tracked_bindings() -> list[Path]:
    out = subprocess.run(['git', 'ls-files', '*.bindings.json'], cwd=ROOT,
                         capture_output=True, text=True, check=True).stdout
    return [ROOT / line for line in out.splitlines() if line]


def describe_real(enc: bytes) -> str:
    fmt = '<f' if len(enc) == 4 else '<d'
    return f'{struct.unpack(fmt, enc)[0]!r} ({enc.hex()})'


def check_one(sym: str, va: int, pe) -> str | None:
    """A description of the disagreement, or None when the bytes agree or the key is not a literal."""
    enc = decode_real(sym)
    if enc is not None:
        got = pe.bytes_at_va(va, len(enc))
        if got != enc:
            return f'holds {describe_real(got)}, the key names {describe_real(enc)}'
        return None
    decoded = decode_string(sym)
    if decoded is not None:
        length, prefix = decoded
        if DRIVE_PATH.match(prefix):
            # __FILE__ names the build machine's path; bindings deliberately
            # point it at retail's own D:\aardvark\... string.
            return None
        got = pe.bytes_at_va(va, length)
        if not got.startswith(prefix) or len(got) != length or got[-1] != 0 or 0 in got[:-1]:
            return f'holds {got[:40]!r}, the key names {length} bytes starting {prefix[:40]!r}'
    return None


def check(files: list[Path], pe) -> list[str]:
    problems = []
    for path in files:
        bindings = json.loads(path.read_text(encoding='utf-8'))
        for sym, va in bindings.items():
            if not isinstance(va, str):
                continue
            why = check_one(sym, int(va, 16), pe)
            if why:
                rel = path.relative_to(ROOT) if path.is_relative_to(ROOT) else path
                problems.append(f'{rel}: {sym} -> {va} {why}')
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
    print(f'{len(problems)} mismatched literal bindings in {len(files)} files')
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
