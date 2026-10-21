"""Actual race debug arrays/scalars: retail storage, uses, CRT startup and writes."""
import hashlib
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from test_race_event_connections import cases, fingerprint

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/BikeRaceDebugState.cpp'
CALLERS = ('src/reconstructed/BikeRace.cpp', 'samples/race/BikeRaceNearMisses.cpp')
# Existing names describe debug uses; no original translation-unit identity is inferred.
DATA = (
    ('g_ObstacleDebugLeftLine', 0x5779d8, 24, 0),
    ('g_ObstacleDebugUpperHit', 0x577a00, 4, 0),
    ('g_ObstacleDebugRightLine', 0x577a18, 24, 0),
    ('g_ObstacleDebugLine', 0x577a30, 24, 0),
    ('g_ObstacleDebugLowerLine', 0x577a48, 24, 0),
    ('g_ObstacleDebugHit', 0x578da8, 4, 0),
    ('g_ObstacleDebugLowerHit', 0x578dac, 4, 0),
    ('g_PathTargetDebugLine', 0x578dc0, 24, 0),
    ('g_ObstacleDebugRightHit', 0x578dd8, 4, 0),
    ('g_ObstacleDebugUpperLine', 0x578de0, 24, 0),
    ('g_ObstacleDebugLeftHit', 0x578df8, 4, 0),
    ('g_LodDebugSelectedRow', 0x567a88, 4, 2))


