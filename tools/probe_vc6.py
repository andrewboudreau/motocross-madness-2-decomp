#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, subprocess
from pathlib import Path
from mcm2tool.toolchain import find_vc6_bin, find_child_ci, fingerprint_toolchain
from mcm2tool.vc6_runtime import executable_command, runner_kind


def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--vc6-root',default=os.environ.get('VC6_ROOT')); ap.add_argument('--json',action='store_true'); a=ap.parse_args()
    if not a.vc6_root: raise SystemExit('set VC6_ROOT or pass --vc6-root')
    root=Path(a.vc6_root).expanduser().resolve()
    try: b=find_vc6_bin(root)
    except FileNotFoundError as e: raise SystemExit(str(e))
    cl=find_child_ci(b,'cl.exe')
    payload=fingerprint_toolchain(root)
    expected_path=Path('config/vc6_sp3_expected.json')
    expected=json.loads(expected_path.read_text()) if expected_path.exists() else {'core_files':{}}
    checks=[]
    by_name={f['name'].upper():f for f in payload['files']}
    for name,prefixes in expected.get('core_files',{}).items():
        f=by_name.get(name.upper())
        versions=(f or {}).get('version_strings',[])
        matched=any(any(v.startswith(pref) for pref in prefixes) for v in versions)
        checks.append({'file':name,'present':f is not None,'versions':versions,'expected_prefixes':prefixes,'matches_sp3':matched})
    payload['sp3_checks']=checks
    payload['sp3_core_match']=bool(checks) and all(c['present'] and c['matches_sp3'] for c in checks)
    payload['compiler_banner']=None
    payload['wine_available']=False
    runner = runner_kind()
    payload['runner'] = runner
    try:
        cmd = executable_command(cl, runner)
    except FileNotFoundError:
        cmd = None
    payload['wine_available'] = runner == 'wine' and bool(cmd)
    if cmd:
        r=subprocess.run(cmd,text=True,encoding='latin-1',stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env={**os.environ,'WINEDEBUG':'-all'})
        payload['compiler_banner']=r.stdout.strip()
        payload['compiler_probe_returncode']=r.returncode
    if a.json:
        print(json.dumps(payload,indent=2)); return
    if payload['compiler_banner']:
        print(payload['compiler_banner'])
    else:
        print(f'Compiler execution probe skipped: {runner} unavailable. Static toolchain fingerprint follows.')
    print('Tool files:')
    for f in payload['files']:
        versions=', '.join(f['version_strings'][:6]) or '-'
        print(f"  {f['name']:12} {f['size']:9} bytes sha256={f['sha256']} versions={versions}")
    print(f"Headers: {'yes' if payload['has_include'] else 'no'}   Libs: {'yes' if payload['has_lib'] else 'no'}")
    print('SP3 core version check:', 'MATCH' if payload['sp3_core_match'] else 'NOT YET CONFIRMED')
    for c in payload['sp3_checks']:
        if not c['matches_sp3']:
            print(f"  {c['file']}: present={c['present']} versions={c['versions']} expected~={c['expected_prefixes']}")
    print('\nMCM2 evidence expects the VC6/SP3 toolchain family (dominant Rich Utc12_CPP build 8447).')
    print('Do not trust cl.exe banner alone; preserve hashes and validate emitted code with the byte matcher.')

if __name__=='__main__': main()
