"""Complete edit-box Draw with strict source helpers and finite platform contracts."""
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

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/ui/EditBoxDraw.cpp'
SYMBOL = '?Draw@UIEditBox@@UAEHXZ'
DIRECT = {0x4738e0:0x473da0,0x47392d:0x404480,0x473a69:0x46ed70,
          0x473abc:0x46f550,0x473b7d:0x46eeb0,0x473bb7:0x404c80,
          0x473bc9:0x533076,0x473bd1:0x533070,0x473bfc:0x53306a,0x473c13:0x533064}
INDIRECT = {0x4738ab:'dword ptr [eax + 0xf4]',0x473962:'dword ptr [edx + 0xc]',
            0x473989:'dword ptr [eax + 0xf4]',0x473b5b:'dword ptr [0x55005c]',
            0x473b6c:'dword ptr [0x5502ec]',0x473c47:'dword ptr [edx + 0xc8]'}
GROUPS = {
    'ui_helpers':('GameUi',None,[
        ('int UIControl::GetWidth(',0x470410,9),
        ('void UIControl::RequestRedraw(',0x470640,21),
        ('void UIEditBox::SetEditText(',0x473da0,333),
        ('int UIDialog::BeginControlDraw(',0x46ed70,316),
        ('void UIDialog::EndControlDraw(',0x46eeb0,75),
        ('void UIDialog::DrawAlignedText(',0x46f550,354),
        ('void UIDialog::DrawShadowText(',0x46f3c0,93)]),
    'background_helpers':('BackgroundImage',
        '#include "BackgroundImage.h"\n#define Target() ((PCRenderTarget*)renderContext)\n'
        'CameraRect g_DrawImageClipRect,g_DrawImageSourceRect;\n',[
        ('int BackgroundImage::DrawImage(',0x404480,639),
        ('int BackgroundImage::ReleaseDc(',0x404c80,41),
        ('void BackgroundImage::MarkRegionPending(',0x404cb0,23),
        ('void BackgroundImage::SetRegionRect(',0x404240,158)]),
    'texture_helpers':('PCTextureMap','#include "PCTextureMap.h"\n',[
        ('int PCTextureMap::BlitTo(',0x4c7b00,54)]),
    'alloc_helpers':('MemTag','#include <malloc.h>\nextern int g_UnclaimedBytes;\n'
        '#include "MemTag.h"\n#include "DebugAlloc.h"\n',[
        ('void* DebugMalloc(',0x4a2e20,57)]),
}


def complete_object(obj, portable=False):
    """Include all public offsets, storage flags, bytes and relocation destinations."""
    files = {}
    if portable:
        allowed = {'src/reconstructed/GameUi.cpp','src/reconstructed/ContainerList.h',
                   'samples/ui/GameUiNearMisses.cpp'}
        for symbol in obj.symbols:
            if symbol.name.startswith('??_C@') and symbol.section_number>0:
                section=obj.section(symbol.section_number)
                raw=obj.data[section.raw_ptr+symbol.value:section.raw_ptr+section.raw_size]
                text,sep,tail=raw.partition(b'\0')
                try:
                    relative=Path(os.path.normpath(text.decode('latin1').replace('\\','/'))).relative_to(ROOT).as_posix()
                except ValueError:
                    continue
                if sep and relative in allowed:
                    files[section.index]=(relative,b'filename:'+relative.encode()+sep+tail)
    def name(symbol):
        return 'filename:'+files[symbol.section_number][0] if symbol.name.startswith('??_C@') and symbol.section_number in files else symbol.name
    keys={};anonymous={}
    sections=[s for s in obj.sections if not s.name.startswith('.debug')]
    for section in sections:
        labels=tuple(sorted((name(s),s.value) for s in obj.symbols if s.section_number==section.index and (s.storage_class==2 or s.type==32)))
        if labels: keys[section.index]=('named',labels)
        else:
            n=anonymous.get(section.name,0);anonymous[section.name]=n+1
            keys[section.index]=('anonymous',section.name,n)
    assert len(set(keys.values()))==len(keys)
    def target(s):
        return ('defined',keys[s.section_number],s.value) if s.section_number>0 else ('external',s.name,s.value,s.section_number)
    rows=[]
    for section in sections:
        data=obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        if section.index in files:data=files[section.index][1]
        rows.append((keys[section.index],section.characteristics,data.hex(),
                     [(r.virtual_address,r.type,target(obj.symbol_by_index[r.symbol_index])) for r in obj.relocations if r.section_number==section.index]))
    return sorted(rows,key=lambda row:repr(row[0]))


