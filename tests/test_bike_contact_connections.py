"""Bike contact allocation/removal joins the actual CollisionPoint providers."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
BIKE = 'src/krusty2/vehicle/Bike.cpp'
POINT = 'src/krusty2/collision/CollisionPoint.cpp'
ADD = '?AddCollisionPoint@@YAPAVCollisionPoint@@HPAPAV1@PBUCollisionVec3@@PAVCollisionContactOwner@@MPAHMH@Z'
REMOVE = '?RemoveCollisionPoint@@YAXPAPAVCollisionPoint@@PAV1@PAH@Z'
# Whole resolved baseline bodies, including the nonexact loader. These guard
# preservation, not retail fidelity or source acceptance of the partials.
PARTIAL = [
    ('UnknownVirtualSlot38@Bike', 4215184, 2874, 2865, 1758, '2ee145a08a8327876d857cb620c7c7e00da6085641564e4882ba36aca63c9f30'),
    ('UnknownVirtualSlot7@Bike', 4219072, 1678, 1678, 1677, 'aaa8295baf0865125e0a657a14607ee3f28d8ad2446b0dd2a609d03475342193'),
    ('UnknownVirtualSlot89@Bike', 4221664, 2396, 2120, 115, '229fbd5f8409e5a532e1c2e25d98e2f2bdb7d79f1e8583547ca7542fd7cc6dc5'),
    ('UnknownVirtualSlot102@Bike', 4224064, 485, 474, 188, 'b54d0bbdb1c68a8d8c58a2368d2f31f5f0df23c1f6d12380fa84191bd009289e'),
    ('UnknownVirtualSlot61@Bike', 4243456, 1015, 946, 162, '32acc17b8099039c96f1a4f5576778df600564fea8905fa707f1b8937d6ead76'),
    ('Method_0x0040a520@Bike', 4236576, 2272, 2272, 2112, 'c9494a3e330380062a18ab3f5b21d4ee4ee8dcd628b7cd45f155c9ff10322a3e'),
    ('LoadBike@Bike', 4225472, 6745, 6745, 6428, '9bcbfd9d8f540b6e0d5940830d4fc4450dd15d499a5018587c34c76ff10e393e'),
]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class BikeContactConnectionsTests(unittest.TestCase):
    def test_complete_provider_objects_callers_and_native_contacts(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         json.loads((ROOT / 'config/private_bundle_expected.json').read_text())['mcm2_sha256'])
        retail = PEImage(image)
        # Tire's CollisionPoint is a secondary base, not its primary pointer.
        col, = struct.unpack('<I', retail.bytes_at_va(0x5584fc - 4, 4))
        _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(col, 20))
        self.assertEqual(offset, 0xb8)
        self.assertEqual(retail.bytes_at_va(descriptor + 8, 40).split(b'\0')[0], b'.?AVTire@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
            td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(td + 8, 40).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVCollisionPoint@@'], (0xb8, -1, 0))
        col, = struct.unpack('<I', retail.bytes_at_va(0x5511d8 - 4, 4))
        _, offset, _, descriptor, _ = struct.unpack('<5I', retail.bytes_at_va(col, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(descriptor + 8, 40).split(b'\0')[0], b'.?AVCollisionPoint@@')
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        for address, size, callee, expected_calls in ((0x4079c0, 6745, 0x43a330, 3),
                (0x409a10, 276, 0x43a5e0, 2), (0x43a330, 565, 0x4a3010, 1)):
            instructions = list(decoder.disasm(retail.bytes_at_va(address, size), address))
            self.assertEqual(sum(i.size for i in instructions), size)
            calls = [i for i in instructions if i.mnemonic == 'call'
                     and i.operands[0].type == capstone.x86.X86_OP_IMM and i.operands[0].imm == callee]
            self.assertEqual(len(calls), expected_calls)
            if address == 0x43a330:
                self.assertEqual(calls[0].address, 0x43a350)
                self.assertEqual([i.address for i in instructions if i.mnemonic == 'call'], [0x43a350])
                self.assertTrue(any(i.mnemonic == 'push' and i.operands[0].type == capstone.x86.X86_OP_IMM
                                    and i.operands[0].imm == 0xc4 for i in instructions))
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-contact-') as directory:
            output = Path(directory)
            fixture = 'tests/fixtures/bike_contact_compare.cpp'
            objects, paths = {}, {}
            for source in (BIKE, POINT, fixture):
                path = output / (Path(source).stem + '.obj')
                command = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                           '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                           '--include', str(ROOT / 'src/krusty2')]
                if source == fixture:
                    command += ['--extra', '/Od']
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            for symbol, address, size in ((ADD, 0x43a330, 565), (REMOVE, 0x43a5e0, 85)):
                entry = objects[POINT].find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, objects[POINT], symbol, address, size,
                                       (ROOT / POINT).with_suffix('.bindings.json'))['strict_exact'])
            imports = {s.name for s in objects[BIKE].symbols if s.section_number == 0}
            self.assertTrue({ADD, REMOVE} <= imports)
            self.assertFalse(any('BikeAddContactPoint' in s or 'BikeRemoveCollisionPoint' in s for s in imports))
            for caller, symbol, count in (('LoadBike@Bike', ADD, 3), ('??1Bike@@UAE@XZ', REMOVE, 2)):
                _, _, relocs = objects[BIKE].symbol_extent(objects[BIKE].find_symbol(caller))
                self.assertEqual(sum(r.type == 20 and objects[BIKE].symbol_by_index[r.symbol_index].name == symbol
                                     for r in relocs), count)
            partial = {tuple(row[:3]): row[3:] for row in PARTIAL}
            strict_count = partial_count = 0
            for manifest in ('src/krusty2/vehicle/targets.json', 'src/krusty2/collision/targets.json'):
                for target in json.loads((ROOT / manifest).read_text()):
                    if 'source' not in target or not target.get('bindings'):
                        continue
                    source = (Path(manifest).parent / target['source']).as_posix()
                    if source not in objects:
                        continue
                    symbol, address, size = target['candidate_symbol_contains'], int(target['target_va'], 0), target['target_size']
                    bindings_path = ROOT / Path(manifest).parent / target['bindings']
                    result = compare(retail, objects[source], symbol, address, size, bindings_path)
                    key = (symbol, address, size)
                    if source == BIKE and key in partial:
                        extent, positions, digest = partial[key]
                        self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']), (extent, positions))
                        with patch('mcm2tool.resolved_match.compare_bytes',
                                   lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                            resolved = match_object(objects[source], symbol, address, b'',
                                {k: int(v, 0) for k, v in json.loads(bindings_path.read_text()).items()})
                        self.assertEqual(resolved['digest'], digest)
                        partial_count += 1
                    else:
                        self.assertTrue(result['strict_exact'], (source, symbol))
                        strict_count += 1
            self.assertEqual(partial_count, 7)
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'contacts.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed',
                '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}', str(paths[POINT]), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(image)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Bike contacts: 13056 allocation, capacity, layout, removal and failure cases match', result.stdout)
            print('%d strict provider/caller checks, %d complete partial fingerprints; %s' %
                  (strict_count, partial_count, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
