"""Record what the last run in a game directory was: executable, build, commit, frames.

    python tools/record_run.py                    # work/run/srcbuild
    python tools/record_run.py work/run/demo

Writes ``run_record.json`` into the run directory (ignored, like the run itself)
and prints it. The executable's SHA-256 is looked up among the
``work/game-build/build-*/report.json`` link hashes; a match gives the build
directory and, for builds made since ``build_game.py`` records it, the commit the
build started from and whether tracked files differed from it (older builds: null,
with only the build time). The demo build is recognised by its own hash.

The run itself comes from the last session in ``mcm2_runtime.log`` (the shim
appends one per start): its first and last lines, the hash the shim saw, whether
its hooks were on, and how it ended. Frames are the ``frames/*.bmp`` written at
or after the session start; access violations are counted from ``errorlog.txt``
entries written since then (by file time, as entries carry no date).

Nothing here launches the game; it only reads what a run left behind.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timedelta
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
DEMO_SHA256 = '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874'
LOG_LINE = re.compile(r'^(\d\d:\d\d:\d\d\.\d{3}) (.*)$')


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open('rb') as handle:
        for block in iter(lambda: handle.read(1 << 20), b''):
            digest.update(block)
    return digest.hexdigest()


def find_build(sha256: str, builds: Path) -> dict | None:
    """The build whose linked executable has this hash, newest first."""
    reports = sorted(builds.glob('build-*/report.json'), key=lambda p: p.stat().st_mtime,
                     reverse=True)
    for report_path in reports:
        try:
            report = json.loads(report_path.read_text(encoding='utf-8'))
        except (OSError, ValueError):
            continue
        if (report.get('link') or {}).get('sha256') == sha256:
            revision = report.get('source_revision') or {}
            return {'directory': report_path.parent.relative_to(ROOT).as_posix()
                    if report_path.parent.is_relative_to(ROOT) else str(report_path.parent),
                    'built': datetime.fromtimestamp(report_path.stat().st_mtime)
                    .isoformat(timespec='seconds'),
                    'commit': revision.get('commit'), 'dirty': revision.get('dirty')}
    return None


def last_session(log: Path) -> dict | None:
    """The last start-to-end session in the shim's appended log."""
    if not log.is_file():
        return None
    lines = log.read_text(encoding='latin-1', errors='replace').splitlines()
    starts = [i for i, line in enumerate(lines) if 'runtime d3drm.dll loaded into' in line]
    if not starts:
        return None
    session = [LOG_LINE.match(line) for line in lines[starts[-1]:]]
    session = [m for m in session if m]
    texts = [m.group(2) for m in session]
    logged_sha = next((t.split()[2] for t in texts if t.startswith('executable sha256 ')), None)
    # What the shim did, not what it said: before the source-build route it logged
    # "hooks stay off" for an unknown hash even when MCM2_FORCE_RUNTIME turned them on.
    hooks = ('on' if any(t.startswith(('recorded a portable', 'trace: patched')) for t in texts)
             else 'off')
    end = next((t for t in reversed(texts) if t.startswith(('ExitProcess', 'unloading'))), None)
    return {'start': session[0].group(1), 'end': session[-1].group(1), 'lines': len(session),
            'logged_sha256': logged_sha, 'hooks': hooks,
            'camera_ran': any('first D3DRMVectorRotate call' in t for t in texts),
            'last_line': end or texts[-1]}


def session_date(log: Path, start: str) -> datetime:
    """The session's start as a date and time: the log's own date, its line's time."""
    stamp = datetime.fromtimestamp(log.stat().st_mtime)
    hour, minute, second = start.split(':')
    started = stamp.replace(hour=int(hour), minute=int(minute), second=int(float(second)),
                            microsecond=0)
    return started if started <= stamp else started - timedelta(days=1)


def record(run: Path, builds: Path) -> dict:
    exe = run / 'mcm2.exe'
    if not exe.is_file():
        raise FileNotFoundError(f'no mcm2.exe in {run}')
    sha = sha256_file(exe)
    result: dict = {'run_directory': run.relative_to(ROOT).as_posix()
                    if run.is_relative_to(ROOT) else str(run),
                    'recorded': datetime.now().isoformat(timespec='seconds'),
                    'exe_sha256': sha, 'demo_build': sha == DEMO_SHA256,
                    'build': find_build(sha, builds),
                    'map_beside_exe': (run / 'mcm2.map').is_file()}
    log = run / 'mcm2_runtime.log'
    session = last_session(log)
    result['session'] = session
    since = None
    if session:
        since = session_date(log, session['start'])
        session['date'] = since.isoformat(timespec='seconds')
        session['ran_this_exe'] = session['logged_sha256'] == sha
    frames = sorted(p for p in (run / 'frames').glob('*.bmp')
                    if since is None or datetime.fromtimestamp(p.stat().st_mtime) >= since)
    result['frames'] = {'count': len(frames), 'names': [p.name for p in frames]}
    errors = run / 'errorlog.txt'
    if errors.is_file():
        written = datetime.fromtimestamp(errors.stat().st_mtime)
        count = errors.read_text(encoding='latin-1', errors='replace').count('caused an access violation')
        result['errorlog'] = {'access_violations': count,
                              'written': written.isoformat(timespec='seconds'),
                              'written_during_session': since is not None and written >= since}
    state = run / 'statelog.txt'
    if state.is_file():
        written = datetime.fromtimestamp(state.stat().st_mtime)
        lines = state.read_text(encoding='latin-1', errors='replace').splitlines()
        result['state_log'] = {'frames': len(lines),
                               'race_frames': sum(' | ' in line for line in lines),
                               'faults': sum(line.endswith(' fault') for line in lines),
                               'written_during_session': since is not None and written >= since}
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('run', nargs='?', type=Path, default=ROOT / 'work/run/srcbuild')
    parser.add_argument('--builds', type=Path, default=ROOT / 'work/game-build')
    args = parser.parse_args()
    try:
        result = record(args.run.resolve(), args.builds.resolve())
    except FileNotFoundError as exc:
        print(exc, file=sys.stderr)
        return 1
    text = json.dumps(result, indent=2) + '\n'
    (args.run / 'run_record.json').write_text(text, encoding='utf-8')
    print(text, end='')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
