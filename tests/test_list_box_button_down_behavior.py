"""Complete list-box mouse selection with real row heights and Windows hit testing."""
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
SYMBOL='?ButtonDown@UIListBox@@UAEHHH@Z'
DIRECT={0x477f1b:0x4769a0,0x477f58:0x4769a0,0x477f7c:0x476a60,0x477fc2:0x4721b0,0x477fd8:0x4721b0}
@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class ListBoxButtonDownBehaviorTests(unittest.TestCase):
    def test_complete_list_button_and_actual_row_helper(self):
        import capstone
        reference=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference),'31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image=PEImage(reference);decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        code=list(decoder.disasm(image.bytes_at_va(0x477e90,342),0x477e90))
        self.assertEqual(sum(i.size for i in code),342)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],[(a,b'\xc2\x08\0') for a in (0x477fa4,0x477fcd,0x477fe3)])
        direct={i.address:i.operands[0].imm for i in code if i.mnemonic=='call' and i.operands[0].type==capstone.x86.X86_OP_IMM}
        indirect={i.address:i.op_str for i in code if i.mnemonic=='call' and i.operands[0].type!=capstone.x86.X86_OP_IMM}
        self.assertEqual(direct,DIRECT);self.assertEqual(indirect,{0x477f36:'dword ptr [0x5502d4]',0x477f8a:'dword ptr [edx + 0x108]'})
        self.assertFalse(any('fs:' in i.op_str for i in code));boundaries={i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):self.assertIn(i.operands[0].imm,boundaries)
        self.assertEqual(image.bytes_at_va(0x477fe6,10),b'\x90'*10)
        classes={c.name:c for c in parse_rtti(image)}
        self.assertEqual([(r.object_offset,r.vtables) for r in classes['UIListBox'].vtable_records],[(0,[0x553100])])
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x553100+55*4,4))[0],0x477e90)
        self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x553100+66*4,4))[0],0x477d30)
        # Associate the absolute IAT slot with its actual import, not its label.
        imports=image.directories[1][0];found=[]
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
        self.assertEqual([(a,n.lower()) for a,n in found],[(0x5502d4,'user32.dll')])
        compiler=Path(os.environ['VC6_ROOT']);linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-list-button-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                obj=output/(name+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),'--vc6-root',str(compiler),
                    '--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed'),*(['--extra','/Od'] if fixture else [])],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return obj,CoffObject(obj)
            candidate_path,candidate=compile_source(ROOT/'samples/ui/ListBoxButtonDown.cpp','candidate')
            fixture_path,_=compile_source(ROOT/'tests/fixtures/list_box_button_down_compare.cpp','fixture',True)
            _,aggregate=compile_source(ROOT/'samples/ui/GameUiNearMisses.cpp','aggregate')
            def fingerprint(obj,symbol):
                s=obj.find_symbol(symbol);body,size,relocations=obj.symbol_extent(s)
                return body,size,[(r.virtual_address-s.value,r.type,obj.symbol_by_index[r.symbol_index].name) for r in relocations]
            self.assertEqual(candidate.symbol_extent(candidate.find_symbol(SYMBOL))[1],342)
            self.assertEqual(fingerprint(candidate,SYMBOL),fingerprint(aggregate,SYMBOL))
            report=compare(image,candidate,SYMBOL,0x477e90,342,ROOT/'samples/ui/ListBoxButtonDown.bindings.json')
            self.assertFalse(report['strict_exact']);self.assertEqual(report['strict_match']['matching_positions'],306)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==0x20},{SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},
                {'?GetRowHeight@UIListBox@@QAEHH@Z','?SelectRow@UIListBox@@QAEHH@Z','?ButtonDown@UIControl@@UAEHHH@Z','__imp__PtInRect@12'})
            self.assertFalse(any(s.name.startswith(('.CRT','.text$x','.xdata')) for s in candidate.sections))
            def extract(text,signature):
                start=text.index(signature);end=text.index('{',start)+1;depth=1
                while depth:depth+=(text[end]=='{')-(text[end]=='}');end+=1
                return text[start:end]
            ui=(ROOT/'src/reconstructed/GameUi.cpp').read_text();helper_file=output/'helpers.cpp'
            helper_file.write_text('#include "GameUi.h"\nclass ActualList:public UIListBox {public:int Height(int);};\n'+
                extract(ui,'int UIListBox::GetRowHeight(').replace('UIListBox::GetRowHeight','ActualList::Height')+'\n')
            helper_path,helper=compile_source(helper_file,'helpers');_,canonical=compile_source(ROOT/'src/reconstructed/GameUi.cpp','gameui')
            for original,va,size in [('?GetRowHeight@UIListBox@@QAEHH@Z',0x4769a0,49),('?SelectRow@UIListBox@@QAEHH@Z',0x476a60,110),('?ButtonDown@UIControl@@UAEHHH@Z',0x4721b0,158),('?ReportClick@UIListBox@@UAEXPAH@Z',0x477d30,301)]:
                self.assertEqual(canonical.symbol_extent(canonical.find_symbol(original))[1],size)
                self.assertTrue(compare(image,canonical,original,va,size,ROOT/'src/reconstructed/GameUi.bindings.json')['strict_exact'])
            self.assertEqual(helper.symbol_extent(helper.find_symbol('?Height@ActualList@@QAEHH@Z'))[1],49)
            self.assertTrue(compare(image,helper,'?Height@ActualList@@QAEHH@Z',0x4769a0,49,ROOT/'src/reconstructed/GameUi.bindings.json')['strict_exact'])
            table=aggregate.find_symbol('??_7UIListBox@@6B@')
            routes={r.virtual_address-table.value:aggregate.symbol_by_index[r.symbol_index].name for r in aggregate.relocations if r.section_number==table.section_number}
            self.assertEqual(routes[55*4],SYMBOL);self.assertEqual(routes[66*4],'?ReportClick@UIListBox@@UAEXPAH@Z')
            layout=output/'layout.cpp';layout.write_text('#include "GameUi.h"\n#include "UIDialog.h"\n#include "GUIManager.h"\n#include <stddef.h>\n'
                'typedef char Layout[(sizeof(UIListBox)==0x250 && sizeof(UnknownGameUiListRow)==56 && '
                'offsetof(UIControl,ownerDialog)==0xb8 && offsetof(UIListBox,firstVisibleRow)==0x1f0 && '
                'offsetof(UIListBox,visibleRowCount)==0x200 && offsetof(UIListBox,rowTable)==0x214 && '
                'offsetof(UIListBox,selectable)==0x224 && offsetof(UIListBox,allowWScroll)==0x228 && '
                'offsetof(UIListBox,dragStartX)==0x22c && offsetof(UIDialog,guiUser)==0x34)?1:-1];\n')
            compile_source(layout,'layout');executable=output/'list.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',
                str(candidate_path),str(fixture_path),str(helper_path),'kernel32.lib','user32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(reference)],capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('ListBoxButtonDown: 8707 row, real-helper, API, callback and fault scenarios match',result.stdout);print(result.stdout.strip())

if __name__=='__main__':unittest.main()
