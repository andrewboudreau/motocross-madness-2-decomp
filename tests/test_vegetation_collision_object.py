"""Vegetation collision placement and its observable transform-sum correction."""
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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.build_game import compile_source as game_compile
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
CANDIDATE = 'samples/ecosystem/VegetationCollisionObject.cpp'
ECO = 'src/reconstructed/EcoSystem.cpp'
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
QUERY = 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp'
SYMBOL = '?GetCollisionObject@Vegetation@@QAEPAVCollisionObject@@H@Z'


def extract(text, signature):
    start = text.index(signature)
    brace = text.index('{', start)
    depth, end = 1, brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def function(obj, query):
    symbol = obj.find_symbol(query)
    raw, _, relocs = obj.symbol_extent(symbol)
    targets = []
    for rel in relocs:
        target = obj.symbol_by_index[rel.symbol_index]
        identity = ('local', target.value-symbol.value) if target.section_number == symbol.section_number else ('named', target.name, target.value)
        targets.append((rel.virtual_address-symbol.value, rel.type, identity))
    return raw, targets

OLD={}
def view(o, normalize_paths=()):
 sections=[s for s in o.sections if not s.name.startswith('.debug')]
 names={}
 def symbol_name(t):
  if t.name.startswith('??_C@') and t.section_number>0:
   sec=o.sections[t.section_number-1];v=o.data[sec.raw_ptr+t.value:sec.raw_ptr+sec.raw_size].split(b'\0')[0].decode('latin1').replace('\\','/')
   for prefix in [str(path).replace('\\','/') for path in normalize_paths]:
    if v.lower().startswith(prefix.lower()+'/'):return 'source:'+v[len(prefix)+1:]
  return names.get(t.name,t.name)
 keys={};anonymous={}
 for sec in sections:
  labels=tuple(sorted((symbol_name(t),t.value) for t in o.symbols if t.section_number==sec.index and (t.storage_class==2 or t.type==32 or t.name.startswith('_kCollisionPoint'))))
  if labels:keys[sec.index]=('named',labels)
  else:
   count=anonymous.get(sec.name,0);anonymous[sec.name]=count+1;keys[sec.index]=('anonymous',sec.name,count)
 assert len(set(keys.values()))==len(keys)
 bss=[s for s in sections if s.name=='.bss'];bsskey=keys[bss[0].index] if bss else None
 def target(t):
  if t.name in OLD and t.section_number==0:return ('defined',bsskey,OLD[t.name])
  if t.section_number>0:return ('defined',keys[t.section_number],t.value)
  return ('external',names.get(t.name,t.name),t.value,t.section_number)
 rows=[];exceptions=[]
 for sec in sections:
  data=o.data[sec.raw_ptr:sec.raw_ptr+sec.raw_size] if sec.raw_ptr else bytes(sec.raw_size)
  value,sep,tail=data.partition(b'\0');v=value.decode('latin1').replace('\\','/')
  for prefix in [str(path).replace('\\','/') for path in normalize_paths]:
   if sep and v.lower().startswith(prefix.lower()+'/'):
    suffix=v[len(prefix)+1:];data=('source:'+suffix).encode()+sep+tail;exceptions.append((keys[sec.index],suffix));break
  rels=[(r.virtual_address,r.type,target(o.symbol_by_index[r.symbol_index])) for r in o.relocations if r.section_number==sec.index]
  rows.append((keys[sec.index],sec.characteristics,data.hex(),rels))
 return sorted(rows,key=lambda r:repr(r[0])),exceptions

