#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, os, subprocess, sys, tempfile
from pathlib import Path

CASES = [
    {
        'name': 'BaseObject::BaseObject constructor',
        'source': 'samples/base_object/BaseObjectSpecialMembers.cpp',
        'symbol': '??0BaseObject',
        'target_va': '0x00405120',
        'target_size': 16,
        'reason': 'natural constructor initializes the BaseObject vptr and refCount=1; retail register/ModRM shape is VC6-specific',
    },
    {
        'name': 'BaseObject scalar deleting destructor',
        'source': 'samples/base_object/BaseObjectSpecialMembers.cpp',
        'symbol': '??_GBaseObject',
        'target_va': '0x00405130',
        'target_size': 30,
        'reason': 'canonical VC6 scalar deleting-destructor wrapper; modern clang uses different delete/flag codegen',
    },
    {
        'name': 'BaseObject::~BaseObject destructor core',
        'source': 'samples/base_object/BaseObjectSpecialMembers.cpp',
        'symbol': '??1BaseObject',
        'target_va': '0x00405150',
        'target_size': 7,
        'reason': 'retail VC6 writes the BaseObject vptr in the destructor core while modern clang optimizes the empty body to ret',
    },
    {
        'name': 'BaseObject::Release',
        'source': 'samples/base_object/BaseObjectReleaseCandidate.cpp',
        'symbol': 'Release@BaseObject',
        'target_va': '0x00405170',
        'target_size': 32,
        'reason': 'small nontrivial refcount/delete control flow; strong VC6 codegen discriminator',
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
        'source': 'samples/camera/FollowCameraProbe.cpp',
        'symbol': 'UnknownVirtualSlot63@FollowCamera',
        'target_va': '0x00466c90',
        'target_size': 21,
        'reason': 'retail VC6 zeros EAX once and fans it out to three fields; clang emits repeated immediate-zero stores',
    },
    {\n        'name': 'FollowCamera::slot69 cached 12-byte aggregate',\n        'source': 'samples/camera/FollowCameraProbe.cpp',\n        'symbol': 'UnknownVirtualSlot69@FollowCamera',\n        'target_va': '0x00466a80',\n        'target_size': 65,\n        'reason': 'hidden 12-byte return-buffer ABI plus three-dword cache copy; clang /GS- is 64 bytes but schedules copy/registers differently',\n    },\n    {
        'name': 'FollowCamera::slot70 mode/state toggle',
        'source': 'samples/camera/FollowCameraProbe.cpp',
        'symbol': 'UnknownVirtualSlot70@FollowCamera',
        'target_va': '0x00467040',
        'target_size': 101,
        'reason': 'strongly reconstructed state/save/restore flow; modern clang chooses different register allocation and branch shape',
    },
    {
        'name': 'FollowCamera::slot72 cyclic state-list advance',
        'source': 'samples/camera/FollowCameraProbe.cpp',
        'symbol': 'UnknownVirtualSlot72@FollowCamera',
        'target_va': '0x00466fb0',
        'target_size': 62,
        'reason': 'retail keeps explicit increment/store/compare/wrap; modern P6 clang uses CMOV and Pentium clang still folds more aggressively',
    },
    {
        'name': 'FollowCamera::slot68 bounded distance-derived parameter',
        'source': 'samples/camera/FollowCameraStateProbe.cpp',
        'symbol': 'UnknownVirtualSlot68@FollowCamera',
        'target_va': '0x00466d50',
        'target_size': 241,
        'reason': 'reconstructed 12-byte source value, planar distance/sqrt-like calculation, 10..70 clamp, and virtual slot-29 dispatch; clang x87/register scheduling differs from retail VC6',
    },
    {
        'name': 'FollowCamera::slot69 cached 12-byte virtual return',
        'source': 'samples/camera/FollowCameraStateProbe.cpp',
        'symbol': 'UnknownVirtualSlot69@FollowCamera',
        'target_va': '0x00466a80',
        'target_size': 65,
        'reason': 'derived slot-57 implementation confirms a 12-byte by-value return with one float argument; retail materializes a temporary then caches it at +0x2A8 before slot-43 dispatch',
    },
    {
        'name': 'FollowCamera::slot71 state dispatcher',
        'source': 'samples/camera/FollowCameraStateProbe.cpp',
        'symbol': 'UnknownVirtualSlot71@FollowCamera',
        'target_va': '0x00466e50',
        'target_size': 171,
        'reason': 'state switch, preset dispatch, +0x258 save/restore and +0x220/+0x22C/+0x234 snapshot flow are strongly reconstructed; clang folds the switch/virtual dispatch differently',
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
            mr=subprocess.run([sys.executable,'tools/match.py','--exe',args.exe,'--target-va',c['target_va'],'--target-size',str(c['target_size']),'--obj',str(obj),'--symbol',c['symbol'],'--json'],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            try: result=json.loads(mr.stdout)
            except Exception: result={'match_error':mr.stdout}
            rows.append({**c,'compiler':args.compiler,'result':result})
    print(json.dumps(rows,indent=2))

if __name__=='__main__': main()
