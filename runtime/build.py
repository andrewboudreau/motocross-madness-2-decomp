"""Build the MCM2 runtime modules with VC6 and optionally stage a game copy.

    python runtime/build.py                         # builds work/runtime/d3drm.dll
    python runtime/build.py --stage work/run/demo   # also copies the demo there

--stage copies the game directory (default: the directory of $MCM2_EXE, or
work/game/MCM2demov2) to the target once, then installs the built DLLs into
it. The source game directory is never modified.
"""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from mcm2tool.vc6_runtime import executable_command, runner_kind, windows_path  # noqa: E402

OUT = ROOT / 'work' / 'runtime'
MODULES = {
    'd3drm': {
        'sources': ['runtime/d3drm/d3drm.c', 'runtime/d3drm/trace.c',
                    'runtime/d3drm/vtrace.c', 'runtime/d3drm/sampler.c',
                    'runtime/d3drm/present.c'],
        'def': 'runtime/d3drm/d3drm.def',
        'libs': ['kernel32.lib', 'user32.lib', 'gdi32.lib', 'advapi32.lib'],
    },
    'dplayx': {
        'sources': ['runtime/dplayx/dplayx.c'],
        'def': 'runtime/dplayx/dplayx.def',
        'libs': ['kernel32.lib'],
    },
}


def vc6_root() -> Path:
    for candidate in (os.environ.get('VC6_ROOT'), ROOT / 'work' / 'vc6sp3-disc'):
        if candidate and (Path(candidate) / 'VC98' / 'BIN').is_dir():
            return Path(candidate)
    raise SystemExit('VC6 not found: set VC6_ROOT (see docs/TOOLCHAIN.md)')


def build(name: str, spec: dict, root: Path, runner: str) -> Path:
    vc98 = root / 'VC98'
    env = dict(os.environ)
    env['INCLUDE'] = windows_path(vc98 / 'INCLUDE', runner)
    env['LIB'] = windows_path(vc98 / 'LIB', runner)
    env['PATH'] = os.pathsep.join([str(vc98 / 'BIN'), str(root / 'COMMON' / 'MSDEV98' / 'BIN'),
                                   env.get('PATH', '')])
    OUT.mkdir(parents=True, exist_ok=True)
    dll = OUT / f'{name}.dll'
    command = executable_command(vc98 / 'BIN' / 'CL.EXE', runner) + [
        '/nologo', '/O2', '/MT', '/W3', '/LD',
        f'/Fo{windows_path(OUT, runner)}\\',
        f'/Fe{windows_path(dll, runner)}',
        *[windows_path(ROOT / s, runner) for s in spec['sources']],
        '/link', f'/DEF:{windows_path(ROOT / spec["def"], runner)}', *spec['libs'],
    ]
    result = subprocess.run(command, env=env, cwd=OUT, text=True, capture_output=True)
    sys.stdout.write(result.stdout)
    sys.stderr.write(result.stderr)
    if result.returncode or not dll.is_file():
        raise SystemExit(f'{name}: VC6 build failed ({result.returncode})')
    print(f'built {dll.relative_to(ROOT)} ({dll.stat().st_size} bytes)')
    return dll


def default_game_dir() -> Path:
    exe = os.environ.get('MCM2_EXE')
    return Path(exe).parent if exe else ROOT / 'work' / 'game' / 'MCM2demov2'


def stage(target: Path, source: Path, dlls: list[Path]) -> None:
    if not (target / 'mcm2.exe').is_file():
        if not (source / 'mcm2.exe').is_file():
            raise SystemExit(f'no mcm2.exe in {source}; pass --game-dir')
        shutil.copytree(source, target, dirs_exist_ok=True)
        print(f'copied {source} -> {target}')
    for dll in dlls:
        shutil.copy2(dll, target / dll.name)
        print(f'installed {dll.name} -> {target}')


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--stage', type=Path, help='game copy to install the DLLs into')
    parser.add_argument('--game-dir', type=Path, default=None, help='game directory to copy from')
    args = parser.parse_args()
    root, runner = vc6_root(), runner_kind()
    dlls = [build(name, spec, root, runner) for name, spec in MODULES.items()]
    if args.stage:
        stage(args.stage.resolve(), (args.game_dir or default_game_dir()).resolve(), dlls)


if __name__ == '__main__':
    main()
