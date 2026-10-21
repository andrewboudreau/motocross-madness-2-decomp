"""Terrain's repaired cell selection, not acceptance of its full interpolation."""
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class TerrainCellConversionTests(unittest.TestCase):
    def test_rounding_modes_exceptions_and_actual_first_cell_boundary(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        code = list(decoder.disasm(image.bytes_at_va(0x507c10, 0xed), 0x507c10))
        self.assertEqual(sum(i.size for i in code), 0xed)
        self.assertEqual([(i.address, i.operands[0].size) for i in code if i.mnemonic == 'fistp'],
                         [(0x507c86, 4), (0x507ca0, 4)])
        self.assertEqual([(i.address, i.operands[0].imm) for i in code if i.mnemonic == 'call'],
                         [(0x507cf8, 0x483910)])
        boundaries = {i.address for i in code} | {0x50879a}
        for instruction in code:
            if instruction.group(capstone.CS_GRP_JUMP):
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertIn(instruction.operands[0].imm, boundaries)
        self.assertEqual(image.bytes_at_va(0x50879a, 9), bytes.fromhex('5f5e5b8be55dc21000'))
        self.assertEqual(struct.unpack('<f', image.bytes_at_va(0x5507f4, 4))[0], .5)
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        source = ROOT / 'src/krusty2/broadphase/Terrain.cpp'
        with tempfile.TemporaryDirectory(prefix='mcm2-terrain-cell-') as directory:
            output = Path(directory)
            # The original TU imports unfinished game code. Extract the entire
            # unchanged query and its local helpers for execution, then require
            # identity with both actual production function bodies/relocations.
            text = source.read_text()
            first = text.index('static const TerrainVec3 kVec3Zero')
            last = text.index('// X-axis sibling', first)
            extracted = output / 'query.cpp'
            extracted.write_text('#include "' + (ROOT / 'src/krusty2/broadphase/Terrain.h').as_posix()
                                 + '"\n#include <float.h>\n#include <string.h>\n' + text[first:last])
            objects = []
            for path in (source, extracted, ROOT / 'tests/fixtures/terrain_cell_conversion.cpp'):
                obj = output / (path.stem + '.obj')
                command = [sys.executable, str(ROOT / 'tools/compile.py'), str(path), '-o', str(obj),
                           '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                           '--include', str(ROOT / 'src/krusty2')]
                if path.name == 'terrain_cell_conversion.cpp':
                    command += ['--extra', '/Od']
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                        encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(obj)
            production, candidate = (CoffObject(path) for path in objects[:2])
            for symbol, size in (('?QueryGround@Terrain@@', 3255), ('?FloatToInt@@', 370)):
                records = []
                for obj in (production, candidate):
                    entry = obj.find_symbol(symbol)
                    if symbol == '?FloatToInt@@':
                        self.assertEqual(entry.extent_source, 'codeview_proc')
                    else:
                        self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                    body, extent, relocations = obj.symbol_extent(entry)
                    self.assertEqual(extent, size)
                    records.append((body, extent, [(r.virtual_address - entry.value, r.type,
                                                   obj.symbol_by_index[r.symbol_index].name) for r in relocations]))
                self.assertEqual(records[0], records[1], symbol)
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'cells.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                str(objects[1]), str(objects[2]), 'kernel32.lib'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference)], capture_output=True, text=True, timeout=90)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('Terrain cell boundary: 278411 cases match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
