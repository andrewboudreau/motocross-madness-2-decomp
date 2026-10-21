"""Vehicle gearbox providers, complete caller emissions and scalar boundaries."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/krusty2/vehicle/Vehicle.cpp', 'src/krusty2/vehicle/KrustyBike.cpp',
           'src/krusty2/vehicle/Bike.cpp', 'samples/physics/krustybike/KrustyBikeNearMisses.cpp',
           'samples/physics/bikeai/BikeAINearMisses.cpp', 'src/reconstructed/GearRatios.cpp')
PROVIDERS = (('??0UnknownEngineGearbox@@', 0x4d2940, 1542, 36),
             ('?UpdateThrottle@UnknownEngineGearbox@@', 0x4d2f50, 215, 12),
             ('?AutoShift@UnknownEngineGearbox@@', 0x4d3030, 373, 8),
             ('?DriveForce@UnknownEngineGearbox@@', 0x4d31b0, 388, 28))


def cases():
    rows = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], c['bindings'])
            for c in CASES if c['source'] in SOURCES]
    for base in ('src/krusty2', 'samples/physics'):
        for manifest in (ROOT / base).rglob('targets.json'):
            for target in json.loads(manifest.read_text()):
                if not target.get('source') or not target.get('bindings'):
                    continue
                source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                if source in SOURCES:
                    rows.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                 target['target_size'], (manifest.parent / target['bindings']).relative_to(ROOT).as_posix()))
    rows += [(SOURCES[3], symbol, va, size, SOURCES[3].replace('.cpp', '.bindings.json'))
             for source, symbol, va, size, _ in list(rows) if source == SOURCES[1]]
    return list(dict.fromkeys(rows))


def fingerprint(obj, symbol):
    entry = obj.find_symbol(symbol)
    body, size, relocs = obj.symbol_extent(entry)
    def identity(target):
        if target.name.startswith('??_C@') and target.section_number > 0:
            section = obj.sections[target.section_number - 1]
            value = obj.data[section.raw_ptr + target.value:section.raw_ptr + section.raw_size].split(b'\0')[0]
            filename = value.decode('latin1').replace('\\', '/')
            # Physical __FILE__ spelling depends on the checkout location.
            # Normalize only a complete recognized source-path suffix.
            for source in SOURCES:
                if filename.endswith('/' + source):
                    return 'source-filename:' + source
        return target.name
    targets = [(r.virtual_address - entry.value, r.type,
                identity(obj.symbol_by_index[r.symbol_index]),
                obj.symbol_by_index[r.symbol_index].value) for r in relocs]
    return hashlib.sha256(body + json.dumps(targets).encode()).hexdigest()


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class VehicleGearboxConnectionsTests(unittest.TestCase):
    def test_complete_providers_callers_layout_and_scalar_boundaries(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        partials = {tuple(row[:4]): row[4:] for row in json.loads(
            (ROOT / 'tests/fixtures/vehicle_gearbox_partials.json').read_text())['partials']}
        with tempfile.TemporaryDirectory(prefix='mcm2-gearbox-') as directory:
            output = Path(directory)
            fixture = 'tests/fixtures/vehicle_gearbox_boundaries.cpp'
            objects, paths = {}, {}
            for index, source in enumerate((*SOURCES, fixture)):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(obj), obj
            addresses = {}
            for symbol, va, size, popped in PROVIDERS:
                entry = objects[SOURCES[5]].find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, objects[SOURCES[5]], symbol, va, size,
                    ROOT / 'src/reconstructed/GearRatios.bindings.json')['strict_exact'])
                instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                self.assertEqual(sum(i.size for i in instructions), size)
                returns = [i for i in instructions if i.mnemonic == 'ret']
                self.assertTrue(returns)
                self.assertTrue(all(i.operands[0].imm == popped for i in returns))
                addresses[entry.name] = va
            strict = preserved = 0
            connected_callers = {}
            for source, symbol, va, size, binding in cases():
                obj = objects[source]
                result = compare(retail, obj, symbol, va, size, ROOT / binding)
                key = (source, symbol, va, size)
                if key not in partials:
                    self.assertTrue(result['strict_exact'], key)
                    strict += 1
                else:
                    extent, positions, digest = partials[key]
                    self.assertEqual((result['candidate_size'], result['strict_match'].get('matching_positions')),
                                     (extent, positions), key)
                    self.assertEqual(fingerprint(obj, symbol), digest, key)
                    preserved += 1
                entry = obj.find_symbol(symbol)
                _, _, relocs = obj.symbol_extent(entry)
                connected = Counter(obj.symbol_by_index[r.symbol_index].name for r in relocs
                                    if r.type == 0x14 and obj.symbol_by_index[r.symbol_index].name in addresses)
                if connected:
                    instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
                    original = Counter(i.operands[0].imm for i in instructions if i.mnemonic == 'call'
                                       and i.operands[0].type == capstone.x86.X86_OP_IMM)
                    for name, count in connected.items():
                        self.assertEqual(original[addresses[name]], count, (source, symbol, name))
                    connected_callers[(source, entry.name)] = connected
            self.assertEqual(preserved, len(partials))
            production_calls = sum(sum(c.values()) for (s, _), c in connected_callers.items() if s in SOURCES[:3])
            self.assertEqual(production_calls, 5)
            for source in SOURCES:
                self.assertFalse(any(('UpdateThrottle@VehicleSpeedState' in s.name or
                                     'AutoShift@VehicleSpeedState' in s.name or
                                     'DriveForce@VehicleWheelAux' in s.name or
                                     'DriveForce@BikeWheelQ' in s.name or
                                     '??0VehicleSpeedState' in s.name) for s in objects[source].symbols))
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'boundaries.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                str(paths[SOURCES[5]]), str(paths[fixture]), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Gearbox boundaries: 48 native cases passed')
            print('%d strict bodies, %d full partial fingerprints, five production calls; %s' %
                  (strict, preserved, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
