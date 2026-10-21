"""Complete motion-start candidate, current-rounding conversion and real pose walker."""
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.build_game import compile_source as build_compile_source
from tools.match import compare
from test_bike_text_connections import extract

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/physics/motion/CharacterMotionStart.cpp'
CANONICAL = 'src/krusty2/motion/Motnctrl.cpp'
SYMBOL = '?Method_0x004a8bf0@Character@@QAEXHM@Z'
HELPER = '?ApplyPoseListSlot4@Character@@QAEXPAUMotionPoseList@@H@Z'
CONVERSION = '?FloatToInt@@YAXPAHM@Z'


def complete(obj, symbol):
    entry = obj.find_symbol(symbol)
    body, size, relocations = obj.symbol_extent(entry)
    return body, size, [(r.virtual_address-entry.value, r.type,
                        obj.symbol_by_index[r.symbol_index].name) for r in relocations]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the pinned private retail image')
class CharacterMotionStartTests(unittest.TestCase):
    def test_complete_start_with_real_pose_walker_and_fp_exceptions(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(retail.bytes_at_va(0x4a8bf0, 81), 0x4a8bf0))
        self.assertEqual(sum(i.size for i in instructions), 81)
        self.assertEqual([(i.address, i.op_str) for i in instructions if i.mnemonic == 'ret'],
                         [(0x4a8c3e, '8')])
        self.assertEqual([(i.address, i.op_str) for i in instructions if i.mnemonic == 'call'],
                         [(0x4a8c36, '0x4a8a80')])
        self.assertFalse(any(i.group(capstone.CS_GRP_JUMP) for i in instructions))
        self.assertEqual(retail.bytes_at_va(0x4a8c41, 15), b'\x90'*15)
        operations = [(i.mnemonic, i.op_str) for i in instructions]
        self.assertIn(('fistp', 'dword ptr [eax]'), operations)
        # The same local holds count-1 and the FISTP destination, with an explicit
        # binary32 spill/reload before conversion. No clamp or handle check exists.
        self.assertIn(('dec', 'eax'), operations)
        self.assertIn(('mov', 'dword ptr [ebp - 4], eax'), operations)
        self.assertIn(('lea', 'eax, [ebp + 8]'), operations)
        self.assertIn(('mov', 'dword ptr [ebp + 8], eax'), operations)
        self.assertIn(('mov', 'eax, dword ptr [ebp - 4]'), operations)
        self.assertIn(('fstp', 'dword ptr [ebp + 0xc]'), operations)
        self.assertIn(('fld', 'dword ptr [ebp + 0xc]'), operations)
        self.assertLess(operations.index(('mov', 'dword ptr [ecx + 0x18], edx')),
                        operations.index(('mov', 'edx, dword ptr [edx + 0xa4]')))
        self.assertEqual(struct.unpack('<f', retail.bytes_at_va(0x5507f4, 4))[0], 0.5)

        # Character is the primary base of the allocated motion receiver, not
        # the separate virtual GameObject view. No BikeA5C4 alias is changed here.
        locator, = struct.unpack('<I', retail.bytes_at_va(0x551530-4, 4))
        _, offset, _, descriptor, hierarchy = struct.unpack('<5I', retail.bytes_at_va(locator, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(retail.bytes_at_va(descriptor+8, 40).split(b'\0')[0],
                         b'.?AVD3DIMSoultreeCharacter@@')
        _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<'+'I'*count, retail.bytes_at_va(array, count*4)):
            descriptor, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
            bases[retail.bytes_at_va(descriptor+8, 40).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVCharacter@@'], (0, -1, 0))
        self.assertNotEqual(bases[b'.?AVGameObject@@'][1], -1)

        compiler = Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-motion-start-') as directory:
            output = Path(directory)
            extracted = output/'pose_walker.cpp'
            extracted.write_text('#include "motion/Motnctrl.h"\n'+extract(ROOT/CANONICAL,
                                                                         'void Character::ApplyPoseListSlot4('))
            paths, objects = {}, {}
            def compile(source, name, flags=()):
                obj = output/(name+'.obj')
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source),
                    '-o', str(obj), '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'),
                    *[part for flag in flags for part in ('--extra', flag)]], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin1')
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                paths[name], objects[name] = obj, CoffObject(obj)
            compile(ROOT/SOURCE, 'candidate', ('/QIfist',))
            compile(ROOT/CANONICAL, 'canonical')
            compile(extracted, 'helper')
            compile(ROOT/'tests/fixtures/character_motion_start_compare.cpp', 'fixture')
            for name in ('canonical', 'helper'):
                self.assertEqual(complete(objects[name], HELPER)[1], 59)
                self.assertTrue(compare(retail, objects[name], HELPER, 0x4a8a80, 59,
                                        ROOT/'src/krusty2/motion/Motnctrl.bindings.json')['strict_exact'])
            self.assertEqual(complete(objects['canonical'], HELPER), complete(objects['helper'], HELPER))
            candidate = objects['candidate']
            self.assertEqual(complete(candidate, SYMBOL)[1], 93)
            self.assertEqual(complete(candidate, CONVERSION)[1], 368)
            self.assertEqual({s.name for s in candidate.symbols if s.section_number == 0 and s.storage_class == 2},
                             {'__fltused', '__control87', HELPER})
            self.assertNotIn('__ftol', {s.name for s in candidate.symbols})
            conversion = list(decoder.disasm(complete(candidate, CONVERSION)[0], 0))
            self.assertEqual([i.op_str for i in conversion if i.mnemonic == 'fistp'],
                             ['qword ptr [esp + 0x18]'])
            self.assertEqual(sum(i.mnemonic == 'call' for i in conversion), 1)  # _control87 before conversion
            self.assertNotIn(CONVERSION, json.loads((ROOT/SOURCE).with_suffix('.bindings.json').read_text()))

            # Independently invoke the real game builder's compilation path with
            # its exact include inventory/profile and the one required source flag.
            # Compare every nondebug section byte and relocation identity, not just
            # the top-level method or a fixture source copy.
            spec = json.loads((ROOT/'config/game_build.json').read_text())
            built = build_compile_source(ROOT/SOURCE, output/'game-command', compiler,
                [ROOT/p for p in spec['includes']], spec['profile'],
                {**os.environ, 'PYTHONPATH': str(ROOT)}, ('/QIfist',))
            self.assertTrue(built['compiled'], Path(built['log']).read_text())
            self.assertIn('/QIfist', built['command'])
            second = CoffObject(Path(built['object']))
            def sections(obj):
                result = []
                for section in obj.sections:
                    if section.name.startswith('.debug'):
                        continue
                    data = obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
                    rels = [(r.virtual_address, r.type, obj.symbol_by_index[r.symbol_index].name)
                            for r in obj.relocations if r.section_number == section.index]
                    result.append((section.name, section.characteristics, section.raw_size, data, rels))
                return result
            self.assertEqual(sections(candidate), sections(second))
            executable = output/'compare.exe'
            mapfile = output/'compare.map'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')),
                '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', f'/out:{executable}',
                f'/map:{mapfile}', f'/include:{SYMBOL}',
                f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                *[str(paths[name]) for name in ('candidate', 'helper', 'fixture')], 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            address = re.search(re.escape(SYMBOL)+r'\s+([0-9a-fA-F]{8})\s', mapfile.read_text()).group(1)
            result = subprocess.run([str(executable), str(reference), address], capture_output=True,
                                    text=True, timeout=60)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),
                'Character motion start: 320999 state, callback, rounding and exception scenarios match')
            print('Two strict real-helper objects; complete game-command object identical; '+result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
