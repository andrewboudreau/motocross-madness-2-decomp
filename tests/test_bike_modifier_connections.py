"""Existing modifier providers retain model and modifier receiver boundaries."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/krusty2/vehicle/KrustyBike.cpp',
           'samples/physics/krustybike/KrustyBikeNearMisses.cpp',
           'src/reconstructed/GhostMod.cpp', 'src/reconstructed/MorphBastardModifier.cpp',
           'src/reconstructed/D3DIMSoultreeModifier.cpp', 'src/reconstructed/D3DIMSoulTree.cpp')
NATIVE = ((SOURCES[2], 'void GhostMod1::StartFade(', '?StartFade@GhostMod1@@', 0x47bbf0, 46),
          (SOURCES[4], 'void D3DIMSoultreeModifier::AddObject(', '?AddObject@D3DIMSoultreeModifier@@', 0x4452f0, 104),
          (SOURCES[4], 'void D3DIMSoultreeModifier::RemoveObject(', '?RemoveObject@D3DIMSoultreeModifier@@', 0x445360, 115),
          (SOURCES[5], 'void D3DIMSoultreeObject::AddModifier(', '?AddModifier@D3DIMSoultreeObject@@', 0x444d80, 84),
          (SOURCES[5], 'void D3DIMSoultreeObject::RemoveModifier(', '?RemoveModifier@D3DIMSoultreeObject@@', 0x444de0, 145),
          (SOURCES[5], 'void D3DIMSoultreeObject::AddSecondModifier(', '?AddSecondModifier@D3DIMSoultreeObject@@', 0x444eb0, 84),
          (SOURCES[5], 'void D3DIMSoultreeObject::RemoveSecondModifier(', '?RemoveSecondModifier@D3DIMSoultreeObject@@', 0x444f10, 145))
PARTIAL = [['samples/physics/krustybike/KrustyBikeNearMisses.cpp',
  'UnknownVirtualSlot70@KrustyBike',
  4790688,
  229,
  230,
  151,
  '3124025ed125d088a9cf9d706154f76eb28d5e8661543863782a91d540c2e19f'],
 ['samples/physics/krustybike/KrustyBikeNearMisses.cpp',
  'UnknownVirtualSlot11@KrustyBike',
  4791568,
  1296,
  1288,
  526,
  '2260143a9d880dfabfff868e69178f34173f372aa53195cc6d885ed5f14bc576'],
 ['samples/physics/krustybike/KrustyBikeNearMisses.cpp',
  'Fn_00492AD0@KrustyBike',
  4795088,
  2315,
  2312,
  192,
  '1941ddbc72fdfd7b01b5ad4ea65fcace024e8e828a4019450c91e9f4d1fb6a95'],
 ['samples/physics/krustybike/KrustyBikeNearMisses.cpp',
  'Fn_00493660@KrustyBike',
  4798048,
  8545,
  8558,
  2305,
  'ffc638804e115abac59baeed51fe496e2c2167f5f006b1c337aaee4f7e249815'],
 ['samples/physics/krustybike/KrustyBikeNearMisses.cpp',
  '??0KrustyBike@@QAE@H@Z',
  4782688,
  532,
  532,
  472,
  'f333644c75f0d39f6e5af3c3d8c898b85d85f9467e953b8d0e6f4c5771d00276'],
 ['samples/physics/krustybike/KrustyBikeNearMisses.cpp',
  'UnknownVirtualSlot102@KrustyBike',
  4779680,
  2920,
  2952,
  608,
  '1bbe35c0f4249dd69e1255c94f129b84d7d0f79ae2e94173ce021cf0a3a4b87c']]

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
class BikeModifierConnectionsTests(unittest.TestCase):
    def test_complete_providers_callers_and_native_boundaries(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         json.loads((ROOT / 'config/private_bundle_expected.json').read_text())['mcm2_sha256'])
        retail = PEImage(image)
        for table, name, game_offset in ((0x553d70, b'.?AVGhostMod1@@', 0),
                (0x555230, b'.?AVMorphBastardModifier@@', 0),
                (0x5513ec, b'.?AVD3DIMSoultreeObject@@', 12)):
            col, = struct.unpack('<I', retail.bytes_at_va(table - 4, 4))
            _, offset, _, td, hierarchy = struct.unpack('<5I', retail.bytes_at_va(col, 20))
            self.assertEqual(offset, 0)
            self.assertEqual(retail.bytes_at_va(td + 8, 48).split(b'\0')[0], name)
            _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
            bases = {}
            for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
                td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
                bases[retail.bytes_at_va(td + 8, 48).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
            self.assertEqual(bases[b'.?AVGameObject@@'], (game_offset, -1, 0))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        code = list(decoder.disasm(retail.bytes_at_va(0x48fc80, 5384), 0x48fc80))
        calls = [i.operands[0].imm for i in code if i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM]
        for address in (0x47bb20, 0x4a3150, 0x444eb0):
            self.assertEqual(calls.count(address), 1)
        pushes = [i.operands[0].imm for i in code if i.mnemonic == 'push' and i.operands[0].type == capstone.x86.X86_OP_IMM]
        self.assertIn(0x48, pushes)
        self.assertIn(0x58, pushes)
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-modifier-') as directory:
            output = Path(directory)
            extracted = []
            for source in dict.fromkeys(row[0] for row in NATIVE):
                path = output / Path(source).name
                header = (ROOT / source).with_suffix('.h')
                path.write_text('#include "' + header.as_posix() + '"\n#include "' +
                    (ROOT / 'src/reconstructed/D3DIMSoultreeModifier.h').as_posix() + '"\n#include "' +
                    (ROOT / 'src/reconstructed/DebugAlloc.h').as_posix() + '"\n'
                    '#define GHOST_MOD_NONE 3.402823466e+38F\n' + '\n\n'.join(
                        function_text((ROOT / source).read_text(), row[1]) for row in NATIVE if row[0] == source))
                extracted.append((source, str(path)))
            fixtures = ('tests/fixtures/bike_modifier_routes.cpp', 'tests/fixtures/bike_modifier_layouts.cpp')
            objects, paths = {}, {}
            for number, source in enumerate((*SOURCES, *(p for _, p in extracted), *fixtures)):
                path = output / ('unit%d.obj' % number)
                args = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                        '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if '/krusty2/' in source or '/physics/' in source or source == fixtures[1]:
                    args += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            for source, path in extracted:
                binding = (ROOT / source).with_suffix('.bindings.json')
                for _, _, symbol, va, size in (row for row in NATIVE if row[0] == source):
                    for obj in (objects[source], objects[path]):
                        entry = obj.find_symbol(symbol)
                        self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                        self.assertTrue(compare(retail, obj, symbol, va, size, binding)['strict_exact'], symbol)
            for source, symbol, va, size in ((SOURCES[2], '??0GhostMod1@@QAE@H@Z', 0x47bb20, 33),
                    (SOURCES[3], '??0MorphBastardModifier@@QAE@H@Z', 0x4a3150, 48)):
                self.assertTrue(compare(retail, objects[source], symbol, va, size,
                                       (ROOT / source).with_suffix('.bindings.json'))['strict_exact'])
            imports = {s.name for s in objects[SOURCES[0]].symbols if s.section_number == 0}
            wanted = ('??0GhostMod1@@', '??0MorphBastardModifier@@', '?StartFade@GhostMod1@@',
                      '?AddModifier@D3DIMSoultreeObject@@', '?RemoveModifier@D3DIMSoultreeObject@@',
                      '?AddSecondModifier@D3DIMSoultreeObject@@')
            for name in wanted:
                self.assertEqual(sum(s.startswith(name) for s in imports), 1)
            self.assertFalse(any('Modifier@KbXform' in s or 'StartFade@KbObj128' in s or
                                 '??0KbGhostMod' in s or '??0KbMorphMod' in s for s in imports))
            self.assertTrue(any(s.startswith('?LoadParameterFile@KbMorphMod@@') for s in imports))
            for name in ('_GhostPrimary', '_MorphPrimary'):
                self.assertEqual(objects[fixtures[1]].symbol_extent(objects[fixtures[1]].find_symbol(name)),
                                 (bytes.fromhex('8b442404c3'), 5, []))
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            for manifest in ('src/krusty2/vehicle/targets.json', 'samples/physics/krustybike/targets.json'):
                for target in json.loads((ROOT / manifest).read_text()):
                    if 'source' not in target or not target.get('bindings'): continue
                    source = (Path(manifest).parent / target['source']).as_posix()
                    if source in SOURCES:
                        cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                      target['target_size'], (Path(manifest).parent / target['bindings']).as_posix()))
            partial = {tuple(row[:4]): row[4:] for row in PARTIAL}
            strict = preserved = 0
            for source, symbol, va, size, binding in dict.fromkeys(cases):
                result = compare(retail, objects[source], symbol, va, size, ROOT / binding)
                key = (source, symbol, va, size)
                if key not in partial:
                    self.assertTrue(result['strict_exact'], key); strict += 1
                else:
                    extent, positions, digest = partial[key]
                    self.assertEqual((result['candidate_size'], result['strict_match']['matching_positions']), (extent, positions))
                    with patch('mcm2tool.resolved_match.compare_bytes',
                               lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                        resolved = match_object(objects[source], symbol, va, b'',
                            {k: int(v, 0) for k, v in json.loads((ROOT / binding).read_text()).items()})
                    self.assertEqual(resolved['digest'], digest, key); preserved += 1
            self.assertEqual(preserved, 6)
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'modifiers.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', *(str(paths[p]) for _, p in extracted),
                *(str(paths[p]) for p in fixtures), 'kernel32.lib'], cwd=ROOT, capture_output=True,
                text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Bike modifiers: 3712 attachment, removal and fade boundary cases passed', result.stdout)
            print('%d strict checks, six complete partial fingerprints, 14 dual-strict body checks; %s' %
                  (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
