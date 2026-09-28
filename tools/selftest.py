#!/usr/bin/env python3
from __future__ import annotations
import hashlib,json,sys
from pathlib import Path

EXPECTED_SHA='31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874'

def load(path):
    p=Path(path)
    if not p.exists(): raise AssertionError(f'missing generated artifact: {path}')
    return json.loads(p.read_text())

def check(cond,msg):
    if not cond: raise AssertionError(msg)

def main():
    checks=[]
    def ok(msg): checks.append(msg)

    exe=Path('work/game/mcm2.exe')
    check(exe.exists(),'work/game/mcm2.exe missing; run make bootstrap INSTALLER=/path/to/MCM2PCG.exe')
    sha=hashlib.sha256(exe.read_bytes()).hexdigest()
    check(sha==EXPECTED_SHA,f'unexpected mcm2.exe SHA-256: {sha}')
    ok('retail executable SHA-256 matches known supplied build')

    fp=load('analysis/fingerprint.json'); check(fp.get('sha256')==EXPECTED_SHA,'fingerprint.json is stale or for another executable'); ok('fingerprint matches runtime executable')
    src=load('analysis/source_manifest.json'); check(sum(x.get('kind')=='cpp' for x in src)==107,'expected 107 recovered cpp filenames'); ok('107 cpp source-unit names recovered')
    td=load('analysis/rtti_type_descriptors.json'); check(len(td)==252,'expected 252 RTTI type descriptors'); ok('252 RTTI type descriptors')
    cls=load('analysis/rtti_classes.json'); check(len(cls)==249,'expected 249 logical RTTI class records'); ok('249 logical RTTI class records')
    vt=load('analysis/vtables.json'); check(len(vt)==271,'expected 271 concrete vtables'); check(sum(int(x.get('object_offset',0))!=0 for x in vt)==22,'expected 22 secondary vtables'); ok('271 vtables / 22 secondary vtables')
    dt=load('analysis/deleting_destructors.json'); check(len(dt)==145,'expected 145 scalar deleting destructors'); ok('145 scalar deleting-destructor wrappers')
    th=load('analysis/vtable_thunks.json'); check(len(th)==28,'expected 28 this-adjustor thunks'); ok('28 this-adjustor thunks')
    vw=load('analysis/vtable_write_xrefs.json'); check(len(vw)==492,'expected 492 vtable-write sites'); ok('492 vtable-write sites')
    easy=load('analysis/easy_targets.json'); check(len(easy)>=57,f'expected at least 57 easy targets, got {len(easy)}'); ok(f'{len(easy)} easy targets classified')
    layout=load('analysis/class_layout_hints.json'); check(len(layout)>=14,'expected layout evidence for at least 14 classes'); check(sum(len(x.get('fields',[])) for x in layout)>=39,'expected at least 39 direct field offsets'); ok('class layout evidence meets v0.6 floor')
    dossiers=load('analysis/class_dossiers.json'); check(len(dossiers)==249,'expected a dossier for each logical RTTI class'); ok('249 class dossiers generated')
    funcs=load('analysis/function_manifest.json'); check(len(funcs)>=56,'expected at least 56 function queue records'); ok(f'{len(funcs)} function manifest records')
    queue=load('analysis/work_queue.json'); check(len(queue.get('validated',[]))>=35,'expected at least 35 validated targets'); ok(f"{len(queue.get('validated',[]))} validated targets / {len(queue.get('next',[]))} next targets")

    smoke=load('analysis/smoke_test_clang.json'); check(smoke and all(x.get('exact_after_relocation_mask') for x in smoke),'manual clang smoke gate not all exact'); check(len(smoke)>=19,'expected at least 19 manual exact smoke samples'); ok(f'{len(smoke)}/{len(smoke)} manual clang smoke exact')
    ep=load('analysis/easy_probe_clang.json'); check(ep and all(x.get('exact_after_relocation_mask') for x in ep),'generated clang easy gate not all exact'); check(len(ep)>=39,'expected at least 39 generated exact probes'); ok(f'{len(ep)}/{len(ep)} generated clang probes exact')
    cal=load('analysis/calibration_clang.json'); check(len(cal)>=12,'expected at least 12 compiler calibration cases'); ok(f'{len(cal)} compiler calibration cases recorded')

    print('MCM2 bootstrap self-test: PASS')
    for c in checks: print('  [ok]',c)

if __name__=='__main__':
    try: main()
    except Exception as e:
        print('MCM2 bootstrap self-test: FAIL',file=sys.stderr)
        print(' ',e,file=sys.stderr)
        raise SystemExit(1)
