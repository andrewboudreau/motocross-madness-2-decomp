"""Loader-zero quaternion storage and real normalized/reset consumers."""
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci,find_vc6_bin,infer_vc98_root
from tools.build_game import ROOT as TOOL_ROOT
from tools.match import compare
from tools.run_calibration import CASES
from test_edit_box_on_char import extract
import test_eco_quadtree_connections as views

ROOT=Path(__file__).resolve().parents[1]
STATE='src/reconstructed/QuatState.cpp'
QUAT='src/reconstructed/QuatUtil.cpp'
RIGID='samples/physics/rigidbody/PhysicsRigidBody.cpp'
VALUE='?g_QuatIdentity@@3UQuat@@A'
NORMALIZE='?QuatNormalize@@YA?AUQuat@@ABU1@@Z'
RESET='?ResetState@PhysicsRigidBody@@UAEXXZ'
FILENAMES=(QUAT,RIGID,'src/reconstructed/Math3D.cpp')

def whole(obj):
    with patch.object(views,'FILENAMES',FILENAMES):
        return views.whole_object(obj,(ROOT,))

def fingerprint(rows):
    return dict(sections=len(rows),relocations=sum(len(row[-1])for row in rows),
                sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest())

def function(obj,name):
    entry=obj.find_symbol(name);raw,size,relocs=obj.symbol_extent(entry)
    return raw,size,[(r.virtual_address-entry.value,r.type,obj.symbol_by_index[r.symbol_index].name)for r in relocs]

