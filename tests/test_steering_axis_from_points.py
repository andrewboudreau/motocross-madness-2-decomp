"""Complete steering operation with real scene helpers, cache, FP and fault state."""
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

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.build_game import compile_source as build_compile_source
from test_vehicle_gearbox_connections import fingerprint as caller_fingerprint

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/physics/motion/SteeringAxisFromPoints.cpp'
AGGREGATE = 'samples/physics/motion/SteeringControl.cpp'
CANONICAL = 'src/krusty2/motion/SteeringControl.cpp'
TREE = 'src/krusty2/soultree/soultree.cpp'
TRANSFORM = 'samples/physics/helpers/SoultreeTransform.cpp'
SYMBOL = '?SetAxisFromPoints@SteeringControl@@QAEHPAVSoultreeObject@@00@Z'
HELPERS = (
 ('?GetPositionIn@SoultreeObject@@QAEXPAV1@PAUVec3@@@Z',0x4fc9a0,134),
 ('?UpdateWorldMatrix@SoultreeObject@@QAEXXZ',0x4fb4f0,858),
 ('?WorldToLocalPoint@SoultreeObject@@QAE?AUVec3@@ABU2@@Z',0x4fd7f0,285),
)


def signature(obj, query):
    symbol = obj.find_symbol(query)
    data, size, relocations = obj.symbol_extent(symbol)
    return data, size, [(r.virtual_address-symbol.value, r.type,
                        obj.symbol_by_index[r.symbol_index].name) for r in relocations]


def complete_object(obj):
    """Every nondebug byte/flag/destination; only a known checkout prefix varies."""
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    prefix = str(ROOT).replace('\\', '/')+'/'
    def filename(data):
        text, nul, tail = data.partition(b'\0')
        value = text.decode('latin1').replace('\\', '/')
        if nul and value.lower().startswith(prefix.lower()) and value[len(prefix):] == CANONICAL:
            return ('source:'+CANONICAL).encode()+nul+tail
        return data
    def name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.section(symbol.section_number)
            data = obj.data[section.raw_ptr+symbol.value:section.raw_ptr+section.raw_size]
            normalized = filename(data)
            if normalized != data:
                return normalized.split(b'\0')[0].decode()
        return symbol.name
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
        return ('external', symbol.name, symbol.value, symbol.section_number)
    rows = []
    for section in sections:
        data = obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        relocations = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                       for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.characteristics, filename(data).hex(), relocations))
    return sorted(rows, key=lambda r: repr(r[0]))


def definition(text, start):
    at = text.index(start)
    end, depth = text.index('{', at)+1, 1
    while depth:
        depth += (text[end] == '{')-(text[end] == '}')
        end += 1
    return text[at:end]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and hash-pinned retail')
