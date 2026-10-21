"""Input handlers retain complete strict bodies and call the real predicate."""
import hashlib
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

ROOT = Path(__file__).resolve().parents[1]
PREDICATE = '?TestInputEvent@@YAHHHPBUUnknownControlEvent@@H@Z'
CASES = (
    ('src/krusty2/collision/CollisionObject.cpp', '?OnControlPress@CollisionObject@@', 0x434970, 153, 3),
    ('src/krusty2/broadphase/Terrain.cpp', '?OnControlPress@Terrain@@', 0x508850, 276, 3),
    ('src/krusty2/visibility/VisibilityQuadTree.cpp', '?OnControlPress@VisibilityQuadTree@@', 0x52eec0, 135, 2),
    ('src/krusty2/soultree/SoultreeQuadTreeRenderer.cpp', '?OnControlPress@SoultreeQuadTreeRenderer@@', 0x5048d0, 101, 1),
    ('src/reconstructed/ControlInterface.cpp', PREDICATE, 0x43caa0, 63, 0),
    ('src/reconstructed/KeyboardDevice.cpp', '?ModifierHolds@KeyboardDevice@@', 0x48a240, 139, 0),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class InputProviderConnectionsTests(unittest.TestCase):
    def test_complete_callers_and_actual_input_provider(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-input-provider-') as directory:
            output = Path(directory)
            paths = {}
            fixture = 'tests/fixtures/input_provider_compare.cpp'
            for source in [case[0] for case in CASES] + [fixture]:
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'), *(['--extra', '/Od'] if source == fixture else [])],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[source] = obj
            for source, symbol, address, size, calls in CASES:
                obj = CoffObject(paths[source])
                entry = obj.find_symbol(symbol)
                self.assertEqual(entry.function_size, size)
                self.assertTrue(compare(retail, obj, symbol, address, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'])
                code = list(decoder.disasm(retail.bytes_at_va(address, size), address))
                self.assertEqual(sum(i.size for i in code), size)
                if calls:
                    _, _, relocations = obj.symbol_extent(entry)
                    self.assertEqual(sum(r.type == 0x14 and obj.symbol_by_index[r.symbol_index].name == PREDICATE
                                         for r in relocations), calls)
                    self.assertEqual(sum(i.mnemonic == 'call' and i.operands[0].type == capstone.x86.X86_OP_IMM
                                         and i.operands[0].imm == 0x43caa0 for i in code), calls)
                    self.assertFalse(any(s.section_number == 0 and (s.name.startswith('?CheckKey@@') or
                                         s.name == '?TestInputEvent@@YAHHHHH@Z') for s in obj.symbols))
            # Link only these complete ordinary C++ bodies. Compare both extracted
            # bodies strictly as well as the full production objects above.
            bodies = []
            for source, signature in ((CASES[4][0], 'int TestInputEvent('),
                                      (CASES[5][0], 'int KeyboardDevice::ModifierHolds(')):
                text = (ROOT / source).read_text()
                start = text.index(signature)
                end = text.index('{', start) + 1
                depth = 1
                while depth:
                    depth += (text[end] == '{') - (text[end] == '}')
                    end += 1
                bodies.append(text[start:end])
            extracted = output / 'input_provider.cpp'
            extracted.write_text('#include "TrackGame.h"\n#include "KeyboardDevice.h"\n' + '\n'.join(bodies))
            native = output / 'input_provider.obj'
            result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(extracted),
                '-o', str(native), '--vc6-root', str(compiler), '--include', str(ROOT / 'src/reconstructed'),
                '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            obj = CoffObject(native)
            for source, symbol, address, size, _ in CASES[4:]:
                self.assertEqual(obj.find_symbol(symbol).function_size, size)
                self.assertTrue(compare(retail, obj, symbol, address, size,
                                        (ROOT / source).with_suffix('.bindings.json'))['strict_exact'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'input.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed',
                '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                str(native), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('6720 complete event and modifier cases match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
