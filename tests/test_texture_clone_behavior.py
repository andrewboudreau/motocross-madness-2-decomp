"""Complete texture clone candidate, allocation unwind and scripted surface boundaries."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare
ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?Clone@PCTextureMap@@UAEPAVTextureMap@@XZ'
RANGES = {0x4c71c0: 600, 0x54cd00: 26, 0x54cd1a: 26, 0x54cd34: 10}
DIRECT = {0x4c7203: 0x4a3010, 0x4c7221: 0x510500, 0x4c7246: 0x50c6c0,
          0x4c7265: 0x4a3010, 0x4c7280: 0x4c5f00, 0x4c731b: 0x511970,
          0x54cd11: 0x4a3060, 0x54cd2b: 0x4a3060, 0x54cd39: 0x53471a}
INDIRECT = {0x4c72c7: 'dword ptr [eax + 0x64]', 0x4c733c: 'dword ptr [edi + 0x10]',
            0x4c7349: 'dword ptr [edx + 0x80]', 0x4c739c: 'dword ptr [edx + 0x48]',
            0x4c73e9: 'dword ptr [edx + 0x48]', 0x4c73fb: 'dword ptr [eax]'}
IMPORTS = {'??3@YAXPAXPBDH@Z', '?PixelFormatBytesPerPixel@@YAHH@Z',
           '??0PCTextureMap@@QAE@PAVTextureMapManager@@H@Z', '?g_TrackGame@@3PAVTrackGame@@A',
           '?AddTexture@ManagedTextureGroup@@QAEXPAVManagedTexture@@@Z',
           '??0ManagedTexture@@QAE@PAVTextureMapManager@@@Z', '??2@YAPAXIPBDH@Z',
           '___CxxFrameHandler', '__except_list'}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class TextureCloneBehaviorTests(unittest.TestCase):
    def test_complete_clone_state_color_keys_and_allocation_unwind(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        direct, indirect, patches = {}, {}, []
        for start, size in RANGES.items():
            instructions = list(decoder.disasm(image.bytes_at_va(start, size), start))
            self.assertEqual(sum(i.size for i in instructions), size)
            self.assertEqual(instructions[-1].mnemonic, 'jmp' if start == 0x54cd34 else 'ret')
            if start == 0x4c71c0:
                self.assertEqual([(i.address, bytes(i.bytes)) for i in instructions if i.mnemonic == 'ret'], [(0x4c7417, b'\xc3')])
            boundaries = {i.address for i in instructions}
            for instruction in instructions:
                if instruction.mnemonic != 'call' and not instruction.group(capstone.CS_GRP_JUMP):
                    continue
                operand = instruction.operands[0]
                if operand.type != capstone.x86.X86_OP_IMM:
                    self.assertEqual(instruction.mnemonic, 'call')
                    indirect[instruction.address] = instruction.op_str
                elif start <= operand.imm < start + size:
                    self.assertIn(operand.imm, boundaries)
                else:
                    self.assertIn(instruction.bytes[0], (0xe8, 0xe9))
                    direct[instruction.address] = operand.imm
                    patches.append(struct.pack('<III', instruction.address, operand.imm, instruction.bytes[0]))
        self.assertEqual(direct, DIRECT)
        self.assertEqual(indirect, INDIRECT)
        self.assertEqual(image.bytes_at_va(0x4c7418, 8), b'\x90' * 8)
        self.assertEqual(struct.unpack('<I', image.bytes_at_va(0x555fec + 6 * 4, 4))[0], 0x4c71c0)
        col = struct.unpack('<I', image.bytes_at_va(0x555fe8, 4))[0]
        self.assertEqual(struct.unpack('<I', image.bytes_at_va(col + 4, 4))[0], 0)
        self.assertEqual(struct.unpack('<8I', image.bytes_at_va(0x5629b0, 32)), (0x19930520, 2, 0x5629d0, 0, 0, 0, 0, 0))
        self.assertEqual(struct.unpack('<iIiI', image.bytes_at_va(0x5629d0, 16)), (-1, 0x54cd00, -1, 0x54cd1a))
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-texture-clone-') as directory:
            output = Path(directory)

            def compile_source(source, name, fixture=False):
                obj = output / (name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'), *(['--extra', '/Od'] if fixture else [])],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                return obj, CoffObject(obj)

            candidate_path, candidate = compile_source(ROOT / 'samples/render/TextureClone.cpp', 'candidate')
            fixture_path, _ = compile_source(ROOT / 'tests/fixtures/texture_clone_compare.cpp', 'fixture', True)
            _, aggregate = compile_source(ROOT / 'samples/render/PCTextureMapNearMisses.cpp', 'aggregate')
            # Private validation compiles the complete current canonical helper body;
            # the generated name avoids a collision with its observing callback.
            canonical = (ROOT / 'src/reconstructed/Tgafile.cpp').read_text()
            start = canonical.index('int PixelFormatBytesPerPixel(')
            end, depth = canonical.index('{', start) + 1, 1
            while depth:
                depth += (canonical[end] == '{') - (canonical[end] == '}')
                end += 1
            helper_file = output / 'format_helper.cpp'
            helper_file.write_text(canonical[start:end].replace('PixelFormatBytesPerPixel', 'ActualPixelFormatBytesPerPixel') + '\n')
            helper_path, helper = compile_source(helper_file, 'helper')
            helper_symbol = '?ActualPixelFormatBytesPerPixel@@YAHH@Z'
            helper_binding = output / 'helper.bindings.json'
            helper_binding.write_text(json.dumps({helper_symbol: '0x00511970'}))
            self.assertEqual(helper.find_symbol(helper_symbol).function_size, 78)
            self.assertTrue(compare(image, helper, helper_symbol, 0x511970, 78, helper_binding)['strict_exact'])
            self.assertEqual(candidate.find_symbol(SYMBOL).function_size, 590)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}, {SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number == 0}, IMPORTS)
            self.assertFalse(any(s.name.startswith('.CRT') for s in candidate.sections))
            for coff, binding in [(candidate, 'TextureClone'), (aggregate, 'PCTextureMapNearMisses')]:
                report = compare(image, coff, SYMBOL, 0x4c71c0, 600, ROOT / f'samples/render/{binding}.bindings.json')
                self.assertFalse(report['strict_exact'])
                self.assertEqual(report['strict_match']['matching_positions'], 333)

            def normalized(coff):
                symbol = coff.find_symbol(SYMBOL)
                body, size, relocs = coff.symbol_extent(symbol)
                rows = []
                for relocation in relocs:
                    target = coff.symbol_by_index[relocation.symbol_index]
                    name = target.name
                    if name.startswith(('$L', '$T')):
                        name = f'{coff.section(target.section_number).name}+{target.value}'
                    rows.append((relocation.virtual_address - symbol.value, relocation.type, name))
                return body, size, rows

            self.assertEqual(normalized(candidate), normalized(aggregate))

            def owned_sections(coff):
                main = coff.find_symbol(SYMBOL).section_number
                owned = [s for s in coff.sections if s.index == main or s.name in ('.text$x', '.xdata$x', '.data')]
                rows = []
                for section in owned:
                    relocations = []
                    for relocation in coff.relocations:
                        if relocation.section_number != section.index:
                            continue
                        target = coff.symbol_by_index[relocation.symbol_index]
                        name = target.name
                        if name.startswith(('$L', '$T')):
                            name = f'{coff.section(target.section_number).name}+{target.value}'
                        relocations.append((relocation.virtual_address, relocation.type, name))
                    rows.append((section.name, section.characteristics, section.raw_size,
                                 coff.data[section.raw_ptr:section.raw_ptr + section.raw_size], relocations))
                return rows

            # Retain the complete two cleanup bodies, handler, FuncInfo/unwind
            # map and filename storage, not just the principal function bytes.
            self.assertEqual([(s.name, s.raw_size) for s in candidate.sections if s.name in ('.text$x', '.xdata$x')],
                             [('.text$x', 62), ('.xdata$x', 48)])
            self.assertEqual(owned_sections(candidate), owned_sections(aggregate))
            for owner, constructor, destructor, wrapper, table, ctor_va, ctor_size, dtor_va, dtor_size, wrapper_va in [
                ('PCTextureMap', '??0PCTextureMap@@QAE@PAVTextureMapManager@@H@Z', '??1PCTextureMap@@UAE@XZ',
                 '??_GPCTextureMap@@UAEPAXI@Z', 0x555fec, 0x4c5f00, 46, 0x4c5f50, 227, 0x4c5f30),
                ('ManagedTexture', '??0ManagedTexture@@QAE@PAVTextureMapManager@@@Z', '??1ManagedTexture@@UAE@XZ',
                 '??_GManagedTexture@@UAEPAXI@Z', 0x558430, 0x510500, 123, 0x5105a0, 110, 0x510580),
            ]:
                _, provider = compile_source(ROOT / f'src/reconstructed/{owner}.cpp', owner)
                self.assertEqual(struct.unpack('<I', image.bytes_at_va(table, 4))[0], wrapper_va)
                provider_table = provider.find_symbol(f'??_7{owner}@@6B@')
                first_slot = [r for r in provider.relocations if r.section_number == provider_table.section_number
                              and r.virtual_address == provider_table.value]
                self.assertEqual(len(first_slot), 1)
                # VC6 emits its ordinary vector-deleting weak external with
                # the same TU's scalar-deleting wrapper as the fallback.
                target = provider.symbol_by_index[first_slot[0].symbol_index]
                self.assertEqual((target.name, target.storage_class, target.aux_count),
                                 (wrapper.replace('??_G', '??_E'), 105, 1))
                fallback, search = struct.unpack_from('<II', provider.data, provider.sym_ptr + (target.index + 1) * 18)
                self.assertEqual(search, 2)  # IMAGE_WEAK_EXTERN_SEARCH_LIBRARY
                self.assertEqual(provider.symbol_by_index[fallback].name, wrapper)
                for name, va, size in [(constructor, ctor_va, ctor_size), (destructor, dtor_va, dtor_size), (wrapper, wrapper_va, 30)]:
                    self.assertEqual(provider.symbol_extent(provider.find_symbol(name))[1], size)
                    self.assertTrue(compare(image, provider, name, va, size,
                        ROOT / f'src/reconstructed/{owner}.bindings.json')['strict_exact'])
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            executable = output / 'clone.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed',
                '/opt:ref', f'/out:{executable}', f'/libpath:{libraries}', str(candidate_path), str(fixture_path),
                str(helper_path), 'kernel32.lib'], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_file)], capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('TextureClone: 135464 complete allocation, copy, color-key, state, callback and unwind scenarios match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
