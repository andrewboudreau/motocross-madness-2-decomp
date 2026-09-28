#!/usr/bin/env python3
from __future__ import annotations
import argparse,subprocess,sys,shutil,os
from pathlib import Path

def run(cmd,env=None):
 print('+',' '.join(map(str,cmd)));subprocess.run(list(map(str,cmd)),check=True,env=env)
def main():
 ap=argparse.ArgumentParser();ap.add_argument('installer');ap.add_argument('--skip-smoke',action='store_true');a=ap.parse_args()
 root=Path(__file__).resolve().parents[1]; os.chdir(root); env={**os.environ,'PYTHONPATH':str(root)}
 run([sys.executable,'tools/extract_installer.py',a.installer],env)
 run([sys.executable,'tools/analyze.py','work/game/mcm2.exe','--out','analysis','--skeleton-root','src/krusty2'],env)
 run([sys.executable,'tools/build_class_evidence.py'],env)
 run([sys.executable,'tools/analyze_msvc_artifacts.py'],env)
 run([sys.executable,'tools/find_vtable_writes.py'],env)
 if not a.skip_smoke:
  if shutil.which('clang-cl'):
   with open('analysis/smoke_test_clang.json','w') as f:
    print('+ clang-cl smoke tests -> analysis/smoke_test_clang.json')
    subprocess.run([sys.executable,'tools/run_samples.py','--exe','work/game/mcm2.exe','--compiler','clang-cl'],check=True,env=env,stdout=f)
   with open('analysis/easy_probe_clang.json','w') as f:
    print('+ generated high-confidence easy-probe gate -> analysis/easy_probe_clang.json')
    subprocess.run([sys.executable,'tools/run_easy_probes.py','--exe','work/game/mcm2.exe','--compiler','clang-cl'],check=True,env=env,stdout=f)
   with open('analysis/calibration_clang.json','w') as f:
    print('+ clang-cl calibration baseline -> analysis/calibration_clang.json')
    subprocess.run([sys.executable,'tools/run_calibration.py','--exe','work/game/mcm2.exe','--compiler','clang-cl'],check=True,env=env,stdout=f)
  else: print('clang-cl not found; smoke/calibration tests skipped (VC6 path remains available).')
 run([sys.executable,'tools/build_function_manifest.py'],env)
 run([sys.executable,'tools/build_class_dossiers.py'],env)
 run([sys.executable,'tools/build_work_queue.py'],env)
 print('\nBootstrap complete. Start with analysis/fingerprint.json, analysis/rtti_classes.json, analysis/vtables.json, analysis/class_dossiers.json, analysis/function_manifest.json, analysis/work_queue.json, and samples/.')
if __name__=='__main__':main()
