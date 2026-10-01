#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, shutil, subprocess
from pathlib import Path
from mcm2tool.toolchain import find_vc6_bin, find_child_ci, infer_vc98_root

def load_profiles(path: Path): return json.loads(path.read_text())['profiles']

def winepath(path: Path) -> str:
    wp=shutil.which('winepath')
    if not wp: raise SystemExit('winepath not found; install 32-bit Wine (wine + wine32)')
    r=subprocess.run([wp,'-w',str(path.resolve())],check=True,text=True,capture_output=True)
    return r.stdout.strip()

def run_vc6(root:Path,src:Path,out:Path,flags:list[str],extra:list[str]):
    bindir=find_vc6_bin(root); cl=find_child_ci(bindir,'cl.exe')
    vc98=infer_vc98_root(root)
    includes=[]
    inc=find_child_ci(vc98,'Include')
    if inc and inc.is_dir(): includes.append(inc)
    mfc=find_child_ci(vc98,'MFC')
    if mfc:
        mi=find_child_ci(mfc,'Include')
        if mi and mi.is_dir(): includes.append(mi)
    out.parent.mkdir(parents=True,exist_ok=True)
    if os.name=='nt':
        cmd=[str(cl),*flags,*[f'/I{p}' for p in includes],*extra,str(src.resolve()),f'/Fo{out.resolve()}']
    else:
        wine=shutil.which('wine')
        if not wine: raise SystemExit('wine not found; install Wine with 32-bit support')
        cmd=[wine,winepath(cl),*flags,*[f'/I{winepath(p)}' for p in includes],*extra,winepath(src),f'/Fo{winepath(out)}']
    env=os.environ.copy(); env.setdefault('WINEDEBUG','-all')
    print('+',' '.join(cmd))
    r=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env)
    print(r.stdout,end='')
    if r.returncode: raise SystemExit(r.returncode)

def run_clang(src:Path,out:Path,flags:list[str],extra:list[str]):
    cc=shutil.which('clang-cl')
    if not cc: raise SystemExit('clang-cl not found')
    out.parent.mkdir(parents=True,exist_ok=True)
    cmd=[cc,'--target=i686-pc-windows-msvc',*flags,*extra,str(src.resolve()),f'/Fo{out.resolve()}']
    print('+',' '.join(cmd)); raise_on=subprocess.run(cmd).returncode
    if raise_on: raise SystemExit(raise_on)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('source'); ap.add_argument('-o','--out',required=True)
    ap.add_argument('--compiler',choices=['vc6','clang-cl'],default='vc6')
    ap.add_argument('--vc6-root',default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--profile',default=None); ap.add_argument('--profiles',default='config/compile_profiles.json')
    ap.add_argument('--extra',action='append',default=[],help='additional compiler flag (repeatable)')
    a=ap.parse_args(); profiles=load_profiles(Path(a.profiles))
    # VC6 default: /O2 without /G6 is the only tested profile that matches every
    # calibration target (docs/VC6_MATCHING.md); /MT follows the LIBCMT runtime
    # proof. A working hypothesis, not a confirmed project setting.
    profile=a.profile or ('clang_probe' if a.compiler=='clang-cl' else 'vc6_o2_mt')
    if profile not in profiles: raise SystemExit(f'unknown profile {profile}; choices={list(profiles)}')
    src=Path(a.source); out=Path(a.out); flags=profiles[profile]
    if a.compiler=='clang-cl': run_clang(src,out,flags,a.extra)
    else:
        if not a.vc6_root: raise SystemExit('set VC6_ROOT or pass --vc6-root; Microsoft VC6 files are intentionally not bundled')
        run_vc6(Path(a.vc6_root),src,out,flags,a.extra)
if __name__=='__main__': main()
