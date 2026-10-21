"""Complete edit-box input with actual strict UI/sound helpers and bounded OS contracts."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare
from tools.build_game import compile_source as game_compile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/ui/EditBoxOnChar.cpp'
SYMBOL = '?OnChar@UIEditBox@@UAEHH@Z'
HELPERS = [('GameUi','?OnChar@UIControl@@',0x472320,166),
           ('GameUi','?FilterCharacter@UIEditBox@@',0x473fc0,160),
           ('GameUi','?GetName@UIControl@@',0x470df0,7),
           ('GameUi','?RequestRedraw@UIControl@@',0x470640,21),
           ('GameObject','?OnChar@GameObject@@',0x469c00,60),
           ('PCAudio','??0UnknownSoundNotifier@@',0x4bb720,26),
           ('PCAudio','?SetVolume@Sound@@',0x4bcbe0,185),
           ('PCAudio','?PlayWithOptions@Sound@@',0x4bc6b0,656),
           ('PCAudio','?IsPlaying@Sound@@',0x4bca80,172)]
DIRECT = {0x474163:0x472320,0x47419d:0x4bcbe0,0x4741ac:0x4bc6b0,
          0x47422d:0x536d00,0x474269:0x536d00,0x4742a1:0x470df0,
          0x474315:0x473fc0,0x474397:0x4bcbe0,0x4743a8:0x4bc6b0,
          0x474499:0x470df0,0x474507:0x536d00}
INDIRECT = {0x4741f6:'dword ptr [0x5502f0]',0x474209:'dword ptr [0x5501bc]',
            0x4742c9:'dword ptr [eax + 0x74]',0x4742f1:'dword ptr [0x5501bc]',
            0x4743d1:'dword ptr [edx + 0x44]',0x4743ff:'edi',0x474421:'ebp',
            0x474445:'ebp',0x474457:'edi',0x47446d:'dword ptr [edx + 0x68]',
            0x474475:'dword ptr [edx + 0xc8]',0x4744c1:'dword ptr [eax + 0x74]'}


def extract(text, signature):
    start = text.index(signature)
    end = text.index('{', start)+1
    depth = 1
    while depth:
        depth += (text[end]=='{')-(text[end]=='}')
        end += 1
    return text[start:end]


def signature(obj, query):
    entry = obj.find_symbol(query)
    data, size, relocs = obj.symbol_extent(entry)
    result = []
    for r in relocs:
        s = obj.symbol_by_index[r.symbol_index]
        key = s.name
        if s.section_number == entry.section_number:
            key = ('self', s.value-entry.value)
        elif s.name.startswith('??_C@') and s.section_number>0:
            section = obj.section(s.section_number)
            string = obj.data[section.raw_ptr+s.value:section.raw_ptr+section.raw_size]
            if string.rstrip(b'\0').endswith((b'/helpers.cpp',b'\\helpers.cpp',b'/PCAudio.cpp',b'\\PCAudio.cpp')):
                key = 'verified __FILE__ argument'
        result.append((r.virtual_address-entry.value,r.type,key))
    return data,size,result


def complete_object(obj):
    # The retained portable pin normalizes only the three known __FILE__ paths.
    # The original same-path extraction comparison required no exceptions.
    files = {}
    for symbol in obj.symbols:
        if symbol.section_number>0 and symbol.name.startswith('??_C@'):
            section = obj.section(symbol.section_number)
            data = obj.data[section.raw_ptr+symbol.value:section.raw_ptr+section.raw_size]
            path = data.rstrip(b'\0').decode('latin1').replace('\\','/')
            for tail in ('src/reconstructed/GameUi.cpp','src/reconstructed/ContainerList.h','samples/ui/GameUiNearMisses.cpp'):
                if path.endswith(tail) and 'samples/' in path:
                    files[symbol.section_number] = ('filename',tail)
    keys = {s.index:files.get(s.index,tuple(sorted(t.name for t in obj.symbols if t.section_number==s.index and t.storage_class==2)) or ('section',s.index)) for s in obj.sections}
    def target(symbol):
        return (keys[symbol.section_number],symbol.value) if symbol.section_number>0 else ('external',symbol.name,symbol.value,symbol.section_number)
    rows=[]
    for section in obj.sections:
        if section.name.startswith('.debug'):
            continue
        data = obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        if section.index in files:
            data = ('filename:'+files[section.index][1]).encode()+b'\0'
        rows.append((keys[section.index],section.name,section.characteristics,hashlib.sha256(data).hexdigest(),
                     [(r.virtual_address,r.type,target(obj.symbol_by_index[r.symbol_index])) for r in obj.relocations if r.section_number==section.index]))
    return rows


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class EditBoxOnCharBehaviorTests(unittest.TestCase):
    def test_complete_handler_actual_helpers_and_platform_contracts(self):
        import capstone
        image_path=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(image_path),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image=PEImage(image_path)
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        code=list(decoder.disasm(image.bytes_at_va(0x474150,0x3ed),0x474150))
        self.assertEqual(sum(i.size for i in code),0x3ed)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],[(0x4742e3,b'\xc2\x04\0'),(0x4744d1,b'\xc2\x04\0')])
        self.assertEqual((code[-1].address,code[-1].op_str),(0x474538,'0x4743ad'))
        self.assertEqual({i.address:i.operands[0].imm for i in code if i.mnemonic=='call' and i.operands[0].type==capstone.x86.X86_OP_IMM},DIRECT)
        self.assertEqual({i.address:i.op_str for i in code if i.mnemonic=='call' and i.operands[0].type!=capstone.x86.X86_OP_IMM},INDIRECT)
        branches={i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):
                if i.operands[0].type==capstone.x86.X86_OP_IMM:self.assertIn(i.operands[0].imm,branches)
                else:self.assertEqual((i.address,i.op_str),(0x4741c8,'dword ptr [ecx*4 + 0x474540]'))
        self.assertEqual(image.bytes_at_va(0x47453d,3),b'\x8d\x49\0')
        self.assertEqual(struct.unpack('<4I',image.bytes_at_va(0x474540,16)),(0x4741cf,0x4743ad,0x474290,0x4742e6))
        self.assertEqual(list(image.bytes_at_va(0x474550,20)),[0,1,3,3,3,2,3,3,3,3,3,3,3,3,3,3,3,3,3,1])
        self.assertEqual(image.bytes_at_va(0x474564,12),b'\x90'*12)
        self.assertFalse(any('fs:' in i.op_str for i in code))
        classes={c.name:c for c in parse_rtti(image)}
        self.assertEqual([(r.object_offset,r.vtables) for r in classes['UIEditBox'].vtable_records],[(0,[0x552de8])])
        col=struct.unpack('<I',image.bytes_at_va(0x552de4,4))[0]
        words=struct.unpack('<5I',image.bytes_at_va(col,20))
        self.assertEqual((col,words[1]),(0x55c4c0,0))
        self.assertEqual(image.bytes_at_va(words[3]+8,32).split(b'\0')[0],b'.?AVUIEditBox@@')
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x552de8+20*4,4))[0],0x474150)
        imports={};rva=image.directories[1][0]
        while rva:
            oft,ts,fc,name,ft=struct.unpack('<5I',image.bytes_at_va(image.image_base+rva,20))
            if not any((oft,ts,fc,name,ft)):break
            index=0
            while True:
                thunk=struct.unpack('<I',image.bytes_at_va(image.image_base+oft+index*4,4))[0]
                if not thunk:break
                if not thunk&0x80000000:imports[image.read_c_string_rva(thunk+2)]=(image.image_base+ft+4*index,image.read_c_string_rva(name).lower())
                index+=1
            rva+=20
        for name,address,dll in [('CharPrevA',0x5502f0,'user32.dll'),('IsDBCSLeadByte',0x5501bc,'kernel32.dll'),('SelectObject',0x550078,'gdi32.dll'),('GetTextExtentPoint32A',0x550084,'gdi32.dll')]:
            self.assertEqual(imports[name],(address,dll))
        filter_code=list(decoder.disasm(image.bytes_at_va(0x473fc0,160),0x473fc0))
        self.assertEqual([(i.address,i.op_str) for i in filter_code if i.mnemonic=='call'],[(a,'0x535f10') for a in (0x47400e,0x47402f,0x47404c)])
        compiler=Path(os.environ['VC6_ROOT']);linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-edit-box-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                obj=output/(name+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),'--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/reconstructed'),'--include',str(ROOT/'src/krusty2'),*(['--extra','/Od'] if fixture else [])],cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return obj,CoffObject(obj)
            candidate_path,candidate=compile_source(ROOT/SOURCE,'candidate')
            fixture_path,_=compile_source(ROOT/'tests/fixtures/edit_box_on_char_compare.cpp','fixture',True)
            _,aggregate=compile_source(ROOT/'samples/ui/GameUiNearMisses.cpp','aggregate')
            self.assertEqual(signature(candidate,SYMBOL),signature(aggregate,SYMBOL))
            baseline=json.loads((ROOT/'tests/fixtures/edit_box_on_char_aggregate.json').read_text())
            view=complete_object(aggregate)
            self.assertEqual((len(view),sum(len(row[-1]) for row in view)),(733,3912))
            self.assertEqual(hashlib.sha256(json.dumps(view,separators=(',',':')).encode()).hexdigest(),baseline['sha256'])
            entry=candidate.find_symbol(SYMBOL)
            self.assertEqual((entry.function_size,entry.extent_source),(1044,'coff_function_aux'))
            report=compare(image,candidate,SYMBOL,0x474150,1044,(ROOT/SOURCE).with_suffix('.bindings.json'))
            self.assertFalse(report['strict_exact']);self.assertEqual(report['strict_match']['matching_positions'],1018)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==32},{SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},
                {'?OnChar@UIControl@@UAEHH@Z','?GetName@UIControl@@QAEPADXZ','?FilterCharacter@UIEditBox@@QAEDD@Z','?SetVolume@Sound@@QAEHJH@Z','?PlayWithOptions@Sound@@QAEHHKH@Z','_memmove','__imp__CharPrevA@8','__imp__IsDBCSLeadByte@4','__imp__SelectObject@8','__imp__GetTextExtentPoint32A@16'})
            self.assertEqual([(s.name,s.raw_size) for s in candidate.sections if not s.name.startswith('.debug')],[('.drectve',82),('.text',1056)])
            self.assertFalse(any(s.name.startswith(('.CRT','.data','.bss','.xdata')) for s in candidate.sections))
            table=aggregate.find_symbol('??_7UIEditBox@@6B@')
            routes={r.virtual_address-table.value:aggregate.symbol_by_index[r.symbol_index].name for r in aggregate.relocations if r.section_number==table.section_number}
            self.assertEqual(routes[20*4],SYMBOL)
            ui=(ROOT/'src/reconstructed/GameUi.cpp').read_text();audio=(ROOT/'src/reconstructed/PCAudio.cpp').read_text();game=(ROOT/'src/reconstructed/GameObject.cpp').read_text()
            helpers='#include <string.h>\n#include "GameUi.h"\n#include "DialogProc.h"\n#include "PCAudio.h"\n#include "SoundInterface.h"\n#include "TrackGame.h"\n#include "DebugAlloc.h"\n#define SoundSystem() ((PCSoundInterface*)g_TrackGame->soundInterface)\n'
            for sig in ['int UIControl::OnChar(','char UIEditBox::FilterCharacter(','char* UIControl::GetName(','void UIControl::RequestRedraw(']:helpers+=extract(ui,sig)+'\n'
            helpers+=extract(game,'int GameObject::OnChar(')+'\n'+extract(audio,'UnknownSoundNotifier::UnknownSoundNotifier(')+'\n'
            for sig in ['int Sound::SetVolume(','int Sound::PlayWithOptions(','int Sound::IsPlaying(']:helpers+=extract(audio,sig)+'\n'
            helper_file=output/'helpers.cpp';helper_file.write_text(helpers)
            helper_path,helper=compile_source(helper_file,'helpers')
            canonical={}
            for name in ('GameUi','PCAudio','GameObject'):
                _,canonical[name]=compile_source(ROOT/f'src/reconstructed/{name}.cpp',name)
            strict=0
            for name,symbol,va,size in HELPERS:
                for obj in (helper,canonical[name]):
                    self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1],size)
                    result=compare(image,obj,symbol,va,size,ROOT/f'src/reconstructed/{name}.bindings.json')
                    self.assertTrue(result['strict_exact'],result);strict+=1
                self.assertEqual(signature(helper,symbol),signature(canonical[name],symbol))
            # The complete real PlayWithOptions body includes unvisited streaming
            # branches. Those dependencies fail closed; none are simulated as success.
            fail=output/'unvisited.cpp'
            fail.write_text('#include "PCAudio.h"\n#include "DebugAlloc.h"\nextern "C" void Unexpected();\n'
                'UnknownSoundNotifier::~UnknownSoundNotifier(){Unexpected();}\n'
                'int UnknownSoundNotifier::StartRefillThread(Sound*){Unexpected();return 0;}\n'
                'int Sound::Play(){Unexpected();return 0;}\n'
                'void* operator new(unsigned int,const char*,int){Unexpected();return 0;}\n'
                'void operator delete(void*,const char*,int){Unexpected();}\n'
                'void UnknownPCAudioObject::QueueSoundLoad(Sound*){Unexpected();}\n')
            fail_path,_=compile_source(fail,'unvisited')
            layout=output/'layout.cpp'
            layout.write_text('#include "GameUi.h"\n#include "UIDialog.h"\n#include "GUIManager.h"\n#include "PCAudio.h"\n#include "PCRenderTarget.h"\n#include <stddef.h>\n'
                'typedef char Layout[(sizeof(UIEditBox)==0x230 && sizeof(Sound)==0x1f8 && sizeof(int(UIEditBox::*)(int))==4 && '
                'offsetof(UIEditBox,textLength)==0x1ec && offsetof(UIEditBox,textCapacity)==0x1f0 && '
                'offsetof(UIEditBox,leadBytePending)==0x228 && offsetof(UIEditBox,dropTrailByte)==0x22c && '
                'offsetof(UIControl,controlText)==0xc0 && offsetof(UIControl,controlTextLength)==0xd0 && '
                'offsetof(UIDialog,guiUser)==0x34 && offsetof(UIDialog,dialogFont)==0xd8 && '
                'offsetof(PCRenderTarget,renderSurface)==0x48 && offsetof(Sound,dsBuffer)==0xc)?1:-1];\n')
            compile_source(layout,'layout')
            spec=json.loads((ROOT/'config/game_build.json').read_text())
            built=game_compile(ROOT/SOURCE,output/'game-command',compiler,[ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(built['compiled'],built)
            self.assertEqual(complete_object(candidate),complete_object(CoffObject(Path(built['object']))))
            executable=output/'edit.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',str(candidate_path),str(fixture_path),str(helper_path),str(fail_path),'kernel32.lib','user32.lib','gdi32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image_path)],capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('EditBoxOnChar: 20962 actual-helper, text, callback and fault scenarios match',result.stdout)
            print(f'{strict} strict helper checks; 733 sections/3912 relocations preserved; '+result.stdout.strip())


if __name__=='__main__':
    unittest.main()
