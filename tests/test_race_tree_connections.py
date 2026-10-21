"""Race switches call the complete canonical named-descendant providers."""
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
CASES = (
    ('src/reconstructed/BikeRace.cpp', '?Tick@BikeRace@@UAEHM@Z', 0x41d2b0, 5980),
    ('src/reconstructed/GameObject.cpp', '?DisableByClassName@GameObject@@IAEXPBD@Z', 0x468dd0, 308),
    ('src/reconstructed/GameObject.cpp', '?EnableByClassName@GameObject@@IAEXPBD@Z', 0x468f10, 308),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class RaceTreeConnectionsTests(unittest.TestCase):
    def test_complete_caller_and_descendant_providers(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-race-tree-') as directory:
            paths = {}
            for source in dict.fromkeys(case[0] for case in CASES):
                obj = Path(directory) / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True, text=True,
                    encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[source] = obj
            for source, symbol, address, size in CASES:
                obj = CoffObject(paths[source])
                self.assertEqual(obj.find_symbol(symbol).function_size, size)
                self.assertTrue(compare(retail, obj, symbol, address, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'])
            obj = CoffObject(paths[CASES[0][0]])
            _, _, relocations = obj.symbol_extent(obj.find_symbol(CASES[0][1]))
            # The full strict extent above includes trailing jump-table data.
            code_size = 0x41e9e2 - 0x41d2b0  # through the final ret 4
            code = list(decoder.disasm(retail.bytes_at_va(0x41d2b0, code_size), 0x41d2b0))
            self.assertEqual(sum(i.size for i in code), code_size)
            self.assertEqual((code[-1].mnemonic, code[-1].op_str), ('ret', '4'))
            for _, symbol, address, _ in CASES[1:]:
                self.assertEqual(sum(r.type == 0x14 and obj.symbol_by_index[r.symbol_index].name == symbol
                                     for r in relocations), 9)
                calls = [i for i in code if i.mnemonic == 'call' and
                         i.operands[0].type == capstone.x86.X86_OP_IMM and i.operands[0].imm == address]
                self.assertEqual(len(calls), 9)
                # Every original call loads the child-root primary pointer (+0x34)
                # into ECX without a secondary-base adjustment.
                for call in calls:
                    previous = code[code.index(call)-2:code.index(call)]
                    receiver = [i for i in previous if i.mnemonic == 'mov' and
                                i.operands[0].type == capstone.x86.X86_OP_REG and
                                i.operands[0].reg == capstone.x86.X86_REG_ECX]
                    self.assertEqual(len(receiver), 1)
                    operand = receiver[0].operands[1]
                    self.assertEqual(operand.type, capstone.x86.X86_OP_MEM)
                    self.assertEqual((operand.mem.disp, operand.mem.index), (0x34, 0))
                    self.assertIn(operand.mem.base, (capstone.x86.X86_REG_EAX,
                                                    capstone.x86.X86_REG_ECX, capstone.x86.X86_REG_EDX))
            self.assertFalse(any('UnknownBikeRaceRoot' in symbol.name for symbol in obj.symbols))


if __name__ == '__main__':
    unittest.main()