class SteeringAxisFromPointsTests(unittest.TestCase):
    def test_complete_steering_real_cache_helpers_and_native_state(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        def decode(va, size):
            instructions = list(decoder.disasm(retail.bytes_at_va(va,size),va))
            self.assertEqual(sum(i.size for i in instructions),size)
            return instructions
        body = decode(0x504d30,234)
        self.assertEqual([(i.address,i.op_str) for i in body if i.mnemonic=='ret'],
                         [(0x504e0e,'0xc'),(0x504e17,'0xc')])
        self.assertEqual(retail.bytes_at_va(0x504e1a,6),b'\x90'*6)
        self.assertEqual({i.address:i.operands[0].imm for i in body if i.mnemonic=='call'},
                         {0x504d41:0x4fc9a0,0x504d51:0x4fc9a0,0x504ded:0x4fd710})
        boundaries = {i.address for i in body}
        for i in body:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertEqual(i.operands[0].type,capstone.x86.X86_OP_IMM)
                self.assertIn(i.operands[0].imm,boundaries)
            if i.mnemonic=='call':
                self.assertEqual(i.operands[0].type,capstone.x86.X86_OP_IMM)
        self.assertEqual(sum(i.mnemonic=='fsqrt' for i in body),1)
        self.assertFalse(any('fs:' in i.op_str for i in body))
        # Decode the full caller before selecting the receiver/argument preparation.
        caller = {i.address:(i.mnemonic,i.op_str) for i in decode(0x4079c0,6745)}
        for va, expected in {
            0x408b47:('mov','ecx, dword ptr [ebp + 0x42c]'),
            0x408b4d:('mov','edx, dword ptr [ebp + 0x60c]'),
            0x408b53:('push','ecx'),
            0x408b54:('mov','ecx, dword ptr [ebp + 0x5f0]'),
            0x408b5a:('mov','eax, dword ptr [edx + 0x40]'),
            0x408b5d:('mov','edx, dword ptr [ecx + 0x1bc]'),
            0x408b63:('mov','ecx, dword ptr [ebp + 0x47c]'),
            0x408b69:('push','eax'),0x408b6a:('push','edx'),
            0x408b6b:('call','0x504d30'),
        }.items():
            self.assertEqual(caller[va],expected)
        self.assertEqual(retail.bytes_at_va(0x5745ac,80).split(b'\0')[0],
                         b'D:\\aardvark\\VC\\krusty2\\SteeringControl.cpp')
        # Primary scene receiver identity; the GameObject secondary view is distinct.
        col, = struct.unpack('<I',retail.bytes_at_va(0x557c18-4,4))
        _,offset,_,td,_ = struct.unpack('<5I',retail.bytes_at_va(col,20))
        self.assertEqual(offset,0)
        self.assertEqual(retail.bytes_at_va(td+8,40).split(b'\0')[0],b'.?AVSoultreeObject@@')
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler),'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-steering-axis-') as directory:
            output = Path(directory)
            def compile_source(source,name):
                path = output/(name+'.obj')
                result = subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2')],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return path,CoffObject(path)
            candidate_path,candidate = compile_source(ROOT/SOURCE,'candidate')
            _,aggregate = compile_source(ROOT/AGGREGATE,'aggregate')
            _,canonical = compile_source(ROOT/CANONICAL,'canonical')
            _,tree = compile_source(ROOT/TREE,'tree')
            _,bike = compile_source(ROOT/'src/krusty2/vehicle/Bike.cpp','bike')
            caller_bytes,caller_size,caller_relocs = signature(bike,'?LoadBike@Bike@@')
            self.assertEqual(caller_size,6745)
            self.assertEqual(len(caller_relocs),219)
            self.assertEqual(hashlib.sha256(caller_bytes).hexdigest(),
                             '1dba68b1345a120c9f1ad806330cd4a2582f02905a47873db0063946f74a97e3')
            # The sole round50 change was $L7843 -> $L7844 at +22: the same
            # .text$x EH target at +208. Published and current complete objects
            # agree under the existing payload/destination-aware local identity.
            # Emitter construction joins preserve these bytes and exact targets;
            # only the three verified external constructor names change.
            self.assertEqual(caller_fingerprint(bike, '?LoadBike@Bike@@'),
                             '70f599f7d80e0211525484f6b33710483fa20d92edacf6d319d58ea726353220')
            self.assertEqual(sum(name==SYMBOL for _,_,name in caller_relocs),1)
            transform_path,transform = compile_source(ROOT/TRANSFORM,'transform')
            self.assertEqual(signature(candidate,SYMBOL),signature(aggregate,SYMBOL))
            entry = candidate.find_symbol(SYMBOL)
            self.assertEqual((entry.function_size,entry.extent_source),(230,'coff_function_aux'))
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},
                {'__fltused',HELPERS[0][0],'?WorldToLocalDirection@SoultreeObject@@QAE?AUVec3@@ABU2@@Z'})
            report = compare(retail,candidate,SYMBOL,0x504d30,234,(ROOT/SOURCE).with_suffix('.bindings.json'))
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'],145)
            # Ordinary Math3D.h private vector initializers remain; this is not original-TU proof.
            initializers = ['_$E2','_$E1','_$E5','_$E4','_$E8','_$E7','_$E11','_$E10']
            for name in initializers:
                self.assertEqual(signature(candidate,name),signature(aggregate,name))
            self.assertEqual({s.name for s in candidate.symbols if s.type==32 and s.section_number>0},
                             set(initializers)|{SYMBOL})
            rows = complete_object(aggregate)
            self.assertEqual((len(rows),sum(len(r[3]) for r in rows)),(32,90))
            self.assertEqual(hashlib.sha256(json.dumps(rows).encode()).hexdigest(),
                             'e531924a92d27f5b10d5c7ffb04c910d3346eaf0f90aabfee97511445d5470a1')
            spec = json.loads((ROOT/'config/game_build.json').read_text())
            built = build_compile_source(ROOT/SOURCE,output/'game-command',compiler,
                [ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(built['compiled'],Path(built['log']).read_text())
            self.assertEqual(complete_object(candidate),complete_object(CoffObject(Path(built['object']))))
            strict = 0
            targets = json.loads((ROOT/'src/krusty2/motion/targets.json').read_text())
            selected = [t for t in targets if t['source']=='SteeringControl.cpp' and t['expect']=='exact']
            self.assertEqual(len(selected),21)
            for target in selected:
                for obj in (canonical,aggregate):
                    result = compare(retail,obj,target['candidate_symbol_contains'],int(target['target_va'],0),
                                     target['target_size'],ROOT/'src/krusty2/motion'/target['bindings'])
                    self.assertTrue(result['strict_exact'],result);strict += 1
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
                    result = compare(retail,obj,symbol,va,size,(ROOT/TREE).with_suffix('.bindings.json'))
                    self.assertTrue(result['strict_exact'],result);strict += 1
                self.assertEqual(signature(helpers,symbol),signature(tree,symbol))
            direction = '?WorldToLocalDirection@SoultreeObject@@QAE?AUVec3@@ABU2@@Z'
            result = compare(retail,transform,direction,0x4fd710,209,(ROOT/TRANSFORM).with_suffix('.bindings.json'))
            self.assertEqual((result['candidate_size'],result['strict_match']['matching_positions']),(209,207))
            # No patching: the source links actual source helpers; the reference runs its own full helpers.
            fixture,_ = compile_source(ROOT/'tests/fixtures/steering_axis_from_points_compare.cpp','fixture')
            executable = output/'steering.exe'
            result = subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                f'/out:{executable}',f'/libpath:{libraries}',str(candidate_path),str(helper_path),str(transform_path),
                str(fixture),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result = subprocess.run([str(executable),str(image)],capture_output=True,text=True,timeout=120)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('SteeringAxis: 100852 full-state, cache and FP scenarios match',result.stdout)
            print('SteeringAxis:',strict,'strict checks;',result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
