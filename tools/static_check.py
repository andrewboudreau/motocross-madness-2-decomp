#!/usr/bin/env python3
from __future__ import annotations
from pathlib import Path
import subprocess
import sys

BLOCKED_SUFFIXES = {
    '.exe', '.dll', '.cab', '.iso', '.msi', '.lib', '.pdb', '.idb', '.ilk',
    '.zip', '.7z', '.rar'
}
BLOCKED_PARTS = {'work', 'input', 'toolchains', '__pycache__'}


def tracked() -> list[Path]:
    r = subprocess.run(
        ['git', 'ls-files', '-z'],
        stdout=subprocess.PIPE,
        check=True,
    )
    return [Path(x.decode()) for x in r.stdout.split(b'\0') if x]


def main() -> None:
    files = tracked()
    bad: list[str] = []

    for path in files:
        if (
            path.suffix.lower() in BLOCKED_SUFFIXES
            or any(part in BLOCKED_PARTS for part in path.parts)
        ):
            bad.append(str(path))

    if bad:
        print('Blocked proprietary/generated paths are tracked:', file=sys.stderr)
        for path in bad:
            print(f'  {path}', file=sys.stderr)
        raise SystemExit(1)

    python_files = [str(path) for path in files if path.suffix == '.py']
    if python_files:
        subprocess.run(
            [sys.executable, '-m', 'py_compile', *python_files],
            check=True,
        )

    print(
        f'static-check: PASS '
        f'({len(files)} tracked files, {len(python_files)} Python files)'
    )


if __name__ == '__main__':
    main()
