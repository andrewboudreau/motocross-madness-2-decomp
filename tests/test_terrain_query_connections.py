"""Terrain query call boundaries; provider remains an explicitly partial body."""
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
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
PROVIDER = '?QueryGround@Terrain@@QAEXPAUTerrainVec3@@0HPAE@Z'
CALLERS = (
    ('FollowCamera', '?UnknownVirtualSlot47@FollowCamera@@', 0x465720, 1267, 1),
    ('VehicleCamera', '?UnknownVirtualSlot68@VehicleCamera@@', 0x52bcf0, 684, 1),
    ('VehicleCamera', '?UnknownVirtualSlot35@VehicleCamera@@', 0x52c510, 1267, 1),
    ('KrustyBikeCamera', '?UnknownVirtualSlot52@KrustyBikeCamera@@', 0x497fa0, 217, 1),
    ('AuralScape', '?Tick@AuralScape@@', 0x403710, 1025, 1),
    ('EcoSystem', '?PlaceAuthoredObjects@EcoSystem@@', 0x459b40, 412, 1),
    ('Krusty3DObjects', '?Tick@RunwayLights@@', 0x48a760, 1509, 5),
    ('PlacementQueries', '?ClampToPlacementSquare@@', 0x4b0b80, 619, 4),
    ('BikeRace', '?DrawTrack@BikeRace@@', 0x423140, 715, 6),
    ('FollowCameraTick', '?Tick@FollowCamera@@', 0x465c20, 3672, 2),
)
SOURCES = {stem: ('samples/camera/' if stem == 'FollowCameraTick' else 'src/reconstructed/')
           + stem + '.cpp' for stem, *_ in CALLERS}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class TerrainQueryConnectionsTests(unittest.TestCase):
    def test_query_receivers_arguments_and_complete_caller_preservation(self):
        import capstone
        image_path = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image_path)
        # Both primary RTTI hierarchies independently identify the shared
        # nonpolymorphic four-byte base, at the same secondary offset.
        for vtable, name in ((0x55825c, b'.?AVTerrain@@'), (0x552508, b'.?AVEcoSystem@@')):
            col = struct.unpack('<I', retail.bytes_at_va(vtable - 4, 4))[0]
            _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(col, 20))
            self.assertEqual(offset, 0)
            self.assertEqual(retail.bytes_at_va(descriptor + 8, len(name) + 1), name + b'\0')
            _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
            bases = {}
            for address in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
                type_, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(address, 24))
                bases[retail.bytes_at_va(type_ + 8, 40).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
            self.assertEqual(bases[b'.?AVGroundFogableObject@@'], (0x2c, -1, 0))
        self.assertEqual(retail.bytes_at_va(0x4aae20, 9), bytes.fromhex('8bc1c70000000000c3'))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        with tempfile.TemporaryDirectory(prefix='mcm2-terrain-query-') as directory:
            sources = dict(SOURCES)
            sources['Terrain'] = 'src/krusty2/broadphase/Terrain.cpp'
            sources['layout'] = 'tests/fixtures/terrain_query_layout.cpp'
            sources['eco_layout'] = 'tests/fixtures/terrain_query_layout.cpp'
            objects = {}
            for stem, source in sources.items():
                path = Path(directory) / (stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                    *(['--extra', '/DCHECK_ECO'] if stem == 'eco_layout' else [])], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[stem] = CoffObject(path)
            for stem, symbol, va, size, count in CALLERS:
                obj = objects[stem]
                entry = obj.find_symbol(symbol)
                self.assertEqual(entry.function_size, size)
                _, extent, relocations = obj.symbol_extent(entry)
                self.assertEqual(extent, size)
                self.assertEqual(sum(obj.symbol_by_index[r.symbol_index].name == PROVIDER
                                     and r.type == 0x14 for r in relocations), count)
                instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                self.assertEqual(sum(i.size for i in instructions), size)
                self.assertEqual(sum(i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM
                                     and i.operands[0].imm == 0x507c10 for i in instructions), count)
                self.assertFalse(any(s.section_number == 0 and 'QueryGround@' in s.name
                                     and s.name != PROVIDER for s in obj.symbols))
            cases = [c for c in CASES if c['source'] in SOURCES.values()]
            for case in cases:
                obj = objects[Path(case['source']).stem]
                self.assertTrue(compare(retail, obj, case['symbol'], int(case['target_va'], 0),
                                        case['target_size'], ROOT / case['bindings'])['strict_exact'],
                                (case['source'], case['symbol']))
            provider = objects['Terrain']
            entry = provider.find_symbol(PROVIDER)
            body, size, relocations = provider.symbol_extent(entry)
            self.assertEqual((entry.function_size, size), (3321, 3321))
            records = [(r.virtual_address - entry.value, r.type,
                        provider.symbol_by_index[r.symbol_index].name) for r in relocations]
            self.assertEqual(hashlib.sha256(body + json.dumps(records).encode()).hexdigest(),
                             '07ea9bdeb16235841aaf1d125c77ee580b9b1667ec900de488c74df2f57207b3')
            print(f'{len(cases)} strict caller comparisons; 23 query call boundaries and complete partial provider preserved')


if __name__ == '__main__':
    unittest.main()
