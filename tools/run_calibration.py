#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, subprocess, sys, tempfile
from pathlib import Path

CASES = [
    {
        'name': 'BaseObject::BaseObject constructor',
        'bindings': 'src/reconstructed/BaseObject.bindings.json',
        'source': 'src/reconstructed/BaseObject.cpp',
        'symbol': '??0BaseObject',
        'target_va': '0x00405120',
        'target_size': 16,
        'reason': 'constructor-body assignment preserves the retail vptr-before-field store order; vtable relocation is resolved',
    },
    {
        'name': 'BaseObject scalar deleting destructor',
        'bindings': 'src/reconstructed/BaseObject.bindings.json',
        'source': 'src/reconstructed/BaseObject.cpp',
        'symbol': '??_GBaseObject',
        'target_va': '0x00405130',
        'target_size': 30,
        'reason': 'canonical VC6 wrapper; CodeView supplies its length, both direct-call relocations are resolved',
    },
    {
        'name': 'BaseObject::~BaseObject destructor core',
        'bindings': 'src/reconstructed/BaseObject.bindings.json',
        'source': 'src/reconstructed/BaseObject.cpp',
        'symbol': '??1BaseObject',
        'target_va': '0x00405150',
        'target_size': 7,
        'reason': 'retail VC6 writes the BaseObject vptr in the destructor core while modern clang optimizes the empty body to ret',
    },
    {
        'name': 'BaseObject::Release',
        'source': 'src/reconstructed/BaseObject.cpp',
        'symbol': 'Release@BaseObject',
        'target_va': '0x00405170',
        'target_size': 32,
        'reason': 'local remaining count across virtual deletion; exact with VC6 /O2 without /G6',
    },
    {
        'name': 'UIControl::slot61 field difference',
        'source': 'samples/calibration/CompilerShapeProbe.cpp',
        'symbol': 'UnknownVirtualSlot61@UIControl',
        'target_va': '0x00470410',
        'target_size': 9,
        'reason': 'retail VC6 uses a second register for RHS; clang folds RHS into sub memory operand',
    },
    {
        'name': 'UIControl::slot62 field difference',
        'source': 'samples/calibration/CompilerShapeProbe.cpp',
        'symbol': 'UnknownVirtualSlot62@UIControl',
        'target_va': '0x00470420',
        'target_size': 9,
        'reason': 'same codegen discriminator with adjacent layout offsets',
    },
    {
        'name': 'UIStatic::slot30 return zero',
        'source': 'samples/calibration/CompilerShapeProbe.cpp',
        'symbol': 'UnknownVirtualSlot30@UIStatic',
        'target_va': '0x00478fe0',
        'target_size': 3,
        'reason': 'retail uses 33 C0; clang uses equivalent 31 C0, making this a tiny encoder fingerprint',
    },
    {
        'name': 'UIMultiState::slot34 indexed field +8',
        'source': 'samples/calibration/CompilerShapeProbe.cpp',
        'symbol': 'UnknownVirtualSlot34@UIMultiState',
        'target_va': '0x00478260',
        'target_size': 20,
        'reason': 'retail and clang agree on semantics/size but choose opposite equivalent SIB base/index ordering',
    },
    {
        'name': 'UIMultiState::slot35 indexed field +12',
        'source': 'samples/calibration/CompilerShapeProbe.cpp',
        'symbol': 'UnknownVirtualSlot35@UIMultiState',
        'target_va': '0x00478280',
        'target_size': 20,
        'reason': 'same VC6-vs-clang SIB discriminator; exposes 32-byte element stride',
    },
    {
        'name': 'UIMultiState::slot36 indexed field +28',
        'source': 'samples/calibration/CompilerShapeProbe.cpp',
        'symbol': 'UnknownVirtualSlot36@UIMultiState',
        'target_va': '0x004782a0',
        'target_size': 20,
        'reason': 'same VC6-vs-clang SIB discriminator with a third element field',
    },
    {
        'name': 'UIMultiState::slot37 indexed address +20',
        'source': 'samples/calibration/CompilerShapeProbe.cpp',
        'symbol': 'UnknownVirtualSlot37@UIMultiState',
        'target_va': '0x004782c0',
        'target_size': 20,
        'reason': 'retail VC6 uses LEA [eax+ecx+20], while clang separates add/add-immediate',
    },
    {
        'name': 'FollowCamera::slot63 zero camera preset',
        'bindings': 'src/reconstructed/FollowCamera.bindings.json',
        'source': 'src/reconstructed/FollowCamera.cpp',
        'symbol': 'UnknownVirtualSlot63@FollowCamera',
        'target_va': '0x00466c90',
        'target_size': 21,
        'reason': 'retail VC6 zeros EAX once and fans it out to three fields; clang emits repeated immediate-zero stores',
    },
    {
        'name': 'Camera::slot5 base call + owner sync',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/Camera.cpp',
        'symbol': 'UnknownVirtualSlot5@Camera',
        'target_va': '0x0042f050',
        'target_size': 32,
        'reason': 'calls GameObject slot 5 and the owner helper 0x4e8cf0, then stores owner+0x14 plus one at +0x1d0',
    },
    {
        'name': 'Camera::slot13 rectangle + refresh',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/Camera.cpp',
        'symbol': 'UnknownVirtualSlot13@Camera',
        'target_va': '0x0042e630',
        'target_size': 92,
        'reason': 'optional x/y/width/height rectangle to owner slot 12, then the non-virtual 0x42e8e0; /G6 schedules differently',
    },
    {
        'name': 'Camera::slot18 base call + counter',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/Camera.cpp',
        'symbol': 'UnknownVirtualSlot18@Camera',
        'target_va': '0x0042f070',
        'target_size': 28,
        'reason': 'GameObject slot 18 then +0x1d0 = owner+0x14 plus one; /G6 schedules differently',
    },
    {
        'name': 'Camera::slot30 64-byte block +0x2c',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/Camera.cpp',
        'symbol': 'UnknownVirtualSlot30@Camera',
        'target_va': '0x0042edd0',
        'target_size': 26,
        'reason': '16-dword struct copy (rep movsd) returning 1; /G6 orders the return load differently',
    },
    {
        'name': 'Camera::slot31 64-byte block +0xac',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/Camera.cpp',
        'symbol': 'UnknownVirtualSlot31@Camera',
        'target_va': '0x0042edf0',
        'target_size': 29,
        'reason': '16-dword struct copy returning 1',
    },
    {
        'name': 'Camera::slot32 64-byte block +0x6c',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/Camera.cpp',
        'symbol': 'UnknownVirtualSlot32@Camera',
        'target_va': '0x0042ee10',
        'target_size': 26,
        'reason': '16-dword struct copy returning 1',
    },
    {
        'name': 'PCCamera::slot13 re-send view/projection',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/PCCamera.cpp',
        'symbol': 'UnknownVirtualSlot13@PCCamera',
        'target_va': '0x004bee80',
        'target_size': 65,
        'reason': 'Camera slot 13, then interface method 11 with kinds 2 and 3 when owner+0x08 is this camera',
    },
    {
        'name': 'PCCamera::slot30 block + interface kind 1',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/PCCamera.cpp',
        'symbol': 'UnknownVirtualSlot30@PCCamera',
        'target_va': '0x004bedc0',
        'target_size': 54,
        'reason': 'Camera slot 30, then optional interface method 11 (COM-style __stdcall) with kind 1',
    },
    {
        'name': 'PCCamera::slot31 block + interface kind 2',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/PCCamera.cpp',
        'symbol': 'UnknownVirtualSlot31@PCCamera',
        'target_va': '0x004bee00',
        'target_size': 54,
        'reason': 'Camera slot 31, then optional interface method 11 with kind 2',
    },
    {
        'name': 'PCCamera::slot32 block + interface kind 3',
        'bindings': 'src/reconstructed/Camera.bindings.json',
        'source': 'src/reconstructed/PCCamera.cpp',
        'symbol': 'UnknownVirtualSlot32@PCCamera',
        'target_va': '0x004bee40',
        'target_size': 54,
        'reason': 'Camera slot 32, then optional interface method 11 with kind 3',
    },
    {
        'name': 'FollowCamera::slot68 distance-derived +0x258 parameter',
        'bindings': 'src/reconstructed/FollowCamera.bindings.json',
        'source': 'src/reconstructed/FollowCamera.cpp',
        'symbol': 'UnknownVirtualSlot68@FollowCamera',
        'target_va': '0x00466d50',
        'target_size': 241,
        'reason': 'x87 distance/clamp path; every float constant binding is checked against the retail literal and the call resolves to 0x460b50',
    },
    {
        'name': 'FollowCamera::slot69 cached 12-byte aggregate',
        'bindings': 'src/reconstructed/FollowCamera.bindings.json',
        'source': 'src/reconstructed/FollowCamera.cpp',
        'symbol': 'UnknownVirtualSlot69@FollowCamera',
        'target_va': '0x00466a80',
        'target_size': 65,
        'reason': 'hidden 12-byte return-buffer ABI plus three-dword cache copy; clang /GS- is 64 bytes but schedules copy/registers differently',
    },
    {
        'name': 'FollowCamera::slot70 mode/state toggle',
        'bindings': 'src/reconstructed/FollowCamera.bindings.json',
        'source': 'src/reconstructed/FollowCamera.cpp',
        'symbol': 'UnknownVirtualSlot70@FollowCamera',
        'target_va': '0x00467040',
        'target_size': 101,
        'reason': 'strongly reconstructed state/save/restore flow; modern clang chooses different register allocation and branch shape',
    },
    {
        'name': 'FollowCamera::slot71 state dispatcher',
        'bindings': 'src/reconstructed/FollowCamera.bindings.json',
        'source': 'src/reconstructed/FollowCamera.cpp',
        'symbol': 'UnknownVirtualSlot71@FollowCamera',
        'target_va': '0x00466e50',
        'target_size': 192,
        'reason': 'state 0-4 virtual dispatch plus camera-preset snapshot is strongly reconstructed; clang /GS- emits 151 bytes with different VC6-era switch/register scheduling. Extent is 171 code bytes + 1 alignment NOP + the 5-entry jump table (retail entries 0x466e77..0x466eb3 all point into this function)',
    },
    {
        'name': 'FollowCamera::slot72 cyclic state-list advance',
        'bindings': 'src/reconstructed/FollowCamera.bindings.json',
        'source': 'src/reconstructed/FollowCamera.cpp',
        'symbol': 'UnknownVirtualSlot72@FollowCamera',
        'target_va': '0x00466fb0',
        'target_size': 62,
        'reason': 'retail keeps explicit increment/store/compare/wrap; modern P6 clang uses CMOV and Pentium clang still folds more aggressively',
    },
]

