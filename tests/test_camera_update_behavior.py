"""Complete camera-update candidate and native matrix interfaces."""
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
SYMBOL = '?UpdateMatrices@Camera@@UAEXXZ'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class CameraUpdateBehaviorTests(unittest.TestCase):
    def test_whole_camera_state_mutating_helpers_and_float_edges(self):
        self.check_native(False)

    def test_actual_matrix_vector_and_fastmath_providers(self):
        self.check_native(True)

    def check_native(self, real):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(retail.bytes_at_va(0x42ee30,492),0x42ee30))
        self.assertEqual(sum(i.size for i in instructions),492)
        self.assertEqual((instructions[-1].address,instructions[-1].mnemonic),(0x42f01b,'ret'))
        self.assertEqual(retail.bytes_at_va(0x42f01c,4),b'\x90'*4)
        self.assertFalse(any(i.group(capstone.CS_GRP_JUMP) for i in instructions))
        calls = [(i.address,i.operands[0].imm) for i in instructions if i.mnemonic=='call']
        self.assertEqual(calls,[(0x42eea4,0x4a1500),(0x42ef06,0x4a1460),(0x42ef19,0x4a1410),
                                (0x42ef7b,0x4a1860),(0x42efb3,0x4a1860)])
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-camera-update-') as directory:
            output = Path(directory)
            objects = []
            sources = ['samples/camera/CameraNearMisses.cpp', 'tests/fixtures/camera_update_compare.cpp']
            if real:
                sources += ['src/reconstructed/MatrixUtil.cpp', 'src/reconstructed/VectorCalls.cpp',
                            'src/reconstructed/FastMath.cpp']
            for source in sources:
                obj = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    *(['--extra','/DCAMERA_REAL_HELPERS'] if real and source.startswith('tests/') else [])],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects.append(str(obj))
                if source.startswith('samples/'):
                    candidate = CoffObject(obj)
                    self.assertEqual({s.name for s in candidate.symbols
                                      if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}, {SYMBOL})
                    entry = candidate.find_symbol(SYMBOL)
                    self.assertEqual(entry.extent_source, 'coff_function_aux')
                    self.assertEqual(entry.function_size, 492)
                    report = compare(retail, candidate, SYMBOL, 0x42ee30, 492,
                                     ROOT / 'samples/camera/CameraNearMisses.bindings.json')
                    self.assertEqual(report['strict_match']['matching_positions'], 487)
            executable = output / 'camera.exe'
            mapping = output / 'camera.map'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/map:{mapping}', f'/include:{SYMBOL}', f'/libpath:{libraries}',
                *objects, 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            import struct,re
            address = re.search(re.escape(SYMBOL)+r'\s+([0-9a-fA-F]{8})\s', mapping.read_text(encoding='latin-1'))
            self.assertIsNotNone(address)
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(struct.pack('<II',*c) for c in calls))
            result = subprocess.run([str(executable), str(reference), str(patch_file), address.group(1)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            message = ('CameraUpdate: 9216 scenarios match with actual matrix providers' if real else
                       'CameraUpdate: 76288 scripted state, argument, exception and floating-point scenarios match')
            self.assertIn(message, result.stdout)


if __name__ == '__main__':
    unittest.main()
