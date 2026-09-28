#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, subprocess, sys
from pathlib import Path


def run_json(cmd):
    r=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    if r.returncode and not r.stdout.strip().startswith(('[','{')):
        return {'returncode':r.returncode,'error':r.stdout}
    try:
        return json.loads(r.stdout)
    except Exception:
        return {'returncode':r.returncode,'error':r.stdout}


def all_exact(rows):
    return bool(isinstance(rows,list) and rows and all(r.get('exact_after_relocation_mask') for r in rows))


def main():
    ap=argparse.ArgumentParser(description='Run the authoritative VC6 compiler gate and write one machine-readable report.')
    ap.add_argument('--vc6-root',default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--exe',default='work/game/mcm2.exe')
    ap.add_argument('--out',default='analysis/vc6_gate.json')
    a=ap.parse_args()
    if not a.vc6_root: raise SystemExit('set VC6_ROOT or pass --vc6-root')
    probe=run_json([sys.executable,'tools/probe_vc6.py','--vc6-root',a.vc6_root,'--json'])
    smoke=run_json([sys.executable,'tools/run_samples.py','--exe',a.exe,'--compiler','vc6','--vc6-root',a.vc6_root])
    easy=run_json([sys.executable,'tools/run_easy_probes.py','--exe',a.exe,'--compiler','vc6','--vc6-root',a.vc6_root])
    calibration=run_json([sys.executable,'tools/run_calibration.py','--exe',a.exe,'--compiler','vc6','--vc6-root',a.vc6_root])
    smoke_exact=all_exact(smoke); easy_exact=all_exact(easy)
    report={
        'toolchain':probe,
        'manual_smoke':smoke,'manual_smoke_all_exact':smoke_exact,
        'generated_easy_probes':easy,'generated_easy_probes_all_exact':easy_exact,
        'all_exact':smoke_exact and easy_exact,
        'calibration':calibration,
    }
    out=Path(a.out); out.parent.mkdir(parents=True,exist_ok=True); out.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))
    if not report['all_exact']: raise SystemExit(1)

if __name__=='__main__': main()
