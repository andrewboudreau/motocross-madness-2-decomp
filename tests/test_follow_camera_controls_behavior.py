"""Complete camera input handler, full retail return extent and native behavior."""
import json
import os
from pathlib import Path
import subprocess
import struct
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?UnknownVirtualSlot45@FollowCamera@@UAE_N_NM@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class FollowCameraControlsBehaviorTests(unittest.TestCase):
    def test_complete_controls_input_state_and_floats(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x463a30,4058),0x463a30))
        self.assertEqual(sum(i.size for i in instructions),4058)
        self.assertEqual(instructions[-1].address,0x464a07)
        self.assertEqual(bytes(instructions[-1].bytes),bytes.fromhex('c20800'))
        self.assertEqual(image.bytes_at_va(0x464a0a,6),b'\x90'*6)
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x55282c+45*4,4))[0],0x463a30)
        boundaries = {i.address for i in instructions}
        calls=[]
        for i in instructions:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertEqual(i.operands[0].type,capstone.x86.X86_OP_IMM)
                self.assertIn(i.operands[0].imm,boundaries)
            if i.mnemonic=='call':
                self.assertEqual(i.operands[0].type,capstone.x86.X86_OP_MEM)
                calls.append(i.operands[0].mem.disp)
        self.assertEqual(calls.count(12),30)
        self.assertEqual(calls.count(16),3)
        self.assertEqual(len(calls),33)
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        compiler=Path(os.environ['VC6_ROOT']).resolve()
        linker=find_child_ci(find_vc6_bin(compiler),'link.exe')
        libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        binding_path=ROOT/'samples/camera/FollowCameraControls.bindings.json'
        bindings=json.loads(binding_path.read_text())
        with tempfile.TemporaryDirectory(prefix='mcm2-follow-controls-') as directory:
            output=Path(directory);objects=[];compiled={}
            for source in ('samples/camera/FollowCameraControls.cpp','samples/camera/FollowCameraNearMisses.cpp',
                           'tests/fixtures/follow_camera_controls_compare.cpp'):
                obj=output/(Path(source).stem+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(obj),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),
                    # VC6 optimizer ICE in the fixture's main callback setup;
                    # both actual candidate compilations remain /O2.
                    *(['--extra','/Od'] if source.startswith('tests/') else [])],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin-1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                compiled[source]=CoffObject(obj)
                if not source.endswith('FollowCameraNearMisses.cpp'):objects.append(str(obj))
            candidate=compiled['samples/camera/FollowCameraControls.cpp']
            entry=candidate.find_symbol(SYMBOL)
            self.assertEqual(entry.extent_source,'coff_function_aux')
            self.assertEqual(entry.function_size,4046)
            self.assertEqual({s.name for s in candidate.symbols if s.section_number>0 and s.storage_class==2
                              and s.type==0x20},{SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.section_number==0 and s.storage_class==2},
                             {'?g_TrackGame@@3PAVTrackGame@@A','__fltused'})
            self.assertFalse(any(s.name.startswith('.CRT') for s in candidate.sections))
            report=compare(image,candidate,SYMBOL,0x463a30,4058,binding_path)
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'],927)
            # Historical 4055 omitted the final ret8. Keep the old comparison
            # separate; the native/full-body proof always uses 4058.
            historical=compare(image,candidate,SYMBOL,0x463a30,4055,binding_path)
            self.assertEqual(historical['strict_match']['matching_positions'],927)
            for entry in candidate.symbols:
                if entry.name.startswith('__real@'):
                    section=candidate.section(entry.section_number)
                    self.assertFalse(section.characteristics&0x80000000)
                    size=8 if entry.name.startswith('__real@8@') else 4
                    start=section.raw_ptr+entry.value
                    self.assertEqual(candidate.data[start:start+size],image.bytes_at_va(int(bindings[entry.name],0),size))
            def complete(coff):
                entry=coff.find_symbol(SYMBOL);body,size,rs=coff.symbol_extent(entry)
                return body,size,[(r.virtual_address-entry.value,r.type,coff.symbol_by_index[r.symbol_index].name) for r in rs]
            self.assertEqual(complete(candidate),complete(compiled['samples/camera/FollowCameraNearMisses.cpp']))
            executable=output/'controls.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                f'/out:{executable}',f'/libpath:{libraries}',*objects,'kernel32.lib'],cwd=ROOT,
                capture_output=True,text=True,encoding='latin-1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(reference)],capture_output=True,text=True,timeout=120)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('FollowCameraControls: 195339 complete input, movement, callback and floating-point scenarios match',result.stdout)
            print(result.stdout.strip())


if __name__=='__main__':
    unittest.main()
