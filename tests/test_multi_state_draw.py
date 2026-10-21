"""Complete multistate Draw with unchanged source helpers and finite OS contracts."""
import hashlib
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
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.build_game import compile_source as game_compile
from test_edit_box_on_char import extract
from test_edit_box_draw import complete_object, signature

ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'samples/ui/MultiStateDraw.cpp'
SYMBOL='?Draw@UIMultiState@@UAEHXZ'
DIRECT={0x4785e9:0x534a88,0x4785fd:0x534a88,0x47864f:0x404480,0x4786c2:0x46ed70,
        0x47870c:0x472f80,0x478791:0x46f550,0x4787d8:0x46f6c0,0x4787e8:0x46eeb0}
GROUPS={
    'ui':('GameUi',None,[
        ('int UIDialog::BeginControlDraw(',0x46ed70,316),
        ('void UIDialog::EndControlDraw(',0x46eeb0,75),
        ('void UIDialog::DrawAlignedText(',0x46f550,354),
        ('void UIDialog::DrawControlText(',0x46f6c0,454),
        ('void UIDialog::DrawShadowText(',0x46f3c0,93),
        ('void UIDialog::DrawTextRuns(',0x46f420,293),
        ('UIFrame* UIAnim::GetCurrentFrame(',0x472f80,8),
        ('int UIMultiState::GetControlText(',0x478260,20),
        ('int UIMultiState::GetTextLength(',0x478280,20),
        ('int UIMultiState::GetTextLines(',0x4782a0,20),
        ('void* UIMultiState::GetTextExtent(',0x4782c0,20)]),
    'background':('BackgroundImage','#include "BackgroundImage.h"\n#define Target() ((PCRenderTarget*)renderContext)\nCameraRect g_DrawImageClipRect,g_DrawImageSourceRect;\n',[
        ('int BackgroundImage::DrawImage(',0x404480,639),
        ('int BackgroundImage::ReleaseDc(',0x404c80,41),
        ('void BackgroundImage::MarkRegionPending(',0x404cb0,23),
        ('void BackgroundImage::SetRegionRect(',0x404240,158)]),
    'texture':('PCTextureMap','#include "PCTextureMap.h"\n',[
        ('int PCTextureMap::BlitTo(',0x4c7b00,54)]),
}
LAYOUT={
    'UIControl':{'renderContext':0x18,'screenArea':0x2c,'sourceBlit':0xa4,'ownerDialog':0xb8,
                 'controlText':0xc0,'textColor':0xc4,'dropColor':0xc8,'controlTextLength':0xd0,
                 'textDrop':0xd4,'textAlign':0xd8,'textExtent':0xdc,'textLines':0xe4,
                 'textLeftMargin':0xe8,'textTopMargin':0xec,'fontHandle':0x128,'fontHeight':0x160,
                 'backgroundRegion':0x1b8,'redrawFrames':0x1bc,'needsRedraw':0x1c0,'drawnTexture':0x1cc},
    'UIMultiState':{'stateCount':0x1ec,'selectedState':0x1f0,'stateTable':0x1f4},
    'UIDialog':{'renderContext':0x18,'dialogFont':0xd8,'dialogFontHeight':0xdc,'dialogBackground':0x110,
                'scaleX':0x9c4,'scaleY':0x9c8,'drawClipRegion':0x7f00,'previousFont':0x7f04},
    'TextureMap':{'textureWidth':0x14,'textureHeight':0x18,'usesColorKey':0x30},
    'PCTextureMap':{'systemSurface':0x70},
    'PCRenderTarget':{'renderSurface':0x48},
    'RenderTarget':{'targetWidth':0xc,'targetHeight':0x10,'frameModulus':0x14,'frameIndex':0x18,'frameCount':0x1c},
    'BackgroundImage':{'renderContext':0x18,'offscreenCopy':0x3c,'copyValid':0x40,'regionCapacity':0x48,
                       'regionTable':0x50,'heldDcSurface':0x6c,'heldDc':0x70},
    'UIAnim':{'currentFrame':8,'frameList':0x28},'UIFrame':{'frameWidth':0xc},
    'UnknownGameUiState':{'image':0,'text':8,'textLength':0xc,'textExtent':0x14,'textLines':0x1c},
    'UnknownGameUiTextRun':{'text':0,'kind':4,'length':8,'color':12,'width':16,'position':20},
    'UnknownBackgroundRegion':{'framesLeft':0,'frameRects':4,'lastFrameCount':0x38},
}


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class MultiStateDrawTests(unittest.TestCase):
    def test_full_draw_actual_helpers_state_and_finite_platform_contract(self):
        import capstone
        image_path=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image=PEImage(image_path);decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        def decoded(va,size):return list(decoder.disasm(image.bytes_at_va(va,size),va))
        code=decoded(0x478570,672)
        self.assertEqual(sum(i.size for i in code),672)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],[(0x4787c9,b'\xc3'),(0x47880f,b'\xc3')])
        self.assertEqual({i.address:i.operands[0].imm for i in code if i.mnemonic=='call' and i.operands[0].type==capstone.x86.X86_OP_IMM},DIRECT)
        self.assertEqual({i.address:i.op_str for i in code if i.mnemonic=='call' and i.operands[0].type!=capstone.x86.X86_OP_IMM},{0x4787b7:'dword ptr [edx + 0xc]'})
        addresses={i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):self.assertIn(i.operands[0].imm,addresses)
        self.assertFalse(any('fs:' in i.op_str for i in code))
        ftol=decoded(0x534a88,39)
        self.assertEqual(sum(i.size for i in ftol),39)
        self.assertEqual((ftol[-1].address,ftol[-1].mnemonic),(0x534aae,'ret'))
        # Native fixture also requires exact equality of these complete CRT bytes
        # before redirecting the reference's two conversion calls to linked VC6.
        classes={c.name:c for c in parse_rtti(image)}
        cls=classes['UIMultiState']
        self.assertEqual([(r.object_offset,r.vtables) for r in cls.vtable_records],[(0,[0x553210])])
        self.assertTrue(any(b.name=='UIControl' and b.mdisp==0 and b.pdisp==-1 for b in cls.bases))
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x55320c,4))[0],0x55c620)
        col=struct.unpack('<5I',image.bytes_at_va(0x55c620,20));self.assertEqual(col[1],0)
        self.assertEqual(image.bytes_at_va(col[3]+8,64).split(b'\0')[0],b'.?AVUIMultiState@@')
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x553210+40*4,4))[0],0x478570)
        self.assertEqual(struct.unpack('<4I',image.bytes_at_va(0x553210+34*4,16)),(0x478260,0x478280,0x4782a0,0x4782c0))
        self.assertIn((0x47822f,'mov','dword ptr [esi], 0x553210'),[(i.address,i.mnemonic,i.op_str) for i in decoded(0x4781f0,112)])
        # Actual allocation argument and transfer of its returned primary pointer.
        allocation={i.address:(i.mnemonic,i.op_str)for i in decoded(0x46a920,0x46c2de-0x46a920)}
        for va,op in [(0x46c2af,('push','0x1f8')),(0x46c2b4,('call','0x4a3010')),
                      (0x46c2d3,('mov','ecx, eax')),(0x46c2d5,('call','0x4781f0'))]:
            self.assertEqual(allocation[va],op)
        compiler=Path(os.environ['VC6_ROOT']);linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-multi-draw-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                path=output/(name+'.obj')
                includes=[] if name in ('candidate','aggregate') else ['--include',str(ROOT/'src/reconstructed'),'--include',str(ROOT/'src/krusty2')]
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),'--vc6-root',str(compiler),'--profile','vc6_o2_mt',*includes,*(['--extra','/Od'] if fixture else [])],cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return path,CoffObject(path)
            candidate_path,candidate=compile_source(SOURCE,'candidate')
            _,aggregate=compile_source(ROOT/'samples/ui/GameUiNearMisses.cpp','aggregate')
            self.assertEqual(signature(candidate,SYMBOL),signature(aggregate,SYMBOL))
            result=compare(image,candidate,SYMBOL,0x478570,672,SOURCE.with_suffix('.bindings.json'))
            self.assertEqual((result['candidate_size'],result['strict_match']['matching_positions'],result['strict_exact']),(682,139,False))
            self.assertEqual(hashlib.sha256(signature(candidate,SYMBOL)[0]).hexdigest(),'ebdb222ce29b0364f742278e72d98b1e75bd78aa9aa85dbdfcf1a8dcfb565c9b')
            self.assertEqual([(s.name,s.raw_size,s.characteristics) for s in candidate.sections if not s.name.startswith('.debug')],[('.drectve',40,0x100a00),('.text',688,0x60501020)])
            self.assertEqual([s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0],[SYMBOL])
            expected=set(json.loads(SOURCE.with_suffix('.bindings.json').read_text()))-{SYMBOL}|{'__fltused'}
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},expected)
            pin=json.loads((ROOT/'tests/fixtures/multi_state_draw_aggregate.json').read_text());view=complete_object(aggregate,True)
            self.assertEqual((len(view),sum(len(r[-1])for r in view)),(733,3912))
            self.assertEqual(hashlib.sha256(json.dumps(view,separators=(',',':')).encode()).hexdigest(),pin['sha256'])
            table=aggregate.find_symbol('??_7UIMultiState@@6B@')
            routes={r.virtual_address-table.value:aggregate.symbol_by_index[r.symbol_index].name for r in aggregate.relocations if r.section_number==table.section_number}
            self.assertEqual(routes[40*4],SYMBOL)
            paths=[candidate_path];strict=0;canonical={}
            ui_source=(ROOT/'src/reconstructed/GameUi.cpp').read_text()
            run_type=extract(ui_source,'struct UnknownGameUiTextRun {')+';\n'
            for name,(unit,header,functions) in GROUPS.items():
                source=(ROOT/f'src/reconstructed/{unit}.cpp').read_text()
                if header is None:
                    header=source[source.index('#include'):source.index('// The four vector constants')]+run_type
                    header+='CameraRect* g_ControlDrawClipRect;CameraRect g_ControlDrawRect;int g_ControlDrawResult;SIZE g_AlignedTextSize,g_ControlTextSize;int g_AlignedTextX,g_AlignedTextY,g_ControlTextX,g_ControlTextY,g_TextRunIndex,g_TextRunX;unsigned int g_TextRunColor;\n'
                helper_file=output/(name+'.cpp');helper_file.write_text(header+'\n'.join(extract(source,sig) for sig,_,_ in functions)+'\n')
                path,helper=compile_source(helper_file,name);paths.append(path)
                _,provider=compile_source(ROOT/f'src/reconstructed/{unit}.cpp',unit);canonical[unit]=provider
                for source_signature,va,size in functions:
                    cls,method=source_signature.split(' ')[-1].rstrip('(').split('::');symbol='?'+method+'@'+cls+'@@'
                    for obj in (helper,provider):
                        proof=compare(image,obj,symbol,va,size,ROOT/f'src/reconstructed/{unit}.bindings.json')
                        self.assertTrue(proof['strict_exact'],proof);strict+=1
                    self.assertEqual(signature(helper,symbol),signature(provider,symbol))
            for symbol,va,size,site in [('??0UIMultiState@@',0x4781f0,112,None),('?SetStateEntry@UIMultiState@@',0x4789f0,92,None),('?GetStateTexture@UIMultiState@@',0x478810,66,None),('?RenderPre3D@UIControl@@',0x471140,83,0x47117b),('?RenderPost3D@UIControl@@',0x4711a0,146,0x4711de)]:
                proof=compare(image,canonical['GameUi'],symbol,va,size,ROOT/'src/reconstructed/GameUi.bindings.json')
                self.assertTrue(proof['strict_exact'],proof);strict+=1
                if site:self.assertIn((site,'call','dword ptr [eax + 0xa0]'),[(i.address,i.mnemonic,i.op_str)for i in decoded(va,size)])
            layout=output/'layout.cpp'
            text='#include "GameUi.h"\n#include "UIDialog.h"\n#include "BackgroundImage.h"\n#include "PCTextureMap.h"\n#include <stddef.h>\n'+run_type
            for cls,fields in LAYOUT.items():
                for member,value in fields.items():
                    if member=='renderContext':
                        text+=f'class Probe_{cls}:public {cls} {{public:static void Check(){{typedef char Field[(offsetof(Probe_{cls},renderContext)=={value})?1:-1];}}}};\n'
                    else:
                        text+=f'typedef char Check_{cls}_{member}[(offsetof({cls},{member})=={value})?1:-1];\n'
            for cls,size in [('UIMultiState',0x1f8),('UIDialog',0x7f58),('UIAnim',0xf8),('UIFrame',0x24),('UnknownGameUiState',32),('UnknownGameUiTextRun',24),('UnknownBackgroundRegion',64)]:text+=f'typedef char Size_{cls}[(sizeof({cls})=={size})?1:-1];\n'
            text+='typedef char Method[(sizeof(int(UIMultiState::*)())==4)?1:-1];\n';layout.write_text(text);compile_source(layout,'layout')
            spec=json.loads((ROOT/'config/game_build.json').read_text())
            built=game_compile(SOURCE,output/'game-command',compiler,[ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(built['compiled'],built)
            self.assertEqual(complete_object(candidate),complete_object(CoffObject(Path(built['object']))))
            fixture_path,_=compile_source(ROOT/'tests/fixtures/multi_state_draw_compare.cpp','fixture',True);paths.append(fixture_path)
            executable=output/'draw.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',*[str(p) for p in paths],'kernel32.lib','user32.lib','gdi32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image_path)],cwd=ROOT,capture_output=True,text=True,timeout=240)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('MultiStateDraw: 16181 actual-helper scenarios match; reached 35 callback sites',result.stdout)
            self.assertEqual(strict,37)
            print(f'MultiStateDraw: {strict} strict;733 sections/3912 relocations; '+result.stdout.strip())


if __name__=='__main__':unittest.main()
