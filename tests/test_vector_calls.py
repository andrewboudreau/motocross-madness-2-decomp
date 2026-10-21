"""Complete vector call providers, real imports, and private native ABI behavior."""
import json
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
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))
PROVIDERS = {
    '?Vec3AddCall@@YAPAUVec3@@PAU1@PBU1@1@Z': (0x421cb0, 69),
    '?Vec3SubtractCall@@YAPAUVec3@@PAU1@PBU1@1@Z': (0x421d00, 69),
    '?Vec3ScaleCall@@YAPAUVec3@@PAU1@PBU1@M@Z': (0x5015b0, 69),
    '?Vec3DotCall@@YAMPBUVec3@@0@Z': (0x40ae30, 29),
}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires native VC6_ROOT and private retail image')
class VectorCallTests(unittest.TestCase):
    def test_full_providers_actual_callers_and_native_alias_behavior(self):
        import capstone
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        for va, size in PROVIDERS.values():
            instructions = list(decoder.disasm(retail.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in instructions), size)
            self.assertEqual(instructions[-1].mnemonic, 'ret')
            self.assertFalse(any(i.mnemonic.startswith(('call', 'j')) for i in instructions))

        sources = {
            'VectorCalls': 'src/reconstructed/VectorCalls.cpp',
            'fixture': 'tests/fixtures/vector_calls_compare.cpp',
            'Vehicle': 'src/krusty2/vehicle/Vehicle.cpp',
            'Motnctrl': 'src/krusty2/motion/Motnctrl.cpp',
        }
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-vector-calls-') as directory:
            output = Path(directory)
            objects = {}
            for name, source in sources.items():
                obj = output / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(obj)

            provider = objects['VectorCalls']
            self.assertEqual({s.name for s in provider.symbols if s.section_number == 0}, {'__fltused'})
            self.assertEqual({s.name for s in provider.symbols
                              if s.section_number > 0 and s.storage_class == 2}, set(PROVIDERS))
            self.assertFalse(any(s.name.startswith('.CRT') for s in provider.sections))
            for symbol, (va, size) in PROVIDERS.items():
                entry = provider.find_symbol(symbol)
                self.assertEqual(entry.extent_source, 'coff_function_aux')
                self.assertEqual(entry.function_size, size)
                proof = compare(retail, provider, symbol, va, size,
                                ROOT / 'src/reconstructed/VectorCalls.bindings.json')
                self.assertTrue(proof['strict_exact'], proof)
                self.assertEqual(proof['relocations_masked'], 0)

            used = set()
            for name in ('Vehicle', 'Motnctrl'):
                obj = objects[name]
                imports = {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                           if obj.symbol_by_index[r.symbol_index].section_number == 0}
                bindings = json.loads((ROOT / Path(sources[name]).with_suffix('.bindings.json')).read_text())
                for symbol in imports & PROVIDERS.keys():
                    self.assertEqual(int(bindings[symbol], 0), PROVIDERS[symbol][0])
                    used.add(symbol)
            self.assertEqual(used, set(PROVIDERS))

            # Link and execute the same standalone production object proved above.
            # No test implementations or patched copies stand in for these bodies.
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'vectors.exe'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed',
                '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                str(objects['VectorCalls'].path), str(objects['fixture'].path), 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(EXE.resolve())],
                                    capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('VectorCalls: 31873 alias, return and arithmetic scenarios match', result.stdout)


if __name__ == '__main__':
    unittest.main()
