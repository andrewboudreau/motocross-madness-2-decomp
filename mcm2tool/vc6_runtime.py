"""Host runners for the original VC6 executables; never substitutes a compiler."""
from __future__ import annotations

import os
from pathlib import Path
import shutil
import subprocess


def runner_kind() -> str:
    if os.name == 'nt':
        return 'windows'
    choice = os.environ.get('VC6_RUNNER', 'auto')
    if choice == 'auto':
        return 'wibo' if shutil.which('wibo') else 'wine'
    if choice not in ('wine', 'wibo'):
        raise ValueError('VC6_RUNNER must be auto, wine or wibo')
    return choice


def required_tool(name: str) -> str:
    tool = shutil.which(name)
    if not tool:
        raise FileNotFoundError(f'{name} not found; run tools/setup_vc6_linux.sh')
    return tool


def windows_path(path: Path, runner: str) -> str:
    if runner == 'windows':
        return str(path.resolve())
    cmd = ([required_tool('wibo'), 'path', '-w'] if runner == 'wibo'
           else [required_tool('winepath'), '-w'])
    return subprocess.run(cmd + [str(path.resolve())], check=True, text=True,
                          capture_output=True).stdout.strip()


def executable_command(program: Path, runner: str) -> list[str]:
    if runner == 'windows':
        return [str(program.resolve())]
    # wibo loads the host path directly; guest arguments use Windows paths.
    if runner == 'wibo':
        return [required_tool('wibo'), str(program.resolve())]
    return [required_tool('wine'), windows_path(program, runner)]
