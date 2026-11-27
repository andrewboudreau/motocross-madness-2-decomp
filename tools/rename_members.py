"""Compiler-driven member rename: rename fields of one struct and fix every use the compiler finds.

Member names such as `heading` or `position` are too common to rename with a regex. This tool
renames the declarations in the header, compiles every unit whose include closure reaches it with
VC6, and rewrites the `->old` / `.old` access on each line VC6 reports as
"error C2039: 'old' : is not a member of 'Struct'". It repeats until the units compile.

usage:
  python tools/rename_members.py HEADER STRUCT MAP.json --vc6-root DIR [--units FILE] [--dry-run]

MAP.json is {"oldName": "newName", ...}. The header's declarations must already use the new names
(or pass --rename-decls to rewrite `old;` / `old[` declarations inside STRUCT's body).
Generated objects go to work/rename_members/ (ignored).
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INCLUDE_DIRS = ['src/krusty2', 'src/reconstructed']
UNIT_ROOTS = ['src', 'samples', 'tests/fixtures']
INC_RE = re.compile(r'^\s*#\s*include\s*"([^"]+)"', re.M)
ERR_RE = re.compile(r"^(.*?)\((\d+)\) : error C2039: '(\w+)' : (?:is not a member of|Ist kein Element von) '(\w+)'", re.M)


def closure(path: Path, cache: dict) -> set[Path]:
    if path in cache:
        return cache[path]
    cache[path] = set()
    out: set[Path] = set()
    try:
        text = path.read_text(encoding='latin-1')
    except OSError:
        return out
    for name in INC_RE.findall(text):
        for c in [path.parent / name] + [ROOT / d / name for d in INCLUDE_DIRS]:
            if c.is_file():
                c = c.resolve()
                out.add(c)
                out |= closure(c, cache)
                break
    cache[path] = out
    return out


def units_reaching(header: Path) -> list[Path]:
    header = header.resolve()
    cache: dict = {}
    found = []
    for root in UNIT_ROOTS:
        for cpp in sorted((ROOT / root).rglob('*.cpp')):
            cpp = cpp.resolve()
            if header in closure(cpp, cache):
                found.append(cpp)
    return found


def compile_errors(unit: Path, vc6_root: str) -> tuple[bool, str]:
    out = ROOT / 'work/rename_members' / (unit.relative_to(ROOT).as_posix().replace('/', '__') + '.obj')
    out.parent.mkdir(parents=True, exist_ok=True)
    cmd = [sys.executable, str(ROOT / 'tools/compile.py'), str(unit), '-o', str(out),
           '--compiler', 'vc6', '--vc6-root', vc6_root, '--profile', 'vc6_o2_mt']
    for d in INCLUDE_DIRS:
        cmd += ['--include', str(ROOT / d)]
    r = subprocess.run(cmd, cwd=ROOT, env=dict(os.environ, PYTHONPATH=str(ROOT)),
                       capture_output=True, text=True, encoding='latin-1')
    return r.returncode == 0, r.stdout + r.stderr


def rename_decls(header: Path, struct: str, mapping: dict[str, str]) -> None:
    text = header.read_text(encoding='latin-1')
    m = re.search(r'\bstruct\s+' + struct + r'\b[^;{]*\{', text)
    if not m:
        raise SystemExit(f'{struct} body not found in {header}')
    depth, i = 1, m.end()
    while depth:
        depth += {'{': 1, '}': -1}.get(text[i], 0)
        i += 1
    body = text[m.end():i]
    lines = body.split('\n')
    for old, new in mapping.items():
        total = 0
        for k, line in enumerate(lines):
            code, sep, comment = line.partition('//')
            code, n = re.subn(r'(?<=[\s*&])' + old + r'(?=\s*[;\[])', new, code)
            total += n
            lines[k] = code + sep + comment
        if total != 1:
            raise SystemExit(f'{old}: {total} declarations in {struct}')
    body = '\n'.join(lines)
    header.write_text(text[:m.end()] + body + text[i:], encoding='latin-1', newline='')


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('header')
    ap.add_argument('struct')
    ap.add_argument('map')
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--units', help='file listing units (default: every unit reaching HEADER)')
    ap.add_argument('--rename-decls', action='store_true')
    ap.add_argument('--max-rounds', type=int, default=8)
    a = ap.parse_args(argv)
    mapping = json.loads(Path(a.map).read_text())
    header = (ROOT / a.header).resolve()
    if a.rename_decls:
        rename_decls(header, a.struct, mapping)
    if a.units:
        units = [(ROOT / l.strip()).resolve() for l in Path(a.units).read_text().split() if l.strip()]
    else:
        units = units_reaching(header)
    print(f'{len(units)} units reach {a.header}')
    pending = list(units)
    for rnd in range(a.max_rounds):
        failing = []
        edits = 0
        for unit in pending:
            ok, log = compile_errors(unit, a.vc6_root)
            if ok:
                continue
            hits = [(f, int(l), o) for f, l, o, s in ERR_RE.findall(log) if s == a.struct and o in mapping]
            if not hits:
                print(f'FAIL {unit.relative_to(ROOT).as_posix()} (no {a.struct} member errors)')
                print('\n'.join(l for l in log.splitlines() if 'error' in l)[:1500])
                continue
            failing.append(unit)
            for f, line, old in sorted(set(hits)):
                path = Path(f.strip())
                if not path.is_absolute():
                    path = (unit.parent / path)
                lines = path.read_text(encoding='latin-1').split('\n')
                new_line, n = re.subn(r'(->|\.)' + old + r'\b', r'\1' + mapping[old], lines[line - 1])
                if n == 0:
                    print(f'  ? {path}:{line} no access to {old}: {lines[line - 1].strip()[:100]}')
                    continue
                lines[line - 1] = new_line
                path.write_text('\n'.join(lines), encoding='latin-1', newline='')
                edits += n
        print(f'round {rnd + 1}: {len(failing)} units fixed up, {edits} accesses renamed')
        if not failing:
            return 0
        pending = failing
    return 1


if __name__ == '__main__':
    raise SystemExit(main(sys.argv[1:]))