@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'authentic native VC6 and pinned retail required')
class QuaternionStorageTests(unittest.TestCase):
    def test_zero_storage_actual_consumers_and_mutable_shared_reads(self):
        import capstone
        image=Path(os.environ['MCM2_EXE']).resolve();pe=PEImage(image)
        self.assertEqual(hashlib.sha256(pe.data).hexdigest(),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        address=0x6850b0;section=pe.section_for_rva(address-pe.image_base)
        self.assertEqual(section.name,'.data');self.assertTrue(section.characteristics&0x80000000)
        self.assertGreaterEqual(address-pe.image_base,section.virtual_address+section.raw_size)
        self.assertLessEqual(address+16-pe.image_base,section.virtual_address+section.virtual_size)
        # The fixed retail image has stripped relocations. Absence of relocation
        # records is not offered as evidence of absence of indirect writes.
        self.assertTrue(pe.characteristics&1);self.assertEqual(pe.directories[5],(0,0))
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        def decoded(va,size):
            instructions=list(decoder.disasm(pe.bytes_at_va(va,size),va))
            self.assertEqual(sum(i.size for i in instructions),size)
            return instructions
        loads=[]
        table=0x556ec0
        self.assertEqual(struct.unpack('<I',pe.bytes_at_va(table+44*4,4))[0],0x4cc500)
        col=struct.unpack('<I',pe.bytes_at_va(table-4,4))[0]
        self.assertEqual(struct.unpack('<I',pe.bytes_at_va(col+4,4))[0],0)
        descriptor=struct.unpack('<I',pe.bytes_at_va(col+12,4))[0]
        self.assertEqual(pe.bytes_at_va(descriptor+8,64).split(b'\0')[0],b'.?AVPhysicsRigidBody@@')
        for va,size in ((0x4a1d60,173),(0x4cc500,256)):
            for instruction in decoded(va,size):
                for operand in instruction.operands:
                    if operand.type==capstone.x86.X86_OP_MEM and address<=operand.mem.disp<address+16:
                        self.assertEqual((operand.mem.base,operand.mem.index,operand.size,operand.access),(0,0,4,1))
                        loads.append((instruction.address,operand.mem.disp))
        self.assertEqual(loads,[(0x4a1da0,address),(0x4a1da6,address+4),(0x4a1dac,address+8),(0x4a1db2,address+12),
                               (0x4cc5b8,address),(0x4cc5c5,address+4),(0x4cc5ce,address+8),(0x4cc5d7,address+12)])
        literals=[]
        for item in pe.sections:
            raw=pe.data[item.raw_offset:item.raw_offset+item.raw_size]
            for delta in range(16):
                token=struct.pack('<I',address+delta);at=raw.find(token)
                while at>=0:
                    literals.append((pe.image_base+item.virtual_address+at,address+delta));at=raw.find(token,at+1)
        self.assertEqual(sorted(literals),sorted([(a+2,v)for a,v in loads]+[(0x44015e,address+2)]))
        lod={i.address:(i.mnemonic,i.op_str,i.bytes.hex())for i in decoded(0x440060,1964)}
        self.assertEqual(lod[0x44015c][:2],('lea','eax, [edx + esi*4]'))
        self.assertEqual(lod[0x44015f][:2],('push','eax'))
        self.assertEqual(lod[0x440160][:2],('push','0xbf800000'))
        # These literal hits span three instructions; they are not a ninth
        # address-taking operation. No target pointer is stored in PE data.
        self.assertEqual(pe.bytes_at_va(0x44015e,4),struct.pack('<I',address+2))
        xcu=struct.unpack('<352I',pe.bytes_at_va(0x566004,352*4))
        self.assertEqual(xcu[179:184],(0x4a2210,0x4a2260,0x4a22b0,0x4a2300,0x4a2ac0))
        writes=[]
        for va in (0x4a2220,0x4a2270,0x4a22c0,0x4a2310):
            for instruction in decoded(va,60):
                for operand in instruction.operands:
                    if operand.type==capstone.x86.X86_OP_MEM and operand.access&2 and not operand.mem.base and not operand.mem.index:
                        writes.append((operand.mem.disp,operand.size))
        self.assertEqual(sorted(writes),[(p,4)for base in (0x685070,0x685080,0x685090,0x6850a0)for p in range(base,base+12,4)])

        with tempfile.TemporaryDirectory(prefix='mcm2-quaternion-storage-')as directory:
            out=Path(directory);vc=Path(os.environ['VC6_ROOT']).resolve();objects={};paths={}
            def compile_source(source,key,includes=True):
                destination=out/(key+'.obj');command=[sys.executable,str(TOOL_ROOT/'tools/compile.py'),str(source),'-o',str(destination),'--vc6-root',str(vc),'--profile','vc6_o2_mt']
                if includes:command+=['--include',str(ROOT/'src/krusty2')]
                result=subprocess.run(command,capture_output=True,text=True,env={**os.environ,'PYTHONPATH':str(TOOL_ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                paths[key]=destination;objects[key]=CoffObject(destination);return objects[key]
            state=compile_source(ROOT/STATE,'state',False)
            nonempty=[s for s in state.sections if s.raw_size and not s.name.startswith('.debug') and s.name!='.drectve']
            self.assertEqual([(s.name,s.raw_size)for s in nonempty],[('.bss',16)])
            self.assertEqual(nonempty[0].characteristics,0xc0400080)
            self.assertEqual((state.find_symbol(VALUE).section_number,state.find_symbol(VALUE).value),(nonempty[0].index,0))
            self.assertFalse([r for r in state.relocations if not state.section(r.section_number).name.startswith('.debug')])
            self.assertEqual([s.name for s in state.symbols if s.storage_class==2],[VALUE])
            self.assertFalse(any(s.type==32 for s in state.symbols))
            self.assertFalse(any(s.name.startswith('.CRT')for s in state.sections))
            pins=json.loads((ROOT/'tests/fixtures/quaternion_storage_objects.json').read_text())
            for source,pin in pins['objects'].items():
                obj=compile_source(ROOT/source,Path(source).stem)
                self.assertEqual(fingerprint(whole(obj)),pin,source)
            full=objects['PhysicsRigidBody']
            original=(ROOT/RIGID).read_text();reset_source=out/'ResetState.cpp'
            reset_source.write_text('#include <string.h>\n#include "'+str(ROOT/'samples/physics/rigidbody/PhysicsBody.h').replace('\\','/')+'"\n'+extract(original,'void PhysicsRigidBody::ResetState()')+'\n')
            reset=compile_source(reset_source,'reset')
            self.assertEqual(function(reset,RESET),function(full,RESET))
            self.assertTrue(compare(pe,reset,RESET,0x4cc500,256,ROOT/'samples/physics/rigidbody/PhysicsRigidBody.bindings.json')['strict_exact'])
            strict=1
            for case in CASES:
                if case['source']!=QUAT:continue
                self.assertTrue(compare(pe,objects['QuatUtil'],case['symbol'],int(case['target_va'],16),case['target_size'],ROOT/case['bindings'])['strict_exact'],case)
                strict+=1
            # The extracted consumer's actual local zero vector is initialized
            # through the same complete strict code as the original sample.
            for va,size,name in ((0x4cca90,5,'_$E2'),(0x4ccaa0,60,'_$E1')):
                self.assertTrue(compare(pe,reset,name,va,size,ROOT/'samples/physics/rigidbody/PhysicsRigidBody.bindings.json')['strict_exact'])
                self.assertEqual(function(reset,name),function(full,name));strict+=1
            fixture=compile_source(ROOT/'tests/fixtures/quaternion_storage_compare.cpp','fixture')
            def execute(provider,label):
                exe=out/(label+'.exe');mapping=out/(label+'.map')
                command=[str(find_child_ci(find_vc6_bin(vc),'link.exe')),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref','/include:'+NORMALIZE,'/include:'+RESET,'/map:'+str(mapping),'/out:'+str(exe),'/libpath:'+str(find_child_ci(infer_vc98_root(vc),'lib')),str(provider),str(paths['QuatUtil']),str(paths['reset']),str(paths['fixture']),'kernel32.lib']
                result=subprocess.run(command,capture_output=True,text=True);self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                symbols={m[1]:m[2]for m in re.finditer(r'^\s+\S+:\S+\s+(\S+)\s+([0-9a-fA-F]{8})\s',mapping.read_text(),re.M)}
                return subprocess.run([str(exe),str(image),symbols[NORMALIZE],symbols[RESET]],capture_output=True,text=True,timeout=30)
            result=execute(paths['state'],'actual')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),'Quaternion storage: 2 initial, 10848 normalize, 128 reset, 156 guarded reset (148 AV) cases')
            control=out/'wrong-identity.cpp';control.write_text((ROOT/STATE).read_text().replace('Quat g_QuatIdentity;','Quat g_QuatIdentity = {1,0,0,0};'))
            compile_source(control,'wrong-identity',False)
            negative=execute(paths['wrong-identity'],'wrong-identity')
            self.assertEqual(negative.returncode,2,negative.stdout+negative.stderr)
            self.assertEqual(strict,14)
            print('14 strict checks; unchanged complete consumers; 11134 native cases; conventional-identity control rejected')

if __name__=='__main__':unittest.main()