@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class VegetationCollisionTests(unittest.TestCase):
    def test_complete_method_transform_order_and_actual_math(self):
        import capstone
        exe = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(exe.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(exe)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x4570a0, 386), 0x4570a0))
        self.assertEqual(sum(i.size for i in instructions), 386)
        starts = {i.address for i in instructions}
        self.assertEqual({i.address: i.operands[0].imm for i in instructions if i.mnemonic == 'call'},
                         {0x45711c: 0x455ff0, 0x457131: 0x455f90, 0x457212: 0x435830})
        self.assertEqual([(i.address, i.op_str) for i in instructions if i.mnemonic == 'ret'],
                         [(0x457189, '4'), (0x4571c7, '4'), (0x45721f, '4')])
        for ins in instructions:
            if ins.mnemonic.startswith('j'):
                self.assertEqual(ins.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertIn(ins.operands[0].imm, starts)
        self.assertEqual(image.bytes_at_va(0x457222, 14), b'\x90'*14)
        classes = {c.name: c for c in parse_rtti(image)}
        self.assertTrue(any(b.name == 'QuadTreeObject' and b.mdisp == 0 and b.pdisp == -1
                            for b in classes['Vegetation'].bases))
        col = struct.unpack('<I', image.bytes_at_va(0x5524f4, 4))[0]
        self.assertEqual(col, 0x55bef0)
        words = struct.unpack('<5I', image.bytes_at_va(col, 20))
        self.assertEqual(words[1], 0)
        self.assertEqual(image.bytes_at_va(words[3]+8, 80).split(b'\0')[0], b'.?AVVegetation@@')
        def decoded(va, size):
            rows = list(decoder.disasm(image.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in rows), size)
            return {i.address: (i.mnemonic, i.op_str) for i in rows}
        ctor = decoded(0x456650, 58)
        self.assertEqual(ctor[0x45665c], ('mov', 'dword ptr [eax], 0x5524f8'))
        place = decoded(0x4598d0, 455)
        for va, op in {0x45994e: ('mov', 'edx, dword ptr [esi + 0x38]'),
                       0x459954: ('mov', 'ecx, ebp'), 0x459956: ('add', 'ecx, edx'),
                       0x459958: ('call', '0x4567a0')}.items():
            self.assertEqual(place[va], op)
        caller = decoded(0x438e70, 1102)
        self.assertEqual(caller[0x439079], ('call', '0x4570a0'))
        self.assertEqual(caller[0x439077], ('mov', 'ecx, edi'))
        pins = json.loads((ROOT/'tests/fixtures/vegetation_collision_objects.json').read_text())
        compiler = Path(os.environ['VC6_ROOT'])
        with tempfile.TemporaryDirectory(prefix='mcm2-vegetation-') as directory:
            out = Path(directory)
            def compile_source(source, name, physics=False):
                path = out/(name+'.obj')
                command = [sys.executable, str(ROOT/'tools/compile.py'), str(source), '-o', str(path),
                           '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt']
                if physics:
                    command += ['--include', str(ROOT/'src/krusty2')]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                return path, CoffObject(path)
            objects, paths = {}, {}
            sources = [CANDIDATE, ECO, COLLISION, QUERY,
                       'src/reconstructed/CollisionTransforms.cpp', 'src/reconstructed/VectorCalls.cpp',
                       'src/reconstructed/FastMath.cpp', 'samples/physics/collision/CollisionObjectNearMisses.cpp',
                       'samples/ecosystem/EcoSystemNearMisses.cpp']
            for index, source in enumerate(sources):
                paths[source], objects[source] = compile_source(ROOT/source, 'unit%d'%index,
                                                               source not in (CANDIDATE, ECO, sources[-1]))
                if source in pins['objects']:
                    rows, exceptions = view(objects[source], [ROOT])
                    pin = pins['objects'][source]
                    self.assertEqual(sorted(x[1] for x in exceptions), pin['filenames'])
                    self.assertEqual((len(rows), sum(len(r[3]) for r in rows)),
                                     (pin['sections'], pin['relocations']))
                    self.assertEqual(hashlib.sha256(json.dumps(rows).encode()).hexdigest(), pin['sha256'])
            self.assertEqual(function(objects[CANDIDATE], SYMBOL), function(objects[sources[-1]], SYMBOL))
            query = objects[COLLISION].find_symbol('QueryCollisions@CollisionObject')
            _, _, query_relocations = objects[COLLISION].symbol_extent(query)
            self.assertEqual([(r.virtual_address-query.value, r.type) for r in query_relocations
                              if objects[COLLISION].symbol_by_index[r.symbol_index].name == SYMBOL], [(522, 20)])
            self.assertNotIn('?GetObject@CollisionVegetation@@QAEPAVCollisionObject@@H@Z',
                             {s.name for s in objects[COLLISION].symbols if s.section_number == 0})
            proof = compare(image, objects[CANDIDATE], SYMBOL, 0x4570a0, 386,
                            ROOT/Path(CANDIDATE).with_suffix('.bindings.json'))
            self.assertEqual((proof['candidate_size'], proof['strict_match']['matching_positions']), (384, 103))
            self.assertFalse(proof['strict_exact'])
            proof = compare(image, objects[COLLISION], 'SetTransform@CollisionObject', 0x435830, 1676,
                            ROOT/Path(COLLISION).with_suffix('.bindings.json'))
            self.assertEqual((proof['candidate_size'], proof['strict_match']['matching_positions']), (1684, 890))
            spec = json.loads((ROOT/'config/game_build.json').read_text())
            built = game_compile(ROOT/CANDIDATE, out/'game-command', compiler,
                                 [ROOT/p for p in spec['includes']], spec['profile'],
                                 {**os.environ, 'PYTHONPATH': str(ROOT)})
            self.assertTrue(built['compiled'], built)
            self.assertEqual(view(objects[CANDIDATE]), view(CoffObject(Path(built['object']))))
            eco_text = (ROOT/ECO).read_text()
            parameters = out/'parameters.cpp'
            parameters.write_text('#include "'+str((ROOT/'src/reconstructed/EcoSystem.h').resolve()).replace('\\','/')+'"\n'
                                  'EcoSystem* g_EcoSystem; // validation-only copy of canonical pointer storage\n'+
                                  extract(eco_text, 'float UnknownEcoDefinition::HeightForParameter')+'\n'+
                                  extract(eco_text, 'float UnknownEcoDefinition::RadiusForParameter')+'\n')
            parameters_path, parameters_object = compile_source(parameters, 'parameters')
            collision_text = (ROOT/COLLISION).read_text()
            headers = []
            for line in collision_text.splitlines():
                if line.startswith('#include'):
                    relative = line.split('"')[1]
                    path = ROOT/'src/krusty2/collision'/relative if relative.startswith('..') else ROOT/'src/krusty2'/relative
                    headers.append('#include "'+str(path.resolve()).replace('\\','/')+'"')
                if line.startswith('// Section:'):
                    break
            headers.insert(0, '#include "'+str((ROOT/'src/reconstructed/CollisionTransforms.h').resolve()).replace('\\','/')+'"')
            signatures = ['void CollisionMatrixMultiply', 'void CollisionRelativeTransform', 'float CollisionLength',
                          'static inline void CollisionMatrixMultiplyInline', 'static inline void CollisionTransformPointInline',
                          'static inline float CollisionLengthInline', 'void CollisionObject::SetTransform']
            transform_text = '\n'.join(headers)+'\n'+'\n'.join(extract(collision_text, name) for name in signatures)+'\n'
            transform_text += '#include <windows.h>\nvoid CollisionObject::UpdateModelBounds(){ExitProcess(94);}\n'
            transform_text += 'void SoultreeObject::GetMatrixIn(SoultreeObject*,Matrix4*){ExitProcess(95);}\n'
            transform_text += extract((ROOT/QUERY).read_text(), 'void Vec3TransformPoint')+'\n'
            transform_text += 'extern "C" void RunTransformSource(void* o,const void* m){((CollisionObject*)o)->SetTransform((const Matrix4*)m);}\n'
            transform = out/'transform.cpp'; transform.write_text(transform_text)
            transform_path, transform_object = compile_source(transform, 'transform', True)
            for query in ['SetTransform@CollisionObject', '?CollisionRelativeTransform@@', '?CollisionMatrixMultiply@@', '?CollisionLength@@']:
                self.assertEqual(function(transform_object, query), function(objects[COLLISION], query))
            self.assertEqual(function(transform_object, '?Vec3TransformPoint@@'), function(objects[QUERY], '?Vec3TransformPoint@@'))
            strict = 0
            checks = [(parameters_object, ECO, '?RadiusForParameter@UnknownEcoDefinition@@', 0x455ff0, 95),
                      (parameters_object, ECO, '?HeightForParameter@UnknownEcoDefinition@@', 0x455f90, 95)]
            checks += [(transform_object, COLLISION, '?CollisionRelativeTransform@@', 0x432180, 215),
                       (transform_object, COLLISION, '?CollisionMatrixMultiply@@', 0x436500, 532),
                       (transform_object, COLLISION, '?CollisionLength@@', 0x435ec0, 74),
                       (transform_object, QUERY, '?Vec3TransformPoint@@', 0x42a510, 100),
                       (objects[sources[4]], sources[4], '?MoveBox@@', 0x428db0, 789),
                       (objects[sources[5]], sources[5], '?Vec3DotCall@@', 0x40ae30, 29),
                       (objects[sources[6]], sources[6], '?FastSqrt@@', 0x460b50, 85),
                       (objects[sources[6]], sources[6], '?InitFastSqrtTable@@', 0x460ae0, 97),
                       (objects[ECO], ECO, '??0Vegetation@@', 0x456650, 58),
                       (objects[ECO], ECO, '?PlaceStoredObjects@EcoSystem', 0x4598d0, 455),
                       (objects[COLLISION], COLLISION, '?QueryCollisions@', 0x438e70, 1102)]
            for obj, source, query, va, size in checks:
                self.assertTrue(compare(image, obj, query, va, size, ROOT/Path(source).with_suffix('.bindings.json'))['strict_exact'], query)
                strict += 1
            for query in ('?HeightForParameter@', '?RadiusForParameter@'):
                self.assertEqual(function(parameters_object, query), function(objects[ECO], query))
            self.assertEqual(len([s for s in objects[CANDIDATE].sections if not s.name.startswith('.debug')]), 2)
            # The 16 products' addition chains are decoded independently of the source spelling.
            def sum_expressions(raw, base):
                stack, rows, stores, integer_ops = [], {}, [], []
                def operand(op):
                    self.assertEqual(op.type, capstone.x86.X86_OP_MEM)
                    self.assertEqual(op.size, 4)
                    self.assertEqual(op.mem.index, 0)
                    return ('load', decoder.reg_name(op.mem.base), op.mem.disp)
                def binary(kind, left, right):
                    # Finite operands: retain grouping, permit commutation, never flatten.
                    return (kind, tuple(sorted((left, right), key=repr)))
                for ins in decoder.disasm(raw, base):
                    if ins.mnemonic == 'fld':
                        self.assertEqual(ins.operands[0].type, capstone.x86.X86_OP_MEM)
                        stack.insert(0, operand(ins.operands[0]))
                    elif ins.mnemonic == 'fmul':
                        self.assertGreaterEqual(len(stack), 1)
                        stack[0] = binary('multiply', stack[0], operand(ins.operands[0]))
                    elif ins.mnemonic == 'faddp':
                        self.assertGreaterEqual(len(stack), 2)
                        self.assertEqual(ins.op_str, 'st(1)')
                        result = binary('add', stack[1], stack[0])
                        stack[:2] = [result]
                    elif ins.mnemonic == 'fxch':
                        self.assertRegex(ins.op_str, r'^st\([0-7]\)$')
                        index = int(ins.op_str[3:-1])
                        self.assertGreater(len(stack), index)
                        stack[0], stack[index] = stack[index], stack[0]
                    elif ins.mnemonic == 'fstp':
                        self.assertEqual(len(stack), 1)
                        _, register, displacement = operand(ins.operands[0])
                        self.assertEqual(register, 'ebx')
                        self.assertIn(displacement, range(0x48, 0x88, 4))
                        self.assertNotIn(displacement, rows)
                        rows[displacement] = stack.pop(0)
                        stores.append(displacement)
                    elif ins.mnemonic == 'call':
                        break
                    elif ins.mnemonic.startswith('f'):
                        self.fail('Unexpected x87 instruction: '+ins.mnemonic+' '+ins.op_str)
                    else:
                        integer_ops.append((ins.mnemonic, ins.op_str))
                self.assertEqual(integer_ops, [('lea', 'edi, [ebx + 8]'),
                                              ('push', 'edx'), ('push', 'eax'), ('push', 'edi')])
                self.assertEqual(stores, list(range(0x48, 0x88, 4)))
                self.assertEqual(stack, [])
                return rows
            retail_expressions = sum_expressions(image.bytes_at_va(0x43587d, 0x435b55-0x43587d), 0x43587d)
            raw, _ = function(transform_object, 'SetTransform@CollisionObject')
            source_expressions = sum_expressions(raw[0x4d:], 0x4d)
            self.assertEqual(len(retail_expressions), 16)
            self.assertEqual(source_expressions, retail_expressions)
            old = out/'original.cpp'
            old.write_text(transform_text.replace(extract(transform_text, 'static inline void CollisionMatrixMultiplyInline'), pins['original_inline']))
            original_path, original_object = compile_source(old, 'original', True)
            original_raw, _ = function(original_object, 'SetTransform@CollisionObject')
            self.assertEqual(len(original_raw), pins['original_transform_size'])
            self.assertEqual(hashlib.sha256(original_raw).hexdigest(), pins['original_transform_raw_sha256'])
            original_score = compare(image, original_object, 'SetTransform@CollisionObject', 0x435830, 1676,
                                     ROOT/Path(COLLISION).with_suffix('.bindings.json'))
            self.assertEqual(original_score['strict_match']['matching_positions'], 902)
            original_expressions = sum_expressions(original_raw[0x4d:], 0x4d)
            self.assertNotEqual(original_expressions[0x48], retail_expressions[0x48])
            fixture_path, _ = compile_source(ROOT/'tests/fixtures/vegetation_collision_compare.cpp', 'fixture', True)
            common = [paths[CANDIDATE], parameters_path, fixture_path,
                      paths[sources[4]], paths[sources[5]], paths[sources[6]]]
            for label, provider, exit_code in [('original', original_path, 9), ('corrected', transform_path, 0)]:
                executable = out/(label+'.exe')
                result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')),
                    '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref', '/out:'+str(executable),
                    '/libpath:'+str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                    *map(str, common+[provider]), 'kernel32.lib'], capture_output=True, text=True)
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                result = subprocess.run([str(executable), str(exe)], capture_output=True, text=True, timeout=90)
                self.assertEqual(result.returncode, exit_code, result.stdout+result.stderr)
                if label == 'original':
                    self.assertIn('HULL FAIL n256 outer2 offset2408', result.stdout)
                    self.assertIn('wordsbfbfc9e6/bfbfc9e7', result.stdout)
                else:
                    self.assertEqual(result.stdout.strip(), 'Vegetation with hull paths: 39936 cases, 3048 access faults')
            self.assertEqual(strict, 13)
            print('13 strict checks; three complete object pins; 39936 cases, 3048 access faults; original control fails as expected')


if __name__ == '__main__':
    unittest.main()
