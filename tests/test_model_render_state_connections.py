"""Primary model render-state calls use complete existing source providers."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
MODEL = 'src/reconstructed/D3DIMSoulTree.cpp'
NODE = 'src/krusty2/soultree/soultree.cpp'
SOURCES = ('src/krusty2/vehicle/Bike.cpp', 'src/krusty2/vehicle/Vehicle.cpp',
    'src/krusty2/vehicle/KrustyBike.cpp', 'src/reconstructed/BikeRace.cpp',
    'src/reconstructed/KrustyUI.cpp', 'src/reconstructed/SceneManager.cpp',
    'src/krusty2/soultree/SoulTreePhysics.cpp',
    'samples/physics/krustybike/KrustyBikeNearMisses.cpp', 'samples/race/BikeRaceNearMisses.cpp',
    'samples/track/SceneManagerNearMisses.cpp', MODEL, NODE)
PROVIDERS = (
    ('SetUseLighting', 'int value', 0x4444c0, 23),
    ('ClearScreenRects', '', 0x4444e0, 55),
    ('SetLod', 'int lod', 0x444d00, 56),
    ('SetLowestLod', 'int lod', 0x444d40, 32),
    ('SetLodThreshold', 'float value', 0x444d60, 19),
    ('ApplyLodSettings', 'int index', 0x4451e0, 81),
)
RENAMES = {
    '?SetUseLighting@' + view + '@@QAEXH@Z': '?SetUseLighting@D3DIMSoultreeObject@@QAEXH@Z'
    for view in ('BikeA1A0', 'UnknownKrustyUISoultree', 'UnknownSceneLodObject', 'SoultreeD3DNode')}
RENAMES.update({
    '?ClearScreenRects@BikeA1A0@@QAEXXZ': '?ClearScreenRects@D3DIMSoultreeObject@@QAEXXZ',
    '?Method_004444E0@VehSceneNodeView@@QAEXXZ': '?ClearScreenRects@D3DIMSoultreeObject@@QAEXXZ',
    '?SetLod@KbXform@@QAEXH@Z': '?SetLod@D3DIMSoultreeObject@@QAEXH@Z',
    '?SetLowestLod@KbXform@@QAEXH@Z': '?SetLowestLod@D3DIMSoultreeObject@@QAEXH@Z',
    '?ApplyLodSettings@UnknownBikeRaceModel@@QAEXH@Z': '?ApplyLodSettings@D3DIMSoultreeObject@@QAEXH@Z',
    '?ApplyLodSettings@UnknownSceneLodObject@@QAEXH@Z': '?ApplyLodSettings@D3DIMSoultreeObject@@QAEXH@Z',
})


def function_text(text, signature):
    start = text.index(signature)
    end = text.index('{', start) + 1
    depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class ModelRenderStateConnectionsTests(unittest.TestCase):
    def test_complete_callers_and_native_state_boundaries(self):
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
            json.loads((ROOT / 'config/private_bundle_expected.json').read_text())['mcm2_sha256'])
        retail = PEImage(image)
        classes = {c.name: c for c in parse_rtti(retail)}
        model = classes['D3DIMSoultreeObject']
        self.assertEqual({r.object_offset for r in model.vtable_records}, {0, 12})
        for name, offset in (('SoultreeObject', 0), ('GameObject', 12)):
            relation = next(b for b in model.bases if b.name == name)
            self.assertEqual((relation.mdisp, relation.pdisp), (offset, -1))
        relation = next(b for b in classes['SoultreePhysicsObject'].bases if b.name == 'D3DIMSoultreeObject')
        self.assertEqual((relation.mdisp, relation.pdisp), (0x21c, -1))
        evidence = json.loads((ROOT / 'tests/fixtures/model_render_state_partials.json').read_text())
        with tempfile.TemporaryDirectory(prefix='mcm2-model-state-') as directory:
            output = Path(directory)
            extracted = output / 'D3DIMSoulTree.cpp'
            extracted.write_text('#include "' + (ROOT / MODEL).with_suffix('.h').as_posix() + '"\n' +
                '\n\n'.join(function_text((ROOT / MODEL).read_text(), 'void D3DIMSoultreeObject::' + name + '(')
                             for name, _, _, _ in PROVIDERS) + '\n\n' +
                function_text((ROOT / NODE).read_text(), 'void SoultreeObject::SetFlag14c('))
            fixture = 'tests/fixtures/model_render_state.cpp'
            layout = 'tests/fixtures/model_state_layout.cpp'
            jobs = [(p, p, []) for p in (*SOURCES, str(extracted), fixture)]
            jobs += [('layout', layout, []), ('physics_layout', layout, ['--extra', '/DPHYSICS_VIEW'])]
            objects, paths = {}, {}
            for number, (key, source, extra) in enumerate(jobs):
                path = output / ('unit%d.obj' % number)
                args = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if '/krusty2/' in source or '/physics/' in source or key == 'physics_layout':
                    args += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(args + extra, cwd=ROOT, capture_output=True, text=True,
                    encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[key], paths[key] = CoffObject(path), path
            for name, _, va, size in PROVIDERS:
                symbol = '?' + name + '@D3DIMSoultreeObject@@'
                for key in (MODEL, str(extracted)):
                    entry = objects[key].find_symbol(symbol)
                    self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                    self.assertTrue(compare(retail, objects[key], symbol, va, size,
                        (ROOT / MODEL).with_suffix('.bindings.json'))['strict_exact'], (key, symbol))
            for key in (NODE, str(extracted)):
                symbol = '?SetFlag14c@SoultreeObject@@QAEXXZ'
                self.assertEqual(objects[key].find_symbol(symbol).function_size, 11)
                self.assertTrue(compare(retail, objects[key], symbol, 0x4fdb40, 11,
                    (ROOT / NODE).with_suffix('.bindings.json'))['strict_exact'])
            for key in ('layout', 'physics_layout'):
                self.assertEqual(objects[key].symbol_extent(objects[key].find_symbol('_ModelGameBase')),
                    (bytes.fromhex('b80c000000c3'), 6, []))
            self.assertEqual(objects['physics_layout'].symbol_extent(objects['physics_layout'].find_symbol('_PhysicsModelBase')),
                (bytes.fromhex('b81c020000c3'), 6, []))
            imported = set()
            counts = []
            for source in SOURCES[:7]:
                obj = objects[source]
                names = [obj.symbol_by_index[r.symbol_index].name for r in obj.relocations if r.type == 0x14]
                counts.append(sum(name in RENAMES.values() for name in names))
                imported.update(names)
                self.assertFalse(set(names) & RENAMES.keys(), source)
            # The two Load branches share one emitted SetLod call site.
            self.assertEqual(counts, [2, 1, 7, 6, 3, 4, 2])
            self.assertTrue(set(RENAMES.values()) <= imported)
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            for base in ('src/krusty2', 'samples/physics'):
                for manifest in (ROOT / base).rglob('targets.json'):
                    for target in json.loads(manifest.read_text()):
                        if not target.get('source') or not target.get('bindings'): continue
                        source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                        if source in SOURCES:
                            cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                          target['target_size'], manifest.parent / target['bindings']))
            partial = {tuple(row[:4]): row[4:] for row in evidence['partials']}
            for source, symbol, va, size in partial:
                cases.append((source, symbol, va, size, (ROOT / source).with_suffix('.bindings.json')))
            strict = preserved = 0
            for source, symbol, va, size, binding in dict.fromkeys(cases):
                result = compare(retail, objects[source], symbol, va, size, binding)
                key = (source, symbol, va, size)
                if key not in partial:
                    self.assertTrue(result['strict_exact'], key); strict += 1
                else:
                    extent, positions, digest = partial[key]
                    self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']),
                                     (extent, positions), key)
                    with patch('mcm2tool.resolved_match.compare_bytes',
                               lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                        resolved = match_object(objects[source], symbol, va, b'',
                            {k: int(v, 0) for k, v in json.loads(binding.read_text()).items()})
                    self.assertEqual(resolved['digest'], digest, key); preserved += 1
            self.assertEqual(preserved, 26)
            for source, symbol, va, size, digest in evidence['additional_functions']:
                self.assertEqual(objects[source].find_symbol(symbol).function_size, size)
                bindings = {k: int(v, 0) for k, v in json.loads((ROOT / source).with_suffix('.bindings.json').read_text()).items()}
                with patch('mcm2tool.resolved_match.compare_bytes',
                           lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                    resolved = match_object(objects[source], symbol, va, b'', bindings)
                self.assertEqual(resolved['digest'], digest, symbol)
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'model_state.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(paths[str(extracted)]), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Model render state: 29040 complete storage cases passed', result.stdout)
            print('%d strict cases, 28 complete candidate fingerprints, seven dual-strict helpers; %s' %
                  (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
