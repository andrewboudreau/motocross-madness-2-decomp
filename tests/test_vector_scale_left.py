"""Complete scalar-first provider, actual BVH calls, and native hidden-result ABI."""
import hashlib
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

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/VectorScaleLeft.cpp'
CALLER = 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp'
FIXTURE = 'tests/fixtures/vector_scale_left_compare.cpp'
SYMBOL = '?Vec3ScaleLeft@@YA?AUVec3@@MABU1@@Z'
CAPSULE = '?CapsuleTreeQuery@@YAHPBUVec3@@MMPAUQueryTreeNode@@PBUMatrix4@@H@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and hash-pinned private retail image')
class VectorScaleLeftTests(unittest.TestCase):
    def test_complete_provider_two_bvh_calls_and_native_hidden_result(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True

        def decode(address, size):
            result = list(decoder.disasm(retail.bytes_at_va(address, size), address))
            self.assertEqual(sum(i.size for i in result), size)
            return result

        code = decode(0x428090, 69)
        self.assertEqual([(i.address, i.op_str) for i in code if i.mnemonic == 'ret'],
                         [(0x4280d4, '')])
        self.assertFalse(any(i.group(capstone.CS_GRP_JUMP) or i.mnemonic == 'call' for i in code))
        self.assertEqual(retail.bytes_at_va(0x4280d5, 11), b'\x90'*11)
        at = {i.address: (i.mnemonic, i.op_str) for i in code}
        # Original stack: return address, hidden output, scalar, vector pointer.
        # Three local floats exist before the first output write. No callee pop.
        for address, instruction in (
            (0x428090, ('sub', 'esp, 0xc')),
            (0x428093, ('mov', 'eax, dword ptr [esp + 0x18]')),
            (0x428097, ('fld', 'dword ptr [esp + 0x14]')),
            (0x4280b4, ('fmul', 'dword ptr [eax + 8]')),
            (0x4280b7, ('mov', 'eax, dword ptr [esp + 0x10]')),
            (0x4280bb, ('mov', 'ecx, eax')),
            (0x4280bd, ('fstp', 'dword ptr [esp + 8]')),
            (0x4280c1, ('mov', 'dword ptr [ecx], edx')),
            (0x4280c7, ('mov', 'dword ptr [ecx + 4], edx')),
            (0x4280ce, ('mov', 'dword ptr [ecx + 8], edx')),
            (0x4280d1, ('add', 'esp, 0xc')),
        ):
            self.assertEqual(at[address], instruction)

        caller = decode(0x4298c0, 1436)
        self.assertEqual([i.address for i in caller if i.mnemonic == 'call' and i.op_str == '0x428090'],
                         [0x429a8a, 0x429da5])
        at = {i.address: (i.mnemonic, i.op_str) for i in caller}
        # Triangle normal at +8; the point branch instead passes a local direction.
        # A prior push ESI at 429d9d prepares the outer Vec3Add, not a fourth arg.
        for address, instruction in (
            (0x429992, ('lea', 'edi, [esi + 8]')),
            (0x429a83, ('push', 'edi')),
            (0x429a88, ('push', 'eax')),
            (0x429a89, ('push', 'ecx')),
            (0x429a8f, ('push', 'eax')),
            (0x429d99, ('lea', 'eax, [esp + 0x10]')),
            (0x429d9e, ('push', 'eax')),
            (0x429da3, ('push', 'ecx')),
            (0x429da4, ('push', 'edx')),
            (0x429daa, ('add', 'esp, 0xc')),
            (0x429dad, ('push', 'eax')),
        ):
            self.assertEqual(at[address], instruction)

        compiler = Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-scale-left-') as directory:
            output = Path(directory)
            objects, paths = {}, {}
            for index, source in enumerate((SOURCE, CALLER, FIXTURE)):
                path = output/('unit%d.obj'%index)
                command = [sys.executable, str(ROOT/'tools/compile.py'), str(ROOT/source),
                    '-o', str(path), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2')]
                if source == FIXTURE:
                    command += ['--extra', '/Od']
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                    encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                objects[source], paths[source] = CoffObject(path), path

            provider = objects[SOURCE]
            self.assertEqual({s.name for s in provider.symbols if s.section_number == 0}, {'__fltused'})
            self.assertEqual({s.name for s in provider.symbols if s.section_number > 0 and s.storage_class == 2}, {SYMBOL})
            self.assertFalse(any(s.name.startswith('.CRT') or s.characteristics & 0xC0
                                 for s in provider.sections if not s.name.startswith('.debug')))
            entry = provider.find_symbol(SYMBOL)
            self.assertEqual((entry.function_size, entry.extent_source), (69, 'coff_function_aux'))
            self.assertEqual(provider.symbol_extent(entry)[2], [])
            proof = compare(retail, provider, SYMBOL, 0x428090, 69,
                            (ROOT/SOURCE).with_suffix('.bindings.json'))
            self.assertTrue(proof['strict_exact'], proof)
            self.assertEqual(proof['relocations_masked'], 0)
            self.assertEqual(proof['alignment_padding_bytes'], 0)

            query = objects[CALLER]
            entry = query.find_symbol(CAPSULE)
            self.assertEqual((entry.function_size, entry.extent_source), (1436, 'coff_function_aux'))
            body, _, relocs = query.symbol_extent(entry)
            joined = [r for r in relocs if r.type == 20 and query.symbol_by_index[r.symbol_index].name == SYMBOL]
            self.assertEqual([r.virtual_address-entry.value for r in joined],
                             [0x429a8b-0x4298c0, 0x429da6-0x4298c0])
            self.assertEqual([struct.unpack_from('<I', body, r.virtual_address-entry.value)[0] for r in joined], [0,0])
            proof = compare(retail, query, CAPSULE, 0x4298c0, 1436,
                            (ROOT/CALLER).with_suffix('.bindings.json'))
            self.assertTrue(proof['strict_exact'], proof)

            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output/'scale_left.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                str(paths[SOURCE]), str(paths[FIXTURE]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            result = subprocess.run([str(executable), str(image)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertIn('Vec3ScaleLeft: 135279 hidden-output, overlap, raw-float, FP and fault cases match', result.stdout)
            print('Complete69-byte provider and1436-byte two-call consumer strict; '+result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
