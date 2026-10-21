"""Three actual contact consumers, primary/secondary pointers, and real math."""
from collections import Counter
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
from mcm2tool.resolved_match import match_object
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SOUL = 'src/krusty2/soultree/SoulTreePhysics.cpp'
VEHICLE = 'src/krusty2/vehicle/Vehicle.cpp'
POINT = 'src/krusty2/collision/CollisionPoint.cpp'
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
NODE = 'src/krusty2/soultree/soultree.cpp'
VECTOR = 'src/reconstructed/VectorCalls.cpp'
MATH = 'src/reconstructed/Math3D.cpp'
FAST = 'src/reconstructed/FastMath.cpp'
DIVIDE = 'src/reconstructed/CollisionVectorHelpers.cpp'
CTOR = 'src/reconstructed/CollisionVectorConstructor.cpp'
FIXTURE = 'tests/fixtures/collision_point_consumers.cpp'
SLOT = '?UnknownVirtualSlot31@SoultreePhysicsBaseObject@@UAEXXZ'
RELATIVE = '?UpdateRelativeMotion@CollisionPoint@@QAEXPBUCollisionVec3@@000M@Z'
WORLD = '?UpdateCollisionPointWorldPositions@@YAXHPAPAVCollisionPoint@@@Z'
NORMALIZE = '?Vec3Normalize@@YA?AUVec3@@ABU1@@Z'
OLD_NORMALIZE = '?Vec3Normalize@@YAPAUCollisionVec3@@PAU1@PBU1@@Z'
RENAMES = {
    '?UpdateRelativeMotion@SoultreeContact@@QAEXPAUVec3@@000M@Z': RELATIVE,
    '?SoultreeResetContacts@@YAXHPAPAVSoultreeContact@@@Z': WORLD,
    '?VehContactsC@@YAXHPAPAUVehicleContact@@@Z': WORLD,
}


def definition(text, name):
    match = re.search(r'^[^\n;{}]*\b' + re.escape(name) + r'\([^;{}]*\)\s*\{', text, re.M)
    assert match, name
    depth, end = 1, match.end()
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[match.start():end]


def native_point_bindings(path, destination):
    """Validation-only ABI/storage joins, after proving original object bytes.

    Normalization's explicit-output view has the same cdecl stack words and
    return register as the actual Vec3 hidden result. The external zero/y-axis
    names refer to the actual TU's private, dynamically initialized constants.
    No instruction bytes, addends, implementations, or production objects change.
    """
    obj = CoffObject(path)
    data = bytearray(obj.data)
    local = {
        '?g_CollisionZeroVec3@@3UCollisionVec3@@A': '_kCollisionPointZero',
        '?g_CollisionVec3_579810@@3UCollisionVec3@@A': '_kCollisionPointYAxis',
    }
    for section in obj.sections:
        for index in range(section.reloc_count):
            offset = section.reloc_ptr + 10 * index
            old_index, = struct.unpack_from('<I', data, offset + 4)
            name = obj.symbol_by_index[old_index].name
            if name in local:
                target = obj.find_symbol(local[name])
                assert target.section_number > 0 and target.storage_class == 3
                struct.pack_into('<I', data, offset + 4, target.index)
    for symbol in obj.symbols:
        if symbol.name in local and symbol.section_number == 0:
            target = obj.find_symbol(local[symbol.name])
            struct.pack_into('<Ih', data, obj.sym_ptr + symbol.index*18 + 8, target.value, target.section_number)
            data[obj.sym_ptr + symbol.index*18 + 16] = 3
    assert obj._str_off + obj._str_size == len(data)
    encoded = NORMALIZE.encode() + b'\0'
    for symbol in obj.symbols:
        if symbol.name == OLD_NORMALIZE:
            assert symbol.section_number == 0
            struct.pack_into('<II', data, obj.sym_ptr + symbol.index * 18, 0, obj._str_size)
    data.extend(encoded)
    struct.pack_into('<I', data, obj._str_off, obj._str_size + len(encoded))
    destination.write_bytes(data)
    # All section payloads, including complete method code, remain unchanged.
    after = CoffObject(destination)
    assert [(s.name, obj.data[s.raw_ptr:s.raw_ptr+s.raw_size]) for s in obj.sections] == [
        (s.name, after.data[s.raw_ptr:s.raw_ptr+s.raw_size]) for s in after.sections]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and pinned private retail image')
