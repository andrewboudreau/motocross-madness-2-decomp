"""Rendering hierarchy calls share the actual scene providers and primary slots."""
from collections import Counter
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
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
MODEL = 'src/reconstructed/D3DIMSoulTree.cpp'
NODE = 'src/krusty2/soultree/soultree.cpp'
PARTIAL = 'samples/render/D3DIMSoulTreeNearMisses.cpp'
METHODS = (
    ('LoadHierarchy', 2, 0x4fdc00, 433, 0x43f950, 1257),
    ('LoadHierarchyFromParameters', 3, 0x4fddc0, 605, 0x43fe40, 543),
    ('ResetLocalMatrices', 6, 0x4fec70, 83, 0x4450c0, 280),
    ('CloneFrom', 7, 0x4feb10, 24, 0x444560, 1234),
    ('CopySubtree', 8, 0x4feb30, 252, 0x444b60, 263),
)


@unittest.skipUnless(os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and the private retail image')
class SoultreeHierarchyConnectionsTests(unittest.TestCase):
    def test_complete_providers_callers_and_primary_dispatch(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
            '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        classes = {c.name: c for c in parse_rtti(retail)}
        for name in ('SoultreeObject', 'D3DIMSoultreeObject'):
            self.assertEqual({v.object_offset for v in classes[name].vtable_records}, {0, 12})
            base = next(b for b in classes[name].bases if b.name == 'GameObject')
            self.assertEqual((base.mdisp, base.pdisp), (12, -1))
        base = next(b for b in classes['D3DIMSoultreeObject'].bases if b.name == 'SoultreeObject')
        self.assertEqual((base.mdisp, base.pdisp), (0, -1))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        with tempfile.TemporaryDirectory(prefix='mcm2-hierarchy-') as directory:
            objects = {}
            layout = 'tests/fixtures/soultree_hierarchy_layout.cpp'
            jobs = [(s, s, []) for s in (MODEL, NODE, PARTIAL)]
            jobs += [('render_layout', layout, []), ('physics_layout', layout, ['--extra', '/DPHYSICS_VIEW'])]
            for index, (key, source, extra) in enumerate(jobs):
                path = Path(directory) / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'), *extra], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[key] = CoffObject(path)
            # Verify every selected complete canonical body, not just changed calls.
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'])
                     for c in CASES if c['source'] in (MODEL, NODE)]
            for target in json.loads((ROOT / 'src/krusty2/soultree/targets.json').read_text()):
                if target['source'] == 'soultree.cpp':
                    cases.append((NODE, target['candidate_symbol_contains'], int(target['target_va'], 0), target['target_size']))
            strict = 0
            for source, symbol, va, size in dict.fromkeys(cases):
                self.assertTrue(compare(retail, objects[source], symbol, va, size,
                    (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
                strict += 1
            for method, slot, base_va, base_size, derived_va, derived_size in METHODS:
                for source, class_name, va, size, table, suffix in (
                    (NODE, 'SoultreeObject', base_va, base_size, 0x557c18, 'QuadTreeObject'),
                    (MODEL, 'D3DIMSoultreeObject', derived_va, derived_size, 0x5513ec, 'UnknownSoultreeQuadTreeObject')):
                    obj = objects[source]
                    entry = obj.find_symbol('?' + method + '@' + class_name + '@@')
                    self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                    self.assertEqual(struct.unpack('<I', retail.bytes_at_va(table + slot * 4, 4))[0], va)
                    vtable = obj.find_symbol('??_7' + class_name + '@@6B' + suffix + '@@@')
                    reloc = next(r for r in obj.relocations if r.section_number == vtable.section_number
                                 and r.virtual_address == vtable.value + slot * 4)
                    self.assertEqual(obj.symbol_by_index[reloc.symbol_index].name, entry.name)
                caller = objects[MODEL].find_symbol('?' + method + '@D3DIMSoultreeObject@@')
                _, _, relocs = objects[MODEL].symbol_extent(caller)
                target = objects[NODE].find_symbol('?' + method + '@SoultreeObject@@').name
                self.assertEqual(sum(r.type == 0x14 and objects[MODEL].symbol_by_index[r.symbol_index].name == target
                                     for r in relocs), 1)
                decoded = list(decoder.disasm(retail.bytes_at_va(derived_va, derived_size), derived_va))
                self.assertEqual(sum(i.size for i in decoded), derived_size)
                self.assertEqual(sum(i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM
                                     and i.operands[0].imm == base_va for i in decoded), 1)
            loader = objects[NODE].find_symbol('?LoadHierarchy@SoultreeObject@@')
            _, _, relocs = objects[NODE].symbol_extent(loader)
            calls = Counter(objects[NODE].symbol_by_index[r.symbol_index].name for r in relocs if r.type == 0x14)
            self.assertEqual(calls['?Read@UnknownTextureStream@@QAEHPAXHH@Z'], 11)
            self.assertFalse(any('SoultreeFileStream' in s.name for s in objects[NODE].symbols))
            # Independent compiler views must dispatch through the same primary
            # slots without applying the secondary GameObject adjustment.
            for method, slot in (('Load', 2), ('Parameters', 3), ('Reset', 6), ('Clone', 7), ('Copy', 8)):
                extents = []
                for key in ('render_layout', 'physics_layout'):
                    obj = objects[key]
                    body, size, relocs = obj.symbol_extent(obj.find_symbol('_DispatchHierarchy' + method))
                    self.assertFalse(relocs)
                    extents.append((body, size))
                    instructions = list(decoder.disasm(body, 0))
                    dispatch = [i for i in instructions if i.mnemonic in ('call', 'jmp')]
                    self.assertEqual(len(dispatch), 1)
                    self.assertEqual(dispatch[0].operands[0].type, capstone.x86.X86_OP_MEM)
                    self.assertEqual(dispatch[0].operands[0].mem.disp, slot * 4)
                self.assertEqual(*extents)
            for key in ('render_layout', 'physics_layout'):
                obj = objects[key]
                self.assertEqual(obj.symbol_extent(obj.find_symbol('_HierarchyGameBase')),
                                 (bytes.fromhex('b80c000000c3'), 6, []))
            for source, symbol, va, size, extent, digest in json.loads(
                    (ROOT / 'tests/fixtures/soultree_hierarchy_partials.json').read_text())['functions']:
                obj = objects[source]
                self.assertEqual(obj.find_symbol(symbol).function_size, extent)
                bindings = {k: int(v, 0) for k, v in json.loads((ROOT / source).with_suffix('.bindings.json').read_text()).items()}
                with patch('mcm2tool.resolved_match.compare_bytes',
                           lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                    resolved = match_object(obj, symbol, va, b'', bindings)
                self.assertEqual(resolved['digest'], digest, symbol)
            print('%d strict bodies; five base connections, eleven stream reads, five dual-view dispatch slots and two full partial fingerprints' % strict)


if __name__ == '__main__':
    unittest.main()
