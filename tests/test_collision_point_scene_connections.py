"""Contact vectors and primary scene receivers use existing source providers."""
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
POINT = 'src/krusty2/collision/CollisionPoint.cpp'
SAMPLE = 'samples/physics/collision/CollisionPointNearMisses.cpp'
NODE = 'src/krusty2/soultree/soultree.cpp'
VECTOR = 'src/reconstructed/VectorCalls.cpp'
PART = 'src/reconstructed/MovingPart.cpp'
DOT = '?Vec3DotCall@@YAMPBUVec3@@0@Z'
WORLD = '?LocalToWorldPoint@SoultreeObject@@QAE?AUVec3@@ABU2@@Z'
OLD_WORLD = '?LocalToWorldPoint@CollisionContactOwner@@QAEPAUCollisionVec3@@PAU2@PBU2@@Z'
# Complete resolved partial bodies from the published pre-connection source.
PARTIALS = (
    ('SoultreeMergeContacts', 0x43aa30, 845, 820,
     'b53bd4eb847c2470008cdc15f71c20f908d8ce8a8b3f181ac1c4a8376975428b'),
    ('SoultreeRefreshContacts', 0x43ad80, 615, 604,
     'c1eebf9abd629e2092a54ff39359ae46985dfdb896f6d660bba1e17e974271f1'),
)


