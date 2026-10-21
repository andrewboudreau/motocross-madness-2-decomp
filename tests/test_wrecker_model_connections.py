"""Verify Wrecker's complete callers, typed layouts and real provider closure."""
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
from mcm2tool.toolchain import sha256_file
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
PROVIDERS = {
    0x4a8ac0, 0x4a8b10, 0x4a8b40, 0x4a8b80,
    0x4fc690, 0x4fc740, 0x4fc890, 0x4fc9a0,
    0x4fca80, 0x4fcc70, 0x4fd660, 0x4fe850,
}
REMAINING = {
    '?SetMatrixIn@SoultreeObject@@QAEXPAV1@PAUMatrix4@@@Z',
    '?WorldToLocalDirection@SoultreeObject@@QAE?AUVec3@@ABU2@@Z',
}


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and private retail image')
class WreckerModelConnections(unittest.TestCase):
    def test_complete_callers_layouts_and_provider_imports(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(EXE)
        sources = {
            'wrecker': 'src/reconstructed/Wrecker.cpp',
            'soultree': 'src/krusty2/soultree/soultree.cpp',
            'motion': 'src/krusty2/motion/Motnctrl.cpp',
            'layout': 'tests/fixtures/wrecker_model_layout.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-wrecker-models-') as temp:
            objects = {}
            for name, source in sources.items():
                output = Path(temp) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(output), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(output)

            callers = [case for case in CASES if case['source'] == sources['wrecker']]
            self.assertGreaterEqual(len(callers), 29)
            for case in callers:
                result = compare(image, objects['wrecker'], case['symbol'],
                                 int(case['target_va'], 0), case['target_size'],
                                 ROOT / case['bindings'])
                self.assertTrue(result['strict_exact'], result)
            folded = compare(image, objects['wrecker'],
                             '?UnknownVirtualSlot14@Wrecker@@UAEHXZ', 0x4de400, 5,
                             ROOT / 'src/reconstructed/Wrecker.bindings.json')
            self.assertTrue(folded['strict_exact'], folded)

            caller = objects['wrecker']
            imports = {caller.symbol_by_index[r.symbol_index].name for r in caller.relocations
                       if caller.symbol_by_index[r.symbol_index].section_number == 0}
            self.assertFalse(any('@UnknownWreckerSkeleton@@' in name
                                 or '@UnknownWreckerCharacter@@' in name for name in imports))
            self.assertTrue(REMAINING <= imports)
            definitions = {s.name for name in ('soultree', 'motion')
                           for s in objects[name].symbols
                           if s.section_number > 0 and s.storage_class == 2}
            self.assertTrue(REMAINING.isdisjoint(definitions))

            matched = set()
            for name in ('soultree', 'motion'):
                target_file = ROOT / ('src/krusty2/' + name + '/targets.json')
                for case in json.loads(target_file.read_text()):
                    va = int(case['target_va'], 0)
                    if va not in PROVIDERS:
                        continue
                    result = compare(image, objects[name], case['candidate_symbol_contains'],
                                     va, case['target_size'], target_file.parent / case['bindings'])
                    self.assertTrue(result['strict_exact'], result)
                    self.assertIn(result['symbol'], imports)
                    self.assertIn(result['symbol'], definitions)
                    matched.add(va)
            self.assertEqual(matched, PROVIDERS)


if __name__ == '__main__':
    unittest.main()
