"""Native caller/provider and retail inheritance proof for garage lights."""
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
PROVIDERS = {
    '??0LightEmitter@@QAE@H@Z': (0x49deb0, 227),
    '?Create@LightEmitter@@QAEPAV1@PAXHIPBUVector3@@1MHHHHH@Z': (0x49e230, 348),
    '??0LightManager@@QAE@H@Z': (0x49e390, 78),
    '?AddLight@LightManager@@QAEXPAVLightEmitter@@@Z': (0x49e470, 23),
}


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires authentic VC6_ROOT and private retail EXE')
class GarageLightConnectionTests(unittest.TestCase):
    def test_complete_garage_caller_joins_existing_light_providers(self):
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(sha256_file(EXE), expected['mcm2_sha256'])
        retail = PEImage(EXE)
        classes = {c.name: c for c in parse_rtti(retail)}
        for name, vtable in [('LightEmitter', 0x5550cc), ('LightManager', 0x555144)]:
            cls = classes[name]
            self.assertTrue(any(r.object_offset == 0 and vtable in r.vtables
                                for r in cls.vtable_records))
            base = next(b for b in cls.bases if b.name == 'GameObject')
            self.assertEqual((base.mdisp, base.pdisp), (0, -1))
        sources = {
            'KrustyUI': 'src/reconstructed/KrustyUI.cpp',
            'LightEmitter': 'src/reconstructed/LightEmitter.cpp',
            'layout': 'tests/fixtures/garage_light_connections.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-garage-lights-') as directory:
            objects = {}
            for name, source in sources.items():
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
            caller = objects['KrustyUI']
            imports = {caller.symbol_by_index[r.symbol_index].name for r in caller.relocations
                       if caller.symbol_by_index[r.symbol_index].section_number == 0}
            self.assertTrue(set(PROVIDERS) <= imports)
            self.assertFalse(any('UnknownKrustyUILight' in s.name for s in caller.symbols))
            proof = compare(retail, caller, '?BuildGarageScene@KrustyUI@@QAEXH@Z',
                            0x498cf0, 3114, ROOT / 'src/reconstructed/KrustyUI.bindings.json')
            self.assertTrue(proof['strict_exact'], proof)
            for symbol, (address, size) in PROVIDERS.items():
                proof = compare(retail, objects['LightEmitter'], symbol, address, size,
                                ROOT / 'src/reconstructed/LightEmitter.bindings.json')
                self.assertTrue(proof['strict_exact'], proof)
            # Native base conversions must preserve both null and nonnull addresses.
            for symbol in ('?GarageEmitterBase@@YAPAVGameObject@@PAVLightEmitter@@@Z',
                           '?GarageManagerBase@@YAPAVGameObject@@PAVLightManager@@@Z'):
                obj = objects['layout']
                entry = next(s for s in obj.symbols if s.name == symbol)
                section = obj.sections[entry.section_number - 1]
                start = section.raw_ptr + entry.value
                self.assertEqual(entry.function_size, 5)
                self.assertEqual(obj.data[start:start + 5], bytes.fromhex('8b442404c3'))


if __name__ == '__main__':
    unittest.main()
