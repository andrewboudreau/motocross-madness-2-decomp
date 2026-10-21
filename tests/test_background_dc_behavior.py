"""Complete BackgroundImage DC selection with real region/release source helpers."""
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
SYMBOL='?GetDc@BackgroundImage@@QAEHPAPAXPAUCameraRect@@HHPAH21@Z'
GLOBALS={'?g_GetDcRegionIndex@@3HA':(0x5777a0,4),'?g_GetDcClipRect@@3UCameraRect@@A':(0x5777a8,16)}
HELPERS=[('?MarkRegionPending@BackgroundImage@@QAEXH@Z',0x404cb0,23),('?SetRegionRect@BackgroundImage@@QAEXHPAUCameraRect@@@Z',0x404240,158),('?ReleaseDc@BackgroundImage@@QAEHXZ',0x404c80,41)]
@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class BackgroundDcBehaviorTests(unittest.TestCase):
    def test_complete_dc_selection_storage_and_actual_helpers(self):
        import capstone
        reference=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image=PEImage(reference);decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        code=list(decoder.disasm(image.bytes_at_va(0x4049d0,678),0x4049d0))
        self.assertEqual(sum(i.size for i in code),678)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],[(a,b'\xc2\x1c\0') for a in (0x404c4a,0x404c6b,0x404c73)])
        direct={i.address:i.operands[0].imm for i in code if i.mnemonic=='call' and i.operands[0].type==capstone.x86.X86_OP_IMM}
        indirect={i.address:i.op_str for i in code if i.mnemonic=='call' and i.operands[0].type!=capstone.x86.X86_OP_IMM}
        self.assertEqual(direct,{0x404be3:0x404cb0,0x404bfa:0x404240,0x404c27:0x404c80})
        self.assertEqual(indirect,{0x404c36:'dword ptr [edx + 0x44]',0x404c57:'dword ptr [ecx + 0x44]'})
        self.assertFalse(any('fs:' in i.op_str for i in code));boundaries={i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):self.assertIn(i.operands[0].imm,boundaries)
        self.assertEqual(image.bytes_at_va(0x404c76,10),b'\x90'*10)
        accesses={}
        for i in code:
            for op in i.operands:
                if op.type==capstone.x86.X86_OP_MEM and not op.mem.base and not op.mem.index and (op.mem.disp==0x5777a0 or 0x5777a8<=op.mem.disp<0x5777b8):
                    self.assertEqual(op.size,4);accesses.setdefault(op.mem.disp,set()).add(i.mnemonic)
        self.assertEqual(set(accesses),{0x5777a0,0x5777a8,0x5777ac,0x5777b0,0x5777b4})
        for va,size in GLOBALS.values():
            self.assertEqual(image.bytes_at_va(va,size),bytes(size))
            section=next(s for s in image.sections if s.virtual_address<=va-image.image_base<s.virtual_address+s.virtual_size)
            self.assertEqual(section.characteristics&0xe0000000,0xc0000000)
            self.assertLessEqual(va-image.image_base+size,section.virtual_address+section.raw_size) # Raw-backed zero, not PE zero-fill tail.
        compiler=Path(os.environ['VC6_ROOT']);linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-background-dc-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                obj=output/(name+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),'--vc6-root',str(compiler),
                    '--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed'),*(['--extra','/Od'] if fixture else [])],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return obj,CoffObject(obj)
            candidate_path,candidate=compile_source(ROOT/'samples/render/BackgroundImageNearMisses.cpp','candidate')
            fixture_path,_=compile_source(ROOT/'tests/fixtures/background_dc_compare.cpp','fixture',True)
            self.assertEqual(candidate.symbol_extent(candidate.find_symbol(SYMBOL))[1],678)
            report=compare(image,candidate,SYMBOL,0x4049d0,678,ROOT/'samples/render/BackgroundImageNearMisses.bindings.json')
            self.assertFalse(report['strict_exact']);self.assertEqual(report['strict_match']['matching_positions'],115)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==0x20},{SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},{s for s,_,_ in HELPERS})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0},{SYMBOL,*GLOBALS})
            sections={s.index:s for s in candidate.sections};storage=[]
            for name,(va,size) in GLOBALS.items():
                symbol=candidate.find_symbol(name);section=sections[symbol.section_number]
                self.assertEqual(section.name,'.bss');self.assertEqual(section.raw_ptr,0);self.assertEqual(section.characteristics&0xe0000000,0xc0000000)
                storage.extend(range(symbol.value,symbol.value+size));self.assertEqual(section.raw_size,20)
            self.assertEqual(sorted(storage),list(range(20)))
            self.assertFalse(any(s.name.startswith(('.CRT','.text$x','.xdata')) for s in candidate.sections))
            def extract(text,signature):
                start=text.index(signature);end=text.index('{',start)+1;depth=1
                while depth:depth+=(text[end]=='{')-(text[end]=='}');end+=1
                return text[start:end]
            bg=(ROOT/'src/reconstructed/BackgroundImage.cpp').read_text();helper_file=output/'helpers.cpp'
            helper_file.write_text('#include "BackgroundImage.h"\n#define Target() ((PCRenderTarget*)renderContext)\n'+
                '\n'.join(extract(bg,s) for s in ['void BackgroundImage::MarkRegionPending(', 'void BackgroundImage::SetRegionRect(', 'int BackgroundImage::ReleaseDc('])+'\n')
            helper_path,helper=compile_source(helper_file,'helpers');_,canonical=compile_source(ROOT/'src/reconstructed/BackgroundImage.cpp','background')
            for symbol,va,size in HELPERS:
                for obj in (canonical,helper):
                    self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1],size)
                    self.assertTrue(compare(image,obj,symbol,va,size,ROOT/'src/reconstructed/BackgroundImage.bindings.json')['strict_exact'])
            _,gameui=compile_source(ROOT/'src/reconstructed/GameUi.cpp','gameui')
            caller='?BeginControlDraw@UIDialog@@QAEHPAPAXPAUCameraRect@@PAHPAVUIControl@@@Z'
            self.assertTrue(compare(image,gameui,caller,0x46ed70,316,ROOT/'src/reconstructed/GameUi.bindings.json')['strict_exact'])
            calls=[(i.address,i.op_str) for i in decoder.disasm(image.bytes_at_va(0x46ed70,316),0x46ed70) if i.mnemonic=='call']
            self.assertIn((0x46edfa,'0x4049d0'),calls)
            s=gameui.find_symbol(caller);_,_,rs=gameui.symbol_extent(s)
            self.assertEqual(sum(gameui.symbol_by_index[r.symbol_index].name==SYMBOL for r in rs),1)
            layout=output/'layout.cpp';layout.write_text('#include "BackgroundImage.h"\n#include <stddef.h>\n'
                'class LayoutCheck:public BackgroundImage {public:static void Check(){typedef char Layout[(sizeof(BackgroundImage)==0x74 && sizeof(UnknownBackgroundRegion)==64 && sizeof(CameraRect)==16 && '
                'offsetof(LayoutCheck,renderContext)==0x18 && offsetof(BackgroundImage,offscreenCopy)==0x3c && '
                'offsetof(BackgroundImage,copyValid)==0x40 && offsetof(BackgroundImage,regionCapacity)==0x48 && '
                'offsetof(BackgroundImage,regionTable)==0x50 && offsetof(BackgroundImage,copyDcNext)==0x60 && '
                'offsetof(BackgroundImage,lastDcRegion)==0x64 && offsetof(BackgroundImage,lastDcRegionCopied)==0x68 && '
                'offsetof(BackgroundImage,heldDcSurface)==0x6c && offsetof(BackgroundImage,heldDc)==0x70 && '
                'offsetof(PCRenderTarget,renderSurface)==0x48 && offsetof(RenderTarget,frameIndex)==0x18)?1:-1];}};\n')
            compile_source(layout,'layout');executable=output/'dc.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',
                str(candidate_path),str(fixture_path),str(helper_path),'kernel32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(reference)],capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('BackgroundDc: 21611 region, actual-helper, COM, alias and fault scenarios match',result.stdout);print(result.stdout.strip())

if __name__=='__main__':unittest.main()
