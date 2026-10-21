"""Complete InlineShock setup, unchanged callers/aggregate and actual scene helpers."""
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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from test_steering_axis_from_points import signature, definition, HELPERS

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/InlineShockAxis.cpp'
BIKE = 'src/krusty2/vehicle/Bike.cpp'
SHOCK = 'samples/physics/suspension/Shock.cpp'
TREE = 'src/krusty2/soultree/soultree.cpp'
SYMBOL = '?UpdateAxis@InlineShock@@QAEXPAVShockNode@@0@Z'
RENAMES = {
 '?UpdateAxis@VehicleInlineShock@@QAEXPAVSoultreeObject@@0@Z': SYMBOL,
 '?GetPositionIn@ShockNode@@QAEXPAV1@PAUShockVec3@@@Z': HELPERS[0][0],
}


def fingerprint(obj, checkout, renames=None):
    """All nondebug payload/flags/destinations; normalize known source-file roots only."""
    renames = renames or {}
    prefix = str(checkout).replace('\\', '/') + '/'
    def filename(data):
        text, nul, tail = data.partition(b'\0')
        value = text.decode('latin1').replace('\\', '/')
        if nul and value.lower().startswith(prefix.lower()) and value[len(prefix):] in (BIKE, SHOCK):
            return ('source:' + value[len(prefix):]).encode() + nul + tail
        return data
    def name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.section(symbol.section_number)
            data = obj.data[section.raw_ptr+symbol.value:section.raw_ptr+section.raw_size]
            normalized = filename(data)
            if normalized != data:
                return normalized.split(b'\0')[0].decode()
        return renames.get(symbol.name, symbol.name)
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    keys, anonymous = {}, {}
    for section in sections:
        labels = tuple(sorted((name(s), s.value) for s in obj.symbols
                       if s.section_number == section.index and (s.storage_class == 2 or s.type == 32)))
        if labels:
            keys[section.index] = ('named', labels)
        else:
            count = anonymous.get(section.name, 0)
            anonymous[section.name] = count+1
            keys[section.index] = ('anonymous', section.name, count)
    assert len(set(keys.values())) == len(keys)
    def target(symbol):
        if symbol.section_number > 0:
            return ('defined', keys[symbol.section_number], symbol.value)
        return ('external', name(symbol), symbol.value, symbol.section_number)
    rows = []
    for section in sections:
        data = obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        relocs = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                  for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.characteristics, filename(data).hex(), relocs))
    rows.sort(key=lambda r: repr(r[0]))
    return dict(sections=len(rows), relocations=sum(len(r[3]) for r in rows),
                sha256=hashlib.sha256(json.dumps(rows).encode()).hexdigest())


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and the private retail image')
class InlineShockAxisTests(unittest.TestCase):
    def test_complete_method_callers_and_actual_cache_helpers(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        word = lambda va: struct.unpack('<I', retail.bytes_at_va(va,4))[0]
        self.assertEqual(word(0x557b90-4),0x55ef30)
        self.assertEqual(word(0x55ef30+4),0)
        self.assertEqual(retail.bytes_at_va(word(0x55ef30+12)+8,32).split(b'\0')[0],b'.?AVInlineShock@@')
        rtti = next(c for c in parse_rtti(retail) if c.name=='InlineShock')
        self.assertEqual([(b.name,b.mdisp,b.pdisp,b.vdisp) for b in rtti.bases],
                         [('InlineShock',0,-1,0),('Shock',0,-1,0),('MovingPart',4,-1,0)])
        self.assertEqual(word(word(0x557c18-4)+4),0)
        self.assertEqual(retail.bytes_at_va(word(word(0x557c18-4)+12)+8,40).split(b'\0')[0],
                         b'.?AVSoultreeObject@@')
        decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        decoder.detail = True
        def decode(va,size):
            ins = list(decoder.disasm(retail.bytes_at_va(va,size),va))
            self.assertEqual(sum(i.size for i in ins),size)
            return ins
        body = decode(0x4f9f90,250)
        self.assertEqual([(i.address,i.op_str) for i in body if i.mnemonic=='ret'],
                         [(0x4fa07e,'8'),(0x4fa087,'8')])
        self.assertEqual(retail.bytes_at_va(0x4fa08a,6),b'\x90'*6)
        self.assertEqual({i.address:i.op_str for i in body if i.mnemonic=='call'},
                         {0x4f9fa1:'0x4fc9a0',0x4f9fb1:'0x4fc9a0'})
        self.assertEqual(sum(i.mnemonic=='fsqrt' for i in body),1)
        for i in body:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertEqual(i.operands[0].type,capstone.x86.X86_OP_IMM)
                self.assertIn(i.operands[0].imm,{j.address for j in body})
        ctor = {i.address:(i.mnemonic,i.op_str) for i in decode(0x4f9ee0,153)}
        for va,expected in {
            0x4f9f1a:('mov','dword ptr [esi], 0x557b90'),
            0x4f9f20:('mov','dword ptr [esi + 0xcc], ecx'),
            0x4f9f26:('mov','dword ptr [esi + 0xd4], edx'),
            0x4f9f2c:('mov','dword ptr [esi + 0xd0], eax'),
            0x4f9f32:('lea','ecx, [esi + 0xc0]'),
            0x4f9f6d:('mov','eax, esi'),0x4f9f76:('ret','0x24'),
        }.items():self.assertEqual(ctor[va],expected)
        caller = {i.address:(i.mnemonic,i.op_str) for i in decode(0x4079c0,6745)}
        for va,expected in {
            0x408258:('push','0xd8'),0x40825d:('call','0x4a3010'),
            0x4082b1:('mov','ecx, eax'),0x4082b3:('call','0x4f9ee0'),
            0x4082bc:('push','eax'),0x4082ca:('mov','dword ptr [esp + 0x2c], eax'),
            0x4082ce:('call','0x525cb0'),
            0x408641:('push','eax'),0x408644:('push','0'),
            0x408646:('mov','edi, dword ptr [esp + 0x2c]'),
            0x40864c:('push','edi'),0x408652:('call','0x525d30'),
            0x408695:('mov','ecx, dword ptr [ebp + 0x60c]'),
            0x40869b:('mov','eax, dword ptr [esi + 0x1bc]'),
            0x4086a1:('mov','edx, dword ptr [ecx + 0x40]'),
            0x4086a4:('mov','ecx, edi'),0x4086a6:('push','edx'),
            0x4086a7:('push','eax'),0x4086a8:('call','0x4f9f90'),
        }.items():self.assertEqual(caller[va],expected)
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler),'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-inline-shock-') as directory:
            output = Path(directory)
            def compile_source(source,name):
                path = output/(name+'.obj')
                result = subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2')],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return path,CoffObject(path)
            provider_path,provider = compile_source(ROOT/SOURCE,'provider')
            _,bike = compile_source(ROOT/BIKE,'bike')
            _,shock = compile_source(ROOT/SHOCK,'shock')
            _,tree = compile_source(ROOT/TREE,'tree')
            pins = json.loads((ROOT/'tests/fixtures/inline_shock_objects.json').read_text())
            self.assertEqual(fingerprint(bike,ROOT),pins['objects'][BIKE])
            # Other sample methods still use the provisional GetPositionIn name.
            # Canonicalize both sides' target spelling only; all addends/sites remain pinned.
            self.assertEqual(fingerprint(shock,ROOT,RENAMES),pins['objects'][SHOCK])
            self.assertEqual({s.name for s in provider.symbols if s.storage_class==2 and s.section_number==0},
                             {'__fltused',HELPERS[0][0]})
            self.assertEqual({s.name for s in provider.symbols if s.type==32 and s.section_number>0},{SYMBOL})
            self.assertFalse(any(s.name in ('.bss','.CRT$XCU') for s in provider.sections))
            sections = [s for s in provider.sections if not s.name.startswith('.debug')]
            self.assertEqual([(s.name,s.raw_size,s.characteristics) for s in sections],
                             [('.drectve',40,0x100a00),('.text',256,0x60501020),('.rdata',4,0x40301040)])
            constant = provider.find_symbol('__real@4@3fff8000000000000000')
            section = provider.section(constant.section_number)
            self.assertEqual(provider.data[section.raw_ptr:section.raw_ptr+4],retail.bytes_at_va(0x550748,4))
            self.assertEqual(provider.data[section.raw_ptr:section.raw_ptr+4],struct.pack('<f',1.0))
            self.assertEqual(signature(provider,SYMBOL),signature(shock,SYMBOL))
            self.assertEqual(sum(n==SYMBOL for _,_,n in signature(bike,'?LoadBike@Bike@@')[2]),1)
            self.assertEqual([(o,n) for o,_,n in signature(provider,SYMBOL)[2] if n==HELPERS[0][0]],
                             [(18,HELPERS[0][0]),(34,HELPERS[0][0])])
            strict = 0
            for obj in (provider,shock):
                report = compare(retail,obj,SYMBOL,0x4f9f90,250,(ROOT/SOURCE).with_suffix('.bindings.json'))
                self.assertTrue(report['strict_exact'],report);strict += 1
            for folder,source,obj in [('samples/physics/suspension','Shock.cpp',shock),('src/krusty2/vehicle','Bike.cpp',bike)]:
                targets = json.loads((ROOT/folder/'targets.json').read_text())
                selected = [t for t in targets if t['source']==source and t['expect']=='exact']
                self.assertEqual(len(selected),27 if source=='Shock.cpp' else 52)
                for t in selected:
                    report = compare(retail,obj,t['candidate_symbol_contains'],int(t['target_va'],0),
                                     t['target_size'],ROOT/folder/t['bindings'])
                    self.assertTrue(report['strict_exact'],report);strict += 1
            starts = ['static inline void MatrixProduct(', 'static inline void TransposeRotation(',
                'static inline void InvertRigid(', 'static inline Vec3 TransformPoint(',
                'void SoultreeObject::UpdateWorldMatrix()', 'Vec3 SoultreeObject::WorldToLocalPoint(',
                'void SoultreeObject::GetPositionIn(']
            helper_source = output/'helpers.cpp'
            text = (ROOT/TREE).read_text()
            helper_source.write_text('#include "math/Math3D.h"\n'+'\n'.join(definition(text,s) for s in starts)+'\n')
            helper_path,helpers = compile_source(helper_source,'helpers')
            for symbol,va,size in HELPERS:
                for obj in (helpers,tree):
                    report = compare(retail,obj,symbol,va,size,(ROOT/TREE).with_suffix('.bindings.json'))
                    self.assertTrue(report['strict_exact'],report);strict += 1
                self.assertEqual(signature(helpers,symbol),signature(tree,symbol))
            fixture,_ = compile_source(ROOT/'tests/fixtures/inline_shock_axis_compare.cpp','fixture')
            executable = output/'inline-shock.exe'
            result = subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                f'/out:{executable}',f'/libpath:{libraries}',str(provider_path),str(helper_path),
                str(fixture),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result = subprocess.run([str(executable),str(image)],capture_output=True,text=True,timeout=120)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('InlineShockAxis: 149508 complete-state/cache/FP scenarios match',result.stdout)
            print('InlineShockAxis:',strict,'strict checks; two whole objects;',result.stdout.strip())


if __name__=='__main__':unittest.main()
