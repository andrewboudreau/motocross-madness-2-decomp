#!/usr/bin/env python3
"""Recheck two reviewed class targets and generate normal-exit witnesses."""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mcm2tool.pe import PEImage
from mcm2tool.allocation import (ascii_at, decode_image, direct_call_target, hx,
                                 iat_symbols, imported_target, read_va, reviewed_body)
from mcm2tool.coff import CoffObject
from mcm2tool.msvc import scalar_deleting_destructor
from mcm2tool.rtti import parse_rtti
from mcm2tool.resolved_match import match_object
from mcm2tool.category_lifetimes import normal_exit_paths


def validate_range(pe, spec):
    start, size = int(spec['va'], 16), spec['size']
    if isinstance(size, bool) or not isinstance(size,int) or not 0 < size <= 65536:
        raise ValueError('invalid reviewed extent')
    if hashlib.sha256(read_va(pe,start,size)).hexdigest() != spec['sha256']:
        raise ValueError(f'reviewed range hash mismatch at {start:#x}')


def build_review(pe, config, instructions):
    if pe.machine != 0x14c or config.get('schema_version') != 1 or hashlib.sha256(pe.data).hexdigest()!=config['input_sha256']:
        raise ValueError('wrong build or configuration schema')
    for spec in config['boundaries'].values(): validate_range(pe,spec)
    select=int(config['boundaries']['select']['va'],16)
    restore=int(config['boundaries']['restore']['va'],16)
    classes={c.name:c for c in parse_rtti(pe)}
    rows=[]
    for spec in config['pilots']:
        validate_range(pe,spec)
        start=int(spec['va'],16)
        body=reviewed_body(instructions,start,start+spec['size'])
        for ins in body:
            if read_va(pe,ins.va,len(ins.raw))!=ins.raw:raise ValueError('disassembler byte mismatch')
        category=spec['category']
        if ascii_at(pe,int(category['literal_va'],16))!=category['label']:raise ValueError('category literal changed')
        if direct_call_target(instructions[int(category['call_va'],16)])!=select:raise ValueError('selector call changed')
        cls=classes[spec['class_name']]; vt=int(spec['primary_vtable'],16)
        if not any(vt in r.vtables and r.object_offset==0 for r in cls.vtable_records):raise ValueError('primary RTTI table changed')
        fn=struct.unpack('<I',read_va(pe,vt+4*spec['slot'],4))[0]
        evidence={'class_name':cls.name,'direct_bases':cls.direct_bases,'primary_vtable':hx(vt),'slot':spec['slot'],'entry':hx(fn)}
        if 'deleting_wrapper' in spec:
            wrapper=spec['deleting_wrapper'];validate_range(pe,wrapper)
            artifact=scalar_deleting_destructor(fn,read_va(pe,fn,30))
            if fn!=int(wrapper['va'],16) or not artifact or artifact['destructor_va']!=start:raise ValueError('destructor chain changed')
            evidence['deleting_wrapper_to_core']=hx(artifact['destructor_va'])
            base=spec['base_destructor'];validate_range(pe,base['wrapper'])
            bcls=classes[base['class_name']]
            bvt=next(v for r in bcls.vtable_records if r.object_offset==0 for v in r.vtables)
            bw=struct.unpack('<I',read_va(pe,bvt,4))[0]
            a=scalar_deleting_destructor(bw,read_va(pe,bw,30))
            if bw!=int(base['wrapper']['va'],16) or not a or a['destructor_va']!=int(base['core_va'],16):raise ValueError('base destructor evidence changed')
            if direct_call_target(instructions[int(base['call_va'],16)])!=a['destructor_va']:raise ValueError('base call changed')
            evidence['base_cleanup']=base
        elif fn!=start:raise ValueError('virtual slot target changed')
        source=[]
        for ins in body:
            operand=ins.operands.split(',')[-1].strip()
            if ins.mnemonic in ('push','mov') and operand.startswith('0x'):
                try:text=ascii_at(pe,int(operand,16))
                except ValueError:continue
                if text and text.lower().endswith(('.cpp','.h')):
                    source.append({'instruction_va':hx(ins.va),'path':text})
        lifecycle=normal_exit_paths(body,select,restore)
        sites=spec['saved_previous']
        observed=[instructions[int(sites[k],16)] for k in ('save_va','reload_va','restore_call_va')]
        rows.append({'spec':spec,'class_evidence':evidence,'source_references':source,'normal_cfg':lifecycle,
                     'saved_previous_observations':[{'va':hx(i.va),'mnemonic':i.mnemonic,'operands':i.operands} for i in observed],
                     'instruction_count':len(body),'hash_verified':True,
                     'instructions':[{'va':hx(i.va),'size':len(i.raw),'mnemonic':i.mnemonic,'operands':i.operands} for i in body],
                     'original_translation_unit':None,'full_function_byte_match':False})
    timer=config['timer'];validate_range(pe,timer)
    body=reviewed_body(instructions,int(timer['va'],16),int(timer['va'],16)+timer['size'])
    imports=iat_symbols(pe)
    api=[{'site_va':hx(i.va),**imported_target(i,imports)} for i in body if imported_target(i,imports)]
    if [r['name'] for r in api]!=[r['name'] for r in timer['imports']]:raise ValueError('timer import sequence changed')
    return {'schema_version':1,'input_sha256':config['input_sha256'],'pilots':rows,
            'timer':{'spec':timer,'import_calls':api,'probe_result':None},
            'limits':['Normal control-flow only: exceptions and callee effects are not modeled.',
                      'Branch outcomes are explored structurally, not solved symbolically.',
                      'Saved-value stack equivalence is a documented manual review, not an automatic proof.',
                      'Terrain is a normal-path model; the EcoSystem algorithm is reconstructed in src/reconstructed/EcoSystem.cpp.',
                      'No new original translation-unit or full-function byte match is claimed.']}


