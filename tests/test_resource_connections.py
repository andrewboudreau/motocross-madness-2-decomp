"""Native ABI/provider proof for shared scene and resource-manager declarations."""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
PROVIDERS = {
    '??0UnknownResourceManager@@QAE@XZ': (0x4e8e80, 22),
    '??1UnknownResourceManager@@QAE@XZ': (0x4e8ea0, 159),
    '?FindItem@UnknownResourceManager@@QAEPAVResourceItem@@PBDH@Z': (0x4e9360, 133),
    '?AddLooseFile@UnknownResourceManager@@QAEHPBD0@Z': (0x4e9430, 640),
    '?AddObject@UnknownResourceManager@@QAEHPBDPAX@Z': (0x4e96b0, 383),
    '?SetItemObject@UnknownResourceManager@@QAEXPAX0@Z': (0x4e9010, 31),
    '?RemoveArchiveItems@UnknownResourceManager@@QAEHPAVUnknownTextureStream@@@Z': (0x4e9830, 194),
}


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires authentic VC6_ROOT and private retail EXE')
class ResourceConnectionTests(unittest.TestCase):
    def test_real_callers_join_strict_providers_and_scene_lifecycle(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(EXE.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        sources = {name: f'src/reconstructed/{name}.cpp' for name in (
            'ResourceManager', 'SceneManager', 'TextureMap', 'PCAudio',
            'Palette8', 'Quantize', 'GameUi', 'EcoSystem', 'KrustyUI')}
        sources['layout'] = 'tests/fixtures/resource_connections.cpp'
        with tempfile.TemporaryDirectory(prefix='mcm2-resource-connections-') as directory:
            objects = {}
            for name, source in sources.items():
                obj = Path(directory) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(obj)
            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}
            callers = set().union(*(imports(obj) for name, obj in objects.items()
                                    if name not in ('ResourceManager', 'layout')))
            self.assertTrue(set(PROVIDERS) <= callers)
            for symbol, (address, size) in PROVIDERS.items():
                proof = compare(retail, objects['ResourceManager'], symbol, address, size,
                                ROOT / 'src/reconstructed/ResourceManager.bindings.json')
                self.assertTrue(proof['strict_exact'], proof)
            scene = imports(objects['SceneManager'])
            self.assertTrue(set(list(PROVIDERS)[:2]) <= scene)
            for name, obj in objects.items():
                self.assertFalse(any('UnknownSceneResourceManager' in s.name or
                                     'UnknownResourceEntry@@' in s.name for s in obj.symbols), name)
                self.assertNotIn('?SetItemObject@UnknownResourceManager@@QAEXPAXH@Z', imports(obj))
                self.assertNotIn('?AddObject@UnknownResourceManager@@QAEXPBDH@Z', imports(obj))


if __name__ == '__main__':
    unittest.main()
