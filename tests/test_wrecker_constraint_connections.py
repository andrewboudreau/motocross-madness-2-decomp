"""Exact Wrecker constraint calls and finite actual probe/allocation behavior."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.resolved_match import match_object
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.build_game import ROOT as TOOL_ROOT
from tools.match import compare
from tools.run_calibration import CASES
from test_garage_shadow_connections import function_text
import test_eco_quadtree_connections as whole

ROOT = Path(__file__).resolve().parents[1]
CALLER = 'src/reconstructed/Wrecker.cpp'
SAMPLE = 'samples/race/WreckerNearMisses.cpp'
PROVIDER = 'src/krusty2/constraint/ConstraintMethodCollisionModel.cpp'
RENAMES = {
    '??0UnknownWreckerCollisionModel@@QAE@H@Z': '??0ConstraintMethodCollisionModel@@QAE@H@Z',
    '?SetBody@UnknownWreckerCollisionModel@@QAEXPAVUnknownWreckerRigidBody@@HPBD@Z':
        '?SetBody@ConstraintMethodCollisionModel@@QAEXPAVConBody@@HPBD@Z',
    '?AddProbePoint@UnknownWreckerCollisionModel@@QAEXUVector3@@H@Z':
        '?AddProbePoint@ConstraintMethodCollisionModel@@QAEXUConVec3@@PAVConNode@@@Z',
}
FILENAMES = (CALLER, SAMPLE, 'samples/race/../../src/reconstructed/Wrecker.cpp')
CRT_NAMES = {'_FixtureMalloc': '_malloc', '_FixtureFree': '_free', '_FixtureMsize': '__msize'}


def rewrite(value, names):
    if isinstance(value, str): return names.get(value, value)
    if isinstance(value, (list, tuple)): return [rewrite(v, names) for v in value]
    return value


def object_view(obj):
    with patch.object(whole, 'FILENAMES', FILENAMES):
        return whole.whole_object(obj, (ROOT,))


def pin(obj):
    rows = object_view(obj)
    return {'sections': len(rows), 'relocations': sum(len(r[-1]) for r in rows),
            'sha256': hashlib.sha256(json.dumps(rows, separators=(',', ':')).encode()).hexdigest()}


def function(obj, symbol, filenames=()):
    body, _, relocs = obj.symbol_extent(obj.find_symbol(symbol))
    records = []
    for reloc in relocs:
        target = obj.symbol_by_index[reloc.symbol_index]
        name = CRT_NAMES.get(target.name, target.name)
        if name.startswith('??_C@') and target.section_number > 0:
            sec = obj.section(target.section_number)
            value = obj.data[sec.raw_ptr + target.value:sec.raw_ptr + sec.raw_size].split(b'\0')[0]
            if value.decode('latin1').replace('\\', '/') in filenames:
                name = 'the-exact-enumerated-source-filename'
        records.append((reloc.virtual_address, reloc.type, name))
    return body, records


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail image')
class WreckerConstraintConnections(unittest.TestCase):
    def test_complete_bodies_receivers_and_actual_probe_append(self):
        import capstone
        exe = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(exe.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(exe)
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32); md.detail = True
        def decode(va, size):
            result = list(md.disasm(image.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in result), size)
            return result
        classes = {c.name: c for c in parse_rtti(image)}
        for name, table, col, offset in (
            ('ConstraintMethodCollisionModel', 0x551260, 0x55b0f8, 0),
            ('ConstraintMethodCollisionModel', 0x5511f0, 0x55b098, 12),
            ('PhysicsRigidBody', 0x556ec0, 0x55e558, 0),
            ('SoultreeObject', 0x557c18, 0x55f030, 0)):
            self.assertEqual(struct.unpack('<I', image.bytes_at_va(table-4, 4))[0], col)
            words = struct.unpack('<5I', image.bytes_at_va(col, 20))
            self.assertEqual(words[1], offset)
            self.assertEqual(image.bytes_at_va(words[3]+8, 80).split(b'\0')[0], ('.?AV'+name+'@@').encode())
        for cls, base, offset in (('ConstraintMethodCollisionModel', 'CollisionObject', 0),
                                 ('ConstraintMethodCollisionModel', 'GameObject', 12),
                                 ('PhysicsRigidBody', 'PhysicsBody', 0), ('SoultreeObject', 'GameObject', 12)):
            relation = next(b for b in classes[cls].bases if b.name == base)
            self.assertEqual((relation.mdisp, relation.pdisp), (offset, -1))
        instructions = {i.address: (i.mnemonic, i.op_str) for va, size in
                        ((0x5301c0,995), (0x532900,210), (0x43b8d0,121), (0x43b9e0,85),
                         (0x43c7f0,139), (0x4cc120,160), (0x4cbdf0,17)) for i in decode(va,size)}
        expected = {
            0x530276: ('push','0x2a8'), 0x530297: ('call','0x4cc120'),
            0x5302a3: ('mov','dword ptr [ebx + 0x40], eax'),
            0x4cc147: ('mov','dword ptr [esi], 0x556ec0'),
            0x5302c4: ('mov','edx, dword ptr [edx + 0x1a0]'),
            0x5302cc: ('push','edx'), 0x5302cd: ('call','dword ptr [eax + 0x70]'),
            0x4cbdf8: ('mov','dword ptr [ecx + 0x234], eax'),
            0x5302e3: ('push','0x10c'), 0x5302e8: ('call','0x4a3010'),
            0x530300: ('push','1'), 0x530302: ('mov','ecx, eax'), 0x530304: ('call','0x43b8d0'),
            0x530310: ('lea','ecx, [eax + 0xc]'), 0x530313: ('mov','dword ptr [ebx + 0x38], eax'),
            0x530324: ('mov','ecx, dword ptr [ebx + 0x40]'), 0x530328: ('push','1'),
            0x53032a: ('push','ecx'), 0x53032b: ('mov','ecx, dword ptr [ebx + 0x38]'),
            0x53032e: ('call','0x43b9e0'), 0x53033a: ('add','eax, 0xc'),
            0x43b920: ('mov','dword ptr [esi], 0x551260'), 0x43b926: ('mov','dword ptr [esi + 0xc], 0x5511f0'),
            0x43b9f1: ('mov','dword ptr [esi + 0xc4], eax'),
            0x43b9fd: ('mov','eax, dword ptr [eax + 0x234]'),
            0x43ba1c: ('mov','ecx, dword ptr [eax + 0x234]'),
            0x532900: ('mov','eax, dword ptr [esp + 0x10]'), 0x532909: ('push','eax'),
            0x532921: ('mov','ecx, dword ptr [esi + 0x38]'), 0x532924: ('call','0x43c7f0'),
            0x43c81a: ('mov','ecx, dword ptr [esp + 0x24]'),
            0x43c82a: ('mov','dword ptr [edx + eax + 0xc], ecx'),
            0x43c80a: ('shl','eax, 5'), 0x43c878: ('ret','0x10'),
        }
        for va, value in expected.items(): self.assertEqual(instructions[va], value, hex(va))
        self.assertEqual(struct.unpack('<I', image.bytes_at_va(0x556ec0+28*4,4))[0], 0x4cbdf0)
        bike = decode(0x409420,1516)
        find_indices = [n for n,i in enumerate(bike) if (i.mnemonic,i.op_str)==('call','0x4fdae0')]
        self.assertEqual(len(find_indices),15)
        self.assertEqual(sum((i.mnemonic,i.op_str)==('call','0x532900') for i in bike),15)
        # Each finder result is pushed unchanged as the node argument before any
        # EAX write. The legacy int parameter carries a pointer, not an index.
        for n in find_indices:
            j=n+1
            while (bike[j].mnemonic,bike[j].op_str)!=('push','eax'):
                self.assertNotIn(capstone.x86.X86_REG_EAX,bike[j].regs_access()[1]); j+=1
                self.assertLess(j,n+4)
        with tempfile.TemporaryDirectory(prefix='mcm2-wrecker-constraint-') as directory:
            out=Path(directory); objects={}; paths={}
            def compile_source(source,label,include_project=True):
                source=Path(source); source=source if source.is_absolute() else ROOT/source
                dest=out/(label+'.obj')
                command=[sys.executable,str(TOOL_ROOT/'tools/compile.py'),str(source),'-o',str(dest),
                    '--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt']
                if include_project:
                    command+=['--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')]
                run=subprocess.run(command,
                    cwd=TOOL_ROOT,capture_output=True,text=True,env={**os.environ,'PYTHONPATH':str(TOOL_ROOT)})
                self.assertEqual(run.returncode,0,run.stdout+run.stderr)
                objects[label]=CoffObject(dest);paths[label]=dest;return objects[label]
            for label,source in (('caller',CALLER),('sample',SAMPLE),('provider',PROVIDER),
                ('bike','src/krusty2/vehicle/Bike.cpp'),('rigid','samples/physics/rigidbody/PhysicsRigidBody.cpp'),
                ('body','samples/physics/rigidbody/PhysicsBody.cpp'),
                ('scene','src/krusty2/soultree/soultree.cpp'),('allocator','src/reconstructed/MemTag.cpp')):
                # These callers also belong to standalone calibration, where
                # their shared header must resolve without project include flags.
                compile_source(source,label,include_project=label not in ('caller','sample'))
            pins=json.loads((ROOT/'tests/fixtures/wrecker_constraint_objects.json').read_text())['objects']
            for label,source in (('caller',CALLER),('sample',SAMPLE)): self.assertEqual(pin(objects[label]),pins[source])
            binding=json.loads((ROOT/CALLER).with_suffix('.bindings.json').read_text())
            merged=out/'aggregate.json';binding.update(json.loads((ROOT/SAMPLE).with_suffix('.bindings.json').read_text()));merged.write_text(json.dumps(binding))
            strict=0
            for label in ('caller','sample'):
                for case in CASES:
                    if case['source']!=CALLER:continue
                    self.assertTrue(compare(image,objects[label],case['symbol'],int(case['target_va'],0),case['target_size'],merged)['strict_exact'],case)
                    strict+=1
            for case in json.loads((ROOT/'src/krusty2/constraint/targets.json').read_text()):
                self.assertTrue(compare(image,objects['provider'],case['candidate_symbol_contains'],int(case['target_va'],0),case['target_size'],(ROOT/PROVIDER).with_suffix('.bindings.json'))['strict_exact'],case);strict+=1
            for symbol,va,size,score,digest in (
                ('?PushProbesOutOfBox@Wrecker@@',0x5329e0,1312,1200,'ee990cbeffbb403f2238ce88038c256717237b874c57444ad28b5ed7ff53e2cb'),
                ('?SprayContactParticles@Wrecker@@',0x531740,1624,1606,'439fd80876fa804dde2e6522a6deaae3048455a89c2d1074ae21263310e408d6')):
                result=compare(image,objects['sample'],symbol,va,size,merged)
                self.assertEqual((result['candidate_size'],result['strict_match']['matching_positions']),(size,score))
                with patch('mcm2tool.resolved_match.compare_bytes',lambda original,candidate:{'digest':hashlib.sha256(candidate).hexdigest()}):
                    value=match_object(objects['sample'],symbol,va,b'',{k:int(v,0) for k,v in binding.items()})
                self.assertEqual(value['digest'],digest)
            for label,symbol,va,size,binding_path in (
                ('bike','?UnknownVirtualSlot97@Bike@@',0x409420,1516,'src/krusty2/vehicle/Bike.bindings.json'),
                ('rigid','??0PhysicsRigidBody@@',0x4cc120,160,'samples/physics/rigidbody/PhysicsRigidBody.bindings.json'),
                ('scene','?FindByName@SoultreeObject@@',0x4fdae0,84,'src/krusty2/soultree/soultree.bindings.json'),
                ('body','?SetNode@PhysicsBody@@',0x4cbdf0,17,'samples/physics/rigidbody/PhysicsBody.bindings.json')):
                self.assertTrue(compare(image,objects[label],symbol,va,size,ROOT/binding_path)['strict_exact'],symbol);strict+=1
            targets=set(RENAMES.values())
            for label in ('caller','sample'):
                imports=[objects[label].symbol_by_index[r.symbol_index].name for r in objects[label].relocations]
                self.assertTrue(set(RENAMES).isdisjoint(imports))
                for target in targets:self.assertEqual(imports.count(target),1)
            for target in targets:self.assertGreater(objects['provider'].find_symbol(target).section_number,0)
            for source,label,signature,symbol,va,size,file_va in (
                (CALLER,'append','void Wrecker::AddProbe(', '?AddProbe@Wrecker@@',0x532900,210,0x575ab8),
                (PROVIDER,'probe','void ConstraintMethodCollisionModel::AddProbePoint(', '?AddProbePoint@ConstraintMethodCollisionModel@@',0x43c7f0,139,0x568818)):
                src=out/(label+'.cpp');src.write_text('#include "'+str((ROOT/source).with_suffix('.h')).replace('\\','/')+'"\n#include "constraint/ConstraintMethodCollisionModel.h"\n#include "DebugAlloc.h"\n'+function_text((ROOT/source).read_text(),signature)+'\n')
                obj=compile_source(src,label)
                filenames=(str(src).replace('\\','/'),str(ROOT/source).replace('\\','/'))
                self.assertEqual(function(obj,symbol,filenames),function(objects['caller' if label=='append' else 'provider'],symbol,filenames))
                values=json.loads((ROOT/source).with_suffix('.bindings.json').read_text())
                values['__FILE__:'+src.name]=hex(file_va)
                for s in obj.symbols:
                    if s.name.startswith('??_C@') and s.section_number>0:
                        sec=obj.section(s.section_number);value=obj.data[sec.raw_ptr+s.value:sec.raw_ptr+sec.raw_size].split(b'\0')[0].decode('latin1').replace('\\','/')
                        if value==str(src).replace('\\','/'):values[s.name]=hex(file_va)
                bp=out/(label+'.json');bp.write_text(json.dumps(values))
                result=compare(image,obj,symbol,va,size,bp);self.assertTrue(result['strict_exact'],result);strict+=1
            boundary=out/'MemTagBoundary.cpp';boundary.write_text('#define malloc FixtureMalloc\n#define free FixtureFree\n#define _msize FixtureMsize\n#include "'+str(ROOT/'src/reconstructed/MemTag.cpp').replace('\\','/')+'"\n')
            wrapped=compile_source(boundary,'boundary')
            self.assertEqual(rewrite(object_view(wrapped),CRT_NAMES),rewrite(object_view(objects['allocator']),CRT_NAMES))
            for symbol,va,size in (('?DebugMalloc@@',0x4a2e20,57),('?DebugFree@@',0x4a2e60,96),('?DebugRealloc@@',0x4a2ec0,256),('?DebugMemcpy@@',0x4a3120,35)):
                self.assertTrue(compare(image,objects['allocator'],symbol,va,size,ROOT/'src/reconstructed/MemTag.bindings.json')['strict_exact'],symbol)
                self.assertEqual(function(objects['allocator'],symbol),function(wrapped,symbol));strict+=1
            compile_source('tests/fixtures/wrecker_constraint_probe.cpp','fixture')
            vc6=Path(os.environ['VC6_ROOT']);binary=out/'probe.exe'
            run=subprocess.run([str(find_child_ci(find_vc6_bin(vc6),'link.exe')),'/nologo','/subsystem:console',
                '/out:'+str(binary),'/libpath:'+str(find_child_ci(infer_vc98_root(vc6),'lib')),
                *(str(paths[k]) for k in ('append','probe','boundary','fixture')),'kernel32.lib'],cwd=TOOL_ROOT,capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=30)
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            self.assertEqual(run.stdout.strip(),'Wrecker probes: 2568 cases, 8 independently modeled access faults')
            self.assertEqual(strict,87)
            print('%d strict checks; two complete objects and partials; %s'%(strict,run.stdout.strip()))


if __name__=='__main__':unittest.main()
