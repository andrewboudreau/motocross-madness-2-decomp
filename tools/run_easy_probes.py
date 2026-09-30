#!/usr/bin/env python3
from __future__ import annotations
import argparse,json,os,subprocess,sys,tempfile
from pathlib import Path
from mcm2tool.pe import PEImage
from mcm2tool.coff import CoffObject, relocation_mask, alignment_padding

def run(cmd):
    r=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    if r.returncode:
        print(r.stdout,end='',file=sys.stderr); raise SystemExit(r.returncode)
    return r.stdout

def match_one(pe, obj, target):
    va=int(target['target_va'],16); size=int(target['target_size'])
    sym=obj.find_symbol(target['symbol_contains'])
    cand,csize,rels=obj.symbol_extent(sym)
    retail=pe.bytes_at_va(va,size)
    pad=alignment_padding(cand,len(retail),sym.value)
    if pad: cand=cand[:-pad]
    mask=relocation_mask(obj,sym,len(cand),rels)
    n=min(len(retail),len(cand)); mism=[]; comparable=0; matching=0
    for i in range(n):
        if mask[i]: continue
        comparable+=1
        if retail[i]==cand[i]: matching+=1
        elif len(mism)<64: mism.append({'offset':i,'target':retail[i],'candidate':cand[i]})
    exact=(len(retail)==len(cand) and matching==comparable)
    return {'target_va':f'0x{va:08x}','target_size':len(retail),'candidate_size':len(cand),'symbol':sym.name,'relocations_masked':sum(mask),'comparable_bytes':comparable,'matching_bytes':matching,'match_percent':round(100*matching/comparable,4) if comparable else 100.0,'exact_after_relocation_mask':exact,'alignment_padding_bytes':pad,'mismatches':mism,'probe':target}

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--exe',required=True)
    ap.add_argument('--compiler',choices=['clang-cl','vc6'],default='clang-cl')
    ap.add_argument('--vc6-root',default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--stability',default='high')
    ap.add_argument('--profile')
    a=ap.parse_args()
    run([sys.executable,'tools/generate_easy_probes.py','--stability',a.stability])
    manifest=json.loads(Path('generated/easy_probes.json').read_text())
    with tempfile.TemporaryDirectory() as td:
        obj_path=Path(td)/'easy_probes.obj'
        cmd=[sys.executable,'tools/compile.py','generated/easy_probes.cpp','-o',str(obj_path),'--compiler',a.compiler]
        if a.profile: cmd += ['--profile',a.profile]
        if a.compiler=='vc6':
            if not a.vc6_root: raise SystemExit('VC6 run requested: set VC6_ROOT or pass --vc6-root')
            cmd += ['--vc6-root',a.vc6_root]
        run(cmd)
        pe=PEImage(a.exe); obj=CoffObject(obj_path)
        results=[match_one(pe,obj,t) for t in manifest]
    print(json.dumps(results,indent=2))
    if not all(r.get('exact_after_relocation_mask') for r in results): raise SystemExit(1)
if __name__=='__main__': main()
