"""Complete terrain caller/provider proofs and stream boundary preservation."""
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

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/reconstructed/Grid1.cpp', 'src/reconstructed/QuarryStuntEvent.cpp',
           'src/reconstructed/TrackOverlay.cpp', 'src/krusty2/broadphase/Terrain.cpp',
           'src/reconstructed/TerrainSupport.cpp', 'src/reconstructed/FileStream.cpp')
CONSTRUCTION_PROVIDERS = (
    ('MatrixUtil', '?IdentityMatrix@@YA?AUMatrix4@@XZ', 0x4a1410, 71, '??0Terrain@@', 0x505830, 370, 1),
    ('Quantize', '??0ColorMapper@@QAE@PAVUnknownTextureStream@@@Z', 0x4ddf40, 689, '?Load@Terrain@@', 0x5059d0, 2116, 1),
    ('Quantize', '?GetColors@ColorMapper@@QAEPAEXZ', 0x4de270, 4, '?Load@Terrain@@', 0x5059d0, 2116, 1),
    ('Gridbase', '??0GridBaseBlock@@QAE@PAVUnknownTextureStream@@@Z', 0x47dc20, 382, '?Load@Terrain@@', 0x5059d0, 2116, 1),
    ('Grid1', '??0DrawableGridNodeSharedTextures@@QAE@PAVUnknownTextureStream@@HPAVGridNode@@PAUGridTerrain@@HHHHHPAM3@Z',
     0x47c880, 176, '?Load@Terrain@@', 0x5059d0, 2116, 1),
    ('Griddraw', '?DrawVisible@DrawableGridNode@@QAEHPAX@Z', 0x480900, 23, '?UnknownVirtualSlot14@Terrain@@', 0x506220, 3173, 2),
    ('Griddraw', '?DrawVisibleWithDetail@DrawableGridNode@@QAEHPAXH@Z', 0x480920, 26, '?UnknownVirtualSlot14@Terrain@@', 0x506220, 3173, 2),
    ('Griddraw', '?ReleaseVertexBuffers@DrawableGridNode@@QAEXH@Z', 0x47edb0, 441, '??1Terrain@@', 0x5079f0, 441, 1),
)
PROVIDERS = (
    ('?RetireOwnedObject@Terrain@@QAEXPAVBaseObject@@@Z', 0x5057d0, 83),
    ('?SelectQuality@Terrain@@QAEXH@Z', 0x507960, 129),
    ('?CastSegment@Terrain@@QAEHPBUTerrainVec3@@0PAU2@HHH@Z', 0x506e90, 1654),
)
# Preserve the existing incomplete functions too. Their COFF bodies plus named
# relocation records are hashed because QueryGround still has unbound helpers.
# This is deliberately not a successful retail comparison of either function.
PARTIAL = (
    # Canonical ColorMapper, GridBaseBlock and shared-texture node calls.
    # Complete section bytes and relocation destinations are preserved;
    # only the four external names in this body change.
    ('?Load@Terrain@@', 2117, '6981e098cc6f3fe0abf740cd488861b58e7695a5b6cec74e7991323882bac3b9'),
    # Intentional cell-conversion repair, verified at the actual first-cell
    # boundary in test_terrain_cell_conversion; full interpolation remains partial.
    # The sampler, dot and triangle provider names are now canonical, with
    # complete bytes/relocations preserved under those explicit substitutions.
    ('QueryGround', 3255, '6962fa1f6369f5d34b4feb6fe57de2354071bb9dee7893f5a30e008bd10d652c'),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class TerrainConnectionsTests(unittest.TestCase):
    def test_existing_complete_providers_and_unchanged_callers(self):
        image_path = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(image_path)
        col = struct.unpack('<5I', retail.bytes_at_va(0x55f338, 20))
        self.assertEqual(col[1], 0)
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(col[4], 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, 4 * count)):
            descriptor, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            name = retail.bytes_at_va(descriptor + 8, 48).split(b'\0')[0]
            bases[name] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVTerrain@@'], (0, -1, 0))
        self.assertEqual(bases[b'.?AVGameObject@@'], (0, -1, 0))
        self.assertEqual(bases[b'.?AVGroundFogableObject@@'], (0x2c, -1, 0))
        for table, name, expected_bases in (
            (0x557658, b'.?AVColorMapper@@', (b'.?AVBaseObject@@',)),
            (0x553e54, b'.?AVDrawableGridNodeSharedTextures@@',
             (b'.?AVDrawableGridNode@@', b'.?AVGridNode@@')),
        ):
            locator, = struct.unpack('<I', retail.bytes_at_va(table - 4, 4))
            _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
            self.assertEqual(offset, 0)
            self.assertEqual(retail.bytes_at_va(descriptor + 8, 64).split(b'\0')[0], name)
            _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
            bases = {}
            for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
                descriptor, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
                bases[retail.bytes_at_va(descriptor + 8, 64).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
            for base in (name, *expected_bases):
                self.assertEqual(bases[base], (0, -1, 0))

        with tempfile.TemporaryDirectory(prefix='mcm2-terrain-connections-') as directory:
            objects = {}
            additional = tuple('src/reconstructed/' + name + '.cpp'
                               for name in ('MatrixUtil', 'Quantize', 'Gridbase', 'Griddraw'))
            for source in (*SOURCES, *additional, 'tests/fixtures/terrain_connection_layout.cpp'):
                path = Path(directory) / (Path(source).stem + '.obj')
                command = [
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt',
                ]
                # Reconstruction calibration supplies no physics search path.
                # Require its callers and the layout fixture to work that way.
                if source.startswith('src/krusty2/'):
                    command += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(path)
            provider = objects[SOURCES[3]]
            bindings = ROOT / 'src/krusty2/broadphase/Terrain.bindings.json'
            for caller, (symbol, va, size) in zip(SOURCES[:3], PROVIDERS):
                entry = provider.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertEqual(provider.symbol_extent(entry)[1], size)
                self.assertTrue(compare(retail, provider, symbol, va, size, bindings)['strict_exact'])
                obj = objects[caller]
                self.assertIn(symbol, {obj.symbol_by_index[r.symbol_index].name
                                       for r in obj.relocations if r.type == 0x14})
                retired = ('?RetireOwnedObject@Grid1Terrain@@QAEXPAVBaseObject@@@Z',
                           '?SelectQuality@UnknownTerrain@@QAEXH@Z',
                           '?CastSegment@UnknownTerrain@@QAEHPBUVector3@@0PAU2@HHH@Z')
                self.assertFalse(set(retired) & {s.name for s in obj.symbols})
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            manifest = ROOT / 'src/krusty2/broadphase/targets.json'
            for target in json.loads(manifest.read_text()):
                if 'source' not in target:
                    continue
                source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                if source in SOURCES and target.get('expect') != 'partial':
                    cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                  target['target_size'], manifest.parent / target['bindings']))
            for source, symbol, va, size, bindings in dict.fromkeys(cases):
                self.assertTrue(compare(retail, objects[source], symbol, va, size, bindings)['strict_exact'],
                                (source, symbol))
            self.assertEqual(len(dict.fromkeys(cases)), 190)
            # Load remains nonexact; keep its complete candidate instruction
            # bytes, while joining only the three evidenced stream calls.
            entry = provider.find_symbol('?Load@Terrain@@')
            body, size, relocations = provider.symbol_extent(entry)
            self.assertEqual(size, 2117)
            self.assertEqual(hashlib.sha256(body).hexdigest(),
                             '809febfee089efa6c3c7b4cd3ae3ff2e22f73f638260973ff5deb747d414bbaa')
            stream_calls = {'?Read@UnknownTextureStream@@QAEHPAXHH@Z': (0x461640, 14),
                            '?Seek@UnknownTextureStream@@QAEHHHH@Z': (0x461340, 3),
                            '?Tell@UnknownTextureStream@@QAEHXZ': (0x461600, 1)}
            calls = Counter(provider.symbol_by_index[r.symbol_index].name
                            for r in relocations if r.type == 0x14)
            for symbol, (va, count) in stream_calls.items():
                self.assertEqual(calls[symbol], count)
                self.assertNotIn(symbol.replace('@UnknownTextureStream@@', '@TerrainStream@@'), calls)
            import capstone
            decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
            decoder.detail = True
            for unit, symbol, va, extent, caller, caller_va, caller_extent, count in CONSTRUCTION_PROVIDERS:
                source = 'src/reconstructed/' + unit + '.cpp'
                obj = objects[source]
                entry = obj.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (extent, 'coff_function_aux'))
                self.assertTrue(compare(retail, obj, symbol, va, extent,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
                entry = provider.find_symbol(caller)
                _, _, relocations = provider.symbol_extent(entry)
                self.assertEqual(sum(r.type == 0x14 and provider.symbol_by_index[r.symbol_index].name == symbol
                                     for r in relocations), count)
                code = list(decoder.disasm(retail.bytes_at_va(caller_va, caller_extent), caller_va))
                self.assertEqual(sum(i.size for i in code), caller_extent)
                self.assertEqual(sum(i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM
                                     and i.operands[0].imm == va for i in code), count)
            imports = {s.name for s in provider.symbols if s.section_number == 0}
            self.assertFalse(any('TerrainColorMapper' in s or 'TerrainGridBlock' in s or 'GetIdentityMatrix@@' in s
                                 or s.startswith(('??0TerrainShutdownObject', '?Shutdown@TerrainShutdownObject',
                                                  '?DrawVisible@TerrainShutdownObject', '?DrawVisibleWithDetail@TerrainShutdownObject'))
                                 for s in imports))
            decoded = list(decoder.disasm(retail.bytes_at_va(0x5059d0, 2116), 0x5059d0))
            self.assertEqual(sum(i.size for i in decoded), 2116)
            retail_calls = Counter(i.operands[0].imm for i in decoded if i.mnemonic == 'call'
                                   and i.operands[0].type == capstone.x86.X86_OP_IMM)
            for va, count in stream_calls.values():
                self.assertEqual(retail_calls[va], count)
            for symbol, expected_size, digest in PARTIAL:
                entry = provider.find_symbol(symbol)
                body, size, relocations = provider.symbol_extent(entry)
                self.assertEqual(size, expected_size)
                records = [(r.virtual_address - entry.value, r.type,
                            provider.symbol_by_index[r.symbol_index].name) for r in relocations]
                self.assertEqual(hashlib.sha256(body + json.dumps(records).encode()).hexdigest(), digest)


if __name__ == '__main__':
    unittest.main()
