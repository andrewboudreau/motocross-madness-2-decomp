#!/usr/bin/env python3
from __future__ import annotations
import json
from collections import Counter
from pathlib import Path

def load(path):
    p = Path(path)
    return json.loads(p.read_text()) if p.exists() else None

def main():
    fp = load('analysis/fingerprint.json') or {}
    type_descriptors = load('analysis/rtti_type_descriptors.json') or []
    classes = load('analysis/rtti_classes.json') or []
    vtables = load('analysis/vtables.json') or []
    easy = load('analysis/easy_targets.json') or []
    smoke = load('analysis/smoke_test_clang.json') or []
    easy_probe = load('analysis/easy_probe_clang.json') or []
    calibration = load('analysis/calibration_clang.json') or []
    manifest = load('analysis/source_manifest.json') or []
    functions = load('analysis/function_manifest.json') or []
    dossiers = load('analysis/class_dossiers.json') or []
    work_queue = load('analysis/work_queue.json') or {}
    layout = load('analysis/class_layout_hints.json') or []
    multi = load('analysis/multiple_inheritance.json') or []
    vc6_gate = load('analysis/vc6_gate.json')
    dtors = load('analysis/deleting_destructors.json') or []
    thunks = load('analysis/vtable_thunks.json') or []
    vtwrites = load('analysis/vtable_write_xrefs.json') or []
    exact = [r for r in smoke if r.get('exact_after_relocation_mask')]
    easy_exact = [r for r in easy_probe if r.get('exact_after_relocation_mask')]
    workspace_ready = Path('work/game/mcm2.exe').exists()
    kinds = Counter(r['kind'] for r in easy)

    print('MCM2 decomp bootstrap status')
    print('---------------------------')
    print(f"workspace:       {'ready' if workspace_ready else 'NOT BOOTSTRAPPED'}")
    if not workspace_ready:
        print('                 run make bootstrap INSTALLER=/path/to/MCM2PCG.exe before gates')
    print(f"retail exe:      {fp.get('sha256','<not analyzed>')}")
    print(f"source units:    {sum(r.get('kind')=='cpp' for r in manifest)} cpp + {sum(r.get('kind') in ('h','hpp') for r in manifest)} headers")
    print(f"RTTI types:      {len(type_descriptors)}")
    print(f"RTTI hierarchies:{len(classes):>5}")
    print(f"vtables:         {len(vtables)} ({sum(int(v.get('object_offset',0))!=0 for v in vtables)} secondary subobject)")
    print(f"multi-vtable/MI: {len(multi)} classes")
    print(f"layout evidence: {len(layout)} classes / {sum(len(x.get('fields',[])) for x in layout)} direct fields")
    print(f"deleting dtors:  {len(dtors)} canonical VC6 wrappers")
    print(f"vtable thunks:   {len(thunks)} this-adjustor thunks")
    print(f"vtable writes:   {len(vtwrites)} code sites")
    print(f"easy targets:    {len(easy)} ({', '.join(f'{k}={v}' for k,v in sorted(kinds.items()))})")
    print(f"function manifest:{len(functions):>5} records")
    if dossiers: print(f"class dossiers:  {len(dossiers)} classes")
    if work_queue: print(f"agent queue:      {len(work_queue.get('validated',[]))} validated / {len(work_queue.get('next',[]))} next")
    print(f"clang manual:    {len(exact)}/{len(smoke)} exact")
    print(f"clang easy gate: {len(easy_exact)}/{len(easy_probe)} exact generated probes")
    unique_exact = {r.get('target_va') for r in exact + easy_exact if r.get('target_va')}
    print(f"unique exact:    {len(unique_exact)} retail functions across clang gates")
    if calibration:
        print('\nclang calibration baseline:')
        for c in calibration:
            r=c.get('result',{})
            print(f"  {c['name']}: {r.get('match_percent','?')}% bytes, target={r.get('target_size','?')}B candidate={r.get('candidate_size','?')}B")
    print(f"\nVC6 gate:         {'present' if vc6_gate else 'not run (private VC6 SP3 tree not present)'}")
    print('VC6 SP3 remains the authoritative compiler oracle; clang smoke matches validate plumbing/layout only.')

if __name__ == '__main__':
    main()
