"""Complete GUI pointer candidates and unchanged aggregate extraction."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
CASES = (('?ShowControlToolTip@ToolTip@@QAEXPAVUnknownGuiControl@@@Z', 0x486b10, 106, 89),
         ('?SetPointerDevice@GUIUser@@QAEHPAVGUIInputDevice@@@Z', 0x488120, 52, 46))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class GuiPointerHelpersTests(unittest.TestCase):
    def test_complete_ownership_coordinates_callbacks_and_faults(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        for symbol, va, size, matching in CASES:
            instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in instructions), size)
            self.assertEqual(bytes(instructions[-1].bytes), bytes.fromhex('c20400'))
            boundaries = {i.address for i in instructions}
            for i in instructions:
                if i.group(capstone.CS_GRP_JUMP):
                    self.assertIn(i.operands[0].imm, boundaries)
            calls = [i for i in instructions if i.mnemonic == 'call']
            self.assertEqual([(i.address, i.operands[0].imm) for i in calls],
                             [(0x486b6e, 0x486b80)] if size == 106 else [])
            self.assertEqual(retail.bytes_at_va(va + size, 6 if size == 106 else 12), b'\x90' * (6 if size == 106 else 12))
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-gui-pointer-') as directory:
            output = Path(directory)
            objects = []
            for source in ('samples/ui/GuiPointerHelpers.cpp', 'tests/fixtures/gui_pointer_compare.cpp'):
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt'],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if source.startswith('samples/'):
                    candidate = CoffObject(obj)
                    self.assertEqual({s.name for s in candidate.symbols
                                      if s.storage_class == 2 and s.section_number > 0}, {c[0] for c in CASES})
                    self.assertEqual({s.name for s in candidate.symbols
                                      if s.storage_class == 2 and s.section_number == 0},
                                     {'?ShowText@ToolTip@@QAEXPBDPAHM@Z', '__fltused'})
                    for symbol, va, size, matching in CASES:
                        entry = candidate.find_symbol(symbol)
                        self.assertEqual(entry.extent_source, 'coff_function_aux')
                        self.assertEqual(entry.function_size, size)
                        report = compare(retail, candidate, symbol, va, size,
                                         ROOT / 'samples/ui/GuiPointerHelpers.bindings.json')
                        self.assertEqual(report['strict_match']['matching_positions'], matching)
            # The historical aggregate includes the same bodies; compare every
            # byte and relocation operand against the separately compiled candidate.
            aggregate = output / 'aggregate.obj'
            result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'),
                str(ROOT / 'samples/ui/GUIManagerNearMisses.cpp'), '-o', str(aggregate),
                '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt'], cwd=ROOT,
                capture_output=True, text=True, encoding='latin-1',
                env={**os.environ, 'PYTHONPATH': str(ROOT)})
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            combined = CoffObject(aggregate)
            def body(obj, name):
                entry = obj.find_symbol(name)
                section = obj.section(entry.section_number)
                raw = obj.data[section.raw_ptr + entry.value:
                               section.raw_ptr + entry.value + entry.function_size]
                relocations = [(r.virtual_address - entry.value, r.type,
                                obj.symbol_by_index[r.symbol_index].name)
                               for r in obj.relocations if r.section_number == entry.section_number
                               and entry.value <= r.virtual_address < entry.value + entry.function_size]
                return raw, relocations
            for symbol, va, size, matching in CASES:
                self.assertEqual(body(candidate, symbol), body(combined, symbol))
            executable = output / 'pointer.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('GuiPointerHelpers: 19606 ownership, coordinate, callback and fault scenarios match', result.stdout)


if __name__ == '__main__':
    unittest.main()
