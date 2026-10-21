"""Prove inherited render helpers reach complete, existing Soultree providers."""
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
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
PROVIDERS = {
    '?SetAxesPtr@SoultreeObject@@QAEXPBUVec3@@0HH@Z': (0x4fbd70, 733),
    '?SetPosition@SoultreeObject@@QAEXABUVec3@@@Z': (0x4fc660, 38),
    '?Rotate@SoultreeObject@@QAEXUVec3@@M@Z': (0x4fceb0, 465),
    '?GetSubtreeBounds@SoultreeObject@@QAEXPAUVec3@@0@Z': (0x4fe850, 74),
}
CALLERS = ('D3DIMSoulTree', 'ArcadeObject', 'Krusty3DObjects', 'LightEmitter')


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and private retail image')
class RenderNodeConnections(unittest.TestCase):
    def test_complete_callers_and_inherited_provider_imports(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        cls = next(c for c in parse_rtti(retail) if c.name == 'D3DIMSoultreeObject')
        for name, offset in (('SoultreeObject', 0), ('GameObject', 12)):
            relation = next(b for b in cls.bases if b.name == name)
            self.assertEqual((relation.mdisp, relation.pdisp), (offset, -1))
        self.assertEqual({r.object_offset for r in cls.vtable_records}, {0, 12})

        sources = {name: 'src/reconstructed/' + name + '.cpp' for name in CALLERS}
        sources.update({'soultree': 'src/krusty2/soultree/soultree.cpp',
                        'layout': 'tests/fixtures/render_node_layout.cpp'})
        with tempfile.TemporaryDirectory(prefix='mcm2-render-node-') as temp:
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

            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}

            names = set().union(*(imports(objects[name]) for name in CALLERS))
            self.assertTrue(set(PROVIDERS) <= names)
            for method in ('SetAxesPtr', 'SetPosition', 'Rotate', 'GetSubtreeBounds'):
                self.assertFalse(any(s.startswith('?' + method + '@D3DIMSoultreeObject@@')
                                     for s in names))
            # Those two distinct unresolved boundaries are deliberately unchanged.
            self.assertIn('?SetPositionVec3@D3DIMSoultreeObject@@QAEXUVector3@@@Z', names)
            self.assertIn('?Scale@D3DIMSoultreeObject@@QAEXMMM@Z', names)
            definitions = {s.name for s in objects['soultree'].symbols
                           if s.section_number > 0 and s.storage_class == 2}
            self.assertTrue(set(PROVIDERS) <= definitions)
            for symbol, (va, size) in PROVIDERS.items():
                result = compare(retail, objects['soultree'], symbol, va, size,
                                 ROOT / 'src/krusty2/soultree/soultree.bindings.json')
                self.assertTrue(result['strict_exact'], result)

            by_source = {source: name for name, source in sources.items()}
            matched = 0
            for case in CASES:
                if case['source'] not in by_source:
                    continue
                result = compare(retail, objects[by_source[case['source']]], case['symbol'],
                                 int(case['target_va'], 0), case['target_size'],
                                 ROOT / case['bindings'])
                self.assertTrue(result['strict_exact'], result)
                matched += 1
            self.assertGreaterEqual(matched, 125)


if __name__ == '__main__':
    unittest.main()
