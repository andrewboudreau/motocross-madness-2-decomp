"""Whole texture-surface setup with native format helpers and scripted surfaces."""
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
from mcm2tool.toolchain import find_child_ci,find_vc6_bin,infer_vc98_root,sha256_file
from tools.match import compare
ROOT=Path(__file__).resolve().parents[1]
SYMBOL='?CreateSurface@PCTextureMap@@UAEHPAXHHHHHHPAUUnknownTexturePalette@@H0HHHHPAUUnknownTextureFormatChoice@@HI@Z'
HELPERS={'PixelFormatBytesPerPixel':('int',0x511970,78),'FillPixelFormat':('void',0x5119c0,266),'IsFormat4444Or8888':('int',0x511ad0,30)}
DEPENDENCIES={0x511ad0,0x5119c0,0x511970,0x511370,0x4c68e0,0x4d1d20,0x4d24d0,0x4c84e0}
INDIRECT={0x4c6b36:'dword ptr [ecx + 4]',0x4c6c72:'dword ptr [edx + 0x18]',0x4c6c94:'dword ptr [ecx + 4]',0x4c6cb3:'dword ptr [edx + 0x7c]',0x4c6e08:'dword ptr [edx + 4]',0x4c6f4c:'dword ptr [ecx + 4]',0x4c6f63:'dword ptr [edx + 0x7c]',0x4c6f7c:'dword ptr [ecx + 0x38]',0x4c6fbf:'dword ptr [ecx + 0x64]',0x4c7067:'dword ptr [edx + 0x80]',0x4c7080:'dword ptr [eax + 0x3c]',0x4c70db:'dword ptr [edx + 0x48]'}

