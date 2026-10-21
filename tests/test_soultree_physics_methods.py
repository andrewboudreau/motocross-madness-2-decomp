"""Verify promoted methods against their actual retail and source vtable slots."""
from __future__ import annotations

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
from mcm2tool.toolchain import sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and the private retail EXE')
class SoultreePhysicsMethodTests(unittest.TestCase):
    def test_instance_counter_storage_and_complete_initializer(self):
        import capstone
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        name = '?g_SoultreeInstanceCounter@@3HA'
        address = 0x689f14
        section = retail.section_for_rva(address - retail.image_base)
        offset = address - retail.image_base - section.virtual_address
        self.assertEqual(section.name, '.data')
        self.assertTrue(section.characteristics & 0x80000000)
        self.assertGreaterEqual(offset, section.raw_size)
        self.assertLessEqual(offset + 4, section.virtual_size)
        # The PE loader supplies these four zero bytes. They are not file-backed.
        # Scan every explicit encoded address; this does not exclude indirect uses.
        sites, start = [], 0
        needle = struct.pack('<I', address)
        while (found := retail.data.find(needle, start)) >= 0:
            sites.append(retail.offset_to_va(found))
            start = found + 1
        self.assertEqual(sites, [0x500de2, 0x500df3, 0x500dfc, 0x500e04])
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        code = list(decoder.disasm(retail.bytes_at_va(0x500c50, 1500), 0x500c50))
        self.assertEqual(sum(instruction.size for instruction in code), 1500)
        instructions = {i.address: (i.mnemonic, i.op_str) for i in code}
        for at, mnemonic, operands in (
            (0x500c77, 'xor', 'ebx, ebx'),
            (0x500de0, 'mov', 'ecx, dword ptr [0x689f14]'),
            (0x500de6, 'mov', 'dword ptr [ebp + 0x204], ecx'),
            (0x500dec, 'mov', 'dword ptr [ebp + 0x208], ebx'),
            (0x500df2, 'mov', 'eax, dword ptr [0x689f14]'),
            (0x500df7, 'inc', 'eax'), (0x500df8, 'cmp', 'eax, 5'),
            (0x500dfb, 'mov', 'dword ptr [0x689f14], eax'),
            (0x500e00, 'jle', '0x500e08'),
            (0x500e02, 'mov', 'dword ptr [0x689f14], ebx'),
        ):
            self.assertEqual(instructions[at], (mnemonic, operands))
        bindings = ROOT / 'src/reconstructed/SoultreeState.bindings.json'
        self.assertEqual(json.loads(bindings.read_text()), {name: '0x00689f14'})
        with tempfile.TemporaryDirectory(prefix='mcm2-soultree-state-') as directory:
            objects = []
            for stem, source in (
                ('state', 'src/reconstructed/SoultreeState.cpp'),
                ('consumer', 'src/krusty2/soultree/SoulTreePhysics.cpp'),
            ):
                output = Path(directory) / (stem + '.obj')
                run = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(output), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
                objects.append(CoffObject(output))
            provider, consumer = objects
            self.assertEqual(provider.machine, 0x14c)
            definitions = [s for s in provider.symbols if s.storage_class == 2]
            self.assertEqual([(s.name, s.value) for s in definitions], [(name, 0)])
            storage = provider.section(definitions[0].section_number)
            self.assertEqual((storage.name, storage.raw_size, storage.raw_ptr), ('.bss', 4, 0))
            self.assertTrue(storage.characteristics & 0x80000000)
            self.assertTrue(storage.characteristics & 0x80)
            self.assertEqual([(s.name, s.raw_size) for s in provider.sections
                              if not s.name.startswith('.debug')], [('.drectve', 40), ('.bss', 4)])
            self.assertEqual([r for r in provider.relocations
                              if not provider.section(r.section_number).name.startswith('.debug')], [])
            function = consumer.find_symbol('?UnknownVirtualSlot2@SoultreePhysicsBaseObject@@')
            _, extent, relocations = consumer.symbol_extent(function)
            self.assertEqual(extent, 1500)
            self.assertEqual([(0x500c50 + r.virtual_address - function.value, r.type)
                              for r in relocations if consumer.symbol_by_index[r.symbol_index].name == name],
                             [(site, 6) for site in sites])
            result = compare(retail, consumer, function.name, 0x500c50, 1500,
                             ROOT / 'src/krusty2/soultree/SoulTreePhysics.bindings.json')
            self.assertTrue(result['strict_exact'], result)

    def test_exact_methods_and_primary_vtable_ownership(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x557d8c, 4))[0], 0x55f0d0)
        # Primary object offset 0; the GameObject virtual-base table is separate.
        self.assertEqual(struct.unpack('<5I', retail.bytes_at_va(0x55f0d0, 20)),
                         (0, 0, 0, 0x566c48, 0x55f0c0))
        name = b'.?AVSoultreePhysicsBaseObject@@\0'
        self.assertEqual(retail.bytes_at_va(0x566c50, len(name)), name)
        slots = {5, 10, 12, 19, 22, 23, 24, 26, 32, 34, 35}
        sources = {
            'methods': 'src/reconstructed/SoultreePhysicsMethods.cpp',
            'owner': 'src/krusty2/soultree/SoulTreePhysics.cpp',
            'remaining': 'samples/physics/soultree_base/SoultreePhysicsInlines.cpp',
            'node': 'src/krusty2/soultree/soultree.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-soultree-methods-') as directory:
            objects = {}
            for key, source in sources.items():
                path = Path(directory) / (key + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[key] = CoffObject(path)

            owner = objects['owner']
            table = owner.find_symbol('??_7SoultreePhysicsBaseObject@@6B0@@')
            table_refs = {r.virtual_address - table.value:
                          owner.symbol_by_index[r.symbol_index].name
                          for r in owner.relocations if r.section_number == table.section_number}
            definitions = {s.name for s in objects['methods'].symbols
                           if s.storage_class == 2 and s.section_number > 0}
            path = ROOT / 'samples/physics/soultree_base/targets.json'
            checked = set()
            for target in json.loads(path.read_text()):
                slot = int(re.search(r'Slot(\d+)@', target['candidate_symbol_contains'])[1])
                promoted = slot in slots
                source = (path.parent / target['source']).resolve()
                self.assertEqual(source, ROOT / sources['methods' if promoted else 'remaining'])
                obj = objects['methods' if promoted else 'remaining']
                result = compare(retail, obj, target['candidate_symbol_contains'],
                                 int(target['target_va'], 0), target['target_size'],
                                 path.parent / target['bindings'])
                with self.subTest(slot=slot):
                    self.assertEqual(struct.unpack('<I', retail.bytes_at_va(0x557d90 + slot * 4, 4))[0],
                                     int(target['target_va'], 0))
                    self.assertEqual(result['strict_exact'], slot != 6, result)
                    if slot == 6:
                        # The complete nonexact candidate now retains FastSqrt's
                        # result; the previous 432-byte body used squared length.
                        # Native behavior evidence is in test_soultree_drag_behavior.
                        self.assertEqual(result['candidate_size'], 440)
                        self.assertEqual(result['strict_match']['matching_positions'], 126)
                    else:
                        self.assertEqual(result['candidate_size'], target['target_size'])
                    if promoted:
                        self.assertEqual(table_refs[slot * 4], result['symbol'])
                        self.assertIn(table_refs[slot * 4], definitions)
                        checked.add(slot)
            self.assertEqual(checked, slots)
            self.assertFalse(any(re.search(r'Slot(?:6|36)@', name) for name in definitions))

            methods = objects['methods']
            imports = {methods.symbol_by_index[r.symbol_index].name for r in methods.relocations
                       if methods.symbol_by_index[r.symbol_index].section_number == 0}
            node_calls = {
                '?GetAxesIn@SoultreeObject@@QAEXPAV1@PAUVec3@@1@Z',
                '?SetAxesIn@SoultreeObject@@QAEXPAV1@PBUVec3@@1HH@Z',
            }
            self.assertEqual(imports, node_calls)
            self.assertEqual({s.name for s in methods.symbols if s.section_number == 0},
                             node_calls | {'__fltused'})
            node_definitions = {s.name for s in objects['node'].symbols
                                if s.storage_class == 2 and s.section_number > 0}
            self.assertIn('?GetAxesIn@SoultreeObject@@QAEXPAV1@PAUVec3@@1@Z', node_definitions)
            # SetAxesIn is still sample-only; this existing unresolved dependency
            # is shared by other canonical physics and motion callers.
            self.assertNotIn('?SetAxesIn@SoultreeObject@@QAEXPAV1@PBUVec3@@1HH@Z', node_definitions)


if __name__ == '__main__':
    unittest.main()
