#!/usr/bin/env python3
from __future__ import annotations
import argparse, json
from collections import defaultdict
from pathlib import Path


def load(path, default):
    p=Path(path)
    return json.loads(p.read_text()) if p.exists() else default


def nearest_source_hints(source_xrefs, va: int, count=4):
    rows=[]
    for r in source_xrefs:
        for x in r.get('text_xrefs',[]):
            xv=int(x,16) if isinstance(x,str) else int(x)
            rows.append({'path':r['path'],'xref_va':f'0x{xv:08x}','delta':xv-va,'distance':abs(xv-va)})
    rows.sort(key=lambda r:(r['distance'],r['path']))
    return rows[:count]


def main():
    ap=argparse.ArgumentParser(description='Merge discovered targets, sample matches, calibration results, vtable uses and source-xref hints into one agent-friendly manifest.')
    ap.add_argument('--out',default='analysis/function_manifest.json')
    a=ap.parse_args()
    easy=load('analysis/easy_targets.json',[])
    smoke=load('analysis/smoke_test_clang.json',[])
    calibration=load('analysis/calibration_clang.json',[])
    easy_probe=load('analysis/easy_probe_clang.json',[])
    source_xrefs=load('analysis/source_xrefs.json',[])
    rows={}
    for e in easy:
        va=int(e['target_va'],16)
        rows[va]={
            'target_va':e['target_va'], 'target_size':e['target_size'], 'retail_bytes':e['bytes'],
            'kind':e['kind'], 'details':e.get('details',{}), 'compiler_stability':e.get('compiler_stability'),
            'vtable_uses':e.get('uses',[]), 'status':'discovered', 'clang_exact':None, 'vc6_exact':None,
        }
    for s in easy_probe:
        va=int(s['target_va'],16)
        r=rows.setdefault(va,{'target_va':s['target_va'],'target_size':s.get('target_size'),'status':'generated-probe','clang_exact':None,'vc6_exact':None})
        if s.get('exact_after_relocation_mask'):
            r['status']='clang-exact-generated'
        r['clang_exact']=bool(s.get('exact_after_relocation_mask'))
        r['generated_probe_symbol']=s.get('symbol')
        r['generated_probe_match_percent']=s.get('match_percent')
        r['generated_probe_relocations_masked']=s.get('relocations_masked')

    for s in smoke:
        va=int(s['target_va'],16)
        r=rows.setdefault(va,{'target_va':s['target_va'],'target_size':s.get('target_size'),'status':'sampled','clang_exact':None,'vc6_exact':None})
        r.update({
            'status':'clang-exact' if s.get('exact_after_relocation_mask') else 'clang-mismatch',
            'clang_exact':bool(s.get('exact_after_relocation_mask')),
            'candidate_size':s.get('candidate_size'), 'candidate_symbol':s.get('symbol'),
            'candidate_source':s.get('source'), 'sample_notes':s.get('expected',{}).get('notes'),
            'provisional_class':s.get('expected',{}).get('class'),
            'match_percent':s.get('match_percent'),
        })
    for c in calibration:
        rv=c.get('result',{}); va=int(c['target_va'],16)
        r=rows.setdefault(va,{'target_va':c['target_va'],'target_size':c['target_size'],'status':'calibration','clang_exact':None,'vc6_exact':None})
        r.update({
            'status':'calibration', 'calibration_name':c['name'], 'calibration_reason':c.get('reason'),
            'candidate_source':c.get('source'), 'candidate_symbol':rv.get('symbol'),
            'clang_exact':bool(rv.get('exact_after_relocation_mask')) if rv else None,
            'clang_match_percent':rv.get('match_percent'), 'candidate_size':rv.get('candidate_size'),
        })
    out=[]
    for va,r in sorted(rows.items()):
        r['source_xref_hints']=nearest_source_hints(source_xrefs,va)
        out.append(r)
    Path(a.out).write_text(json.dumps(out,indent=2)+'\n')
    print(f'wrote {len(out)} function records -> {a.out}')

if __name__=='__main__': main()
