"""Bike placement calls the actual primary collision base and preserves complete bodies."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare
from test_garage_shadow_connections import function_text

ROOT = Path(__file__).resolve().parents[1]
BIKE = 'src/krusty2/vehicle/Bike.cpp'
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
SNAP = '?SnapPlacement@CollisionObject@@QAEXXZ'
UPDATE = '?UpdatePlacement@CollisionObject@@QAEXXZ'
PARTIALS = {
    0x405190: (2865, 1758, '2ee145a08a8327876d857cb620c7c7e00da6085641564e4882ba36aca63c9f30'),
    0x4060c0: (1678, 1677, 'aaa8295baf0865125e0a657a14607ee3f28d8ad2446b0dd2a609d03475342193'),
    0x406ae0: (2120, 115, '229fbd5f8409e5a532e1c2e25d98e2f2bdb7d79f1e8583547ca7542fd7cc6dc5'),
    0x407440: (474, 188, 'b54d0bbdb1c68a8d8c58a2368d2f31f5f0df23c1f6d12380fa84191bd009289e'),
    0x40c000: (946, 162, '32acc17b8099039c96f1a4f5576778df600564fea8905fa707f1b8937d6ead76'),
    0x40a520: (2272, 2112, 'c9494a3e330380062a18ab3f5b21d4ee4ee8dcd628b7cd45f155c9ff10322a3e'),
    0x4079c0: (6745, 6428, '9bcbfd9d8f540b6e0d5940830d4fc4450dd15d499a5018587c34c76ff10e393e'),
}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native authentic VC6 and private retail image')
class BikeSnapPlacementTests(unittest.TestCase):
    def test_primary_receiver_complete_callers_and_two_pass_updates(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        classes = {c.name: c for c in parse_rtti(retail)}
        for name, offset in (('CollisionObject', 0), ('GameObject', 12)):
            base = next(b for b in classes['ConstraintMethodCollisionModel'].bases if b.name == name)
            self.assertEqual((base.mdisp, base.pdisp), (offset, -1))
        self.assertEqual([(r.object_offset, r.vtables) for r in classes['ConstraintMethodCollisionModel'].vtable_records],
                         [(0, [0x551260]), (12, [0x5511f0])])
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoded = {}
        for va, size in ((0x406a50, 135), (0x5301c0, 995), (0x43b8d0, 121), (0x435fe0, 17), (0x435fb0, 36)):
            instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in instructions), size)
            decoded.update({i.address: (i.mnemonic, i.op_str) for i in instructions})
        for va, operation in (
            (0x406abf, ('mov', 'ecx, dword ptr [esi + 0x604]')),
            (0x406ac5, ('mov', 'ecx, dword ptr [ecx + 0x38]')),
            (0x406ac8, ('call', '0x435fe0')),
            (0x5302e3, ('push', '0x10c')),
            (0x530304, ('call', '0x43b8d0')),
            (0x530310, ('lea', 'ecx, [eax + 0xc]')),
            (0x530313, ('mov', 'dword ptr [ebx + 0x38], eax')),
            (0x43b8d8, ('call', '0x431e70')),
            (0x43b920, ('mov', 'dword ptr [esi], 0x551260')),
            (0x43b926, ('mov', 'dword ptr [esi + 0xc], 0x5511f0')),
            (0x435fe3, ('call', '0x435fb0')), (0x435fea, ('call', '0x435fb0'))):
            self.assertEqual(decoded[va], operation)
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x550900 + 33*4, 4))[0], 0x406a50)
        compiler = Path(os.environ['VC6_ROOT'])
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-snap-') as directory:
            output = Path(directory)
            pair = output / 'pair.cpp'
            text = (ROOT / COLLISION).read_text()
            pair.write_text('#include "collision/CollisionObject.h"\n#include "broadphase/Quadtree.h"\n' +
                '\n'.join(function_text(text, signature) for signature in
                          ('void CollisionObject::UpdatePlacement(', 'void CollisionObject::SnapPlacement(')) + '\n')
            objects, paths = {}, {}
            wrecker = 'src/reconstructed/Wrecker.cpp'
            constraint = 'src/krusty2/constraint/ConstraintMethodCollisionModel.cpp'
            fixture = 'tests/fixtures/bike_snap_placement.cpp'
            for index, source in enumerate((BIKE, COLLISION, wrecker, constraint, str(pair), fixture)):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(obj), obj
            strict, partial = 0, set()
            binding = (ROOT / BIKE).with_suffix('.bindings.json')
            for target in json.loads((ROOT / 'src/krusty2/vehicle/targets.json').read_text()):
                if target['source'] != 'Bike.cpp':
                    continue
                va = int(target['target_va'], 0)
                result = compare(retail, objects[BIKE], target['candidate_symbol_contains'], va, target['target_size'], binding)
                if va not in PARTIALS:
                    self.assertTrue(result['strict_exact'], result); strict += 1
                else:
                    with patch('mcm2tool.resolved_match.compare_bytes', lambda a, b: {'digest': hashlib.sha256(b).hexdigest()}):
                        resolved = match_object(objects[BIKE], result['symbol'], va, b'',
                            {k: int(v, 0) for k, v in json.loads(binding.read_text()).items()})
                    self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions'], resolved['digest']), PARTIALS[va])
                    partial.add(va)
            self.assertEqual(partial, set(PARTIALS))
            self.assertEqual(strict, 52)
            for source, symbol, va, size in (
                (COLLISION, SNAP, 0x435fe0, 17), (COLLISION, UPDATE, 0x435fb0, 36),
                (str(pair), SNAP, 0x435fe0, 17), (str(pair), UPDATE, 0x435fb0, 36),
                (wrecker, '?CreateBodies@Wrecker@@QAEXPAVD3DIMSoultreeCharacter@@PBD@Z', 0x5301c0, 995),
                (constraint, '??0ConstraintMethodCollisionModel@@QAE@H@Z', 0x43b8d0, 121)):
                obj = objects[source]
                self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], size)
                bindings = (ROOT / (COLLISION if source == str(pair) else source)).with_suffix('.bindings.json')
                self.assertTrue(compare(retail, obj, symbol, va, size, bindings)['strict_exact'], symbol)
            obj = objects[BIKE]
            self.assertNotIn('?Method_0x00435fe0@BikeA38@@QAEXXZ', {s.name for s in obj.symbols})
            _, _, relocations = obj.symbol_extent(obj.find_symbol('UnknownVirtualSlot33@Bike'))
            self.assertEqual([r.virtual_address for r in relocations if obj.symbol_by_index[r.symbol_index].name == SNAP], [121])
            executable = output / 'snap.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                str(paths[str(pair)]), str(paths[fixture]), 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Bike snap placement: 10240 two-pass, mutation and exception cases passed')
            print('52 strict Bike rows, seven complete partial fingerprints, six producer/helper checks; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
