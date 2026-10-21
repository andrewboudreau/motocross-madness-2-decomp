"""Complete menu color scan with real strict surface helpers and native COM proof."""
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
ROOT=Path(__file__).resolve().parents[1]
SYMBOL='?TextureHasColor@@YAHPAVTextureMap@@H@Z'

@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class TextureHasColorBehaviorTests(unittest.TestCase):
    def test_complete_color_scan_with_real_surface_helpers(self):
        import capstone
        reference=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image=PEImage(reference)
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        code=list(decoder.disasm(image.bytes_at_va(0x47b490,220),0x47b490))
        self.assertEqual(sum(i.size for i in code),220)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],[(0x47b564,b'\xc3'),(0x47b56b,b'\xc3')])
        self.assertEqual([(i.address,i.op_str) for i in code if i.mnemonic=='call'],[(0x47b4a8,'dword ptr [eax + 0x34]'),(0x47b558,'dword ptr [eax + 0x38]')])
        self.assertFalse(any('fs:' in i.op_str for i in code))
        boundaries={i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertEqual(i.operands[0].type,capstone.x86.X86_OP_IMM)
                self.assertIn(i.operands[0].imm,boundaries)
        self.assertEqual(image.bytes_at_va(0x47b56c,4),b'\x90'*4)
        classes={c.name:c for c in parse_rtti(image)}
        self.assertEqual([(r.object_offset,r.vtables) for r in classes['PCTextureMap'].vtable_records],[(0,[0x555fec])])
        self.assertEqual(struct.unpack('<II',image.bytes_at_va(0x555fec+13*4,8)),(0x4c79e0,0x4c7a50))
        compiler=Path(os.environ['VC6_ROOT']);linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-texture-color-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                obj=output/(name+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),'--vc6-root',str(compiler),
                    '--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed'),*(['--extra','/Od'] if fixture else [])],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return obj,CoffObject(obj)
            candidate_path,candidate=compile_source(ROOT/'samples/ui/TextureHasColor.cpp','candidate')
            fixture_path,_=compile_source(ROOT/'tests/fixtures/texture_has_color_compare.cpp','fixture',True)
            _,aggregate=compile_source(ROOT/'samples/ui/GameUiNearMisses.cpp','aggregate')
            def fingerprint(obj,symbol):
                s=obj.find_symbol(symbol);body,size,relocations=obj.symbol_extent(s)
                return body,size,[(r.virtual_address-s.value,r.type,obj.symbol_by_index[r.symbol_index].name) for r in relocations]
            self.assertEqual(candidate.symbol_extent(candidate.find_symbol(SYMBOL))[1],220)
            self.assertEqual(fingerprint(candidate,SYMBOL),fingerprint(aggregate,SYMBOL))
            report=compare(image,candidate,SYMBOL,0x47b490,220,ROOT/'samples/ui/TextureHasColor.bindings.json')
            self.assertFalse(report['strict_exact']);self.assertEqual(report['strict_match']['matching_positions'],120)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==0x20},{SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},set())
            self.assertFalse(any(s.name.startswith(('.CRT','.text$x','.xdata')) for s in candidate.sections))
            def extract(text,signature):
                start=text.index(signature);end=text.index('{',start)+1;depth=1
                while depth:depth+=(text[end]=='{')-(text[end]=='}');end+=1
                return text[start:end]
            pc=(ROOT/'src/reconstructed/PCTextureMap.cpp').read_text()
            helper_file=output/'helpers.cpp'
            helper_file.write_text('#include <string.h>\n#include "PCTextureMap.h"\n'
                'class ActualTexture:public PCTextureMap {public:void* Lock(void*,long*,int);int Unlock(void*);};\n'+
                extract(pc,'void* PCTextureMap::LockSurface(').replace('PCTextureMap::LockSurface','ActualTexture::Lock')+'\n'+
                extract(pc,'int PCTextureMap::UnlockSurface(').replace('PCTextureMap::UnlockSurface','ActualTexture::Unlock')+'\n')
            helper_path,helper=compile_source(helper_file,'helpers')
            _,canonical=compile_source(ROOT/'src/reconstructed/PCTextureMap.cpp','pctexture')
            for original,renamed,va,size in [
                ('?LockSurface@PCTextureMap@@UAEPAXPAXPAJH@Z','?Lock@ActualTexture@@QAEPAXPAXPAJH@Z',0x4c79e0,99),
                ('?UnlockSurface@PCTextureMap@@UAEHPAX@Z','?Unlock@ActualTexture@@QAEHPAX@Z',0x4c7a50,25)]:
                for obj,symbol in ((canonical,original),(helper,renamed)):
                    self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1],size)
                    self.assertTrue(compare(image,obj,symbol,va,size,ROOT/'src/reconstructed/PCTextureMap.bindings.json')['strict_exact'])
            table=canonical.find_symbol('??_7PCTextureMap@@6B@')
            routes={r.virtual_address-table.value:canonical.symbol_by_index[r.symbol_index].name for r in canonical.relocations if r.section_number==table.section_number}
            self.assertEqual(routes[13*4],'?LockSurface@PCTextureMap@@UAEPAXPAXPAJH@Z')
            self.assertEqual(routes[14*4],'?UnlockSurface@PCTextureMap@@UAEHPAX@Z')
            _,gameui=compile_source(ROOT/'src/reconstructed/GameUi.cpp','gameui')
            for symbol,va,size,call in [('??0UIFrame@@QAE@PBDPAXHH1@Z',0x472960,566,0x472ac9),('?LoadFromStream@UIFrame@@QAEHPAXH0@Z',0x472bc0,315,0x472cb9)]:
                self.assertTrue(compare(image,gameui,symbol,va,size,ROOT/'src/reconstructed/GameUi.bindings.json')['strict_exact'])
                calls=[(i.address,i.op_str) for i in decoder.disasm(image.bytes_at_va(va,size),va) if i.mnemonic=='call']
                self.assertIn((call,'0x47b490'),calls)
                s=gameui.find_symbol(symbol);_,_,relocations=gameui.symbol_extent(s)
                self.assertEqual(sum(gameui.symbol_by_index[r.symbol_index].name==SYMBOL for r in relocations),1)
            layout=output/'layout.cpp'
            layout.write_text('#include "PCTextureMap.h"\n#include <stddef.h>\n'
                'typedef char Layout[(offsetof(TextureMap,textureWidth)==0x14 && offsetof(TextureMap,textureHeight)==0x18 && '
                'offsetof(TextureMap,mapFormat)==0x20 && offsetof(PCTextureMap,systemSurface)==0x70 && sizeof(UnknownSurfaceDesc)==124)?1:-1];\n')
            compile_source(layout,'layout')
            executable=output/'color.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',
                str(candidate_path),str(fixture_path),str(helper_path),'kernel32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(reference)],capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('TextureHasColor: 131899 color, real-helper, COM, callback and guarded-fault scenarios match',result.stdout);print(result.stdout.strip())

if __name__=='__main__':unittest.main()