@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),'requires native VC6 and private retail image')
class TextureSurfaceCreateBehaviorTests(unittest.TestCase):
 def test_complete_descriptors_mips_references_and_failures(self):
  import capstone
  reference=Path(os.environ['MCM2_EXE']).resolve();self.assertEqual(sha256_file(reference),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
  image=PEImage(reference);decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
  instructions=list(decoder.disasm(image.bytes_at_va(0x4c69c0,2033),0x4c69c0));self.assertEqual(sum(i.size for i in instructions),2033)
  self.assertEqual([(i.address,bytes(i.bytes))for i in instructions if i.mnemonic=='ret'],[(a,b'\xc2\x44\0')for a in (0x4c6d47,0x4c7093,0x4c70ed,0x4c71ae)])
  self.assertEqual(image.bytes_at_va(0x4c71b1,15),b'\x90'*15);self.assertFalse(any('fs:'in i.op_str for i in instructions))
  self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x555fec+4*4,4))[0],0x4c69c0)
  col=struct.unpack('<I',image.bytes_at_va(0x555fe8,4))[0];self.assertEqual(struct.unpack('<I',image.bytes_at_va(col+4,4))[0],0)
  boundaries={i.address for i in instructions};patches=[];seen=set();indirect={}
  for i in instructions:
   if i.group(capstone.CS_GRP_JUMP):self.assertEqual(i.operands[0].type,capstone.x86.X86_OP_IMM);self.assertIn(i.operands[0].imm,boundaries)
   if i.mnemonic!='call':continue
   operand=i.operands[0]
   if operand.type==capstone.x86.X86_OP_IMM:
    self.assertIn(operand.imm,DEPENDENCIES);self.assertEqual(i.bytes[0],0xe8);seen.add(operand.imm);patches.append(struct.pack('<III',i.address,operand.imm,i.bytes[0]))
   else:indirect[i.address]=i.op_str
  self.assertEqual(seen,DEPENDENCIES);self.assertEqual(len(patches),20);self.assertEqual(indirect,INDIRECT)
  compiler=Path(os.environ['VC6_ROOT']).resolve();linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
  with tempfile.TemporaryDirectory(prefix='mcm2-texture-create-')as directory:
   output=Path(directory)
   def compile_source(source,name,fixture=False):
    obj=output/(name+'.obj');result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),'--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed'),*(['--extra','/Od']if fixture else[])],cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
    self.assertEqual(result.returncode,0,result.stdout+result.stderr);return obj,CoffObject(obj)
   candidate_path,candidate=compile_source(ROOT/'samples/render/TextureSurfaceCreate.cpp','candidate')
   fixture_path,_=compile_source(ROOT/'tests/fixtures/texture_surface_create_compare.cpp','fixture',True)
   _,aggregate=compile_source(ROOT/'samples/render/PCTextureMapNearMisses.cpp','aggregate')
   # Validation-only extraction of three complete current canonical bodies,
   # never retail bytes. Their emitted full extents must remain strict.
   canonical=(ROOT/'src/reconstructed/Tgafile.cpp').read_text();helper_source='#include <string.h>\n#include "RenderInterfaces.h"\n'
   for name,(result_type,_,_)in HELPERS.items():
    start=canonical.index(result_type+' '+name+'(');a=canonical.index('{',start);depth=1;b=a+1
    while depth:depth+=(canonical[b]=='{')-(canonical[b]=='}');b+=1
    helper_source+=canonical[start:b].replace(name,'Actual'+name)+'\n'
   helper_file=output/'format_helpers.cpp';helper_file.write_text(helper_source)
   helper_path,helpers=compile_source(helper_file,'helpers')
   binding=json.loads((ROOT/'samples/render/TextureSurfaceCreate.bindings.json').read_text())
   helper_binding={s.name:binding[s.name.replace('Actual','')]for s in helpers.symbols if s.type==0x20 and s.section_number>0}
   helper_bindings_path=output/'helpers.bindings.json';helper_bindings_path.write_text(json.dumps(helper_binding))
   self.assertEqual(len(helper_binding),3)
   for symbol,va in helper_binding.items():
    name=symbol.split('@@')[0].removeprefix('?Actual');entry=helpers.find_symbol(symbol);self.assertEqual(entry.function_size,HELPERS[name][2])
    self.assertTrue(compare(image,helpers,symbol,int(va,16),entry.function_size,helper_bindings_path)['strict_exact'])
   self.assertEqual(candidate.find_symbol(SYMBOL).function_size,2028)
   self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==0x20},{SYMBOL})
   self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},set(binding)-{SYMBOL})
   self.assertFalse(any(s.name.startswith(('.CRT','.xdata','.text$x'))for s in candidate.sections))
   for coff,bindings in [(candidate,'TextureSurfaceCreate'),(aggregate,'PCTextureMapNearMisses')]:
    report=compare(image,coff,SYMBOL,0x4c69c0,2033,ROOT/f'samples/render/{bindings}.bindings.json');self.assertFalse(report['strict_exact']);self.assertEqual(report['strict_match']['matching_positions'],518)
   def normalized(coff):
    entry=coff.find_symbol(SYMBOL);body,size,relocs=coff.symbol_extent(entry)
    return body,size,[(r.virtual_address-entry.value,r.type,coff.symbol_by_index[r.symbol_index].name)for r in relocs]
   self.assertEqual(normalized(candidate),normalized(aggregate))
   patch_file=output/'calls.bin';patch_file.write_bytes(b''.join(patches));executable=output/'create.exe'
   result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',str(candidate_path),str(fixture_path),str(helper_path),'kernel32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1');self.assertEqual(result.returncode,0,result.stdout+result.stderr)
   result=subprocess.run([str(executable),str(reference),str(patch_file)],capture_output=True,text=True,timeout=180);self.assertEqual(result.returncode,0,result.stdout+result.stderr)
   self.assertIn('TextureSurfaceCreate: 12226 complete descriptor, mip, reference, conversion-boundary and failure scenarios match',result.stdout);print(result.stdout.strip())

if __name__=='__main__':unittest.main()