def definition(text, name):
    match = re.search(r'^[^\n;{}]*\b' + re.escape(name) + r'\([^;{}]*\)\s*\{', text, re.M)
    assert match, name
    depth, end = 1, match.end()
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[match.start():end]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class CollisionPointSceneConnectionsTests(unittest.TestCase):
    def test_source_providers_receivers_sret_and_native_contacts(self):
        import capstone
        retail_path = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(retail_path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(retail_path)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        def instructions(va, size):
            result = list(decoder.disasm(retail.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in result), size)
            return result
        # The receiver is the primary SoultreeObject, not its +12 GameObject.
        col, = struct.unpack('<I', retail.bytes_at_va(0x557c18 - 4, 4))
        _, offset, _, td, hierarchy = struct.unpack('<5I', retail.bytes_at_va(col, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(td + 8, 40).split(b'\0')[0], b'.?AVSoultreeObject@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
            td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(td + 8, 48).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVSoultreeObject@@'], (0, -1, 0))
        self.assertEqual(bases[b'.?AVGameObject@@'], (12, -1, 0))
        # Producer chain: constructor stores FindByName's returned pointer at
        # MovingPart+0x40. Bike passes that address twice, its model+0x3bc once.
        part = {i.address: i for i in instructions(0x4a23a0, 48)}
        self.assertEqual((part[0x4a23ac].mnemonic, part[0x4a23ac].op_str), ('call', '0x4fdae0'))
        self.assertEqual((part[0x4a23b4].mnemonic, part[0x4a23b4].op_str),
                         ('mov', 'dword ptr [esi + 0x40], eax'))
        bike = {i.address: i for i in instructions(0x4079c0, 6745)}
        for call in (0x4089c2, 0x4089fb, 0x408a38):
            self.assertEqual((bike[call].mnemonic, bike[call].op_str), ('call', '0x43a330'))
        for address, operands in ((0x4089a1, 'ecx, dword ptr [eax + 0x40]'),
                                  (0x4089e0, 'ecx, dword ptr [eax + 0x40]'),
                                  (0x408a0f, 'eax, dword ptr [ebp + 0x3bc]')):
            self.assertEqual((bike[address].mnemonic, bike[address].op_str), ('mov', operands))
        for address, register in ((0x4089b8, 'ecx'), (0x4089f1, 'ecx'), (0x408a2e, 'eax')):
            self.assertEqual((bike[address].mnemonic, bike[address].op_str), ('push', register))
        add = {i.address: i for i in instructions(0x43a330, 565)}
        self.assertEqual((add[0x43a548].mnemonic, add[0x43a548].op_str),
                         ('mov', 'ecx, dword ptr [esp + 0x18]'))
        self.assertEqual((add[0x43a550].mnemonic, add[0x43a550].op_str),
                         ('mov', 'dword ptr [eax + 4], ecx'))
        # Provider reads the point and hidden output from its two stack args,
        # preserves receiver in ESI, returns the output pointer, and pops 8.
        world = {i.address: i for i in instructions(0x4fd660, 168)}
        for address, mnemonic, operands in ((0x4fd664, 'mov', 'esi, ecx'),
            (0x4fd666, 'call', '0x4fb4f0'), (0x4fd66b, 'mov', 'eax, dword ptr [esp + 0x18]'),
            (0x4fd6df, 'mov', 'eax, dword ptr [esp + 0x14]'), (0x4fd705, 'ret', '8')):
            self.assertEqual((world[address].mnemonic, world[address].op_str), (mnemonic, operands))
        with tempfile.TemporaryDirectory(prefix='mcm2-contact-scene-') as directory:
            output = Path(directory)
            scene_source = output / 'scene_helpers.cpp'
            text = (ROOT / NODE).read_text()
            # Unchanged source extraction for a small native executable. Both
            # complete emitted methods must also match the full production TU
            # and retail; the inline matrix product is not a scripted callback.
            scene_source.write_text('#include "math/Vec3.h"\n#include "' +
                (ROOT / 'src/reconstructed/Matrix4.h').as_posix() +
                '"\n#include "core/SoultreeObject.h"\n' +
                '\n'.join(definition(text, n) for n in
                          ('MatrixProduct', 'SoultreeObject::UpdateWorldMatrix', 'SoultreeObject::LocalToWorldPoint')) + '\n')
            fixture = 'tests/fixtures/collision_point_scene.cpp'
            objects, paths = {}, {}
            for index, source in enumerate((POINT, SAMPLE, NODE, VECTOR, PART, str(scene_source), fixture)):
                path = output / ('unit%d.obj' % index)
                command = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                           '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                           '--include', str(ROOT / 'src/krusty2')]
                if source == fixture:
                    command += ['--extra', '/Od']
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            strict_count = 0
            for target in json.loads((ROOT / 'src/krusty2/collision/targets.json').read_text()):
                if target.get('source') != 'CollisionPoint.cpp':
                    continue
                symbol, va, size = target['candidate_symbol_contains'], int(target['target_va'], 0), target['target_size']
                for source in (POINT, SAMPLE):
                    result = compare(retail, objects[source], symbol, va, size, (ROOT / POINT).with_suffix('.bindings.json'))
                    self.assertTrue(result['strict_exact'], (source, symbol))
                    strict_count += 1
            for source, symbol, va, size in ((VECTOR, DOT, 0x40ae30, 29),
                    (NODE, WORLD, 0x4fd660, 168),
                    (NODE, '?UpdateWorldMatrix@SoultreeObject@@', 0x4fb4f0, 858),
                    (NODE, '?FindByName@SoultreeObject@@', 0x4fdae0, 84),
                    (PART, '??0MovingPart@@', 0x4a23a0, 48)):
                entry = objects[source].find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, objects[source], symbol, va, size,
                                       (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
                strict_count += 1
                if source == NODE and 'FindByName' not in symbol:
                    self.assertEqual(objects[str(scene_source)].find_symbol(symbol).function_size, size)
                    self.assertTrue(compare(retail, objects[str(scene_source)], symbol, va, size,
                                           (ROOT / NODE).with_suffix('.bindings.json'))['strict_exact'], symbol)
                    strict_count += 1
            imported = Counter(objects[POINT].symbol_by_index[r.symbol_index].name
                               for r in objects[POINT].relocations if r.type == 20)
            self.assertEqual(imported[DOT], 3)
            self.assertEqual(imported[WORLD], 1)
            self.assertEqual(imported[OLD_WORLD], 0)
            self.assertEqual(imported['?Vec3DotCall@@YAMPBUCollisionVec3@@0@Z'], 0)
            # The nonadmitted sample keeps its own explicit-output scene call.
            sample_imports = Counter(objects[SAMPLE].symbol_by_index[r.symbol_index].name
                                    for r in objects[SAMPLE].relocations if r.type == 20)
            self.assertEqual(sample_imports[WORLD], 1)
            self.assertEqual(sample_imports[OLD_WORLD], 1)
            for symbol, va, size, positions, digest in PARTIALS:
                bindings = (ROOT / SAMPLE).with_suffix('.bindings.json')
                result = compare(retail, objects[SAMPLE], symbol, va, size, bindings)
                self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']), (size, positions))
                with patch('mcm2tool.resolved_match.compare_bytes',
                           lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                    resolved = match_object(objects[SAMPLE], symbol, va, b'',
                        {k: int(v, 0) for k, v in json.loads(bindings.read_text()).items()})
                self.assertEqual(resolved['digest'], digest)
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'contact_scene.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', *(str(paths[s]) for s in (POINT, VECTOR, str(scene_source), fixture)),
                'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Contact scene: 19541 native allocation, cache, transform and dot cases passed', result.stdout)
            print('%d complete strict checks, 2 preserved partials; %s' % (strict_count, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