def signature(obj, query):
    entry=obj.find_symbol(query);body,size,relocations=obj.symbol_extent(entry)
    result=[]
    for r in relocations:
        s=obj.symbol_by_index[r.symbol_index];key=s.name
        if s.section_number==entry.section_number:key=('self',s.value-entry.value)
        elif s.name.startswith('??_C@') and s.section_number>0:
            section=obj.section(s.section_number)
            raw=obj.data[section.raw_ptr+s.value:section.raw_ptr+section.raw_size]
            if raw.rstrip(b'\0').replace(b'\\',b'/').endswith((b'/ui_helpers.cpp',b'/GameUi.cpp')):
                key='verified GameUi __FILE__ argument'
        result.append((r.virtual_address-entry.value,r.type,key))
    return body,size,result


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class EditBoxDrawTests(unittest.TestCase):
    def test_complete_draw_actual_helpers_and_platform_contracts(self):
        import capstone
        image_path=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image=PEImage(image_path)
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        code=list(decoder.disasm(image.bytes_at_va(0x4738a0,973),0x4738a0))
        self.assertEqual(sum(i.size for i in code),973)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],[(0x473c59,b'\xc3'),(0x473c6c,b'\xc3')])
        self.assertEqual(image.bytes_at_va(0x473c6d,3),b'\x90'*3)
        self.assertEqual({i.address:i.operands[0].imm for i in code if i.mnemonic=='call' and i.operands[0].type==capstone.x86.X86_OP_IMM},DIRECT)
        self.assertEqual({i.address:i.op_str for i in code if i.mnemonic=='call' and i.operands[0].type!=capstone.x86.X86_OP_IMM},INDIRECT)
        addresses={i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):self.assertIn(i.operands[0].imm,addresses)
        self.assertFalse(any('fs:' in i.op_str for i in code))
        classes={c.name:c for c in parse_rtti(image)}
        self.assertEqual([(r.object_offset,r.vtables) for r in classes['UIEditBox'].vtable_records],[(0,[0x552de8])])
        words=struct.unpack('<5I',image.bytes_at_va(0x55c4c0,20))
        self.assertEqual(words[1],0)
        self.assertEqual(image.bytes_at_va(words[3]+8,32).split(b'\0')[0],b'.?AVUIEditBox@@')
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x552de4,4))[0],0x55c4c0)
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x552de8+40*4,4))[0],0x4738a0)
        ctor=list(decoder.disasm(image.bytes_at_va(0x473630,286),0x473630))
        self.assertIn((0x473675,'mov','dword ptr [esi], 0x552de8'),[(i.address,i.mnemonic,i.op_str) for i in ctor])
        imports={};rva=image.directories[1][0]
        while rva:
            oft,ts,fc,name,ft=struct.unpack('<5I',image.bytes_at_va(image.image_base+rva,20))
            if not any((oft,ts,fc,name,ft)):break
            n=0
            while True:
                thunk=struct.unpack('<I',image.bytes_at_va(image.image_base+oft+n*4,4))[0]
                if not thunk:break
                if not thunk&0x80000000:imports[image.read_c_string_rva(thunk+2)]=(image.image_base+ft+4*n,image.read_c_string_rva(name).lower())
                n+=1
            rva+=20
        for name,address,dll in [('GetStockObject',0x55005c,'gdi32.dll'),('FrameRect',0x5502ec,'user32.dll')]:
            self.assertEqual(imports[name],(address,dll))
        for name,va in [('ImmGetContext',0x533076),('ImmGetOpenStatus',0x533070),('ImmSetCompositionWindow',0x53306a),('ImmReleaseContext',0x533064)]:
            thunk=image.bytes_at_va(va,6);self.assertEqual(thunk[:2],b'\xff\x25')
            self.assertEqual(imports[name],(struct.unpack('<I',thunk[2:])[0],'imm32.dll'))
        compiler=Path(os.environ['VC6_ROOT']);linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-edit-draw-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                obj=output/(name+'.obj')
                # These two sources must also work in the calibration driver,
                # which does not supply the game's project include directories.
                includes=[] if name in ('candidate','aggregate') else ['--include',str(ROOT/'src/reconstructed'),'--include',str(ROOT/'src/krusty2')]
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),'--vc6-root',str(compiler),'--profile','vc6_o2_mt',*includes,*(['--extra','/Od'] if fixture else [])],cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return obj,CoffObject(obj)
            candidate_path,candidate=compile_source(ROOT/SOURCE,'candidate')
            fixture_path,_=compile_source(ROOT/'tests/fixtures/edit_box_draw_compare.cpp','fixture',True)
            _,aggregate=compile_source(ROOT/'samples/ui/GameUiNearMisses.cpp','aggregate')
            self.assertEqual(signature(candidate,SYMBOL),signature(aggregate,SYMBOL))
            report=compare(image,candidate,SYMBOL,0x4738a0,973,(ROOT/SOURCE).with_suffix('.bindings.json'))
            self.assertEqual(report['candidate_size'],975);self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'],206)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==32},{SYMBOL})
            self.assertEqual([(s.name,s.raw_size) for s in candidate.sections if not s.name.startswith('.debug')],
                             [('.drectve',82),('.text',976),('.bss',1)])
            empty=candidate.find_symbol('??_C@_00A@?$AA@')
            self.assertEqual((empty.value,candidate.section(empty.section_number).name),(0,'.bss'))
            self.assertFalse(any(s.name.startswith(('.CRT','.data','.xdata')) for s in candidate.sections))
            baseline=json.loads((ROOT/'tests/fixtures/edit_box_draw_aggregate.json').read_text())
            view=complete_object(aggregate,True)
            self.assertEqual((len(view),sum(len(row[-1]) for row in view)),(733,3912))
            self.assertEqual(hashlib.sha256(json.dumps(view,separators=(',',':')).encode()).hexdigest(),baseline['sha256'])
            table=aggregate.find_symbol('??_7UIEditBox@@6B@')
            routes={r.virtual_address-table.value:aggregate.symbol_by_index[r.symbol_index].name for r in aggregate.relocations if r.section_number==table.section_number}
            self.assertEqual(routes[40*4],SYMBOL)
            paths=[candidate_path,fixture_path];strict=0
            for name,(unit,header,functions) in GROUPS.items():
                source=(ROOT/f'src/reconstructed/{unit}.cpp').read_text()
                if header is None:
                    header=source[source.index('#include'):source.index('// The four vector constants')]
                    header+='\nCameraRect* g_ControlDrawClipRect;CameraRect g_ControlDrawRect;int g_ControlDrawResult;\nSIZE g_AlignedTextSize;int g_AlignedTextX,g_AlignedTextY;\n'
                helper_file=output/(name+'.cpp');helper_file.write_text(header+'\n'+'\n\n'.join(extract(source,s) for s,_,_ in functions)+'\n')
                path,helper=compile_source(helper_file,name);paths.append(path)
                _,canonical=compile_source(ROOT/f'src/reconstructed/{unit}.cpp',unit)
                bindings=json.loads((ROOT/f'src/reconstructed/{unit}.bindings.json').read_text())
                bindings['__FILE__:ui_helpers.cpp']='0x0056b738'
                binding_path=output/(name+'.bindings.json');binding_path.write_text(json.dumps(bindings))
                for source_signature,va,size in functions:
                    function=source_signature.split(' ')[-1].rstrip('(')
                    cls,method=function.split('::') if '::' in function else ('',function)
                    symbol='?'+method+('@'+cls if cls else '')+'@@'
                    for obj in (helper,canonical):
                        result=compare(image,obj,symbol,va,size,binding_path)
                        self.assertTrue(result['strict_exact'],result);strict+=1
                    self.assertEqual(signature(helper,symbol),signature(canonical,symbol))
                if unit=='GameUi':
                    self.assertTrue(compare(image,canonical,'??0UIEditBox@@',0x473630,286,binding_path)['strict_exact']);strict+=1
            layout=output/'layout.cpp'
            layout.write_text('#include "GameUi.h"\n#include "UIDialog.h"\n#include "GUIManager.h"\n#include "BackgroundImage.h"\n#include <stddef.h>\n'
                'typedef char Layout[(sizeof(UIEditBox)==0x230 && sizeof(int(UIEditBox::*)())==4 && '
                'offsetof(UIEditBox,textWidth)==0x1fc && offsetof(UIEditBox,caretRect)==0x214 && '
                'offsetof(UIControl,controlText)==0xc0 && offsetof(UIControl,controlTextLength)==0xd0 && '
                'offsetof(UIDialog,guiUser)==0x34 && offsetof(UIDialog,dialogBackground)==0x110 && '
                'sizeof(UIDialog)==0x7f58 && offsetof(UIDialog,drawClipRegion)==0x7f00 && '
                'offsetof(UIDialog,previousFont)==0x7f04 && '
                'offsetof(BackgroundImage,heldDcSurface)==0x6c && offsetof(BackgroundImage,regionTable)==0x50 && '
                'sizeof(UnknownBackgroundRegion)==64 && offsetof(PCRenderTarget,renderSurface)==0x48)?1:-1];\n')
            compile_source(layout,'layout')
            spec=json.loads((ROOT/'config/game_build.json').read_text())
            built=game_compile(ROOT/SOURCE,output/'game-command',compiler,[ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(built['compiled'],built)
            self.assertEqual(complete_object(candidate),complete_object(CoffObject(Path(built['object']))))
            executable=output/'draw.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',*[str(p) for p in paths],'kernel32.lib','user32.lib','gdi32.lib','imm32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image_path)],capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('EditBoxDraw: 14793 actual-helper and finite platform scenarios match',result.stdout)
            print(f'{strict} strict checks; 733 sections/3912 relocations preserved; '+result.stdout.strip())


if __name__=='__main__':
    unittest.main()
