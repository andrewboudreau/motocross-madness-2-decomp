"""Complete name-tag drawing and a shared existing scalar, with finite GDI contracts."""
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
from mcm2tool.coff_archive import get_archive_member
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES
from tools.build_game import compile_source as game_compile
from test_edit_box_draw import signature

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT/'samples/track/ChatNameTagDraw.cpp'
CANONICAL = ROOT/'src/reconstructed/TrackOverlay.cpp'
AGGREGATE = ROOT/'samples/track/TrackOverlayNearMisses.cpp'
SYMBOL = '?DrawNameTag@ChatOverlay@@QAEXPAXH@Z'
FLAG = '?s_UnknownGlobal68a444@@3HA'
OLD_FLAG = '_s_UnknownGlobal68a444'
DIRECT = {0x51e439:0x534aaf, 0x51e568:0x534aaf, 0x51e5a1:0x534aaf}
INDIRECT = {0x51e40b:'dword ptr [0x550078]', 0x51e4a4:'dword ptr [0x5502e8]',
            0x51e4ba:'dword ptr [0x550084]', 0x51e6df:'dword ptr [0x550084]',
            0x51e713:'dword ptr [0x5502e8]', 0x51e7ab:'dword ptr [0x550078]'}


def object_view(obj, expose_flag=False, portable=False):
    """All nondebug contents; optional one evidenced flag visibility/name change."""
    files = {}
    if portable:
        allowed = {'src/reconstructed/TrackOverlay.cpp', 'samples/track/TrackOverlayNearMisses.cpp'}
        for symbol in obj.symbols:
            if symbol.name.startswith('??_C@') and symbol.section_number > 0:
                section = obj.section(symbol.section_number)
                raw = obj.data[section.raw_ptr+symbol.value:section.raw_ptr+section.raw_size]
                text, sep, tail = raw.partition(b'\0')
                try:
                    relative = Path(os.path.normpath(text.decode('latin1').replace('\\','/'))).relative_to(ROOT).as_posix()
                except ValueError:
                    continue
                if sep and relative in allowed:
                    files[section.index] = (relative, b'filename:'+relative.encode()+sep+tail)
    if expose_flag:
        flag = obj.find_symbol(OLD_FLAG)
        assert flag.storage_class == 3 and flag.value == 0x40
        section = obj.section(flag.section_number)
        assert (section.name, section.raw_size, section.characteristics) == ('.bss',72,0xc0400080)
    def name(symbol):
        if expose_flag and symbol.name == OLD_FLAG:
            return FLAG
        if symbol.name.startswith('??_C@') and symbol.section_number in files:
            return 'filename:'+files[symbol.section_number][0]
        return symbol.name
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    keys = {}; anonymous = {}
    for section in sections:
        labels = tuple(sorted((name(s),s.value) for s in obj.symbols
                              if s.section_number==section.index and
                              (s.storage_class==2 or s.type==32 or (expose_flag and s.name==OLD_FLAG))))
        if labels:
            keys[section.index] = ('named',labels)
        else:
            index = anonymous.get(section.name,0);anonymous[section.name]=index+1
            keys[section.index] = ('anonymous',section.name,index)
    assert len(set(keys.values())) == len(keys)
    def target(symbol):
        if symbol.section_number > 0:
            return ('defined',keys[symbol.section_number],symbol.value)
        return ('external',name(symbol),symbol.value,symbol.section_number)
    rows = []
    for section in sections:
        data = obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        if section.index in files:
            data = files[section.index][1]
        rows.append((keys[section.index],section.characteristics,data.hex(),
                     [(r.virtual_address,r.type,target(obj.symbol_by_index[r.symbol_index]))
                      for r in obj.relocations if r.section_number==section.index]))
    return sorted(rows,key=lambda row:repr(row[0]))


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class ChatNameTagTests(unittest.TestCase):
    def test_complete_method_shared_storage_crt_and_finite_gdi(self):
        import capstone
        image_path = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(image_path)
        decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        def decoded(va,size):
            return list(decoder.disasm(image.bytes_at_va(va,size),va))
        code = decoded(0x51e3f0,974)
        self.assertEqual(sum(i.size for i in code),974)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],[(0x51e7bb,b'\xc2\x08\0')])
        self.assertEqual(image.bytes_at_va(0x51e7be,2),b'\x90\x90')
        self.assertEqual({i.address:i.operands[0].imm for i in code if i.mnemonic=='call' and i.operands[0].type==capstone.x86.X86_OP_IMM},DIRECT)
        self.assertEqual({i.address:i.op_str for i in code if i.mnemonic=='call' and i.operands[0].type!=capstone.x86.X86_OP_IMM},INDIRECT)
        starts = {i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertIn(i.operands[0].imm,starts)
        self.assertFalse(any('fs:' in i.op_str for i in code))
        crt = decoded(0x534aaf,82)
        self.assertEqual(sum(i.size for i in crt),82)
        self.assertEqual({i.address:i.operands[0].imm for i in crt if i.mnemonic=='call'},
                         {0x534ad8:0x539359,0x534af5:0x539241})
        self.assertEqual((crt[-1].address,crt[-1].mnemonic),(0x534b00,'ret'))
        imports = {}
        offset=image.rva_to_offset(image.directories[1][0])
        while True:
            oft,ts,fc,name,ft=struct.unpack_from('<5I',image.data,offset)
            if not any((oft,ts,fc,name,ft)):break
            index=0
            while True:
                value=image.u32(image.rva_to_offset(oft or ft)+index*4)
                if not value:break
                if not value&0x80000000:
                    imports[image.image_base+ft+index*4]=(image.read_c_string_rva(name),image.read_c_string_at_offset(image.rva_to_offset(value)+2))
                index+=1
            offset+=20
        self.assertEqual({va:imports[va] for va in (0x550078,0x550084,0x5502e8)},
                         {0x550078:('GDI32.dll','SelectObject'),0x550084:('GDI32.dll','GetTextExtentPoint32A'),0x5502e8:('USER32.dll','DrawTextA')})
        classes = {c.name:c for c in parse_rtti(image)}
        for name,table,col in [('ChatOverlay',0x558854,0x55f838),('NameOverlay',0x55860c,0x55f690)]:
            cls=classes[name]
            self.assertEqual([(r.object_offset,r.vtables) for r in cls.vtable_records],[(0,[table])])
            self.assertTrue(any(b.name=='Overlay' and b.mdisp==0 and b.pdisp==-1 for b in cls.bases))
            self.assertEqual(struct.unpack('<I',image.bytes_at_va(table-4,4))[0],col)
            words=struct.unpack('<5I',image.bytes_at_va(col,20));self.assertEqual(words[1],0)
            self.assertEqual(image.bytes_at_va(words[3]+8,64).split(b'\0')[0],('.?AV'+name+'@@').encode())
        # Full entry decodes, then select the independently verified producer/caller sites.
        ins = {i.address:(i.mnemonic,i.op_str) for va,n in
               [(0x51cda0,282),(0x518d80,117),(0x518e30,242),(0x51d730,586),
                (0x4e07c0,1049),(0x51e910,308),(0x51e240,326),(0x51e3f0,974)] for i in decoded(va,n)}
        expected = {
            0x51cdfc:('mov','dword ptr [esi], 0x558854'),
            0x4e08e4:('push','0x3e0'),0x4e08eb:('call','0x4a3010'),
            0x4e0908:('mov','ecx, eax'),0x4e090a:('call','0x51cda0'),
            0x51d7b0:('lea','ebx, [esi + 0x2d8]'),0x51d7c6:('push','0x174'),
            0x51d7e5:('mov','ecx, eax'),0x51d7e7:('call','0x518d80'),
            0x51d815:('call','0x518e30'),0x51d81f:('mov','dword ptr [ebx], eax'),
            0x51d875:('call','0x518d80'),0x51d8ac:('call','0x518e30'),
            0x51d8b9:('mov','dword ptr [esi + edx*4 + 0x2d8], eax'),
            0x51e9d2:('call','0x51e3f0'),0x51e9ec:('call','0x51e3f0'),
            0x51e542:('mov','eax, dword ptr [0x68a444]'),
            0x51e57b:('mov','eax, dword ptr [edx + 0xc38]'),
            0x51e5e2:('mov','edx, dword ptr [ecx + 0xc34]'),
        }
        for address,instruction in expected.items():
            self.assertEqual(ins[address],instruction,hex(address))
        self.assertEqual([at for at,op in ins.items() if op==('call','0x51e3f0')],[0x51e9d2,0x51e9ec])
        # A DWORD in the loader-zeroed writable virtual tail, not a file-backed constant.
        section = next(s for s in image.sections if s.virtual_address<=0x28a444<s.rva_end)
        self.assertEqual((section.name,section.characteristics),('.data',0xc0000040))
        self.assertLessEqual(section.virtual_address+section.raw_size,0x28a444)
        self.assertLessEqual(0x28a448,section.virtual_address+section.virtual_size)
        compiler = Path(os.environ['VC6_ROOT'])
        linker = find_child_ci(find_vc6_bin(compiler),'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-name-tag-') as directory:
            output = Path(directory)
            def compile_source(source,name,fixture=False):
                path=output/(name+'.obj')
                command=[sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),
                         '--vc6-root',str(compiler),'--profile','vc6_o2_mt']
                if fixture:command+=['--include',str(ROOT/'src/reconstructed'),'--include',str(ROOT/'src/krusty2'),'--extra','/Od']
                result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return path,CoffObject(path)
            # Independent archive relocation names establish the two CRT boundary targets.
            crt_path=output/'sprintf.obj'
            crt_path.write_bytes(get_archive_member(find_child_ci(libraries,'libcmt.lib'),'build/intel/mt_obj/sprintf.obj'))
            crt_obj=CoffObject(crt_path);crt_symbol=crt_obj.find_symbol('_sprintf')
            body,size,relocations=crt_obj.symbol_extent(crt_symbol)
            self.assertEqual(size,82)
            self.assertEqual([(r.virtual_address-crt_symbol.value,r.type,crt_obj.symbol_by_index[r.symbol_index].name) for r in relocations],
                             [(42,0x14,'__output'),(71,0x14,'__flsbuf')])
            crt_bindings=output/'sprintf.bindings.json'
            crt_bindings.write_text(json.dumps({'__output':'0x00539359','__flsbuf':'0x00539241'}))
            self.assertTrue(compare(image,crt_obj,'_sprintf',0x534aaf,82,crt_bindings)['strict_exact'])
            candidate_path,candidate = compile_source(SOURCE,'candidate')
            _,canonical = compile_source(CANONICAL,'canonical')
            _,aggregate = compile_source(AGGREGATE,'aggregate')
            result=compare(image,candidate,SYMBOL,0x51e3f0,974,SOURCE.with_suffix('.bindings.json'))
            self.assertEqual((result['candidate_size'],result['strict_match']['matching_positions'],result['strict_exact']),(974,966,False))
            pin=json.loads((ROOT/'tests/fixtures/chat_name_tag_objects.json').read_text())
            for name,obj in [('canonical',canonical),('aggregate',aggregate)]:
                view=object_view(obj,portable=True);record=pin[name]
                self.assertEqual((len(view),sum(len(r[3]) for r in view)),(record['sections'],record['relocations']))
                self.assertEqual(hashlib.sha256(json.dumps(view,separators=(',',':')).encode()).hexdigest(),record['sha256'])
            # Aggregate retains its old private measurement view; standalone has one real import.
            self.assertEqual(aggregate.find_symbol(OLD_FLAG).storage_class,3)
            aggregate.find_symbol(OLD_FLAG).name=FLAG
            self.assertEqual(signature(candidate,SYMBOL),signature(aggregate,SYMBOL))
            aggregate.find_symbol(FLAG).name=OLD_FLAG
            flag=canonical.find_symbol(FLAG);self.assertEqual((flag.storage_class,flag.value),(2,0x40))
            flags=canonical.section(flag.section_number)
            self.assertEqual((flags.name,flags.raw_size,flags.raw_ptr,flags.characteristics),('.bss',72,0,0xc0400080))
            self.assertFalse(any(s.name==OLD_FLAG for s in canonical.symbols))
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},
                             {FLAG,'?g_TrackGame@@3PAVTrackGame@@A','_sprintf','__fltused',
                              '__imp__SelectObject@8','__imp__DrawTextA@20','__imp__GetTextExtentPoint32A@16'})
            self.assertFalse(any(s.name.startswith('.CRT') for s in candidate.sections))
            sections=[(s.name,s.raw_size,s.characteristics) for s in candidate.sections if not s.name.startswith('.debug')]
            self.assertEqual(sections,[('.drectve',40,0x100a00),('.text',976,0x60501020),
                                      ('.data',4,0xc0301040),('.bss',1,0xc0301080),
                                      ('.data',3,0xc0301040),('.data',3,0xc0301040),
                                      ('.data',5,0xc0301040),('.data',3,0xc0301040)])
            strict=0
            for case in CASES:
                if case['source']!='src/reconstructed/TrackOverlay.cpp':continue
                proof=compare(image,canonical,case['symbol'],int(case['target_va'],0),case['target_size'],ROOT/case['bindings'])
                self.assertTrue(proof['strict_exact'],proof);strict+=1
            for symbol,va,n in [('??0ChatOverlay@@',0x51cda0,282),('??0NameOverlay@@',0x518d80,117),
                                ('?Create@NameOverlay@@',0x518e30,242),('?RedrawNameTags@ChatOverlay@@',0x51e910,308),
                                ('?CycleNameTagShade@ChatOverlay@@',0x51e7c0,50)]:
                self.assertTrue(compare(image,canonical,symbol,va,n,CANONICAL.with_suffix('.bindings.json'))['strict_exact'])
            layout=output/'layout.cpp'
            text='#include "'+(ROOT/'src/reconstructed/TrackOverlay.h').as_posix()+'"\n#include "'+(ROOT/'src/reconstructed/TrackGame.h').as_posix()+'"\n#include <stddef.h>\n'
            fields={'ChatOverlay':{'nameTagFont':0x124,'racers':0x16c,'racerCount':0x198,'cueRacerName':0x1d4,'nameTags':0x2d8,'nameTagRects':0x30c},
                    'NameOverlay':{'sourceRect':0xf8,'sourceWidth':0x120,'drawWidth':0x124},
                    'UnknownChatRacer':{'driveForceScale':0x4a4,'name':0x5e0,'position':0x784}}
            for cls,members in fields.items():
                for member,value in members.items():text+=f'typedef char Offset_{cls}_{member}[offsetof({cls},{member})=={value}?1:-1];\n'
            for cls,n in [('ChatOverlay',0x3e0),('NameOverlay',0x174),('UnknownChatRacer',0x788),('UnknownTrackOverlayRect',16)]:text+=f'typedef char Size_{cls}[sizeof({cls})=={n}?1:-1];\n'
            text+='typedef char GameNames[offsetof(TrackGame,mode)+offsetof(TrackGameMode,showRiderNames)==0xc34?1:-1];\n'
            text+='typedef char GamePositions[offsetof(TrackGame,mode)+offsetof(TrackGameMode,showRiderPositions)==0xc38?1:-1];\n'
            text+='typedef char Method[sizeof(void(ChatOverlay::*)(void*,int))==4?1:-1];\n'
            layout.write_text(text);compile_source(layout,'layout')
            # Exact existing definition text; standalone validation storage, not a game replacement.
            definition='int s_UnknownGlobal68a444;'
            self.assertEqual(CANONICAL.read_text().count(definition),1)
            storage=output/'storage.cpp';storage.write_text(definition+'\n')
            storage_path,storage_obj=compile_source(storage,'storage')
            self.assertEqual({s.name for s in storage_obj.symbols if s.storage_class==2 and s.section_number>0},{FLAG})
            self.assertFalse(any(s.section_number==0 and s.storage_class==2 for s in storage_obj.symbols))
            storage_flag=storage_obj.find_symbol(FLAG);storage_section=storage_obj.section(storage_flag.section_number)
            self.assertEqual((storage_flag.value,storage_section.name,storage_section.raw_size,storage_section.raw_ptr,storage_section.characteristics),(0,'.bss',4,0,0xc0300080))
            self.assertFalse(any(s.name.startswith('.CRT') for s in storage_obj.sections))
            spec=json.loads((ROOT/'config/game_build.json').read_text())
            built=game_compile(SOURCE,output/'game-command',compiler,[ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(built['compiled'],built)
            self.assertEqual(object_view(candidate),object_view(CoffObject(Path(built['object']))))
            fixture_path,_=compile_source(ROOT/'tests/fixtures/chat_name_tag_compare.cpp','fixture',True)
            executable=output/'name-tag.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                                   f'/out:{executable}',f'/libpath:{libraries}',str(candidate_path),str(storage_path),str(fixture_path),
                                   'kernel32.lib','user32.lib','gdi32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image_path)],cwd=ROOT,capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('NameTagDraw: 5864 scenarios match, actual sprintf and scripted GDI; 64 AV, 8 throws',result.stdout)
            self.assertEqual(strict,94)
            print(f'NameTagDraw: {strict} game strict + 1 CRT strict;244 sections/1025 relocations; '+result.stdout.strip())


if __name__=='__main__':
    unittest.main()
