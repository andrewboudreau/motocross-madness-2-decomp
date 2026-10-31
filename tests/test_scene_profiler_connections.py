"""Production COFF and retail proof for scene profiling provider connections."""
from __future__ import annotations

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
SOURCES = {
    'visibility': 'src/krusty2/visibility/VisibilityQuadTree.cpp',
    'soultree': 'src/krusty2/soultree/SoultreeQuadTreeRenderer.cpp',
    'terrain': 'src/krusty2/broadphase/Terrain.cpp',
    'overlay': 'src/reconstructed/DebugOverlay.cpp',
    'peak': 'src/reconstructed/PeakHold.cpp',
    'clock': 'src/reconstructed/PCGame.cpp',
    'layout': 'tests/fixtures/scene_profiler_layout.cpp',
}
PROVIDERS = (
    ('overlay', '?Print@DebugOverlay@@QAAXHPBDZZ', 0x447f40, 94),
    ('overlay', '?Title@DebugOverlay@@QAAXHPBDZZ', 0x447fa0, 94),
    ('peak', '??0UnknownPeakHold@@QAE@I@Z', 0x4cb670, 30),
    ('peak', '?Peak@UnknownPeakHold@@QAEHXZ', 0x4cb690, 28),
    ('peak', '?Record@UnknownPeakHold@@QAEXH@Z', 0x4cb6b0, 43),
    ('clock', '?ReadClock@@YAIXZ', 0x4bfa80, 29),
)


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires authentic VC6_ROOT and private retail EXE')
class SceneProfilerConnectionTests(unittest.TestCase):
    def test_production_callers_providers_and_timer_storage(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        classes = {c.name: c for c in parse_rtti(retail)}
        for derived, base in (('TrackGame', 'Game'), ('DebugOverlay', 'GameObject')):
            relation = next(b for b in classes[derived].bases if b.name == base)
            self.assertEqual((relation.mdisp, relation.pdisp), (0, -1))

        with tempfile.TemporaryDirectory(prefix='mcm2-scene-profiler-') as directory:
            objects = {}
            for name, source in SOURCES.items():
                path = Path(directory) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(path)

            imports = {}
            for name in ('visibility', 'soultree', 'terrain'):
                obj = objects[name]
                imports[name] = {obj.symbol_by_index[r.symbol_index].name
                                 for r in obj.relocations
                                 if obj.symbol_by_index[r.symbol_index].section_number == 0}
                self.assertFalse(any(old in s for s in imports[name] for old in
                                     ('VisibilityStat@@', 'TerrainDebugOverlay@@',
                                      'SoultreeLogLine@@', 'SoultreeSetLineName@@',
                                      'LogLine@@', 'SetLineName@@', '?ReadClock@@YAHXZ')))
            for owner, symbol, address, size in PROVIDERS:
                callers = ('visibility', 'soultree', 'terrain') if owner == 'overlay' else ('visibility',)
                for caller in callers:
                    self.assertIn(symbol, imports[caller])
                definitions = [s for o in objects.values() for s in o.symbols
                               if s.name == symbol and s.storage_class == 2 and s.section_number > 0]
                self.assertEqual(len(definitions), 1, symbol)
                proof = compare(retail, objects[owner], symbol, address, size,
                                ROOT / Path(SOURCES[owner]).with_suffix('.bindings.json'))
                self.assertTrue(proof['strict_exact'], proof)

            # Includes complete variadic callers and the actual static initializer.
            for owner, symbol, address, size in (
                ('visibility', '?PrepareGeometry@VisibilityQuadTree', 0x52ef50, 252),
                ('visibility', '_$E9', 0x52d330, 16),
                ('soultree', 'UnknownVirtualSlot14@SoultreeQuadTreeRenderer', 0x504570, 166),
            ):
                proof = compare(retail, objects[owner], symbol, address, size,
                                ROOT / Path(SOURCES[owner]).with_suffix('.bindings.json'))
                self.assertTrue(proof['strict_exact'], proof)

            # Terrain slot 14 is the known-partial case (targets.json expect: partial):
            # declaring Terrain::PrepareGeometry swaps two stack-slot offsets.
            proof = compare(retail, objects['terrain'], '?UnknownVirtualSlot14@Terrain@@', 0x506220, 3173,
                            ROOT / Path(SOURCES['terrain']).with_suffix('.bindings.json'))
            self.assertEqual((proof['strict_match']['matching_positions'],
                              [(m['offset'], m['retail'], m['candidate']) for m in proof['strict_match']['mismatches']]),
                             (3171, [(2200, 24, 28), (2205, 28, 24)]), proof)

            # Guard the production global, not merely a separately compiled size probe:
            # retail initializer passes 0x68ab90; its constructor writes +0,+4,+8.
            obj = objects['visibility']
            timer = next(s for s in obj.symbols
                         if s.name == '?g_visibilityTickStat@@3VUnknownPeakHold@@A')
            section = obj.sections[timer.section_number - 1]
            self.assertEqual(section.name, '.bss')
            self.assertTrue(section.characteristics & 0x80000000)
            self.assertLessEqual(timer.value + 12, section.raw_size)
            neighbors = [s.value for s in obj.symbols if s.section_number == timer.section_number
                         and s.value > timer.value and not s.name.startswith('.')]
            self.assertTrue(all(value >= timer.value + 12 for value in neighbors))


if __name__ == '__main__':
    unittest.main()
