#!/usr/bin/env python3
from __future__ import annotations
import argparse, os, shutil, subprocess
from pathlib import Path

def main():
    ap=argparse.ArgumentParser(description='Initialize/update a dedicated 32-bit Wine prefix for the VC6 compiler oracle.')
    ap.add_argument('--prefix',default=os.environ.get('WINEPREFIX','work/wine-vc6'))
    a=ap.parse_args(); prefix=Path(a.prefix).expanduser().resolve()
    wineboot=shutil.which('wineboot'); wine=shutil.which('wine')
    if not wineboot and not wine: raise SystemExit('Wine is not installed. On Debian/Ubuntu install wine plus 32-bit Wine support, or use docker compose.')
    prefix.parent.mkdir(parents=True,exist_ok=True)
    env={**os.environ,'WINEPREFIX':str(prefix),'WINEARCH':'win32','WINEDEBUG':'-all'}
    cmd=[wineboot,'-u'] if wineboot else [wine,'wineboot','-u']
    print('+',' '.join(cmd))
    subprocess.run(cmd,check=True,env=env)
    print(f'Wine prefix ready: {prefix}')
    print(f'export WINEPREFIX={prefix}')
    print('export WINEARCH=win32')

if __name__=='__main__': main()
