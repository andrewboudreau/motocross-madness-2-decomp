"""Scene reader defaults retain their distinct direct-call contracts."""
from collections import Counter
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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/SceneCallbacks.cpp'
CALLER = 'src/reconstructed/SceneManager.cpp'
METHODS = {
    '?UnknownFunction4de580@Scene@@QAEXPBD00@Z': (0x4de580, 25),
    '?UnknownFunction4de580@Scene@@QAEXPBD0M@Z': (0x4de580, 12),
    '?UnknownFunction4de580@Scene@@QAEXPBD0H@Z': (0x4de580, 9),
    '?UnknownFunction4de580@Scene@@QAEXPBD0PBUVector3@@@Z': (0x4de580, 1),
    '?UnknownFolded4de580@Scene@@QAEXPBD00@Z': (0x4de580, 2),
    '?UnknownFunction464e80@Scene@@QAEXPBD@Z': (0x464e80, 25),
}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class SceneCallbacksTests(unittest.TestCase):
    def test_complete_callbacks_receiver_and_all_reader_calls(self):
        import capstone
        from tools.run_calibration import CASES
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        word = lambda va: struct.unpack('<I', retail.bytes_at_va(va, 4))[0]
        self.assertEqual(word(0x557868), 0x55ecc8)
        self.assertEqual(word(0x55ecc8 + 4), 0)
        self.assertEqual(retail.bytes_at_va(word(0x55ecc8 + 12) + 8, 12), b'.?AVScene@@\0')
        scene = next(c for c in parse_rtti(retail) if c.name == 'Scene')
        self.assertEqual([(b.name, b.mdisp, b.pdisp, b.vdisp) for b in scene.bases],
                         [('Scene', 0, -1, 0), ('GameObject', 0, -1, 0), ('BaseObject', 0, -1, 0)])
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        def instructions(va, size):
            return [(i.mnemonic, i.op_str) for i in decoder.disasm(retail.bytes_at_va(va, size), va)]
        # The actual constructor keeps the incoming primary pointer for its
        # GameObject call, writes the Scene primary table there, and returns it.
        ctor = {i.address: (i.mnemonic, i.op_str) for i in
                decoder.disasm(retail.bytes_at_va(0x4ea660, 338), 0x4ea660)}
        for address, instruction in {
            0x4ea67e: ('mov', 'esi, ecx'), 0x4ea685: ('call', '0x468ca0'),
            0x4ea6f7: ('mov', 'dword ptr [esi], 0x55786c'),
            0x4ea7a1: ('mov', 'eax, esi'), 0x4ea7af: ('ret', '4'),
        }.items():
            self.assertEqual(ctor[address], instruction)
        self.assertEqual(retail.bytes_at_va(0x4de580, 16), b'\xc2\x0c\0' + b'\x90'*13)
        self.assertEqual(retail.bytes_at_va(0x464e80, 16), b'\xc2\x04\0' + b'\x90'*13)

        # One independently decoded stack/receiver boundary per signature.
        # Float values occupy one four-byte slot, rather than a promoted double.
        contexts = {
            0x4eb612: (22, [('lea', 'edx, [esp + 0x5c]'), ('lea', 'eax, [esp + 0x2c]'),
                ('push', 'edx'), ('push', '0x56bae0'), ('push', 'eax'), ('mov', 'ecx, edi'), ('call', '0x4de580')]),
            0x4eb6b7: (21, [('mov', 'eax, dword ptr [esi + 0x18]'), ('lea', 'ecx, [esp + 0x2c]'),
                ('push', 'eax'), ('push', '0x572f20'), ('push', 'ecx'), ('mov', 'ecx, edi'), ('call', '0x4de580')]),
            0x4eb6ff: (19, [('mov', 'ecx, edi'), ('shr', 'eax, 1'), ('and', 'eax, 1'),
                ('push', 'eax'), ('push', '0x572f08'), ('push', 'edx'), ('call', '0x4de580')]),
            0x4eb950: (22, [('mov', 'ecx, dword ptr [esp + 0x10]'), ('lea', 'edx, [esp + 0x2c]'),
                ('push', 'ecx'), ('push', '0x572ee4'), ('push', 'edx'), ('mov', 'ecx, edi'), ('call', '0x4de580')]),
            0x4ec3b2: (22, [('push', '0x5732cc'), ('push', '0x57327c'), ('push', '0x573300'),
                ('mov', 'ecx, ebx'), ('call', '0x4de580')]),
            0x4ebb6f: (12, [('lea', 'edx, [esp + 0x5c]'), ('mov', 'ecx, edi'),
                ('push', 'edx'), ('call', '0x464e80')]),
        }
        for address, (size, expected) in contexts.items():
            self.assertEqual(instructions(address, size), expected)
        receiver_saves = {
            0x4eb570: (0x4eb581, 'edi'), 0x4ebfc0: (0x4ebfc7, 'ebx'),
            0x4ef4c0: (0x4ef4dd, 'ebp'), 0x4efb20: (0x4efb3e, 'esi'),
            0x4f1130: (0x4f114d, 'ebx'), 0x4eb160: (0x4eb173, 'ebx'),
            0x4eb300: (0x4eb311, 'esi'), 0x4ebdb0: (0x4ebdbf, 'esi'),
            0x4eca20: (0x4eca27, 'esi'), 0x4f0d20: (0x4f0d3d, 'esi'),
        }

        with tempfile.TemporaryDirectory(prefix='mcm2-scene-callbacks-') as directory:
            output = Path(directory)
            paths, objects = {}, {}
            for source in (SOURCE, CALLER, 'tests/fixtures/scene_callbacks.cpp'):
                obj = output/(Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(ROOT/source),
                    '-o', str(obj), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/reconstructed')], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[source], objects[source] = obj, CoffObject(obj)
            provider = objects[SOURCE]
            self.assertEqual({s.name for s in provider.symbols if s.storage_class == 2 and
                              s.section_number > 0 and s.type == 0x20}, set(METHODS))
            self.assertFalse([s.name for s in provider.symbols if s.storage_class == 2 and s.section_number == 0])
            self.assertFalse([r for r in provider.relocations
                              if not provider.sections[r.section_number - 1].name.startswith('.debug')])
            self.assertFalse([s for s in provider.sections if s.raw_size and
                              not s.name.startswith(('.text', '.debug', '.drectve'))])
            for symbol, (address, _) in METHODS.items():
                self.assertEqual(provider.symbol_extent(provider.find_symbol(symbol))[1], 3)
                self.assertTrue(compare(retail, provider, symbol, address, 3,
                    ROOT/'src/reconstructed/SceneCallbacks.bindings.json')['strict_exact'])
            obj = objects[CALLER]
            counts, retail_counts, callers, strict = Counter(), Counter(), 0, 0
            for case in (c for c in CASES if c['source'] == CALLER):
                va, extent = int(case['target_va'], 0), case['target_size']
                self.assertTrue(compare(retail, obj, case['symbol'], va, extent,
                    ROOT/case['bindings'])['strict_exact'], case['symbol'])
                strict += 1
                symbol = obj.find_symbol(case['symbol'])
                calls = [r for r in obj.relocations if r.section_number == symbol.section_number and
                         obj.symbol_by_index[r.symbol_index].name in METHODS]
                if not calls:
                    continue
                callers += 1
                decoded = {i.address: i for i in decoder.disasm(retail.bytes_at_va(va, extent), va)}
                save, receiver = receiver_saves[va]
                self.assertEqual((decoded[save].mnemonic, decoded[save].op_str), ('mov', receiver + ', ecx'))
                for relocation in calls:
                    name = obj.symbol_by_index[relocation.symbol_index].name
                    self.assertEqual(relocation.type, 0x14)
                    call = decoded[va + relocation.virtual_address - symbol.value - 1]
                    self.assertEqual((call.mnemonic, call.op_str), ('call', hex(METHODS[name][0])))
                    # Each direct boundary restores the saved primary pointer;
                    # no secondary-base adjustment is introduced by these calls.
                    write = next(i for i in reversed(list(decoded.values())) if i.address < call.address
                                 and capstone.x86_const.X86_REG_ECX in i.regs_access()[1])
                    self.assertEqual((write.mnemonic, write.op_str), ('mov', 'ecx, ' + receiver))
                    counts[name] += 1
                for instruction in decoded.values():
                    if instruction.mnemonic == 'call' and instruction.op_str in ('0x4de580', '0x464e80'):
                        retail_counts[int(instruction.op_str, 0)] += 1
            self.assertEqual(counts, Counter({name: count for name, (_, count) in METHODS.items()}))
            self.assertEqual(retail_counts, Counter({0x4de580: 49, 0x464e80: 25}))
            self.assertEqual((callers, strict), (10, 51))
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output/'callbacks.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', '/out:'+str(executable),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                str(paths[SOURCE]), str(paths['tests/fixtures/scene_callbacks.cpp']), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.strip(), 'Scene callbacks: 36864 complete-storage calls passed')
            print('57 strict bodies, 74 direct calls in ten readers; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
