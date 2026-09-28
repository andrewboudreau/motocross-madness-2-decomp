#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, shutil, subprocess, tempfile
from pathlib import Path
from mcm2tool.toolchain import find_vc6_bin, infer_vc98_root, fingerprint_toolchain


def extract_archive(src: Path, dst: Path) -> None:
    suffix=''.join(src.suffixes).lower()
    if suffix in {'.zip','.tar','.tar.gz','.tgz','.tar.bz2','.tbz2','.tar.xz','.txz'}:
        shutil.unpack_archive(str(src), str(dst))
        return
    seven=shutil.which('7z') or shutil.which('7zz')
    if seven:
        subprocess.run([seven,'x','-y',f'-o{dst}',str(src)],check=True)
        return
    raise SystemExit(f'Unsupported archive {src}. Extract it first, or install 7z for ISO/7z media.')


def locate_vc98(root: Path) -> Path:
    try:
        return infer_vc98_root(root)
    except Exception:
        pass
    for cl in root.rglob('*'):
        if cl.is_file() and cl.name.casefold()=='cl.exe' and cl.parent.name.casefold()=='bin' and cl.parent.parent.name.casefold()=='vc98':
            return cl.parent.parent
    raise SystemExit(f'Could not locate VC98/Bin/CL.EXE below {root}')


def main():
    ap=argparse.ArgumentParser(description='Import a user-supplied Visual C++ 6 VC98 tree into the private decomp toolchain cache.')
    ap.add_argument('source', help='installed Visual Studio/VC98 directory, or an extracted/archive copy containing VC98')
    ap.add_argument('--out', default='toolchains/vc6sp3', help='private destination; ignored by Git')
    ap.add_argument('--overwrite', action='store_true')
    a=ap.parse_args()
    src=Path(a.source).expanduser().resolve(); out=Path(a.out).expanduser().resolve()
    if not src.exists(): raise SystemExit(f'source does not exist: {src}')
    with tempfile.TemporaryDirectory(prefix='mcm2-vc6-import-') as td:
        scan=src
        if src.is_file():
            scan=Path(td)/'media'; scan.mkdir(); extract_archive(src,scan)
        vc98=locate_vc98(scan)
        if out.exists():
            if not a.overwrite: raise SystemExit(f'{out} already exists; use --overwrite to replace it')
            shutil.rmtree(out)
        out.mkdir(parents=True,exist_ok=True)
        shutil.copytree(vc98,out/'VC98')
    fp=fingerprint_toolchain(out)
    (out/'toolchain-fingerprint.json').write_text(json.dumps(fp,indent=2)+'\n')
    print(f'Imported VC98 tree -> {out / "VC98"}')
    print(f'Fingerprint -> {out / "toolchain-fingerprint.json"}')
    print('Next: python3 tools/probe_vc6.py --vc6-root', out)

if __name__=='__main__': main()