def external(name, size):
    return '?' + name + ('@@3PAUVector3@@A' if size == 24 else '@@3HA')


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class BikeRaceDebugStateTests(unittest.TestCase):
    def test_storage_producers_consumers_and_native_startup(self):
        import capstone
        image = Path(os.environ['MCM2_EXE'])
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        decoded = {va: list(decoder.disasm(retail.bytes_at_va(va, size), va)) for va, size in (
            (0x415640, 6112), (0x40d200, 4464), (0x423410, 885),
            (0x41d2b0, 5980), (0x41f5e0, 2644))}
        by_address = {i.address: i for instructions in decoded.values() for i in instructions}
        self.assertEqual((by_address[0x415c09].mnemonic, by_address[0x415c09].op_str), ('xor', 'eax, eax'))
        for va, address in ((0x415c1f, 0x577a00), (0x415c24, 0x578da8),
                            (0x415c29, 0x578dac), (0x415c2e, 0x578df8), (0x415c33, 0x578dd8)):
            self.assertEqual((by_address[va].mnemonic, by_address[va].op_str),
                             ('mov', 'dword ptr [0x%x], eax' % address))
        bindings = json.loads((ROOT / SOURCE.replace('.cpp', '.bindings.json')).read_text())
        self.assertEqual(bindings, {external(n, s): '0x%08x' % va for n, va, s, _ in DATA})
        fixture = 'tests/fixtures/bike_race_debug_state.cpp'
        with tempfile.TemporaryDirectory(prefix='mcm2-race-debug-') as directory:
            output = Path(directory)
            objects, paths = {}, {}
            for index, source in enumerate((SOURCE, fixture, *CALLERS)):
                obj = output / ('unit%d.obj' % index)
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt'],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(obj), obj
            provider = objects[SOURCE]
            self.assertEqual(provider.machine, 0x14c)
            definitions = {s.name: s for s in provider.symbols if s.storage_class == 2 and
                           s.section_number > 0 and not provider.section(s.section_number).characteristics & 0x20}
            self.assertEqual(set(definitions), set(bindings))
            self.assertFalse([s for s in provider.symbols if s.storage_class == 2 and s.section_number == 0])
            consumer = objects[CALLERS[0]]
            self.assertTrue(set(bindings).issubset({s.name for s in consumer.symbols if s.section_number == 0}))
            native_intervals, retail_intervals = [], []
            for name, va, size, initial in DATA:
                symbol = definitions[external(name, size)]
                section = provider.section(symbol.section_number)
                self.assertTrue(section.characteristics & 0x80000000)
                following = [s.value for s in definitions.values() if s.section_number == symbol.section_number
                             and s.value > symbol.value]
                allocation = min(following, default=section.raw_size) - symbol.value
                if size == 24:
                    self.assertEqual(allocation, size)
                else:
                    # Four-byte ints may precede alignment padding for an array.
                    self.assertGreaterEqual(allocation, size)
                native_intervals.append((symbol.section_number, symbol.value, symbol.value + size))
                retail_intervals.append((va, va + size))
                compiled = bytes(size) if section.raw_ptr == 0 else provider.data[
                    section.raw_ptr + symbol.value:section.raw_ptr + symbol.value + size]
                expected = struct.pack('<I', initial) + bytes(size - 4)
                self.assertEqual(compiled, expected)
                original_section = retail.section_for_rva(va - retail.image_base)
                self.assertEqual(original_section.name, '.data')
                self.assertTrue(original_section.characteristics & 0x80000000)
                offset = va - retail.image_base - original_section.virtual_address
                self.assertLessEqual(offset + size, original_section.virtual_size)
                if va < 0x578000:
                    self.assertLessEqual(offset + size, original_section.raw_size)
                    self.assertEqual(retail.bytes_at_va(va, size), expected)
                else:
                    self.assertGreaterEqual(offset, original_section.raw_size)
                    self.assertEqual(expected, bytes(size))
                if size == 24:
                    # Complete producer bodies store each component of both vectors.
                    stores = [op.mem.disp - va for base in (0x415640, 0x40d200) for i in decoded[base]
                              for op in i.operands if op.type == capstone.x86.X86_OP_MEM and
                              not op.mem.base and not op.mem.index and op.access & capstone.CS_AC_WRITE
                              and va <= op.mem.disp < va + size]
                    self.assertEqual(sorted(stores), [0, 4, 8, 12, 16, 20], name)
                    pushes = [i.operands[0].imm - va for i in decoded[0x423410] if i.mnemonic == 'push'
                              and i.operands[0].type == capstone.x86.X86_OP_IMM
                              and va <= i.operands[0].imm < va + size]
                    self.assertEqual(pushes, [12, 0], name)
                elif initial == 0:
                    self.assertTrue(any(i.mnemonic == 'mov' and any(op.type == capstone.x86.X86_OP_MEM and
                        not op.mem.base and not op.mem.index and op.mem.disp == va and op.size == 4
                        for op in i.operands) for i in decoded[0x423410]))
                else:
                    writes = [i for base in (0x41d2b0, 0x41f5e0) for i in decoded[base] for op in i.operands
                              if op.type == capstone.x86.X86_OP_MEM and op.access & capstone.CS_AC_WRITE
                              and not op.mem.base and not op.mem.index and op.mem.disp == va]
                    self.assertEqual(len(writes), 5)
                # Compiler references use direct array addresses and the +12 addend.
                if size == 24:
                    fn = consumer.find_symbol('?UnknownVirtualSlot14@BikeRace@@UAEHXZ')
                    body, _, relocs = consumer.symbol_extent(fn)
                    self.assertEqual(sorted(struct.unpack_from('<I', body, r.virtual_address - fn.value)[0]
                        for r in relocs if consumer.symbol_by_index[r.symbol_index].name == symbol.name), [0, 12])
            for i, (sec, start, end) in enumerate(native_intervals):
                self.assertFalse(any(sec == sec2 and start < end2 and start2 < end
                                     for sec2, start2, end2 in native_intervals[:i]))
            for i, (start, end) in enumerate(retail_intervals):
                self.assertFalse(any(start < end2 and start2 < end for start2, end2 in retail_intervals[:i]))
            # Ordinary Vector3 static initialization is real compiler output. All
            # six registered thunks jump to empty constructor-loop initializers;
            # there is no attribution of these local functions to retail VAs.
            crt = [s for s in provider.sections if s.name == '.CRT$XCU']
            self.assertEqual(len(crt), 1)
            self.assertEqual(crt[0].raw_size, 24)
            startup = [r for r in provider.relocations if r.section_number == crt[0].index]
            self.assertEqual(len(startup), 6)
            for reloc in startup:
                self.assertEqual(reloc.type, 6)
                body, size, relocs = provider.symbol_extent(provider.symbol_by_index[reloc.symbol_index])
                self.assertEqual((body, size), (b'\xe9\0\0\0\0', 5))
                self.assertEqual([(r.virtual_address, r.type) for r in relocs], [(1, 0x14)])
                target = provider.symbol_by_index[relocs[0].symbol_index]
                self.assertEqual(provider.symbol_extent(target), (b'\xc3', 1, []))
            partials = [r for r in json.loads((ROOT / 'tests/fixtures/race_event_partials.json').read_text())['partials']
                        if r[0] in CALLERS]
            partial_keys = {(r[0], r[1]) for r in partials}
            strict = 0
            for source, symbol, va, size, binding in cases():
                if source not in CALLERS or (source, objects[source].find_symbol(symbol).name) in partial_keys:
                    continue
                self.assertTrue(compare(retail, objects[source], symbol, va, size, ROOT / binding)['strict_exact'],
                                (source, symbol))
                strict += 1
            for source, symbol, extent, digest in partials:
                self.assertEqual(objects[source].find_symbol(symbol).function_size, extent)
                self.assertEqual(fingerprint(objects[source], symbol), digest, (source, symbol))
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'debug-state.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', '/out:' + str(executable),
                '/libpath:' + str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                str(paths[SOURCE]), str(paths[fixture]), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Race debug storage: CRT initial state and 336 independent mutations passed')
            print('%d strict callers, %d full partials, 12 storage objects; %s' %
                  (strict, len(partials), result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
