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
from types import SimpleNamespace

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
    def source_filename(symbol):
        if not symbol.name.startswith('??_C@') or symbol.section_number <= 0:
            return None
        section = obj.sections[symbol.section_number - 1]
        value = obj.data[section.raw_ptr + symbol.value:section.raw_ptr + section.raw_size].split(b'\0')[0]
        filename = value.decode('latin1').replace('\\', '/')
        for source in SOURCES:
            if filename.endswith('/' + source):
                return 'source-filename:' + source
        return None

    def section_identity(number, visiting=()):
        section = obj.sections[number - 1]
        public = tuple(sorted((source_filename(s) or s.name, s.value) for s in obj.symbols
                              if s.section_number == number and s.storage_class == 2))
        if public:
            return ('public-section', public)
        # New public COMDATs can renumber later local targets. Anonymous sections
        # retain their ordinal among anonymous sections, flags, complete payload
        # and relocation destinations; distinct storage is not collapsed.
        anonymous = [sec.index for sec in obj.sections if not sec.name.startswith('.debug') and
                     not any(s.section_number == sec.index and s.storage_class == 2 for s in obj.symbols)]
        ordinal = anonymous.index(number)
        data = (obj.data[section.raw_ptr:section.raw_ptr + section.raw_size]
                if section.raw_ptr else bytes(section.raw_size))
        base = ('anonymous-section', ordinal, section.name, section.characteristics, data.hex())
        if number in visiting:
            return base + (('cycle', visiting.index(number)),)
        destinations = []
        for relocation in obj.relocations:
            if relocation.section_number != number:
                continue
            symbol = obj.symbol_by_index[relocation.symbol_index]
            if symbol.section_number > 0:
                destination = (section_identity(symbol.section_number, visiting + (number,)), symbol.value)
            else:
                destination = ('external', symbol.name, symbol.value, symbol.section_number)
            destinations.append((relocation.virtual_address, relocation.type, destination))
        return base + (tuple(destinations),)

    def identity(target):
        # Local label spelling and COFF section numbering are disposable. Their
        # stable public/anonymous section identity and exact offset are retained.
        if target.section_number > 0 and target.storage_class in (3, 6) and target.name.startswith(('$L', '$T')):
            return section_identity(target.section_number)
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


class VehicleGearboxFingerprintTests(unittest.TestCase):
    def test_local_labels_preserve_destination_and_byte_identity(self):
        entry = SimpleNamespace(value=0)
        target = SimpleNamespace(name='$L100', section_number=3, storage_class=6, value=16)
        first = SimpleNamespace(name='?First', section_number=3, storage_class=2, value=0)
        second = SimpleNamespace(name='?Second', section_number=4, storage_class=2, value=0)
        sections = [SimpleNamespace(index=i, name='.text', characteristics=0x20,
                                    raw_ptr=0, raw_size=0) for i in range(1, 5)]
        reloc = SimpleNamespace(virtual_address=1, type=6, symbol_index=1, section_number=1)
        obj = SimpleNamespace(find_symbol=lambda symbol: entry, symbol_by_index={1: target},
                              symbol_extent=lambda symbol: (b'\xa1\x00\x00\x00\x00', 5, [reloc]),
                              symbols=[target, first, second], sections=sections, data=b'', relocations=[])
        original = fingerprint(obj, 'method')
        target.name = '$L109'
        self.assertEqual(fingerprint(obj, 'method'), original)
        target.value = 20
        self.assertNotEqual(fingerprint(obj, 'method'), original)
        target.value = 16
        # Inserting a new earlier section shifts all existing numbers, while
        # the local destination remains exactly ?First+16.
        obj.sections.insert(0, SimpleNamespace(index=1, name='.text', characteristics=0x20,
                                              raw_ptr=0, raw_size=0))
        for i, section in enumerate(obj.sections, 1):
            section.index = i
        target.section_number += 1
        first.section_number += 1
        second.section_number += 1
        self.assertEqual(fingerprint(obj, 'method'), original)
        target.section_number = second.section_number
        self.assertNotEqual(fingerprint(obj, 'method'), original)
        target.section_number = first.section_number
        obj.symbol_extent = lambda symbol: (b'\xa3\x00\x00\x00\x00', 5, [reloc])
        self.assertNotEqual(fingerprint(obj, 'method'), original)
        obj.symbol_extent = lambda symbol: (b'\xa1\x00\x00\x00\x00', 5, [reloc])
        target.section_number, target.storage_class, target.name = 0, 2, '?ExternalOne'
        external = fingerprint(obj, 'method')
        target.name = '?ExternalTwo'
        self.assertNotEqual(fingerprint(obj, 'method'), external)

        # Anonymous local destinations retain payload and relocation identity;
        # inserting an unrelated public COMDAT does not change that identity.
        target.name, target.storage_class, target.section_number, target.value = '$L2', 6, 1, 0
        dependency = SimpleNamespace(name='?Dependency', section_number=0, storage_class=2, value=0)
        anonymous = SimpleNamespace(index=1, name='.rdata', characteristics=0x40, raw_ptr=1, raw_size=4)
        anonymous_reloc = SimpleNamespace(virtual_address=0, type=6, symbol_index=2, section_number=1)
        obj.symbols = [target, dependency]
        obj.symbol_by_index = {1: target, 2: dependency}
        obj.sections = [anonymous]
        obj.data = b'\0\x11\x22\x33\x44'
        obj.relocations = [anonymous_reloc]
        anonymous_original = fingerprint(obj, 'method')
        obj.data = b'\0\x12\x22\x33\x44'
        self.assertNotEqual(fingerprint(obj, 'method'), anonymous_original)
        obj.data = b'\0\x11\x22\x33\x44'
        dependency.name = '?DifferentDependency'
        self.assertNotEqual(fingerprint(obj, 'method'), anonymous_original)
        dependency.name = '?Dependency'
        public = SimpleNamespace(name='?NewComdat', section_number=1, storage_class=2, value=0)
        obj.symbols.append(public)
        obj.sections.insert(0, SimpleNamespace(index=1, name='.text', characteristics=0x20, raw_ptr=0, raw_size=0))
        anonymous.index = target.section_number = anonymous_reloc.section_number = 2
        self.assertEqual(fingerprint(obj, 'method'), anonymous_original)


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