def main():
    ap=argparse.ArgumentParser(description='Run compiler-calibration targets. Non-exact results are data, not failure.')
    ap.add_argument('--exe',default='work/game/mcm2.exe')
    ap.add_argument('--compiler',choices=['clang-cl','vc6'],default='clang-cl')
    ap.add_argument('--vc6-root',default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--profile')
    args=ap.parse_args()
    rows=[]
    with tempfile.TemporaryDirectory() as td:
        for i,c in enumerate(CASES):
            obj=Path(td)/f'cal{i}.obj'
            cmd=[sys.executable,'tools/compile.py',c['source'],'-o',str(obj),'--compiler',args.compiler]
            if args.profile: cmd += ['--profile',args.profile]
            if args.compiler=='vc6':
                if not args.vc6_root: raise SystemExit('VC6 run requested: set VC6_ROOT or pass --vc6-root')
                cmd += ['--vc6-root',args.vc6_root]
            cr=subprocess.run(cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            if cr.returncode:
                rows.append({**c,'compile_error':cr.stdout}); continue
            match_cmd=[sys.executable,'tools/match.py','--exe',args.exe,'--target-va',c['target_va'],'--target-size',str(c['target_size']),'--obj',str(obj),'--symbol',c['symbol'],'--json']
            if c.get('bindings'): match_cmd += ['--bindings',c['bindings']]
            mr=subprocess.run(match_cmd,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            try: result=json.loads(mr.stdout)
            except Exception: result={'match_error':mr.stdout}
            rows.append({**c,'compiler':args.compiler,'result':result})
    print(json.dumps(rows,indent=2))

if __name__=='__main__': main()