def compile_timer(pe,review,config):
    compiler=shutil.which('clang-cl')
    if not compiler:raise ValueError('clang-cl required by --compile-probe')
    spec=config['timer']; bindings={}
    for expected,actual in zip(spec['imports'],review['timer']['import_calls']):
        bindings[f"__imp__{expected['name']}@{expected['stack_bytes']}"]=int(actual['iat_va'],16)
    with tempfile.TemporaryDirectory() as temp:
        obj=Path(temp)/'timer.obj'
        flags=['--target=i686-pc-windows-msvc','/nologo','/c','/O2','/GR','/EHsc']
        subprocess.run([compiler,*flags,str(ROOT/spec['source']),'/Fo'+str(obj)],check=True,capture_output=True,text=True,timeout=45)
        va=int(spec['va'],16)
        result=match_object(CoffObject(obj),spec['symbol'],va,read_va(pe,va,spec['size']),bindings)
        result['compiler']=subprocess.run([compiler,'--version'],check=True,capture_output=True,text=True,timeout=10).stdout.strip()
        result['flags']=flags;result['historical_vc6']=False
        return result


def report(data):
    lines=['# Reviewed category pilot lifetimes','',f"Input SHA-256: `{data['input_sha256']}`",'',
           '## Normal exits','', '| Candidate | Return | Local boundary history |', '|---|---|---|']
    for row in data['pilots']:
        for e in row['normal_cfg']['normal_returns']:
            lines.append(f"| {row['spec']['name']} | `{e['return_va']}` | {e['local_history']} |")
    lines+=['','Terrain is verified through the RTTI slot-0 deleting wrapper to its destructor core. '
            'Its one normal return follows a local restore and a GameObject destructor call. '
            'EcoSystem slot 12 has an early return after category selection without a local restore. '
            'The latter is not by itself proof of a runtime bug or active-category state across unknown callees.','',
            '## Models','',
            'PilotModels.h reconstructs Terrain normal cleanup. '
            'The Terrain model excludes vptr/EH machinery; the EcoSystem algorithm is in samples/ecosystem/. '
            'Native tests do not execute the game.','', '## Timer helper','']
    for api in data['timer']['import_calls']:
        lines.append(f"- `{api['site_va']}` calls `{api['module']}!{api['name']}` through local IAT `{api['iat_va']}`.")
    r=data['timer']['probe_result']
    if r:
        lines+=['',f"Timer C++ probe: {r['matching_positions']}/{r['compared_positions']} positions; "
                f"retail {r['retail_size']} bytes, candidate {r['candidate_size']} bytes. "
                f"Strict exact: {r['strict_exact']}; ignored bytes: {r['ignored_bytes']}; "
                f"applied relocations: {len(r['relocations_applied'])}."]
    lines+=['','## Limits','',*['- '+s for s in data['limits']], '']
    return '\n'.join(lines)


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--exe',type=Path,default=ROOT/'work/game/mcm2.exe')
    ap.add_argument('--config',type=Path,default=ROOT/'config/category_pilots.json')
    ap.add_argument('--out',type=Path,default=ROOT/'work/category_pilots')
    ap.add_argument('--compile-probe',action='store_true')
    ap.add_argument('--objdump',default='objdump')
    a=ap.parse_args(); config=json.loads(a.config.read_text());pe=PEImage(a.exe)
    if hashlib.sha256(pe.data).hexdigest()!=config['input_sha256'] or pe.machine!=0x14c:raise ValueError('unsupported input build')
    instructions,decoder=decode_image(pe,a.objdump)
    data=build_review(pe,config,instructions);data['decoder']=decoder
    if a.compile_probe:data['timer']['probe_result']=compile_timer(pe,data,config)
    files=['mcm2tool/category_lifetimes.py','mcm2tool/category_contexts.py','mcm2tool/allocation.py',
           'mcm2tool/pe.py','mcm2tool/rtti.py','mcm2tool/msvc.py','mcm2tool/coff.py','mcm2tool/resolved_match.py',
           'tools/review_category_pilots.py','samples/category_pilots/PilotModels.h','samples/category_pilots/TimerProbe.cpp']
    data['tool_sha256']={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in files}
    data['config_sha256']=hashlib.sha256(a.config.read_bytes()).hexdigest()
    a.out.mkdir(parents=True,exist_ok=True)
    (a.out/'pilots.json').write_text(json.dumps(data,indent=2,sort_keys=True)+'\n')
    (a.out/'REPORT.md').write_text(report(data))
    print(report(data))


if __name__=='__main__':
    try:main()
    except (ValueError,OSError,KeyError,TypeError,struct.error,subprocess.SubprocessError) as exc:
        print(f'category-pilots: {exc}',file=sys.stderr)
        if isinstance(exc,subprocess.CalledProcessError) and exc.stderr:print(exc.stderr,file=sys.stderr)
        raise SystemExit(2)
