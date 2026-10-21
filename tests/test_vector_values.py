"""Complete vector-value providers, six actual calls, and native hidden-result ABI."""
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
SOURCE = 'src/reconstructed/VectorValueArithmetic.cpp'
FIXTURE = 'tests/fixtures/vector_values_compare.cpp'
PROVIDERS = (
    ('?SegmentAdd@@YA?AUVec3@@ABU1@0@Z', 0x421cb0),
    ('?SegmentSubtract@@YA?AUVec3@@ABU1@0@Z', 0x421d00),
    ('?WreckerSubtractValue@@YA?AUVector3@@ABU1@0@Z', 0x421d00),
)
CALLERS = (
    ('src/krusty2/collision/CollisionObject.cpp', '?SegmentTouchesObject@@YAHPBUCollisionVec3@@PAVCollisionObject@@PAU1@2@Z', 0x439820, 1493, 1309,
     [(0x33, 1), (0x56, 1), (0xf7, 0), (0x106, 1), (0x30a, 0)],
     [(0x43984f, 0x421d00), (0x439872, 0x421d00), (0x439916, 0x421cb0), (0x439925, 0x421d00), (0x439b29, 0x421cb0)]),
    ('src/reconstructed/Wrecker.cpp', '?Tick@Wrecker@@UAEHM@Z', 0x5306e0, 4178, 4178,
     [(0xff8, 2)], [(0x5316d7, 0x421d00)]),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and hash-pinned private retail image')
class VectorValueTests(unittest.TestCase):
    def test_complete_providers_callers_and_native_hidden_result(self):
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
        for _, address in PROVIDERS:
            code = decode(address, 69)
            self.assertEqual([(i.address-address, i.op_str) for i in code if i.mnemonic == 'ret'], [(68, '')])
            self.assertFalse(any(i.group(capstone.CS_GRP_JUMP) or i.mnemonic == 'call' for i in code))
            self.assertEqual(retail.bytes_at_va(address+69, 11), b'\x90'*11)
            at = {i.address-address: (i.mnemonic, i.op_str) for i in code}
            # Cdecl: hidden output, first reference, second reference. All operands
            # are read before any result write; the final x87 store follows x's write.
            for offset, instruction in (
                (0, ('sub', 'esp, 0xc')), (3, ('mov', 'eax, dword ptr [esp + 0x14]')),
                (7, ('mov', 'ecx, dword ptr [esp + 0x18]')),
                (36, ('mov', 'eax, dword ptr [esp + 0x10]')),
                (45, ('mov', 'dword ptr [ecx], edx')),
                (51, ('fstp', 'dword ptr [esp + 8]')),
                (55, ('mov', 'dword ptr [ecx + 4], edx')),
                (62, ('mov', 'dword ptr [ecx + 8], edx')),
                (65, ('add', 'esp, 0xc')),
            ): self.assertEqual(at[offset], instruction)
            arithmetic = [i for i in code if i.mnemonic in ('fadd', 'fsub')]
            self.assertEqual([i.mnemonic for i in arithmetic], ['fadd' if address == 0x421cb0 else 'fsub']*3)
        for _, _, address, size, _, _, calls in CALLERS:
            code = decode(address, size)
            self.assertEqual([(i.address, int(i.op_str, 16)) for i in code
                if i.mnemonic == 'call' and i.op_str in ('0x421cb0', '0x421d00')], calls)
        at = {i.address: (i.mnemonic, i.op_str) for i in decode(0x5306e0, 4178)}
        for address, instruction in (
            (0x5316bf, ('lea', 'ecx, [esp + 0x120]')), (0x5316c6, ('push', 'ecx')),
            (0x5316c7, ('lea', 'edx, [esp + 0x10c]')), (0x5316d5, ('push', 'edx')),
            (0x5316ce, ('lea', 'eax, [esp + 0x130]')), (0x5316d6, ('push', 'eax')),
            (0x5316de, ('add', 'esp, 0xc')),
        ): self.assertEqual(at[address], instruction)

        compiler = Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-vector-values-') as directory:
            output = Path(directory)
            objects, paths = {}, {}
            for index, source in enumerate((SOURCE, FIXTURE) + tuple(row[0] for row in CALLERS)):
                path = output/('unit%d.obj'%index)
                command = [sys.executable, str(ROOT/'tools/compile.py'), str(ROOT/source),
                    '-o', str(path), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2')]
                if source == FIXTURE: command += ['--extra', '/Od']
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                    encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            provider = objects[SOURCE]
            self.assertEqual({s.name for s in provider.symbols if s.section_number == 0}, {'__fltused'})
            self.assertEqual({s.name for s in provider.symbols if s.section_number > 0 and s.storage_class == 2}, {name for name,_ in PROVIDERS})
            self.assertFalse(any(s.name.startswith('.CRT') or s.characteristics & 0xC0
                                 for s in provider.sections if not s.name.startswith('.debug')))
            for name, address in PROVIDERS:
                entry = provider.find_symbol(name)
                self.assertEqual((entry.function_size, entry.extent_source), (69, 'coff_function_aux'))
                self.assertEqual(provider.symbol_extent(entry)[2], [])
                proof = compare(retail, provider, name, address, 69, (ROOT/SOURCE).with_suffix('.bindings.json'))
                self.assertTrue(proof['strict_exact'], proof)
                self.assertEqual(proof['relocations_masked'], 0)
                self.assertEqual(proof['alignment_padding_bytes'], 0)
            for source, name, address, size, matching, calls, _ in CALLERS:
                obj = objects[source]; entry = obj.find_symbol(name)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                body, _, relocs = obj.symbol_extent(entry)
                joined = [r for r in relocs if obj.symbol_by_index[r.symbol_index].name in {n for n,_ in PROVIDERS}]
                self.assertEqual([(r.virtual_address-entry.value, obj.symbol_by_index[r.symbol_index].name) for r in joined],
                                 [(offset, PROVIDERS[index][0]) for offset,index in calls])
                self.assertTrue(all(r.type == 20 for r in joined))
                self.assertEqual([struct.unpack_from('<I', body, r.virtual_address-entry.value)[0] for r in joined], [0]*len(calls))
                proof = compare(retail, obj, name, address, size, (ROOT/source).with_suffix('.bindings.json'))
                self.assertEqual(proof['strict_match']['matching_positions'], matching)
                self.assertEqual(proof['strict_exact'], matching == size)
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output/'vector_values.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000',
                '/fixed', '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}',
                str(paths[SOURCE]), str(paths[FIXTURE]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            result = subprocess.run([str(executable), str(image)], capture_output=True, text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertIn('Vector values: 152535 hidden-output, overlap, raw-float, FP and fault cases match', result.stdout)
            print('Three complete69-byte providers; Wrecker4178strict; collision1493unchanged partial; '+result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
