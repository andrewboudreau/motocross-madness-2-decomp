#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,os,subprocess,sys,tempfile
from pathlib import Path

def run(cmd):
 r=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
 if r.returncode:
  print(r.stdout,end=''); raise SystemExit(r.returncode)
 return r.stdout

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--exe',required=True);ap.add_argument('--compiler',choices=['clang-cl','vc6'],default='clang-cl');ap.add_argument('--vc6-root',default=os.environ.get('VC6_ROOT'));a=ap.parse_args()
 cases=[('samples/base_object/BaseObject.cpp','samples/base_object/targets.json'),('samples/game/GameProbe.cpp','samples/game/targets.json'),('samples/gameui/UIControlProbe.cpp','samples/gameui/targets.json'),('samples/dlgprocs/ConnectionInfoTypeProbe.cpp','samples/dlgprocs/targets.json'),('samples/pccontrol/PCKeyboardDeviceProbe.cpp','samples/pccontrol/targets.json'),('samples/physics/PhysicsBodyProbe.cpp','samples/physics/targets.json'),('samples/quadtree/QuadTreeObjectProbe.cpp','samples/quadtree/targets.json'),('samples/camera/FollowCameraProbe.cpp','samples/camera/targets.json')]
 results=[]
 with tempfile.TemporaryDirectory() as td:
  for idx,(src,tjson) in enumerate(cases):
   obj=Path(td)/f'sample{idx}.obj'
   cmd=[sys.executable,'tools/compile.py',src,'-o',str(obj),'--compiler',a.compiler]
   if a.compiler=='vc6':
    if not a.vc6_root: raise SystemExit('VC6 run requested: set VC6_ROOT or pass --vc6-root')
    cmd += ['--vc6-root',a.vc6_root]
   run(cmd)
   for t in json.loads(Path(tjson).read_text()):
    m=[sys.executable,'tools/match.py','--exe',a.exe,'--target-va',t['target_va'],'--target-size',str(t['target_size']),'--obj',str(obj),'--symbol',t['candidate_symbol_contains'],'--json']
    r=subprocess.run(m,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    try: payload=json.loads(r.stdout)
    except Exception: payload={'error':r.stdout,'exact_after_relocation_mask':False}
    payload['source']=src; payload['expected']=t; results.append(payload)
 print(json.dumps(results,indent=2))
 if not all(r.get('exact_after_relocation_mask') for r in results): raise SystemExit(1)
if __name__=='__main__':main()
