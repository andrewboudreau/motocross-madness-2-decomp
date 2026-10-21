"""Complete two-motion blend, actual math closure, and retained caller objects."""
import hashlib
from collections import Counter
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.build_game import compile_source as game_compile
from test_edit_box_on_char import extract
import test_eco_quadtree_connections as views
ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/physics/motion/CharacterMotionBlend.cpp'
AGGREGATE = 'samples/physics/motion/Motnctrl.cpp'
SYMBOL = '?Method_0x004a8c50@Character@@QAEXHHMM@Z'
INTERPOLATE = '?InterpolatePose@@YAXPAUCharacterPose@@00M@Z'
FILENAMES = ('src/krusty2/vehicle/Bike.cpp', 'src/krusty2/vehicle/KrustyBike.cpp', AGGREGATE)


def complete(obj, name):
    entry = obj.find_symbol(name)
    raw, size, rels = obj.symbol_extent(entry)
    return raw, size, [(r.virtual_address-entry.value, r.type,
                       obj.symbol_by_index[r.symbol_index].name) for r in rels]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and pinned private retail')
class CharacterMotionBlendTests(unittest.TestCase):
    def test_complete_blend_with_actual_math_and_observed_fault_order(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        pe = PEImage(image)
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        md.detail = True
        instructions = list(md.disasm(pe.bytes_at_va(0x4a8c50, 1011), 0x4a8c50))
        self.assertEqual(sum(i.size for i in instructions), 1011)
        self.assertEqual([(i.address, i.op_str) for i in instructions if i.mnemonic == 'ret'], [(0x4a9040, '0x10')])
        self.assertEqual(pe.bytes_at_va(0x4a9043, 13), b'\x90'*13)
        addresses = {i.address for i in instructions}
        for instruction in instructions:
            if instruction.group(capstone.CS_GRP_JUMP):
                self.assertEqual(instruction.operands[0].type, capstone.x86.X86_OP_IMM)
                self.assertIn(instruction.operands[0].imm, addresses)
        calls = [(i.address, i.op_str) for i in instructions if i.mnemonic == 'call']
        self.assertEqual([x for x in calls if not x[1].startswith('0x')], [(0x4a901c, 'dword ptr [edx + 0x10]')])
        direct = [int(target, 16) for _, target in calls if target.startswith('0x')]
        self.assertEqual(set(direct), {0x4a8930, 0x421d00, 0x5015b0, 0x421cb0, 0x5087b0, 0x515600, 0x4a8470})
        self.assertEqual(direct.count(0x4a8470), 2)
        ops = {i.address: (i.mnemonic, i.op_str) for i in instructions}
        for address, operation in {
            0x4a8c8d: ('fistp', 'dword ptr [eax]'),
            0x4a8c92: ('mov', 'dword ptr [ecx + 0x18], edx'),
            0x4a8ca9: ('mov', 'dword ptr [ecx + 0x1c], edi'),
            0x4a8cac: ('mov', 'edi, dword ptr [edx + 0xa4]'),
            0x4a8ce3: ('lea', 'eax, [ebp + 8]'),
            0x4a8d04: ('fistp', 'dword ptr [eax]'),
            0x4a901f: ('mov', 'ecx, dword ptr [edi + 0x1c]'),
            0x4a9029: ('mov', 'edx, dword ptr [ecx + 8]'),
        }.items():
            self.assertEqual(ops[address], operation)
        locator, = struct.unpack('<I', pe.bytes_at_va(0x551530-4, 4))
        _, offset, _, descriptor, hierarchy = struct.unpack('<5I', pe.bytes_at_va(locator, 20))
        self.assertEqual(offset, 0)
        self.assertEqual(pe.bytes_at_va(descriptor+8, 48).split(b'\0')[0], b'.?AVD3DIMSoultreeCharacter@@')
        _, _, count, array = struct.unpack('<4I', pe.bytes_at_va(hierarchy, 16))
        bases = {}
        for pointer in struct.unpack('<'+'I'*count, pe.bytes_at_va(array, count*4)):
            descriptor, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', pe.bytes_at_va(pointer, 24))
            bases[pe.bytes_at_va(descriptor+8, 48).split(b'\0')[0]] = (mdisp, pdisp, vdisp)
        self.assertEqual(bases[b'.?AVCharacter@@'], (0, -1, 0))
        self.assertNotEqual(bases[b'.?AVGameObject@@'][1], -1)
        caller = {i.address: (i.mnemonic, i.op_str) for i in md.disasm(pe.bytes_at_va(0x407440, 485), 0x407440)}
        self.assertEqual(caller[0x4075b0], ('mov', 'ecx, dword ptr [esi + 0x5c4]'))
        self.assertEqual(caller[0x4075b7], ('call', '0x4a8c50'))
        self.assertEqual(caller[0x4075d4], ('lea', 'ecx, [esi + 0x21c]'))
        self.assertEqual(caller[0x4075da], ('call', '0x4a8c50'))
        loader = {i.address: (i.mnemonic, i.op_str) for i in md.disasm(pe.bytes_at_va(0x4079c0, 6745), 0x4079c0)}
        self.assertEqual(loader[0x407c6a], ('mov', 'ecx, eax'))
        self.assertEqual(loader[0x407c6c], ('call', '0x4455b0'))
        self.assertEqual(loader[0x407c84], ('mov', 'dword ptr [ebp + 0x5c4], eax'))
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        pins = json.loads((ROOT/'tests/fixtures/character_motion_blend_objects.json').read_text())
        self.assertEqual(tuple(pins['filenames']), FILENAMES)
        with tempfile.TemporaryDirectory(prefix='mcm2-character-blend-') as temporary:
            output = Path(temporary)
            objects, paths = {}, {}
            def compile_file(source, name, flags=()):
                source = Path(source)
                if not source.is_absolute(): source = ROOT/source
                obj = output/(name+'.obj')
                run = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt', '--include', str(ROOT/'src/krusty2'),
                    '--include', str(ROOT/'src/krusty2/vehicle'),
                    *[item for flag in flags for item in ('--extra', flag)]], capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
                paths[name], objects[name] = obj, CoffObject(obj)
                return objects[name]
            for index, (source, pin) in enumerate(pins['objects'].items()):
                obj = compile_file(source, 'old'+str(index), ('/QIfist',) if 'CharacterMotionStart' in source else ())
                with patch.object(views, 'FILENAMES', FILENAMES): rows = views.whole_object(obj, (ROOT,))
                self.assertEqual(dict(sections=len(rows), relocations=sum(len(row[-1]) for row in rows),
                    sha256=hashlib.sha256(json.dumps(rows, separators=(',', ':')).encode()).hexdigest()), pin, source)
            candidate = compile_file(SOURCE, 'candidate', ('/QIfist',))
            text = (ROOT/AGGREGATE).read_text()
            helper = output/'pose.cpp'
            helper.write_text('#include "motion/Motnctrl.h"\n'+'\n'.join(extract(text, marker) for marker in (
                'inline void ClampFloat(', 'inline float InterpolatedLengthSquared(',
                'inline Vec3 InterpolatedUnitVector(', 'void InterpolatePose('))+'\n')
            compile_file(helper, 'pose')
            compile_file('src/reconstructed/FastMath.cpp', 'fast')
            compile_file('src/reconstructed/VectorCalls.cpp', 'vectors')
            compile_file('src/reconstructed/Math3D.cpp', 'math')
            compile_file('src/reconstructed/CrossProduct.cpp', 'cross')
            compile_file('tests/fixtures/character_motion_blend_compare.cpp', 'fixture', ('/Od', '/Op'))
            self.assertEqual(complete(candidate, SYMBOL)[1], 441)
            self.assertEqual(complete(candidate, '?BlendFirst@@')[1], 652)
            self.assertEqual(complete(candidate, '?FloatToInt@@')[1], 368)
            self.assertEqual(Counter(name.split('@@')[0] for _, kind, name in complete(candidate, '?BlendFirst@@')[2] if kind == 20),
                Counter({'?Vec3SubtractCall': 3, '?Vec3ScaleCall': 9, '?Vec3AddCall': 6,
                         '?Vec3Normalize': 2, '?CrossProductCall': 2}))
            bss = [section for section in candidate.sections if section.name == '.bss']
            crt = [section for section in candidate.sections if section.name == '.CRT$XCU']
            self.assertEqual([(section.raw_size, section.characteristics) for section in bss], [(60, 0xc0400080)])
            self.assertEqual([(section.raw_size, section.characteristics) for section in crt], [(16, 0xc0300040)])
            self.assertEqual([(r.virtual_address, candidate.symbol_by_index[r.symbol_index].name)
                for r in candidate.relocations if r.section_number == crt[0].index],
                [(0, '_$E2'), (4, '_$E5'), (8, '_$E8'), (12, '_$E11')])
            self.assertEqual({symbol.name: symbol.value for symbol in candidate.symbols if symbol.name.startswith('_kVec3')},
                             {'_kVec3Zero': 0, '_kVec3XAxis': 32, '_kVec3YAxis': 48, '_kVec3ZAxis': 16})

            self.assertEqual(complete(objects['pose'], INTERPOLATE), complete(objects['old2'], INTERPOLATE))
            strict = 0
            for name, symbol, va, extent, binding in (
                ('pose', INTERPOLATE, 0x4a8470, 1205, 'samples/physics/motion/Motnctrl.bindings.json'),
                ('old2', INTERPOLATE, 0x4a8470, 1205, 'samples/physics/motion/Motnctrl.bindings.json'),
                ('pose', '??0Vec3@@QAE@MMM@Z', 0x404e60, 25, None),
                ('pose', '??H@YA?AUVec3@@ABU0@0@Z', 0x421cb0, 69, None),
                ('pose', '??D@YA?AUVec3@@ABU0@M@Z', 0x5015b0, 69, None),
                ('pose', '?CrossProduct@@YA?AUVec3@@ABU1@0@Z', 0x515600, 91, None),
                ('vectors', '?Vec3AddCall@@', 0x421cb0, 69, None),
                ('vectors', '?Vec3SubtractCall@@', 0x421d00, 69, None),
                ('vectors', '?Vec3ScaleCall@@', 0x5015b0, 69, None),
                ('vectors', '?Vec3DotCall@@', 0x40ae30, 29, None),
                ('cross', '?CrossProductCall@@', 0x515600, 91, None),
                ('math', '?Vec3Normalize@@', 0x5087b0, 150, 'src/reconstructed/Math3D.bindings.json'),
                ('fast', '?FastInvSqrt@@', 0x460c00, 107, 'src/reconstructed/FastMath.bindings.json'),
                ('fast', '?InitFastInvSqrtTable@@', 0x460bb0, 74, 'src/reconstructed/FastMath.bindings.json'),
            ):
                report = compare(pe, objects[name], symbol, va, extent, ROOT/binding if binding else ROOT/SOURCE.replace(".cpp", ".bindings.json"))
                self.assertTrue(report.get('strict_exact'), report)
                strict += 1
            self.assertEqual(strict, 14)
            # Header-injected constants/CRT functions are real emitted overhead, not retail TU ownership.
            for symbol in ('_$E2', '_$E1', '_$E5', '_$E4', '_$E8', '_$E7', '_$E11', '_$E10'):
                self.assertEqual(complete(candidate, symbol), complete(objects['math'], symbol))
            self.assertEqual(complete(candidate, '?FloatToInt@@'), complete(objects['old3'], '?FloatToInt@@'))
            spec = json.loads((ROOT/'config/game_build.json').read_text())
            built = game_compile(ROOT/SOURCE, output/'game-command', compiler, [ROOT/p for p in spec['includes']],
                spec['profile'], {**os.environ, 'PYTHONPATH': str(ROOT)}, ('/QIfist',))
            self.assertTrue(built['compiled'], Path(built['log']).read_text())
            with patch.object(views, 'FILENAMES', FILENAMES):
                self.assertEqual(views.whole_object(candidate, (ROOT,)), views.whole_object(CoffObject(built['object']), (ROOT,)))
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            library = find_child_ci(infer_vc98_root(compiler), 'lib')
            def execute(label, body='candidate', pose='pose'):
                exe = output/(label+'.exe')
                run = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                    '/out:'+str(exe), '/libpath:'+str(library), *[str(paths[name]) for name in (body, pose, 'fixture', 'fast', 'vectors', 'math', 'cross')], 'kernel32.lib'],
                    capture_output=True, text=True, encoding='latin1')
                self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
                return subprocess.run([str(exe), str(image)], capture_output=True, text=True, timeout=60)
            run = execute('correct')
            self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
            self.assertEqual(run.stdout.strip(), 'Blend initial: 576692 cases, 51240 frame-boundary AV, 176 pose-boundary AV')
            # Three current-harness controls distinguish actual first-path math, helper dot association,
            # and the conversion exception backend. Each must fail a specific independent comparison.
            negative = output/'all-helper.cpp'
            source = (ROOT/SOURCE).read_text().replace('#include "../../../src/krusty2/motion/Motnctrl.h"', '#include "motion/Motnctrl.h"').replace('#include "CharacterMotionFloatToInt.h"', '#include "'+str(ROOT/'samples/physics/motion/CharacterMotionFloatToInt.h').replace('\\','/')+'"')
            self.assertEqual(source.count('BlendFirst(&firstFrame'), 1)
            negative.write_text(source.replace('BlendFirst(&firstFrame', 'InterpolatePose(&firstFrame'))
            compile_file(negative, 'all-helper', ('/QIfist',))
            control = execute('source-control', 'all-helper')
            self.assertEqual(control.returncode, 3, control.stdout+control.stderr)
            self.assertIn('Mismatch seed1 ca1 cb1 phase0 w0', control.stdout)
            dot = output/'old-dot.cpp'
            self.assertEqual(helper.read_text().count('return Vec3DotCall(&v, &v);'), 1)
            dot.write_text(helper.read_text().replace('return Vec3DotCall(&v, &v);', 'return DotProduct(v, v);'))
            compile_file(dot, 'old-dot')
            control = execute('dot-control', pose='old-dot')
            self.assertEqual(control.returncode, 3, control.stdout+control.stderr)
            self.assertIn('Mismatch seed1 ca1 cb1 phase0 w1', control.stdout)
            compile_file(SOURCE, 'old-backend')
            control = execute('backend-control', 'old-backend')
            self.assertEqual(control.returncode, 49, control.stdout+control.stderr)
            self.assertIn('Exception mismatch mask0 mode20000 phase0 offset24', control.stdout)
            print('14 strict helpers; four whole-object pins; '+run.stdout.strip()+'; three required negative controls fail')


if __name__ == '__main__':
    unittest.main()
