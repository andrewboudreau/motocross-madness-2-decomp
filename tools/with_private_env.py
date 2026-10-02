#!/usr/bin/env python3
from __future__ import annotations
import argparse
import json
import os
from pathlib import Path
import subprocess

def main():
    ap = argparse.ArgumentParser(description='Run a command with paths from an installed private MCM2/VC6 bundle.')
    ap.add_argument('--root', type=Path, default=Path(os.environ.get('MCM2_PRIVATE_ROOT', '~/.cache/mcm2-private')).expanduser())
    ap.add_argument('command', nargs=argparse.REMAINDER)
    args = ap.parse_args()
    if not args.command:
        raise SystemExit('command required after --')
    root = args.root.resolve()
    state_path = root / 'private-inputs-state.json'
    if not state_path.exists():
        raise SystemExit(f'private bundle not installed: {state_path}')
    json.loads(state_path.read_text())
    env = os.environ.copy()
    repo = Path(__file__).resolve().parents[1]
    env['PYTHONPATH'] = str(repo) + os.pathsep + env.get('PYTHONPATH', '')
    # Cloud setup installs the pinned host runner here. Setup-shell exports do
    # not survive into a fresh task shell.
    env['PATH'] = str(Path.home() / '.local/bin') + os.pathsep + env.get('PATH', '')
    env.update({
        'MCM2_PRIVATE_ROOT': str(root),
        'VC6_ROOT': str(root / 'toolchains/vc6sp3'),
        'MCM2_EXE': str(root / 'work/game/mcm2.exe'),
        'WINEPREFIX': str(root / 'wine-vc6'),
        'WINEARCH': 'win32',
        'WINEDEBUG': '-all',
    })
    cmd = args.command[1:] if args.command and args.command[0] == '--' else args.command
    raise SystemExit(subprocess.run(cmd, env=env).returncode)

if __name__ == '__main__':
    main()
