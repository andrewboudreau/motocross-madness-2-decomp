"""Complete collision callers connect to supplied query bodies without ABI drift."""
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.resolved_match import match_object
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
COLLISION = 'src/krusty2/collision/CollisionObject.cpp'
QUERY = 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp'
NODE = 'src/krusty2/soultree/soultree.cpp'
SOURCES = (COLLISION, QUERY, 'src/krusty2/motion/CollisionCharacter.cpp',
           'samples/physics/bvh/BoundingBoxTreeQueryNearMisses.cpp',
           'samples/physics/bvh/BoundingBoxTreeBuildNearMisses.cpp',
           'samples/physics/collision/CollisionObjectNearMisses.cpp')
PROVIDERS = (
    ('TreeTreeQuery', 0x428950, 708, 5),
    ('TreeBoxOverlap', 0x428c20, 395, 4),
    ('SphereTreeQuery', 0x429570, 789, 2),
    ('SphereTreeQueryWithVertices', 0x429540, 48, 2),
    ('CapsuleTreeQueryWithVertices', 0x429890, 48, 2),
    ('GetPositionIn@SoultreeObject', 0x4fc9a0, 134, 3),
    ('SweptBoxOverlap', 0x424730, 891, 6),
    ('BoxSphereOverlap', 0x425750, 419, 2),
    ('GrowBoxByTransformedBox', 0x4290d0, 805, 2),
    ('Vec3TransformNormal', 0x42a450, 91, 5),
    ('Vec3TransformPoint', 0x42a510, 100, 3),
)


def function_text(text, name):
    match = re.search(r'^(?:inline )?(?:void|int) ' + re.escape(name) + r'\(', text, re.M)
    assert match, name
    start = text.index('{', match.end())
    depth, end = 1, start + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[match.start():end]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class CollisionQueryConnectionsTests(unittest.TestCase):
    def test_complete_callers_providers_and_native_boundaries(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        evidence = json.loads((ROOT / 'tests/fixtures/collision_query_partials.json').read_text())
        # The node position receiver is the primary SoultreeObject pointer. The
        # separate GameObject base at +12 must not be applied to this call.
        locator, = struct.unpack('<I', retail.bytes_at_va(0x5513ec - 4, 4))
        _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(descriptor + 8, 48).split(b'\0')[0], b'.?AVD3DIMSoultreeObject@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
            td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(td + 8, 48).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVSoultreeObject@@'], (0, -1, 0))
        self.assertEqual(bases[b'.?AVGameObject@@'], (12, -1, 0))
        with tempfile.TemporaryDirectory(prefix='mcm2-collision-query-') as directory:
            output = Path(directory)
            extracted = output / 'query_helpers.cpp'
            text = (ROOT / QUERY).read_text()
            native_names = ('BoxSphereOverlap', 'GrowBoxByTransformedBox', 'Vec3TransformNormal', 'Vec3TransformPoint')
            extracted.write_text('#include "bvh/BoundingBoxTreeQuery.h"\n' +
                function_text(text, 'TransformPointGB') + '\n' +
                '\n'.join(function_text(text, name) for name in native_names) + '\n')
            fixture = 'tests/fixtures/collision_query_boundaries.cpp'
            jobs = [(source, source, []) for source in (*SOURCES, NODE, str(extracted), fixture)]
            jobs.append(('query_layout', fixture, ['--extra', '/DQUERY_LAYOUT']))
            objects, paths = {}, {}
            for index, (key, source, extra) in enumerate(jobs):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'), *extra], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[key], paths[key] = CoffObject(obj), obj
            imported = Counter(objects[COLLISION].symbol_by_index[r.symbol_index].name
                               for r in objects[COLLISION].relocations if r.type == 0x14)
            self.assertFalse(set(imported) & evidence['renames'].keys())
            for name, va, size, sites in PROVIDERS:
                source = NODE if name.startswith('GetPositionIn') else QUERY
                symbol = '?' + name + '@@'
                entry = objects[source].find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, objects[source], symbol, va, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], name)
                self.assertEqual(imported[entry.name], sites, name)
                instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                self.assertEqual(sum(i.size for i in instructions), size)
                returns = [i for i in instructions if i.mnemonic == 'ret']
                if name.startswith('GetPositionIn'):
                    self.assertTrue(returns and all(i.op_str == '8' for i in returns))
                else:
                    self.assertTrue(all(not i.op_str for i in returns), name)
                if name in native_names:
                    other = objects[str(extracted)].find_symbol(symbol)
                    self.assertEqual((other.function_size, other.extent_source), (size, 'coff_function_aux'))
                    self.assertTrue(compare(retail, objects[str(extracted)], symbol, va, size,
                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], name)
            self.assertEqual(sum(row[3] for row in PROVIDERS), 36)
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
            # This aggregate includes the entire canonical CollisionObject.cpp;
            # protect those emitted bodies as well as its additional capsule query.
            cases += [('samples/physics/collision/CollisionObjectNearMisses.cpp', symbol, va, size, binding)
                      for source, symbol, va, size, binding in list(cases) if source == COLLISION]
            partials = {tuple(row[:4]): row[4:] for row in evidence['partials']}
            strict = preserved = 0
            checked_callers = set()
            addresses = {objects[NODE if name.startswith('GetPositionIn') else QUERY].find_symbol('?' + name + '@@').name: va
                         for name, va, _, _ in PROVIDERS}
            for source, symbol, va, size, binding in dict.fromkeys(cases):
                result = compare(retail, objects[source], symbol, va, size, binding)
                if source == COLLISION:
                    entry = objects[source].find_symbol(symbol)
                    _, _, relocs = objects[source].symbol_extent(entry)
                    calls = Counter(objects[source].symbol_by_index[r.symbol_index].name
                                    for r in relocs if r.type == 0x14)
                    connected = {name: count for name, count in calls.items() if name in addresses}
                    if connected:
                        instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                        # Registered partial windows can include jump-table data
                        # after RET. Count direct calls without treating those
                        # legacy windows as independently proven CFG extents.
                        original = Counter(i.operands[0].imm for i in instructions if i.mnemonic == 'call'
                                           and i.operands[0].type == capstone.x86.X86_OP_IMM)
                        for name, count in connected.items():
                            self.assertEqual(original[addresses[name]], count, (symbol, name))
                        checked_callers.add(entry.name)
                key = (source, symbol, va, size)
                if key not in partials:
                    self.assertTrue(result['strict_exact'], key); strict += 1
                else:
                    extent, positions, digest = partials[key]
                    self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']),
                                     (extent, positions), key)
                    with patch('mcm2tool.resolved_match.compare_bytes',
                               lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                        resolved = match_object(objects[source], symbol, va, b'',
                            {k: int(v, 0) for k, v in json.loads(binding.read_text()).items()})
                    self.assertEqual(resolved['digest'], digest, key); preserved += 1
            self.assertEqual(preserved, 31)
            self.assertEqual(len(checked_callers), 17)
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'query_boundaries.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(paths[str(extracted)]), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Collision query boundaries: 10535 native cases passed', result.stdout)
            print('%d strict cases, 31 complete partial fingerprints, 11 supplied providers, 36 call sites; %s' %
                  (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
