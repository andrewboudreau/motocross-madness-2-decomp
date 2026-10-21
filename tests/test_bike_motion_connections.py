"""Bike rider-motion provider and existing TU-local zero-vector connections."""
from collections import Counter
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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from test_character_motion_connections import PROVIDERS as OLD_PROVIDERS, function_text
from test_vehicle_gearbox_connections import cases as gearbox_cases, fingerprint

ROOT = Path(__file__).resolve().parents[1]
CALLERS = ('src/krusty2/vehicle/KrustyBike.cpp', 'samples/physics/krustybike/KrustyBikeNearMisses.cpp')
MOTION = 'src/krusty2/motion/Motnctrl.cpp'
PROVIDERS = (*OLD_PROVIDERS,
    ('void Character::ApplyPoseListSlot6(', '?ApplyPoseListSlot6@Character@@QAEXPAUMotionPoseList@@H@Z', 0x4a8ac0, 59),
    ('void Character::ApplyRestPose(', '?ApplyRestPose@Character@@QAEXXZ', 0x4a8b00, 15))
CONNECTED = (PROVIDERS[1], PROVIDERS[3], PROVIDERS[5])


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class BikeMotionConnectionsTests(unittest.TestCase):
    def test_complete_callers_motion_closure_and_local_storage(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        classes = {c.name: c for c in parse_rtti(retail)}
        relation = next(b for b in classes['D3DIMSoultreeCharacter'].bases if b.name == 'Character')
        self.assertEqual((relation.mdisp, relation.pdisp, relation.vdisp), (0, -1, 0))
        self.assertNotEqual(next(b for b in classes['D3DIMSoultreeCharacter'].bases if b.name == 'GameObject').pdisp, -1)
        # Bike::Load constructs the rider's actual primary object; existing
        # complete caller proofs retain its subsequent store and use at +5c4.
        loader = list(decoder.disasm(retail.bytes_at_va(0x4079c0, 6745), 0x4079c0))
        self.assertEqual(sum(i.mnemonic == 'call' and i.op_str == '0x4455b0' for i in loader), 1)
        partials = {tuple(row[:4]): row[4:] for row in json.loads(
            (ROOT / 'tests/fixtures/vehicle_gearbox_partials.json').read_text())['partials'] if row[0] in CALLERS}
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-motion-') as directory:
            output = Path(directory)
            motion_source = output / 'motion_routes.cpp'
            motion_source.write_text('#include <string.h>\n#include "motion/Motnctrl.h"\n'
                'extern MotionManager* g_pMotionManager;\n' + '\n\n'.join(
                function_text((ROOT / MOTION).read_text(), signature) for signature, _, _, _ in PROVIDERS))
            zero_source = output / 'bike_zero.cpp'
            zero_source.write_text('#include "vehicle/KrustyBike.h"\n#include "math/Math3D.h"\n' +
                function_text((ROOT / CALLERS[0]).read_text(), 'Vec3 KrustyBike::UnknownVirtualSlot76(') +
                '\nextern "C" const Vec3* BikeLocalZero() { return &kVec3Zero; }\n')
            fixture = 'tests/fixtures/bike_motion_routes.cpp'
            paths, objects = {}, {}
            for index, source in enumerate((*CALLERS, MOTION, str(motion_source), str(zero_source), fixture)):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True, text=True,
                    encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[source], objects[source] = obj, CoffObject(obj)
            for _, symbol, va, size in PROVIDERS:
                for source in (MOTION, str(motion_source)):
                    obj = objects[source]
                    self.assertEqual((obj.find_symbol(symbol).function_size, obj.find_symbol(symbol).extent_source),
                                     (size, 'coff_function_aux'))
                    self.assertTrue(compare(retail, obj, symbol, va, size,
                        ROOT / 'src/krusty2/motion/Motnctrl.bindings.json')['strict_exact'], symbol)
            for source in (*CALLERS, str(zero_source)):
                obj = objects[source]
                binding = ROOT / (CALLERS[0] if source == str(zero_source) else source)
                binding = binding.with_suffix('.bindings.json')
                self.assertTrue(compare(retail, obj, '_$E1', 0x4911a0, 60, binding)['strict_exact'])
                zero = obj.find_symbol('_kVec3Zero')
                section = obj.sections[zero.section_number - 1]
                self.assertEqual(zero.storage_class, 3)
                self.assertEqual(section.name, '.bss')
                self.assertTrue(section.characteristics & 0x80000000)  # writable, initialized at startup
                initializer = obj.find_symbol('_$E1')
                _, _, relocs = obj.symbol_extent(initializer)
                self.assertEqual([(r.virtual_address - initializer.value, obj.symbol_by_index[r.symbol_index].name)
                                  for r in relocs], [(40, '_kVec3Zero'), (46, '_kVec3Zero'), (52, '_kVec3Zero')])
                self.assertTrue(compare(retail, obj, '_$E2', 0x491190, 5, binding)['strict_exact'])
                thunk = obj.find_symbol('_$E2')
                self.assertEqual([(r.virtual_address - thunk.value, obj.symbol_by_index[r.symbol_index].name)
                    for r in obj.symbol_extent(thunk)[2]], [(1, '_$E1')])
                self.assertEqual(sum(obj.symbol_by_index[r.symbol_index].name == '_$E2' and
                    obj.sections[r.section_number - 1].name == '.CRT$XCU' for r in obj.relocations), 1)
                if source != CALLERS[1]:
                    self.assertFalse(any('g_kbZeroVec' in s.name for s in obj.symbols))
                self.assertTrue(compare(retail, obj, 'UnknownVirtualSlot76@KrustyBike', 0x491ca0, 103, binding)['strict_exact'])
            addresses = {symbol: va for _, symbol, va, _ in CONNECTED}
            rows = [r for r in gearbox_cases() if r[0] in CALLERS]
            for target in json.loads((ROOT / 'src/krusty2/motion/targets.json').read_text()):
                if target.get('source') == 'Motnctrl.cpp' and target.get('bindings'):
                    rows.append((MOTION, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                 target['target_size'], 'src/krusty2/motion/' + target['bindings']))
            strict = preserved = 0
            calls = {}
            for source, symbol, va, size, binding in dict.fromkeys(rows):
                obj = objects[source]
                result = compare(retail, obj, symbol, va, size, ROOT / binding)
                key = (source, symbol, va, size)
                if key in partials:
                    extent, positions, digest = partials[key]
                    self.assertEqual((result['candidate_size'], result['strict_match'].get('matching_positions')), (extent, positions), key)
                    self.assertEqual(fingerprint(obj, symbol), digest, key)
                    preserved += 1
                else:
                    self.assertTrue(result['strict_exact'], key)
                    strict += 1
                if source in CALLERS:
                    entry = obj.find_symbol(symbol)
                    _, _, relocs = obj.symbol_extent(entry)
                    connected = Counter(obj.symbol_by_index[r.symbol_index].name for r in relocs
                        if r.type == 0x14 and obj.symbol_by_index[r.symbol_index].name in addresses)
                    original = Counter(i.operands[0].imm for i in decoder.disasm(retail.bytes_at_va(va, size), va)
                        if i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM)
                    for name, count in connected.items():
                        self.assertEqual(original[addresses[name]], count, key)
                        calls[(source, entry.name, name)] = count
            self.assertEqual(preserved, len(partials))
            for source, count in zip(CALLERS, (65, 66)):
                self.assertEqual(sum(n for (s, _, _), n in calls.items() if s == source), count)
                self.assertFalse(any(any(old in s.name for old in ('FindMotion@KbA5C4', 'SetMotion@KbA5C4',
                    'ApplyRestPose@KbA5C4')) for s in objects[source].symbols))
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'routes.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                *(str(paths[key]) for key in (str(motion_source), str(zero_source), fixture)), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.splitlines(), [
                'Character motion: 3840 lookup, state and missing-name cases passed',
                'Bike rest pose: 576 actual helper and callback cases passed',
                'Bike motion handles: 256 direct pointer round trips passed',
                'Bike local zero: 256 actual caller storage and return guards passed'])
            print('%d strict bodies, %d complete partial fingerprints, 41 production motion calls; 4928 native cases passed' % (strict, preserved))


if __name__ == '__main__':
    unittest.main()
