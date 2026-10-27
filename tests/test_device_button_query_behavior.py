"""Strict per-device dispatcher with virtual-boundary and real input-helper proof."""
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
SYMBOL = '?IsDeviceButtonDown@PCControlInterface@@UAEHHHHH@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class DeviceButtonQueryTests(unittest.TestCase):
    def test_complete_dispatcher_and_actual_input_queries(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        code = list(decoder.disasm(image.bytes_at_va(0x4bf4f0, 85), 0x4bf4f0))
        self.assertEqual(sum(i.size for i in code), 85)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],
            [(a,b'\xc2\x10\0') for a in (0x4bf518,0x4bf53d,0x4bf542)])
        self.assertEqual([(i.address,i.op_str) for i in code if i.mnemonic=='call'],
            [(0x4bf515,'dword ptr [eax + 0x14]'),(0x4bf53a,'dword ptr [edx + 8]')])
        self.assertFalse(any('fs:' in i.op_str for i in code))
        boundaries = {i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):
                if i.operands[0].type==capstone.x86.X86_OP_IMM:
                    self.assertIn(i.operands[0].imm,boundaries)
                else:
                    self.assertEqual((i.address,i.op_str),(0x4bf4f9,'dword ptr [eax*4 + 0x4bf548]'))
        self.assertEqual(struct.unpack('<4I',image.bytes_at_va(0x4bf548,16)),(0x4bf500,0x4bf51b,0x4bf520,0x4bf520))
        self.assertEqual(image.bytes_at_va(0x4bf558,8),b'\x90'*8)
        classes={c.name:c for c in parse_rtti(image)}
        cls=classes['PCControlInterface']
        self.assertEqual([(b.name,b.mdisp,b.pdisp,b.vdisp) for b in cls.bases],
            [(name,0,-1,0) for name in ('PCControlInterface','ControlInterface')])
        self.assertEqual(cls.vtables,[0x555e2c])
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x555e2c+12,4))[0],0x4bf4f0)
        for name,table,slot,target in [('PCKeyboardDevice',0x555f58,5,0x4c4570),
                ('PCMouseDevice',0x555f78,5,0x4c4a90),('JoystickDevice',0x5541fc,2,0x489980),
                ('PCJoystickDevice',0x555ef8,2,0x489980)]:
            self.assertEqual([(r.object_offset,r.vtables) for r in classes[name].vtable_records],[(0,[table])])
            self.assertEqual(struct.unpack('<I',image.bytes_at_va(table+4*slot,4))[0],target)
        compiler=Path(os.environ['VC6_ROOT']).resolve()
        linker=find_child_ci(find_vc6_bin(compiler),'link.exe')
        libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-device-button-query-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                obj=output/(name+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),
                    '--include',str(ROOT/'src/reconstructed'),*(['--extra','/Od'] if fixture else [])],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return obj,CoffObject(obj)
            def extract(text,signature):
                start=text.index(signature)
                end,depth=text.index('{',start)+1,1
                while depth:
                    depth+=(text[end]=='{')-(text[end]=='}')
                    end+=1
                return text[start:end]
            _,control=compile_source(ROOT/'src/reconstructed/PCControl.cpp','control')
            self.assertEqual(control.symbol_extent(control.find_symbol(SYMBOL))[1],104)
            self.assertTrue(compare(image,control,SYMBOL,0x4bf4f0,104,ROOT/'src/reconstructed/PCControl.bindings.json')['strict_exact'])
            table=control.find_symbol('??_7PCControlInterface@@6B@')
            reloc=next(r for r in control.relocations if r.section_number==table.section_number and r.virtual_address==table.value+12)
            self.assertEqual(control.symbol_by_index[reloc.symbol_index].name,SYMBOL)
            # The native comparison links the canonical body alone, without the
            # rest of PCControl.cpp (DirectInput creation and enumeration).
            candidate_file=output/'candidate.cpp'
            candidate_file.write_text('#include "PCControl.h"\n#include "JoystickDevice.h"\n#include "KeyboardDevice.h"\n#include "MouseDevice.h"\n'
                +extract((ROOT/'src/reconstructed/PCControl.cpp').read_text(),'int PCControlInterface::IsDeviceButtonDown(')+'\n')
            candidate_path,candidate=compile_source(candidate_file,'candidate')
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==0x20},{SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},set())
            self.assertFalse(any(s.name.startswith(('.CRT','.text$x','.xdata')) for s in candidate.sections))
            fixture_path,_=compile_source(ROOT/'tests/fixtures/device_button_query_compare.cpp','fixture',True)
            helper_file=output/'helpers.cpp'
            helper_source='#include "PCKeyboardDevice.h"\n#include "PCMouseDevice.h"\n#include "JoystickDevice.h"\n#include "TrackGame.h"\n'
            providers=[('PCKeyboardDevice','ActualKeyboard',0x4c4570,110),
                       ('PCMouseDevice','ActualMouse',0x4c4a90,125),('JoystickDevice','ActualJoystick',0x489980,145)]
            for owner,actual,_,_ in providers:
                helper_source+=f'class {actual}:public {owner} {{public:int Query(int,int,UnknownInputEntry*);}};\n'
                text=(ROOT/f'src/reconstructed/{owner}.cpp').read_text()
                helper_source+=extract(text,f'int {owner}::IsButtonDown(').replace(f'{owner}::IsButtonDown',f'{actual}::Query')+'\n'
            helper_source+=extract((ROOT/'src/reconstructed/KeyboardDevice.cpp').read_text(),'int KeyboardDevice::ModifierHolds(')+'\n'
            helper_file.write_text(helper_source)
            helper_path,helper=compile_source(helper_file,'helpers')
            for owner,actual,va,size in providers:
                name=f'?IsButtonDown@{owner}@@UAEHHHPAUUnknownInputEntry@@@Z'
                actual_name=f'?Query@{actual}@@QAEHHHPAUUnknownInputEntry@@@Z'
                _,canonical=compile_source(ROOT/f'src/reconstructed/{owner}.cpp',owner)
                vtable=canonical.find_symbol('??_7'+owner+'@@6B@')
                slot=2 if owner=='JoystickDevice' else 5
                target=next(r for r in canonical.relocations if r.section_number==vtable.section_number and r.virtual_address==vtable.value+4*slot)
                self.assertEqual(canonical.symbol_by_index[target.symbol_index].name,name)
                for obj,symbol in ((canonical,name),(helper,actual_name)):
                    self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1],size)
                    self.assertTrue(compare(image,obj,symbol,va,size,ROOT/f'src/reconstructed/{owner}.bindings.json')['strict_exact'])
            _,keyboard=compile_source(ROOT/'src/reconstructed/KeyboardDevice.cpp','keyboard')
            for obj in (keyboard,helper):
                symbol='?ModifierHolds@KeyboardDevice@@QAEHH@Z'
                self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1],139)
                self.assertTrue(compare(image,obj,symbol,0x48a240,139,ROOT/'src/reconstructed/KeyboardDevice.bindings.json')['strict_exact'])
            layout=output/'layout.cpp'
            layout.write_text('#include "PCControl.h"\n#include "PCKeyboardDevice.h"\n#include "PCMouseDevice.h"\n#include "JoystickDevice.h"\n#include <stddef.h>\n'
                'typedef char ControlLayout[(offsetof(ControlInterface,keyboard)==0x34 && offsetof(ControlInterface,mouse)==0x30 && offsetof(ControlInterface,joysticks)==0x10 && sizeof(PCControlInterface)==0xcc8)?1:-1];\n'
                'class KeyLayout:public PCKeyboardDevice {public:static void Check();};void KeyLayout::Check(){typedef char T[(sizeof(PCKeyboardDevice)==0x16dc && offsetof(KeyLayout,keyStates)==0x260 && offsetof(KeyLayout,modifierState)==0x16d8)?1:-1];}\n'
                'class MouseLayout:public PCMouseDevice {public:static void Check();};void MouseLayout::Check(){typedef char T[(sizeof(PCMouseDevice)==0x2e8 && offsetof(MouseLayout,buttonStates)==0x260)?1:-1];}\n'
                'class JoyLayout:public JoystickDevice {public:static void Check();};void JoyLayout::Check(){typedef char T[(sizeof(JoystickDevice)==0x578 && offsetof(JoyLayout,buttonStates)==0x264)?1:-1];}\n')
            compile_source(layout,'layout')
            executable=output/'query.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                f'/out:{executable}',f'/libpath:{libraries}',str(candidate_path),str(fixture_path),str(helper_path),'kernel32.lib'],
                cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(reference)],capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('DeviceButtonQuery: 45489 dispatch, real-helper, callback and fault scenarios match',result.stdout)
            print(result.stdout.strip())


if __name__=='__main__':
    unittest.main()
