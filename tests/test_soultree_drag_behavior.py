"""Complete physics slot 6, real table math, float state, aliases and AV ordering."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/physics/soultree_base/SoultreeDrag.cpp'
AGGREGATE = 'samples/physics/soultree_base/SoultreePhysicsInlines.cpp'
SYMBOL = '?UnknownVirtualSlot6@SoultreePhysicsBaseObject@@UAEXPAUVec3@@PAM@Z'
SLOT36 = '?UnknownVirtualSlot36@SoultreePhysicsBaseObject@@UAEXXZ'


def complete(obj, name):
    entry = obj.find_symbol(name)
    body, size, relocations = obj.symbol_extent(entry)
    return body, size, [(r.virtual_address-entry.value, r.type,
                        obj.symbol_by_index[r.symbol_index].name) for r in relocations]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and pinned private retail image')
class SoultreeDragBehaviorTests(unittest.TestCase):
    def test_complete_drag_with_actual_sqrt_and_faults(self):
        import capstone
        retail = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(retail.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(retail)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        code = list(decoder.disasm(image.bytes_at_va(0x40b410, 432), 0x40b410))
        self.assertEqual(sum(i.size for i in code), 432)
        self.assertEqual([(i.address, i.op_str) for i in code if i.mnemonic == 'ret'],
                         [(0x40b5bd, '8')])
        self.assertEqual([(i.address, i.op_str) for i in code if i.mnemonic == 'call'],
                         [(0x40b4b3, '0x460b50')])
        self.assertFalse(any('fs:' in i.op_str for i in code))
        boundaries = {i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertIn(i.operands[0].imm, boundaries)
        by_address = {i.address: (i.mnemonic, i.op_str) for i in code}
        # The pushed FastSqrt argument moves ESP by four. Its result replaces the
        # original float at ESP+8; the old sample incorrectly discarded this root.
        self.assertEqual(by_address[0x40b489], ('fstp', 'dword ptr [esp + 8]'))
        self.assertEqual(by_address[0x40b4b2], ('push', 'edx'))
        self.assertEqual(by_address[0x40b4b8], ('fst', 'dword ptr [esp + 0xc]'))
        self.assertEqual(by_address[0x40b4c2], ('add', 'esp, 4'))
        self.assertEqual(by_address[0x40b535], ('fld', 'dword ptr [esp + 8]'))
        self.assertEqual(by_address[0x40b542], ('fdiv', 'dword ptr [esp + 8]'))
        self.assertEqual(by_address[0x40b578], ('fld', 'dword ptr [esp + 8]'))

        tables = json.loads((ROOT/'analysis/vtables.json').read_text())
        for name, address in (('Bike',0x550900), ('KrustyBike',0x554548),
                              ('SoultreePhysicsBaseObject',0x557d90),
                              ('SoultreePhysicsObject',0x557f54),
                              ('SoultreePhysicsCharacter',0x5580a8), ('Vehicle',0x558a84)):
            table = next(t for t in tables if t['class'] == name and t['object_offset'] == 0)
            self.assertEqual(int(table['vtable_va'],0), address)
            self.assertEqual(int(table['entries'][6],0), 0x40b410)
            self.assertEqual(struct.unpack('<I',image.bytes_at_va(address+24,4))[0],0x40b410)
            locator = struct.unpack('<I',image.bytes_at_va(address-4,4))[0]
            self.assertEqual(locator,int(table['complete_object_locator_va'],0))
            self.assertEqual(struct.unpack('<I',image.bytes_at_va(locator+4,4))[0],0)
            descriptor = struct.unpack('<I',image.bytes_at_va(locator+12,4))[0]
            decorated = ('.?AV'+name+'@@\0').encode('ascii')
            self.assertEqual(image.bytes_at_va(descriptor+8,len(decorated)),decorated)
        for va, size, site, operand in ((0x502f60,1921,0x5030a0,'dword ptr [eax + 0x18]'),
                                         (0x52a940,3254,0x52ad5d,'dword ptr [edx + 0x18]')):
            caller = list(decoder.disasm(image.bytes_at_va(va,size),va))
            call = next(i for i in caller if i.address == site)
            self.assertEqual((call.mnemonic,call.op_str),('call',operand))

        compiler = Path(os.environ['VC6_ROOT'])
        with tempfile.TemporaryDirectory(prefix='mcm2-soultree-drag-') as directory:
            output = Path(directory)
            paths, objects = {}, {}
            def compile_source(source, name, fixture=False):
                obj = output/(name+'.obj')
                result = subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),
                    '-o',str(obj),'--vc6-root',str(compiler),'--profile','vc6_o2_mt',
                    '--include',str(ROOT/'src/krusty2'), *(['--extra','/Od'] if fixture else [])],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',
                    env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                paths[name], objects[name] = obj, CoffObject(obj)
            compile_source(SOURCE,'candidate')
            compile_source(AGGREGATE,'aggregate')
            compile_source('src/reconstructed/FastMath.cpp','math')
            compile_source('tests/fixtures/soultree_drag_compare.cpp','fixture',True)
            candidate = objects['candidate']
            for name in ('candidate','aggregate'):
                report = compare(image,objects[name],SYMBOL,0x40b410,432,
                                 ROOT/SOURCE.replace('.cpp','.bindings.json'))
                self.assertEqual((report['candidate_size'],report['strict_match']['matching_positions']),
                                 (440,126))
                self.assertFalse(report['strict_exact'])
            self.assertEqual(complete(candidate,SYMBOL),complete(objects['aggregate'],SYMBOL))
            body, size, relocations = complete(objects['aggregate'],SLOT36)
            self.assertEqual(size,305)
            self.assertEqual(hashlib.sha256(body).hexdigest(),
                             '9252482a15e869be2e3d671e145f1c5a9b1047f9fb1c62af77b9a486230c0d99')
            self.assertEqual(relocations,[(17,6,'__real@4@00000000000000000000'),
                (42,6,'__real@4@00000000000000000000'),(61,6,'__real@4@00000000000000000000'),
                (74,6,'__real@4@bfff8000000000000000'),(82,6,'__real@4@3fff8000000000000000'),
                (143,6,'?g_SoultreeVec3_685190@@3UVec3@@A'),
                (164,6,'?g_SoultreeVec3_685190@@3UVec3@@A'),
                (172,6,'?g_SoultreeVec3_685190@@3UVec3@@A')])
            self.assertEqual(hashlib.sha256(complete(candidate,SYMBOL)[0]).hexdigest(),
                             'adae6e8eabb616d3a7b66b9fb31e86f1fff84663f445c69181d1ef32af193048')
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},
                             {'__fltused','?FastSqrt@@YAMM@Z'})
            self.assertNotIn(SLOT36,{s.name for s in candidate.symbols})
            # The unchanged source header emits four local vector initializers.
            # They are compared with the aggregate, not assigned retail TU ownership.
            initializers = [s for s in candidate.symbols if s.name.startswith('_$E') and s.section_number>0]
            self.assertEqual(len(initializers),8)
            for entry in initializers:
                self.assertEqual(complete(candidate,entry.name),complete(objects['aggregate'],entry.name))
            crt = [s for s in candidate.sections if s.name == '.CRT$XCU']
            self.assertEqual(len(crt),1)
            self.assertEqual(crt[0].raw_size,16)
            self.assertEqual(len([r for r in candidate.relocations if r.section_number==crt[0].index]),4)
            bindings = json.loads((ROOT/SOURCE.replace('.cpp','.bindings.json')).read_text())
            for entry in candidate.symbols:
                if entry.name.startswith('__real@'):
                    section = candidate.section(entry.section_number)
                    self.assertFalse(section.characteristics&0x80000000)
                    self.assertEqual(candidate.data[section.raw_ptr+entry.value:section.raw_ptr+entry.value+4],
                                     image.bytes_at_va(int(bindings[entry.name],0),4))
            for symbol,va,size in (('?FastSqrt@@YAMM@Z',0x460b50,85),
                                    ('?InitFastSqrtTable@@YAXXZ',0x460ae0,97)):
                entry = objects['math'].find_symbol(symbol)
                self.assertEqual(entry.function_size,size)
                self.assertTrue(compare(image,objects['math'],symbol,va,size,
                                        ROOT/'src/reconstructed/FastMath.bindings.json')['strict_exact'])
            executable = output/'compare.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')),
                '/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',
                f'/libpath:{find_child_ci(infer_vc98_root(compiler),"lib")}',
                *[str(paths[name]) for name in ('candidate','math','fixture')],'kernel32.lib'],
                cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result = subprocess.run([str(executable),str(retail)],capture_output=True,text=True,timeout=60)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),
                'SoultreeDrag: 78305 real-math, alias, floating-point and fault scenarios match')
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
