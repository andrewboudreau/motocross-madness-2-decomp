"""Complete menu hit tests, strict helper closure, and native surface-state proof."""
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
CASES=[('UIButton',0x4734c0,354,328,256,0x552ce0,[0x4735ff,0x47360e,0x47361f]),
       ('UIMultiState',0x478e10,329,303,231,0x553210,[0x478f36,0x478f45,0x478f56])]
DIRECT={0x47354d:0x534a88,0x473561:0x534a88,0x473597:0x511970,
        0x478e80:0x534a88,0x478e94:0x534a88,0x478ece:0x511970}
INDIRECT={0x473512:'dword ptr [0x5502d4]',0x473576:'dword ptr [eax + 0xc0]',
          0x47358f:'dword ptr [ecx + 0x64]',0x4735eb:'dword ptr [eax + 0x80]',
          0x478e47:'dword ptr [0x5502d4]',0x478ea9:'dword ptr [eax + 0xc0]',
          0x478ec2:'dword ptr [ecx + 0x64]',0x478f22:'dword ptr [eax + 0x80]'}

@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class ControlHitTestBehaviorTests(unittest.TestCase):
    def test_complete_hit_tests_and_actual_helper_contracts(self):
        import capstone
        reference=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image=PEImage(reference)
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        classes={c.name:c for c in parse_rtti(image)}
        direct,indirect={},{}
        for name,va,size,_,_,table,returns in CASES:
            code=list(decoder.disasm(image.bytes_at_va(va,size),va))
            self.assertEqual(sum(i.size for i in code),size)
            self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],[(r,b'\xc2\x0c\0') for r in returns])
            self.assertFalse(any('fs:' in i.op_str for i in code))
            boundaries={i.address for i in code}
            for i in code:
                if i.group(capstone.CS_GRP_JUMP):
                    self.assertEqual(i.operands[0].type,capstone.x86.X86_OP_IMM)
                    self.assertIn(i.operands[0].imm,boundaries)
                if i.mnemonic=='call':
                    if i.operands[0].type==capstone.x86.X86_OP_IMM:direct[i.address]=i.operands[0].imm
                    else:indirect[i.address]=i.op_str
            self.assertEqual([(r.object_offset,r.vtables) for r in classes[name].vtable_records],[(0,[table])])
            self.assertEqual(struct.unpack('<I',image.bytes_at_va(table+28*4,4))[0],va)
            self.assertEqual(struct.unpack('<I',image.bytes_at_va(table+48*4,4))[0],0x471fa0 if name=='UIButton' else 0x478810)
        self.assertEqual(direct,DIRECT);self.assertEqual(indirect,INDIRECT)
        self.assertEqual(image.bytes_at_va(0x473622,14),b'\x90'*14)
        self.assertEqual(image.bytes_at_va(0x478f59,7),b'\x90'*7)
        # Independently associate the absolute IAT site with the API name.
        imports=image.directories[1][0]
        found=[]
        while imports:
            oft,ts,fc,name,ft=struct.unpack('<5I',image.bytes_at_va(image.image_base+imports,20))
            if not any((oft,ts,fc,name,ft)):break
            index=0
            while True:
                thunk=struct.unpack('<I',image.bytes_at_va(image.image_base+oft+index*4,4))[0]
                if not thunk:break
                if not thunk&0x80000000 and image.read_c_string_rva(thunk+2)=='PtInRect':found.append((image.image_base+ft+index*4,image.read_c_string_rva(name)))
                index+=1
            imports+=20
        self.assertEqual([(va,dll.lower()) for va,dll in found],[(0x5502d4,'user32.dll')])
        compiler=Path(os.environ['VC6_ROOT']);linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-control-hit-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                obj=output/(name+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),'--vc6-root',str(compiler),
                    '--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed'),*(['--extra','/Od'] if fixture else [])],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return obj,CoffObject(obj)
            candidate_path,candidate=compile_source(ROOT/'samples/ui/ControlHitTest.cpp','candidate')
            fixture_path,_=compile_source(ROOT/'tests/fixtures/control_hit_test_compare.cpp','fixture',True)
            _,aggregate=compile_source(ROOT/'samples/ui/GameUiNearMisses.cpp','aggregate')
            def fingerprint(obj,symbol):
                s=obj.find_symbol(symbol);body,size,relocations=obj.symbol_extent(s)
                return body,size,[(r.virtual_address-s.value,r.type,obj.symbol_by_index[r.symbol_index].name) for r in relocations]
            for name,va,size,emitted,score,_,_ in CASES:
                symbol=f'?ContainsPoint@{name}@@UAEHUtagPOINT@@H@Z'
                self.assertEqual(candidate.symbol_extent(candidate.find_symbol(symbol))[1],emitted)
                self.assertEqual(fingerprint(candidate,symbol),fingerprint(aggregate,symbol))
                table=aggregate.find_symbol(f'??_7{name}@@6B@')
                routes={r.virtual_address-table.value:aggregate.symbol_by_index[r.symbol_index].name
                        for r in aggregate.relocations if r.section_number==table.section_number}
                self.assertEqual(routes[28*4],symbol)
                self.assertEqual(routes[48*4],f'?GetStateTexture@{"UIControl" if name=="UIButton" else name}@@UAEPAVTextureMap@@H@Z')
                report=compare(image,candidate,symbol,va,size,ROOT/'samples/ui/ControlHitTest.bindings.json')
                self.assertFalse(report['strict_exact']);self.assertEqual(report['strict_match']['matching_positions'],score)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==0x20},
                {f'?ContainsPoint@{n}@@UAEHUtagPOINT@@H@Z' for n,*_ in CASES})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},
                {'?PixelFormatBytesPerPixel@@YAHH@Z','?g_TrackGame@@3PAVTrackGame@@A','__imp__PtInRect@12','__ftol','__fltused'})
            self.assertFalse(any(s.name.startswith(('.CRT','.text$x','.xdata')) for s in candidate.sections))
            def extract(text,signature):
                start=text.index(signature);end=text.index('{',start)+1;depth=1
                while depth:depth+=(text[end]=='{')-(text[end]=='}');end+=1
                return text[start:end]
            ui=(ROOT/'src/reconstructed/GameUi.cpp').read_text();tg=(ROOT/'src/reconstructed/Tgafile.cpp').read_text()
            helper_file=output/'helpers.cpp'
            helper_file.write_text('#include "GameUi.h"\n#include "Tgafile.h"\n'
                'class ActualControl:public UIControl {public:TextureMap* Query(int);};\n'
                'class ActualMulti:public UIMultiState {public:TextureMap* Query(int);};\n'+
                extract(ui,'TextureMap* UIControl::GetStateTexture(').replace('UIControl::GetStateTexture','ActualControl::Query')+'\n'+
                extract(ui,'TextureMap* UIMultiState::GetStateTexture(').replace('UIMultiState::GetStateTexture','ActualMulti::Query')+'\n'+
                extract(ui,'TextureMap* UIAnim::GetCurrentTexture(')+'\n'+
                extract(tg,'int PixelFormatBytesPerPixel(').replace('PixelFormatBytesPerPixel','ActualBytesPerPixel')+'\n')
            helper_path,helper=compile_source(helper_file,'helpers')
            _,gameui=compile_source(ROOT/'src/reconstructed/GameUi.cpp','gameui')
            _,tgobj=compile_source(ROOT/'src/reconstructed/Tgafile.cpp','tgafile')
            for original,renamed,va,size,obj,bindings in [
                ('?GetStateTexture@UIControl@@UAEPAVTextureMap@@H@Z','?Query@ActualControl@@QAEPAVTextureMap@@H@Z',0x471fa0,36,gameui,'GameUi'),
                ('?GetStateTexture@UIMultiState@@UAEPAVTextureMap@@H@Z','?Query@ActualMulti@@QAEPAVTextureMap@@H@Z',0x478810,66,gameui,'GameUi'),
                ('?GetCurrentTexture@UIAnim@@QAEPAVTextureMap@@XZ','?GetCurrentTexture@UIAnim@@QAEPAVTextureMap@@XZ',0x472f90,20,gameui,'GameUi'),
                ('?PixelFormatBytesPerPixel@@YAHH@Z','?ActualBytesPerPixel@@YAHH@Z',0x511970,78,tgobj,'Tgafile')]:
                for compiled,symbol in ((obj,original),(helper,renamed)):
                    self.assertEqual(compiled.symbol_extent(compiled.find_symbol(symbol))[1],size)
                    self.assertTrue(compare(image,compiled,symbol,va,size,ROOT/f'src/reconstructed/{bindings}.bindings.json')['strict_exact'])
            self.assertTrue(compare(image,gameui,'?HitTest@UIControl@@UAEHUtagPOINT@@@Z',0x4709f0,122,ROOT/'src/reconstructed/GameUi.bindings.json')['strict_exact'])
            calls=[(i.address,i.op_str) for i in decoder.disasm(image.bytes_at_va(0x4709f0,122),0x4709f0) if i.mnemonic=='call']
            self.assertIn((0x470a50,'dword ptr [edx + 0x70]'),calls)
            layout=output/'layout.cpp'
            layout.write_text('#include "GameUi.h"\n#include "UIDialog.h"\n#include "PCTextureMap.h"\n#include "TrackGame.h"\n#include <stddef.h>\n'
                'typedef char Layout[(sizeof(UIButton)==0x1f0 && sizeof(UIMultiState)==0x1f8 && sizeof(UnknownSurfaceDesc)==124 && '
                'offsetof(UIControl,screenArea)==0x2c && offsetof(UIControl,ownerDialog)==0xb8 && offsetof(UIControl,stateImages)==0x16c && '
                'offsetof(UIDialog,scaleX)==0x9c4 && offsetof(UIDialog,scaleY)==0x9c8 && offsetof(PCTextureMap,systemSurface)==0x70 && '
                'offsetof(Game,renderTarget)==0x10 && offsetof(RenderTarget,surfacePixelFormat)==0x28)?1:-1];\n')
            compile_source(layout,'layout')
            executable=output/'hit.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',
                str(candidate_path),str(fixture_path),str(helper_path),'kernel32.lib','user32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(reference)],capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('ControlHitTest: 2780 pixel, real-helper, callback, FP and held-lock scenarios match',result.stdout);print(result.stdout.strip())

if __name__=='__main__':unittest.main()
