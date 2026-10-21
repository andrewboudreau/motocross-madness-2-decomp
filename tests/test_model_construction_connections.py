"""Existing model construction and scene helpers preserve primary/base boundaries."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
CAR = 'src/reconstructed/CarProcedural.cpp'
MOTION = 'src/krusty2/motion/D3DIMSoultreeMotnctrl.cpp'
MODEL = 'src/reconstructed/D3DIMSoulTree.cpp'
NODE = 'src/krusty2/soultree/soultree.cpp'
PARTIAL = 'samples/render/D3DIMSoulTreeNearMisses.cpp'
SOURCES = (CAR, MOTION, MODEL, NODE, PARTIAL)
HELPERS = (
    ('Vec3* SoultreeObject::GetPosition(', '?GetPosition@SoultreeObject@@QAEPAUVec3@@PAU2@@Z', 0x4fc970, 33),
    ('SoultreeObject* SoultreeObject::FindByName(', '?FindByName@SoultreeObject@@QAEPAV1@PBD@Z', 0x4fdae0, 84),
    ('void SoultreeObject::GetLocalBounds(', '?GetLocalBounds@SoultreeObject@@QAEXPAUVec3@@0@Z', 0x4fe0a0, 74),
)
CONSTRUCTOR = '??0D3DIMSoultreeObject@@QAE@H@Z'
# Complete pre-change candidate fingerprints: these remain explicitly nonexact.
# Same-path baseline/current object proof additionally checks all section bytes,
# attributes and relocation destinations, including the jump table in DrawCurrentLod.
PARTIALS = (
    ('?PrepareGeometry@D3DIMSoultreeObject@@UAEHXZ', 0x443aa0, 832,
     '5ea97b888c0f3af61fdbaac2464fe3c4b06d884438864907303f96c1f90161a0'),
    ('?DrawCurrentLod@D3DIMSoultreeObject@@QAEXXZ', 0x440f30, 7848,
     'e713ca27d59df9f4ed4c24700c77830bc75bfc652734a4d8298b403ffa42ad6e'),
)


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
class ModelConstructionConnectionsTests(unittest.TestCase):
    def test_complete_callers_providers_and_native_node_boundaries(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         json.loads((ROOT / 'config/private_bundle_expected.json').read_text())['mcm2_sha256'])
        retail = PEImage(image)
        for table, expected_offset in ((0x5513ec, 0), (0x55137c, 12)):
            locator, = struct.unpack('<I', retail.bytes_at_va(table - 4, 4))
            _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
            self.assertEqual(offset, expected_offset)
            self.assertEqual(retail.bytes_at_va(descriptor + 8, 48).split(b'\0')[0], b'.?AVD3DIMSoultreeObject@@')
            _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
            bases = {}
            for pointer in struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, count * 4)):
                td, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
                bases[retail.bytes_at_va(td + 8, 48).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
            self.assertEqual(bases[b'.?AVSoultreeObject@@'], (0, -1, 0))
            self.assertEqual(bases[b'.?AVGameObject@@'], (12, -1, 0))
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        with tempfile.TemporaryDirectory(prefix='mcm2-model-connections-') as directory:
            output = Path(directory)
            extracted = output / 'soultree.cpp'
            extracted.write_text('#define SOULTREE_OBJECT_WITH_BASES\n#include <string.h>\n'
                '#include "math/Math3D.h"\n' + '\n\n'.join(
                    function_text((ROOT / NODE).read_text(), row[0]) for row in HELPERS))
            fixture = 'tests/fixtures/model_node_routes.cpp'
            layout = 'tests/fixtures/model_connection_layout.cpp'
            jobs = [(p, p, []) for p in (*SOURCES, str(extracted), fixture)]
            jobs += [('layout', layout, []), ('motion_layout', layout, ['--extra', '/DMOTION_VIEW'])]
            objects, paths = {}, {}
            for number, (key, source, extra) in enumerate(jobs):
                path = output / ('unit%d.obj' % number)
                args = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                        '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if source in (MOTION, NODE, str(extracted), fixture) or key == 'motion_layout':
                    args += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(args + extra, cwd=ROOT, capture_output=True, text=True,
                    encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[key], paths[key] = CoffObject(path), path
            for _, symbol, va, size in HELPERS:
                for key in (NODE, str(extracted)):
                    obj = objects[key]
                    entry = obj.find_symbol(symbol)
                    self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                    self.assertTrue(compare(retail, obj, symbol, va, size,
                                            (ROOT / NODE).with_suffix('.bindings.json'))['strict_exact'], (key, symbol))
            self.assertEqual(objects[MODEL].find_symbol(CONSTRUCTOR).function_size, 275)
            self.assertTrue(compare(retail, objects[MODEL], CONSTRUCTOR, 0x43f160, 275,
                                    (ROOT / MODEL).with_suffix('.bindings.json'))['strict_exact'])
            for key in ('layout', 'motion_layout'):
                for symbol in ('_LocalModelSecondary', '_CanonicalModelSecondary'):
                    self.assertEqual(objects[key].symbol_extent(objects[key].find_symbol(symbol)),
                                     (bytes.fromhex('b80c000000c3'), 6, []))
            # Four actual allocation/constructor boundaries, and the complete
            # Car and render call paths. Assert the emitted and retail direct calls.
            callers = (
                (CAR, 'UnknownVirtualSlot27@CarProcedural', 0x42f600, 1020,
                 ((CONSTRUCTOR, 0x43f160, 1), (HELPERS[0][1], 0x4fc970, 1),
                  (HELPERS[1][1], 0x4fdae0, 7), (HELPERS[2][1], 0x4fe0a0, 1))),
                (MOTION, 'CharacterVirtualSlot0@D3DIMSoultreeCharacter', 0x445a70, 1021,
                 ((CONSTRUCTOR, 0x43f160, 2),)),
                (MOTION, 'CharacterVirtualSlot9@D3DIMSoultreeCharacter', 0x445ec0, 248,
                 ((CONSTRUCTOR, 0x43f160, 1),)),
                (MODEL, 'ComputeSubtreeBounds@D3DIMSoultreeObject', 0x4434b0, 243,
                 ((HELPERS[2][1], 0x4fe0a0, 1),)),
            )
            for source, symbol, va, size, wanted in callers:
                obj = objects[source]
                self.assertTrue(compare(retail, obj, symbol, va, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'], symbol)
                _, _, relocs = obj.symbol_extent(obj.find_symbol(symbol))
                calls = Counter(obj.symbol_by_index[r.symbol_index].name for r in relocs if r.type == 0x14)
                code = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                self.assertEqual(sum(i.size for i in code), size)
                direct = Counter(i.operands[0].imm for i in code if i.mnemonic == 'call'
                                 and i.operands[0].type == capstone.x86.X86_OP_IMM)
                for target, address, count in wanted:
                    self.assertEqual(calls[target], count)
                    self.assertEqual(direct[address], count)
                if source != MODEL:
                    self.assertEqual(sum(i.mnemonic == 'push' and i.operands[0].type == capstone.x86.X86_OP_IMM
                                         and i.operands[0].imm == 0x2d8 for i in code), wanted[0][2])
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            for manifest in ('src/krusty2/motion/targets.json', 'src/krusty2/soultree/targets.json'):
                for target in json.loads((ROOT / manifest).read_text()):
                    if 'source' not in target or not target.get('bindings'): continue
                    source = (Path(manifest).parent / target['source']).as_posix()
                    if source in SOURCES:
                        cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                      target['target_size'], (Path(manifest).parent / target['bindings']).as_posix()))
            strict = 0
            for source, symbol, va, size, binding in dict.fromkeys(cases):
                self.assertTrue(compare(retail, objects[source], symbol, va, size, ROOT / binding)['strict_exact'],
                                (source, symbol, va, size))
                strict += 1
            bindings = {k: int(v, 0) for k, v in json.loads((ROOT / PARTIAL).with_suffix('.bindings.json').read_text()).items()}
            for symbol, va, size, digest in PARTIALS:
                self.assertEqual(objects[PARTIAL].find_symbol(symbol).function_size, size)
                with patch('mcm2tool.resolved_match.compare_bytes',
                           lambda original, candidate: {'digest': hashlib.sha256(candidate).hexdigest()}):
                    result = match_object(objects[PARTIAL], symbol, va, b'', bindings)
                self.assertEqual(result['digest'], digest, symbol)
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'model_nodes.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(paths[str(extracted)]), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Model nodes: 17416 lookup, position and bounds cases passed', result.stdout)
            print('%d strict cases, two complete partial fingerprints, three dual-strict native helpers; %s' %
                  (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
