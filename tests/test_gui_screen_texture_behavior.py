"""Whole menu screen-capture candidate, pixels and allocation unwind behavior."""
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
ROOT=Path(__file__).resolve().parents[1]
SYMBOL='?CopyScreenToTexture@GUIManager@@QAEPAVPCTextureMap@@HPAUCameraRect@@@Z'
RANGES={0x486170:896,0x54b870:26,0x54b88a:10}
DEPENDENCIES={0x511970,0x404c80,0x468dd0,0x468f10,0x4e8cc0,0x4a2e20,0x4a2e60,0x4a3010,0x4a3060,0x53471a,0x4c5f00}
IMPORTS={'??3@YAXPAXPBDH@Z','?DebugFree@@YAXPAXPBDH@Z','??0PCTextureMap@@QAE@PAVTextureMapManager@@H@Z',
 '??2@YAPAXIPBDH@Z','?DebugMalloc@@YAPAXIPBDH@Z','?EnableByClassName@GameObject@@IAEXPBD@Z',
 '?AdvanceFrame@RenderTarget@@QAEXXZ','?DisableByClassName@GameObject@@IAEXPBD@Z',
 '?ReleaseDc@BackgroundImage@@QAEHXZ','?PixelFormatBytesPerPixel@@YAHH@Z','?g_TrackGame@@3PAVTrackGame@@A','___CxxFrameHandler','__except_list'}

@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
 'requires native VC6 and private retail image')
class GuiScreenTextureBehaviorTests(unittest.TestCase):
 def test_complete_capture_pixels_callbacks_and_unwind(self):
  import capstone
  reference=Path(os.environ['MCM2_EXE']).resolve()
  self.assertEqual(sha256_file(reference),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
  image=PEImage(reference);decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
  patches=[];seen=set();indirect={}
  for start,size in RANGES.items():
   instructions=list(decoder.disasm(image.bytes_at_va(start,size),start));self.assertEqual(sum(i.size for i in instructions),size)
   boundaries={i.address for i in instructions}
   self.assertEqual(instructions[-1].mnemonic,'jmp' if start==0x54b88a else 'ret')
   if start==0x486170:self.assertEqual((instructions[-1].address,bytes(instructions[-1].bytes)),(0x4864ed,b'\xc2\x08\0'))
   for i in instructions:
    if i.mnemonic!='call' and not i.group(capstone.CS_GRP_JUMP):continue
    operand=i.operands[0]
    if operand.type!=capstone.x86.X86_OP_IMM:
     self.assertEqual(i.mnemonic,'call');indirect[i.address]=i.op_str;continue
    if start<=operand.imm<start+size:self.assertIn(operand.imm,boundaries);continue
    self.assertIn(operand.imm,DEPENDENCIES);self.assertIn(i.bytes[0],(0xe8,0xe9));seen.add(operand.imm)
    patches.append(struct.pack('<III',i.address,operand.imm,i.bytes[0]))
  self.assertEqual(seen,DEPENDENCIES);self.assertEqual(len(patches),13)
  self.assertEqual(indirect,{0x4861eb:'dword ptr [edx + 0x20]',0x4861f6:'dword ptr [eax + 0x24]',0x486277:'dword ptr [eax + 0x64]',0x48634e:'dword ptr [ecx + 0x80]',0x4864ba:'dword ptr [edx + 0x10]'})
  self.assertEqual(struct.unpack('<8I',image.bytes_at_va(0x561878,32)),(0x19930520,1,0x561898,0,0,0,0,0))
  self.assertEqual(struct.unpack('<iI',image.bytes_at_va(0x561898,8)),(-1,0x54b870))
  self.assertEqual(image.bytes_at_va(0x4864f0,7),b'\x8b\x81\xf8\x01\0\0\xc3')
  compiler=Path(os.environ['VC6_ROOT']).resolve();linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
  with tempfile.TemporaryDirectory(prefix='mcm2-gui-screen-') as directory:
   output=Path(directory)
   def compile_source(source,name):
    obj=output/(name+'.obj')
    result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(obj),'--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),*(['--extra','/Od'] if source.startswith('tests/') else [])],cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
    self.assertEqual(result.returncode,0,result.stdout+result.stderr);return obj,CoffObject(obj)
   candidate_path,candidate=compile_source('samples/ui/GuiScreenTexture.cpp','candidate')
   fixture_path,_=compile_source('tests/fixtures/gui_screen_texture_compare.cpp','fixture')
   _,aggregate=compile_source('samples/ui/GUIManagerNearMisses.cpp','aggregate')
   self.assertEqual(candidate.find_symbol(SYMBOL).function_size,888)
   self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==0x20},{SYMBOL})
   self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},IMPORTS)
   self.assertFalse(any(s.name.startswith('.CRT') for s in candidate.sections))
   for coff,binding in [(candidate,'GuiScreenTexture'),(aggregate,'GUIManagerNearMisses')]:
    report=compare(image,coff,SYMBOL,0x486170,896,ROOT/f'samples/ui/{binding}.bindings.json')
    self.assertFalse(report['strict_exact']);self.assertEqual(report['strict_match']['matching_positions'],284)
   def normalized(coff):
    symbol=coff.find_symbol(SYMBOL);body,size,relocs=coff.symbol_extent(symbol);rows=[]
    for r in relocs:
     target=coff.symbol_by_index[r.symbol_index];key=target.name
     if key.startswith(('$L','$T')):key=f'{coff.section(target.section_number).name}+{target.value}'
     rows.append((r.virtual_address-symbol.value,r.type,key))
    return body,size,rows
   self.assertEqual(normalized(candidate),normalized(aggregate))
   patch_file=output/'calls.bin';patch_file.write_bytes(b''.join(patches));executable=output/'capture.exe'
   result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',str(candidate_path),str(fixture_path),'kernel32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
   self.assertEqual(result.returncode,0,result.stdout+result.stderr)
   result=subprocess.run([str(executable),str(reference),str(patch_file)],capture_output=True,text=True,timeout=180)
   self.assertEqual(result.returncode,0,result.stdout+result.stderr)
   self.assertIn('GuiScreenTexture: 8237 complete capture, pixels, state, callback, allocation and unwind scenarios match',result.stdout)
   print(result.stdout.strip())

if __name__=='__main__':unittest.main()