class CollisionPointConsumersTests(unittest.TestCase):
    def test_full_callers_actual_providers_and_mixed_arrays(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        def code(address, size):
            instructions = list(decoder.disasm(retail.bytes_at_va(address, size), address))
            self.assertEqual(sum(i.size for i in instructions), size)
            return instructions
        # Independent raw RTTI: array producers supply point-relative pointers.
        col, = struct.unpack('<I', retail.bytes_at_va(0x5584fc-4, 4))
        _, offset, _, td, hierarchy = struct.unpack('<5I', retail.bytes_at_va(col, 20))
        self.assertEqual(offset, 0xb8)
        self.assertEqual(retail.bytes_at_va(td+8, 40).split(b'\0')[0], b'.?AVTire@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<'+'I'*count, retail.bytes_at_va(array, count*4)):
            td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(td+8, 48).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVCollisionPoint@@'], (0xb8, -1, 0))
        self.assertEqual(bases[b'.?AVCollisionObject@@'], (0, -1, 0))
        col, = struct.unpack('<I', retail.bytes_at_va(0x557d90-4, 4))
        _, offset, _, td, _ = struct.unpack('<5I', retail.bytes_at_va(col, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(td+8, 64).split(b'\0')[0], b'.?AVSoultreePhysicsBaseObject@@')
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x557d90+31*4, 4))[0], 0x501e10)
        ctor = {i.address: (i.mnemonic, i.op_str) for i in code(0x512f10, 1168)}
        self.assertEqual(ctor[0x512f66], ('mov', 'dword ptr [esi + 0xb8], 0x5511d8'))
        self.assertEqual(ctor[0x513135], ('mov', 'dword ptr [esi + 0xb8], 0x5584fc'))
        add = {i.address: (i.mnemonic, i.op_str) for i in code(0x525d30, 160)}
        self.assertEqual(add[0x525d7d], ('add', 'eax, 0xb8'))
        self.assertEqual(add[0x525d90], ('push', 'eax'))
        self.assertEqual(add[0x525da1], ('call', '0x43a570'))
        owned = {i.address: (i.mnemonic, i.op_str) for i in code(0x43a330, 565)}
        self.assertEqual(owned[0x43a51c], ('mov', 'dword ptr [ecx + edx*4], eax'))
        self.assertEqual(owned[0x43a550], ('mov', 'dword ptr [eax + 4], ecx'))
        self.assertEqual(owned[0x43a55b], ('inc', 'ecx'))
        self.assertEqual(owned[0x43a55d], ('mov', 'dword ptr [esi], ecx'))
        existing = {i.address: (i.mnemonic, i.op_str) for i in code(0x43a570, 109)}
        self.assertEqual(existing[0x43a58b], ('mov', 'dword ptr [eax + esi*4], edx'))
        self.assertEqual(existing[0x43a5c5], ('mov', 'dword ptr [eax + 4], 0'))
        self.assertEqual(existing[0x43a5d5], ('inc', 'edx'))
        self.assertEqual(existing[0x43a5d6], ('mov', 'dword ptr [ecx], edx'))
        bike = {i.address: (i.mnemonic, i.op_str) for i in code(0x4079c0, 6745)}
        for site in (0x4089c2, 0x4089fb, 0x408a38):
            self.assertEqual(bike[site], ('call', '0x43a330'))
        slot = {i.address: (i.mnemonic, i.op_str) for i in code(0x501e10, 127)}
        self.assertEqual(slot[0x501e26], ('mov', 'ecx, dword ptr [eax + edi*4]'))
        self.assertEqual(slot[0x501e5c], ('lea', 'eax, [ecx + 0x38]'))
        self.assertEqual(slot[0x501e70], ('call', '0x43a640'))
        for va, size, site in ((0x502f60, 1921, 0x5030d6), (0x52a940, 3254, 0x52ae13)):
            self.assertEqual({i.address: (i.mnemonic, i.op_str) for i in code(va, size)}[site],
                             ('call', '0x43aff0'))
        # The strict normalization body consumes out, source, returns out (cdecl).
        normal = code(0x5087b0, 150)
        self.assertEqual(normal[-1].mnemonic, 'ret')
        self.assertEqual(normal[-1].op_str, '')
        normal_at = {i.address: (i.mnemonic, i.op_str) for i in normal}
        self.assertEqual(normal_at[0x5087b0], ('sub', 'esp, 0xc'))
        self.assertEqual(normal_at[0x5087b3], ('push', 'esi'))
        self.assertEqual(normal_at[0x5087b4], ('mov', 'esi, dword ptr [esp + 0x18]'))
        self.assertEqual(normal_at[0x5087e7], ('mov', 'eax, dword ptr [esp + 0x14]'))
        self.assertEqual(normal_at[0x508810], ('mov', 'eax, dword ptr [esp + 0x18]'))
        self.assertEqual(normal_at[0x508814], ('add', 'esp, 4'))
        self.assertEqual(normal_at[0x50883f], ('mov', 'dword ptr [ecx + 8], edx'))
        self.assertEqual([(i.address, i.op_str) for i in normal if i.mnemonic=='ret'],
                         [(0x508801, ''), (0x508845, '')])
        zero = retail.section_for_rva(0x5797f0-retail.image_base)
        self.assertTrue(zero.characteristics & 0x80000000)
        self.assertGreaterEqual(0x5797f0-retail.image_base-zero.virtual_address, zero.raw_size)
        self.assertLessEqual(0x5797fc-retail.image_base-zero.virtual_address, zero.virtual_size)
        with tempfile.TemporaryDirectory(prefix='mcm2-contact-consumers-') as directory:
            output = Path(directory)
            extracted = {
                'slot': '#include "soultree/SoultreePhysicsBaseObject.h"\n#include "soultree/SoultreePhysicsContact.h"\n#include "collision/CollisionPoint.h"\n' + definition((ROOT/SOUL).read_text(), 'SoultreePhysicsBaseObject::UnknownVirtualSlot31'),
                'length': '#include "collision/CollisionPoint.h"\n' + definition((ROOT/COLLISION).read_text(), 'CollisionLength'),
                'scene': '#include "math/Vec3.h"\n#include "' + (ROOT/'src/reconstructed/Matrix4.h').as_posix() + '"\n#include "core/SoultreeObject.h"\n' + '\n'.join(definition((ROOT/NODE).read_text(), n) for n in ('MatrixProduct', 'SoultreeObject::UpdateWorldMatrix', 'SoultreeObject::LocalToWorldPoint')),
            }
            sources = {}
            for key, text in extracted.items():
                sources[key] = output/(key+'.cpp')
                sources[key].write_text(text+'\n')
            jobs = [(s, ROOT/s, []) for s in (SOUL, VEHICLE, POINT, COLLISION, NODE, VECTOR, MATH, FAST, DIVIDE, CTOR)]
            jobs += [(key, source, []) for key, source in sources.items()]
            jobs += [(FIXTURE, ROOT/FIXTURE, ['--extra', '/Od']),
                     ('layout', ROOT/FIXTURE, ['--extra', '/DCONTACT_LAYOUT'])]
            objects, paths = {}, {}
            for index, (key, source, extra) in enumerate(jobs):
                obj = output/('unit%d.obj'%index)
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt', '--include', str(ROOT/'src/krusty2'), *extra],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                objects[key], paths[key] = CoffObject(obj), obj
            providers = (
                (POINT, RELATIVE, 0x43a640, 1003), (POINT, WORLD, 0x43aff0, 88),
                (POINT, '?AddCollisionPoint@@', 0x43a330, 565),
                (POINT, '?AddExistingCollisionPoint@@', 0x43a570, 109),
                (POINT, '?CollisionRejectFrom@@', 0x43b190, 168),
                (COLLISION, '?CollisionLength@@', 0x435ec0, 74),
                (NODE, '?LocalToWorldPoint@SoultreeObject@@', 0x4fd660, 168),
                (NODE, '?UpdateWorldMatrix@SoultreeObject@@', 0x4fb4f0, 858),
                (VECTOR, '?Vec3DotCall@@', 0x40ae30, 29),
                (MATH, NORMALIZE, 0x5087b0, 150),
                (FAST, '?FastSqrt@@', 0x460b50, 85), (FAST, '?FastInvSqrt@@', 0x460c00, 107),
                (FAST, '?InitFastSqrtTable@@', 0x460ae0, 97), (FAST, '?InitFastInvSqrtTable@@', 0x460bb0, 74),
                (DIVIDE, '?CollisionDivide@@', 0x43c890, 71),
                (CTOR, '??0CollisionVec3@@', 0x404e60, 25),
                (SOUL, SLOT, 0x501e10, 127), (SOUL, '?RunSteps@SoultreePhysicsBaseObject@@', 0x502f60, 1921),
            )
            strict = 0
            for source, symbol, va, size in providers:
                entry = objects[source].find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'), symbol)
                self.assertTrue(compare(retail, objects[source], symbol, va, size,
                                       (ROOT/source).with_suffix('.bindings.json'))['strict_exact'], symbol)
                strict += 1
                extract = 'slot' if source==SOUL and symbol==SLOT else 'length' if source==COLLISION else 'scene' if source==NODE else None
                if extract:
                    self.assertTrue(compare(retail, objects[extract], symbol, va, size,
                        (ROOT/source).with_suffix('.bindings.json'))['strict_exact'], (extract, symbol))
                    strict += 1
            for source, caller, callee in ((SOUL, SLOT, RELATIVE), (SOUL, '?RunSteps@SoultreePhysicsBaseObject@@', WORLD),
                                          (VEHICLE, '?UnknownVirtualSlot49@Vehicle@@', WORLD)):
                obj = objects[source]
                _, _, rels = obj.symbol_extent(obj.find_symbol(caller))
                names = Counter(obj.symbol_by_index[r.symbol_index].name for r in rels if r.type==20)
                self.assertEqual(names[callee], 1)
                self.assertFalse(set(names)&RENAMES.keys())
            # Native zero/y storage is the actual TU's storage, with its actual
            # strict CRT initializer thunks/bodies, not fixture-invented vectors.
            for name, va, size in (('_$E2',0x43b050,5), ('_$E1',0x43b060,60),
                                   ('_$E8',0x43b0f0,5), ('_$E7',0x43b100,60)):
                self.assertTrue(compare(retail, objects[POINT], name, va, size,
                    (ROOT/POINT).with_suffix('.bindings.json'))['strict_exact'], name)
                strict += 1
            point_object = objects[POINT]
            crt = [s.index for s in point_object.sections if s.name=='.CRT$XCU']
            self.assertEqual(len(crt), 1)
            crt_names = [point_object.symbol_by_index[r.symbol_index].name
                         for r in point_object.relocations if r.section_number==crt[0]]
            self.assertEqual(crt_names, ['_$E2','_$E5','_$E8','_$E11'])
            for initializer, destination in (('_$E1','_kCollisionPointZero'), ('_$E7','_kCollisionPointYAxis')):
                entry = point_object.find_symbol(initializer)
                body, _, relocations = point_object.symbol_extent(entry)
                self.assertEqual([point_object.symbol_by_index[r.symbol_index].name
                                  for r in relocations if r.type==6], [destination]*3)
                self.assertEqual(sorted(struct.unpack_from('<I', body, r.virtual_address-entry.value)[0]
                                        for r in relocations if r.type==6), [0,4,8])
            result = compare(retail, objects[VEHICLE], '?UnknownVirtualSlot49@Vehicle@@', 0x52a940, 3254,
                             (ROOT/VEHICLE).with_suffix('.bindings.json'))
            self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']), (3254, 3229))
            with patch('mcm2tool.resolved_match.compare_bytes', lambda a,b: {'digest': hashlib.sha256(b).hexdigest()}):
                resolved = match_object(objects[VEHICLE], '?UnknownVirtualSlot49@Vehicle@@', 0x52a940, b'',
                    {k:int(v,0) for k,v in json.loads((ROOT/VEHICLE).with_suffix('.bindings.json').read_text()).items()})
            self.assertEqual(resolved['digest'], '00b155aafdae0021e18ba64055e46609f5ad556b2eb37f6db07844362940ec7d')
            native_point = output/'point_native.obj'
            native_point_bindings(paths[POINT], native_point)
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable, mapping = output/'consumers.exe', output/'consumers.map'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                f'/out:{executable}', f'/map:{mapping}', f'/libpath:{libraries}', str(native_point),
                *(str(paths[k]) for k in ('slot', 'length', 'scene', VECTOR, MATH, FAST, DIVIDE, CTOR, FIXTURE, 'layout')),
                'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            addresses = {}
            for source, symbol, va, _ in providers:
                name = objects[source].find_symbol(symbol).name
                match = re.search(re.escape(name)+r'\s+([0-9a-fA-F]{8})\s', mapping.read_text(encoding='latin1'))
                if match:
                    addresses[va] = int(match.group(1),16)
            patches = []
            seen = Counter()
            for va, size in ((0x43a640,1003),(0x43aff0,88)):
                for instruction in code(va,size):
                    if instruction.mnemonic!='call':
                        continue
                    self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                    target = instruction.operands[0].imm
                    self.assertIn(target, addresses)
                    seen[target] += 1
                    patches.append(struct.pack('<III',instruction.address,target,addresses[target]))
            self.assertEqual(set(seen), {0x404e60,0x43b190,0x5087b0,0x40ae30,0x460c00,0x460b50,0x435ec0,0x43c890,0x4fd660})
            # No replacement of slot31's target or either tested complete provider.
            self.assertNotIn(0x43a640, seen)
            patch_path = output/'calls.bin'
            patch_path.write_bytes(b''.join(patches))
            result = subprocess.run([str(executable), str(image), str(patch_path)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertIn('Collision point consumers: 39557 actual-source, mixed-array, alias, FP and fault scenarios match', result.stdout)
            print('%d strict caller/provider checks and full Vehicle partial preserved; %s'%(strict,result.stdout.strip()))


if __name__=='__main__':
    unittest.main()
