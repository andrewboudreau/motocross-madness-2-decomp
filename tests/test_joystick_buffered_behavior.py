"""Complete buffered joystick reader with real strict binding and queue helpers."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?ReadBufferedState@PCJoystickDevice@@QAEHH@Z'
DIRECT = {0x4c316b:0x4bf6a0,0x4c31d3:0x4c2ef0,0x4c3222:0x43cea0,0x4c3260:0x43cea0,
          0x4c33b6:0x4c3090,0x4c3439:0x4c3090,0x4c3686:0x489c00,
          **{site:0x534a88 for site in (0x4c3385,0x4c339f,0x4c3409,0x4c3423,0x4c34a6,0x4c34c0,
                                      0x4c3521,0x4c353b,0x4c3597,0x4c35b1,0x4c3613,0x4c362d)}}
TABLES = [(0x4c36f8,[0x4c3292,0x4c32b8,0x4c32de,0x4c32fb,0x4c3318,0x4c3335,0x4c368b]),
          (0x4c372c,[0x4c3373,0x4c33f7,0x4c3494,0x4c350f,0x4c3585,0x4c3601,0x4c368b]),
          (0x4c3760,[0x4c36a5,0x4c36af,0x4c36b9,0x4c36c3,0x4c36cb])]



@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class JoystickBufferedBehaviorTests(unittest.TestCase):
    def test_complete_buffered_state_and_real_helper_contract(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x4c3100, 1525), 0x4c3100))
        self.assertEqual(sum(i.size for i in instructions), 1525)
        self.assertEqual([(i.address, bytes(i.bytes)) for i in instructions if i.mnemonic == 'ret'],
            [(site, b'\xc2\x04\0') for site in (0x4c311c, 0x4c317d, 0x4c31ad, 0x4c36f2)])
        self.assertEqual(image.bytes_at_va(0x4c3781, 15), b'\x90' * 15)
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        boundaries = {i.address for i in instructions}
        direct, indirect, patches = {}, {}, []
        for instruction in instructions:
            if instruction.group(capstone.CS_GRP_JUMP):
                if instruction.operands[0].type == capstone.x86.X86_OP_IMM:
                    self.assertIn(instruction.operands[0].imm, boundaries)
                else:
                    self.assertIn(instruction.address, (0x4c328b, 0x4c336c, 0x4c369e))
            if instruction.mnemonic == 'call':
                if instruction.operands[0].type == capstone.x86.X86_OP_IMM:
                    self.assertEqual(instruction.bytes[0], 0xe8)
                    direct[instruction.address] = instruction.operands[0].imm
                    patches.append(struct.pack('<III', instruction.address, instruction.operands[0].imm, 0xe8))
                else:
                    indirect[instruction.address] = instruction.op_str
        self.assertEqual(direct, DIRECT)
        self.assertEqual(indirect, {0x4c313b:'dword ptr [ecx + 0x28]', 0x4c3159:'dword ptr [ecx + 0x1c]', 0x4c319a:'dword ptr [edx + 0x28]'})
        for va, targets in TABLES:
            self.assertEqual(list(struct.unpack('<'+'I'*len(targets),image.bytes_at_va(va,4*len(targets)))),targets)
            self.assertTrue(set(targets) <= boundaries)
        for va, count, missing in ((0x4c3714,21,6),(0x4c3748,21,6),(0x4c3774,13,4)):
            self.assertEqual(list(image.bytes_at_va(va,count)), [n//4 if n%4==0 else missing for n in range(count)])
        self.assertEqual([(i.address,i.op_str) for i in instructions if i.address in (0x4c3285,0x4c3366,0x4c3698)],
            [(0x4c3285,'cl, byte ptr [eax + 0x4c3714]'),(0x4c3366,'dl, byte ptr [eax + 0x4c3748]'),(0x4c3698,'dl, byte ptr [eax + 0x4c3774]')])
        self.assertEqual({i.address:i.op_str for i in instructions if i.mnemonic=='jmp' and i.operands[0].type!=capstone.x86.X86_OP_IMM},
            {0x4c328b:'dword ptr [ecx*4 + 0x4c36f8]',0x4c336c:'dword ptr [edx*4 + 0x4c372c]',0x4c369e:'dword ptr [edx*4 + 0x4c3760]'})
        self.assertEqual([(i.address,i.op_str) for i in instructions if i.address in (0x4c327a,0x4c335b,0x4c3691)],
            [(0x4c327a,'eax, 0x14'),(0x4c335b,'eax, 0x14'),(0x4c3691,'eax, 0xc')])
        caller = list(decoder.disasm(image.bytes_at_va(0x4c3960, 172), 0x4c3960))
        self.assertEqual(sum(i.size for i in caller), 172)
        self.assertEqual([(i.address, i.operands[0].imm) for i in caller if i.mnemonic == 'call'
            and i.operands[0].type == capstone.x86.X86_OP_IMM and i.operands[0].imm == 0x4c3100], [(0x4c39d2, 0x4c3100)])
        cls = next(c for c in parse_rtti(image) if c.name == 'PCJoystickDevice')
        self.assertEqual([(b.name, b.mdisp, b.pdisp, b.vdisp) for b in cls.bases],
            [(name, 0, -1, 0) for name in ('PCJoystickDevice', 'JoystickDevice', 'PCInputDevice', 'InputDevice')])
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-joystick-buffered-') as directory:
            output = Path(directory)

            def compile_source(source, name, fixture=False):
                obj = output / (name + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(source), '-o', str(obj),
                    '--vc6-root', str(compiler), '--profile', 'vc6_o2_mt', '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'), *(['--extra', '/Od'] if fixture else [])],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                return obj, CoffObject(obj)

            candidate_path, candidate = compile_source(ROOT / 'samples/inputdevice/JoystickBufferedState.cpp', 'candidate')
            fixture_path, _ = compile_source(ROOT / 'tests/fixtures/joystick_buffered_compare.cpp', 'fixture', True)
            self.assertEqual(candidate.find_symbol(SYMBOL).function_size, 1641)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number > 0 and s.type == 0x20}, {SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class == 2 and s.section_number == 0},
                {'?FeedAxis@JoystickDevice@@QAEXHMM@Z', '?QueueEvent@ControlInterface@@QAEXHHHH@Z', '?g_TrackGame@@3PAVTrackGame@@A', '__fltused', '__ftol', '?ApplyAxisDeadZone@PCJoystickDevice@@QAEHHH@Z', '?ButtonIndexFromOffset@PCJoystickDevice@@QAEHH@Z', '?UnknownReportError@@YAXJPBDH@Z'})
            self.assertFalse(any(s.name.startswith(('.CRT', '.text$x', '.xdata')) for s in candidate.sections))
            report = compare(image, candidate, SYMBOL, 0x4c3100, 1665, ROOT / 'samples/inputdevice/JoystickBufferedState.bindings.json')
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'], 128)
            _, aggregate = compile_source(ROOT / 'samples/inputdevice/PCJoystickDeviceNearMisses.cpp', 'aggregate')

            def fingerprint(obj):
                symbol = obj.find_symbol(SYMBOL)
                body, size, relocations = obj.symbol_extent(symbol)
                def destination(r):
                    target = obj.symbol_by_index[r.symbol_index]
                    return f'local+{target.value-symbol.value}' if target.section_number == symbol.section_number else target.name
                return body, size, [(r.virtual_address-symbol.value, r.type, destination(r)) for r in relocations]

            self.assertEqual(fingerprint(candidate), fingerprint(aggregate))
            # The observing wrappers have different symbols; the complete real
            # helper bodies remain separately compiled and independently strict.
            def extract(text, signature):
                start = text.index(signature)
                end, depth = text.index('{', start) + 1, 1
                while depth:
                    depth += (text[end] == '{') - (text[end] == '}')
                    end += 1
                return text[start:end]

            pc_source = (ROOT / 'src/reconstructed/PCJoystickDevice.cpp').read_text()
            report_source = (ROOT / 'src/reconstructed/PCControl.cpp').read_text()
            joystick = (ROOT / 'src/reconstructed/JoystickDevice.cpp').read_text()
            control = (ROOT / 'src/reconstructed/ControlInterface.cpp').read_text()
            helper_file = output / 'helpers.cpp'
            helper_file.write_text('#include <stdio.h>\n#include <windows.h>\n#include \"PCJoystickDevice.h\"\n#include "JoystickDevice.h"\n#include "ControlInterface.h"\n'
                'class ActualPCJoystickDevice:public PCJoystickDevice {public:int ButtonIndexFromOffset(int);int ApplyAxisDeadZone(int,int);};\n'
                'class ActualJoystickDevice:public JoystickDevice {public:void FeedAxis(int,float,float);};\n'
                'class ActualControlInterface:public ControlInterface {public:void QueueEvent(int,int,int,int);};\n' +
                extract(pc_source, 'static inline float AxisMax(') + '\n' + extract(pc_source, 'static inline float AxisMin(') + '\n' +
                extract(pc_source, 'int PCJoystickDevice::ButtonIndexFromOffset(').replace('PCJoystickDevice::ButtonIndexFromOffset','ActualPCJoystickDevice::ButtonIndexFromOffset') + '\n' +
                extract(pc_source, 'int PCJoystickDevice::ApplyAxisDeadZone(').replace('PCJoystickDevice::ApplyAxisDeadZone','ActualPCJoystickDevice::ApplyAxisDeadZone') + '\n' +
                extract(report_source, 'void UnknownReportError(').replace('UnknownReportError(', 'ActualReportError(') + '\n' +
                extract(control, 'static inline float BindingMax(') + '\n' + extract(control, 'static inline float BindingMin(') + '\n' +
                extract(control, 'void UnknownControlBinding::SetFromAxis(') + '\n' +
                extract(joystick, 'void JoystickDevice::FeedAxis(').replace('JoystickDevice::FeedAxis', 'ActualJoystickDevice::FeedAxis') + '\n' +
                extract(control, 'void ControlInterface::QueueEvent(').replace('ControlInterface::QueueEvent', 'ActualControlInterface::QueueEvent') + '\n')
            helper_path, helper = compile_source(helper_file, 'helpers')
            for owner, name, actual, va, size in [
                ('PCJoystickDevice','?ButtonIndexFromOffset@PCJoystickDevice@@QAEHH@Z','?ButtonIndexFromOffset@ActualPCJoystickDevice@@QAEHH@Z',0x4c2ef0,412),
                ('PCJoystickDevice','?ApplyAxisDeadZone@PCJoystickDevice@@QAEHHH@Z','?ApplyAxisDeadZone@ActualPCJoystickDevice@@QAEHHH@Z',0x4c3090,102),
                ('PCControl','?UnknownReportError@@YAXJPBDH@Z','?ActualReportError@@YAXJPBDH@Z',0x4bf6a0,978),
                ('JoystickDevice', '?FeedAxis@JoystickDevice@@QAEXHMM@Z', '?FeedAxis@ActualJoystickDevice@@QAEXHMM@Z', 0x489c00, 84),
                ('ControlInterface', '?QueueEvent@ControlInterface@@QAEXHHHH@Z', '?QueueEvent@ActualControlInterface@@QAEXHHHH@Z', 0x43cea0, 82),
                ('ControlInterface', '?SetFromAxis@UnknownControlBinding@@QAEXMM@Z', '?SetFromAxis@UnknownControlBinding@@QAEXMM@Z', 0x43cd20, 104),
            ]:
                _, provider = compile_source(ROOT / f'src/reconstructed/{owner}.cpp', owner)
                for obj, symbol in ((provider, name), (helper, actual)):
                    self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], size)
                    self.assertTrue(compare(image, obj, symbol, va, size, ROOT / f'src/reconstructed/{owner}.bindings.json')['strict_exact'])
            _, pc = compile_source(ROOT / 'src/reconstructed/PCJoystickDevice.cpp', 'pc')
            self.assertTrue(compare(image, pc, '?Update@PCJoystickDevice@@UAEHH@Z', 0x4c3960, 172,
                ROOT / 'src/reconstructed/PCJoystickDevice.bindings.json')['strict_exact'])
            layout = output / 'layout.cpp'
            layout.write_text('#include "PCJoystickDevice.h"\n#include "TrackGame.h"\n#include <stddef.h>\n'
                'class LayoutProbe:public PCJoystickDevice {public:static void Check();}; void LayoutProbe::Check(){typedef char Fields[(sizeof(PCJoystickDevice)==0x5ec && '
                'offsetof(LayoutProbe,device)==0x25c && offsetof(LayoutProbe,joystickIndex)==0x260 && '
                'offsetof(LayoutProbe,buttonStates)==0x264 && offsetof(LayoutProbe,axisValues)==0x4e4 && '
                'offsetof(LayoutProbe,axisBindings)==0x4fc && offsetof(LayoutProbe,axisMaximums)==0x58c && '
                'offsetof(LayoutProbe,previousRawAxes)==0x5a4 && offsetof(LayoutProbe,deadZoneScales)==0x5bc && '
                'offsetof(LayoutProbe,field_0x5d5)==0x5d5 && offsetof(LayoutProbe,povValues)==0x5d8)?1:-1];}\n'
                'typedef char Other[(sizeof(UnknownDeviceObjectData)==16 && sizeof(UnknownControlBinding)==0x3c && '
                'offsetof(ControlInterface,queuedEventCount)==0x38 && offsetof(ControlInterface,events)==0x3c && '
                'sizeof(UnknownControlEvent)==20 && offsetof(TrackGame,controlInterface)==0x14)?1:-1];\n')
            compile_source(layout, 'layout')
            patch_file = output / 'calls.bin'
            patch_file.write_bytes(b''.join(patches))
            executable = output / 'joystick.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', '/base:0x100000', '/fixed', '/opt:ref',
                f'/out:{executable}', f'/libpath:{libraries}', str(candidate_path), str(fixture_path), str(helper_path), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable), str(reference), str(patch_file), str(ROOT / 'samples/inputdevice/JoystickBufferedState.cpp')], capture_output=True, text=True, timeout=180)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('JoystickBuffered: 4520 complete records, axes, timestamps, queues, retry, FP and fault scenarios match', result.stdout)
            print(result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
