"""Retail storage, mutability, lifecycle writes and canonical caller closure."""
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
# Name, VA, scalar format, initial value, independently decoded write.
VALUES = (
    ('LatencyHiding', 0x56cb3c, 'i', 1, 0x490a55),
    ('RateLimiting', 0x56cb40, 'i', 1, 0x490a6b),
    ('AllowWarping', 0x56cb44, 'i', 1, 0x490a81),
    ('UseExtrapLimit', 0x56cb48, 'i', 1, 0x490aad),
    ('ExtrapLimit', 0x56cb4c, 'f', 0.5, 0x490adb),
    ('LatencyThreshold', 0x56cb50, 'f', 0.5, 0x490b04),
    ('WarpThreshold', 0x56cb54, 'f', 0.2, 0x490b29),
    ('UseTimeReceived', 0x56cb58, 'i', 1, 0x490b3a),
    ('AllowNegative', 0x56cb5c, 'i', 1, 0x490b50),
    ('Interpolate', 0x56cb60, 'i', 1, 0x490b60),
    ('UseLatencyThreshold', 0x67c3a0, 'i', 0, 0x490a97),
    ('PadActive', 0x67c3ac, 'i', 0, 0x4964a0),
)


@unittest.skipUnless(os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail image')
class KrustyBikeStateTests(unittest.TestCase):
    def test_full_storage_lifecycle_and_actual_callers(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        bindings = json.loads((ROOT / 'src/reconstructed/KrustyBikeState.bindings.json').read_text())
        expected = {'?g_kb' + n + '@@3' + ('M' if t == 'f' else 'H') + 'A': va
                    for n, va, t, value, write in VALUES}
        self.assertEqual({k: int(v, 0) for k, v in bindings.items()}, expected)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        with tempfile.TemporaryDirectory(prefix='mcm2-bike-state-') as directory:
            objects = {}
            for name, source in (
                ('state', 'src/reconstructed/KrustyBikeState.cpp'),
                ('caller', 'src/krusty2/vehicle/KrustyBike.cpp'),
            ):
                obj = Path(directory) / (name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(obj)
            obj = objects['state']
            definitions = {s.name: s for s in obj.symbols if s.storage_class == 2 and s.section_number > 0}
            self.assertEqual(set(definitions), set(expected))
            self.assertFalse([s for s in obj.symbols if s.storage_class == 2 and s.section_number == 0])
            self.assertFalse([r for r in obj.relocations
                              if not obj.section(r.section_number).name.startswith('.debug')])
            self.assertFalse([s for s in obj.sections if s.name.startswith('.CRT') or s.characteristics & 0x20])
            self.assertEqual(sum(s.raw_size for s in obj.sections if s.characteristics & 0x80000000), 48)
            for name, va, fmt, initial, write in VALUES:
                symbol = '?g_kb' + name + '@@3' + ('M' if fmt == 'f' else 'H') + 'A'
                entry = definitions[symbol]
                section = obj.section(entry.section_number)
                following = [s.value for s in definitions.values() if s.section_number == entry.section_number and s.value > entry.value]
                self.assertEqual(min(following, default=section.raw_size) - entry.value, 4)
                zero = va >= 0x670000
                self.assertTrue(section.characteristics & 0x80000000)
                self.assertTrue(section.characteristics & 0x40000000)
                self.assertEqual(bool(section.characteristics & 0x80), zero)
                data = bytes(4) if zero else obj.data[section.raw_ptr + entry.value:section.raw_ptr + entry.value + 4]
                self.assertEqual(data, struct.pack('<' + fmt, initial))
                original = retail.section_for_rva(va - retail.image_base)
                self.assertEqual(original.name, '.data')
                self.assertEqual(original.characteristics, 0xc0000040)
                offset = va - retail.image_base - original.virtual_address
                self.assertLessEqual(offset + 4, original.virtual_size)
                if zero:
                    self.assertGreaterEqual(offset, original.raw_size)
                else:
                    self.assertLessEqual(offset + 4, original.raw_size)
                    self.assertEqual(retail.bytes_at_va(va, 4), data)
                instruction = next(decoder.disasm(retail.bytes_at_va(write, 15), write))
                self.assertEqual(instruction.mnemonic, 'fstp' if fmt == 'f' else 'mov')
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_MEM)
                self.assertEqual(instruction.operands[0].mem.disp, va)
            clear = next(decoder.disasm(retail.bytes_at_va(0x4964cc, 10), 0x4964cc))
            self.assertEqual(clear.mnemonic, 'mov')
            self.assertEqual(clear.operands[0].mem.disp, 0x67c3ac)
            self.assertEqual(clear.operands[1].imm, 0)
            caller = objects['caller']
            imports = {caller.symbol_by_index[r.symbol_index].name for r in caller.relocations
                       if caller.symbol_by_index[r.symbol_index].section_number == 0}
            self.assertTrue(set(expected) <= imports)
            manifest = ROOT / 'src/krusty2/vehicle/targets.json'
            targets = [t for t in json.loads(manifest.read_text()) if t['source'] == 'KrustyBike.cpp']
            self.assertEqual(len(targets), 76)
            for target in targets:
                proof = compare(retail, caller, target['candidate_symbol_contains'],
                    int(target['target_va'], 0), target['target_size'], manifest.parent / target['bindings'])
                self.assertTrue(proof['strict_exact'], (target, proof))


if __name__ == '__main__':
    unittest.main()
