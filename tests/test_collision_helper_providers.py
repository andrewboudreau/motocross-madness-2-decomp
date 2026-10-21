"""Verify exact helper bodies, decoded direct calls, and supplied source symbols."""
from __future__ import annotations

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
from mcm2tool.toolchain import sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and the private retail EXE')
class CollisionHelperProviderTests(unittest.TestCase):
    def test_exact_bodies_direct_call_identity_and_dependency_closure(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        # Decoded instruction boundaries in QueryCollisions, HullVsModel,
        # UpdateRelativeMotion, and the still-partial ApplyContactImpulse.
        call_sites = {
            0x438f45: 0x43a270, 0x439055: 0x43a1e0,
            0x437581: 0x43c890, 0x4376b2: 0x43c890, 0x43aa1e: 0x43c890,
            0x43c524: 0x43ca20, 0x43c75b: 0x43ca20,
        }
        for site, target in call_sites.items():
            instruction = retail.bytes_at_va(site, 5)
            self.assertEqual(instruction[0], 0xe8)
            self.assertEqual(site + 5 + struct.unpack('<i', instruction[1:])[0], target)

        providers = ('CollisionQueries', 'CollisionVectorHelpers')
        sources = {name: f'src/reconstructed/{name}.cpp' for name in providers}
        sources.update({name: f'src/krusty2/collision/{name}.cpp'
                        for name in ('CollisionObject', 'CollisionPoint')})
        with tempfile.TemporaryDirectory(prefix='mcm2-collision-helpers-') as directory:
            objects = {}
            for name, source in sources.items():
                path = Path(directory) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(path)

            path = ROOT / 'samples/physics/collision/targets.json'
            matched = {}
            for target in json.loads(path.read_text()):
                source = (path.parent / target['source']).resolve()
                if source.stem not in providers:
                    continue
                self.assertEqual(source, ROOT / sources[source.stem])
                result = compare(retail, objects[source.stem], target['candidate_symbol_contains'],
                                 int(target['target_va'], 0), target['target_size'],
                                 path.parent / target['bindings'])
                self.assertTrue(result['strict_exact'], result)
                self.assertEqual(result['candidate_size'], target['target_size'])
                matched[int(target['target_va'], 0)] = result['symbol']
            self.assertEqual(set(matched), {0x43a1e0, 0x43a270, 0x43c890, 0x43ca20})

            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}

            self.assertTrue({matched[va] for va in (0x43a1e0, 0x43a270, 0x43c890)}
                            <= imports(objects['CollisionObject']))
            self.assertIn(matched[0x43c890], imports(objects['CollisionPoint']))
            for name in providers:
                obj = objects[name]
                self.assertEqual({s.name for s in obj.symbols if s.section_number == 0},
                                 {'__fltused'})
                definitions = {s.name for s in obj.symbols
                               if s.storage_class == 2 and s.section_number > 0}
                expected = {matched[va] for va in ((0x43a1e0, 0x43a270)
                            if name == 'CollisionQueries' else (0x43c890, 0x43ca20))}
                self.assertTrue(expected <= definitions)
                # Header-generated vector storage and initializer entry points
                # must remain private; no retail TU ownership is inferred here.
                constants = [s for s in obj.symbols if s.name.startswith('_kVec3')]
                self.assertEqual(len(constants), 4)
                self.assertTrue(all(s.storage_class == 3 for s in constants))
                self.assertEqual([(s.raw_size, s.reloc_count) for s in obj.sections
                                  if s.name == '.CRT$XCU'], [(16, 4)])


if __name__ == '__main__':
    unittest.main()
