"""Complete camera-file loader, actual stream/math/CRT, and bounded fault proof."""
import hashlib
import json
import os
import re
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from mcm2tool.coff_archive import get_archive_member
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.build_game import ROOT as TOOL_ROOT, compile_source as game_compile
from tools.run_calibration import CASES
from tools.match import compare
from test_edit_box_on_char import extract
from test_edit_box_draw import signature

ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'samples/camera/FollowCameraLoadFile.cpp'
AGGREGATE=ROOT/'samples/camera/FollowCameraNearMisses.cpp'
SYMBOL='?LoadCameraFile@FollowCamera@@QAEXPBD@Z'
DIRECT={0x46510f:0x4a3010,0x465131:0x460d10,0x46515f:0x460f50,
        0x46518a:0x461aa0,0x4651a2:0x5476f0,0x4651c3:0x461aa0,
        0x4651e3:0x4a3010,0x465212:0x4a3010,0x46522c:0x461340,
        0x465240:0x461aa0,0x46525e:0x5476f0,0x4652b3:0x53530b,
        0x4653b6:0x5087b0,0x4653dc:0x5087b0,0x46540a:0x515600,
        0x465498:0x461aa0,0x4654ae:0x460d60,0x4654b4:0x4a30c0}


def object_view(obj,portable=False):
    sections=[s for s in obj.sections if not s.name.startswith('.debug')]
    filenames={}
    if portable:
        for s in obj.symbols:
            if s.name.startswith('??_C@') and s.section_number>0:
                sec=obj.section(s.section_number)
                raw=obj.data[sec.raw_ptr+s.value:sec.raw_ptr+sec.raw_size]
                value=raw.rstrip(b'\0').replace(b'\\',b'/')
                if value.rsplit(b'/',1)[-1] in (b'FollowCameraLoadFile.cpp',b'FollowCameraNearMisses.cpp',b'FollowCameraTick.cpp',b'FollowCameraControls.cpp'):
                    assert raw.endswith(b'\0') and s.value==0
                    assert Path(value.decode()).resolve()==(ROOT/'samples/camera'/value.rsplit(b'/',1)[-1].decode()).resolve()
                    filenames[sec.index]='filename:'+value.rsplit(b'/',1)[-1].decode()
    keys={};anonymous={}
    for sec in sections:
        labels=tuple(sorted((filenames.get(sec.index,s.name),s.value,s.storage_class,s.type)
            for s in obj.symbols if s.section_number==sec.index and s.storage_class in(2,3)
            and s.name!=sec.name and not s.name.startswith('$T')))
        if labels:keys[sec.index]=('named',labels)
        else:
            n=anonymous.get(sec.name,0);anonymous[sec.name]=n+1
            keys[sec.index]=('anonymous',sec.name,n)
    assert len(set(keys.values()))==len(keys)
    rows=[]
    for sec in sections:
        data=obj.data[sec.raw_ptr:sec.raw_ptr+sec.raw_size] if sec.raw_ptr else bytes(sec.raw_size)
        if sec.index in filenames:data=filenames[sec.index].encode()+b'\0'
        rels=[]
        for rel in obj.relocations:
            if rel.section_number!=sec.index:continue
            s=obj.symbol_by_index[rel.symbol_index]
            target=('defined',keys[s.section_number],s.value) if s.section_number>0 else ('external',s.name,s.value,s.section_number)
            rels.append((rel.virtual_address,rel.type,target))
        rows.append((keys[sec.index],sec.characteristics,data.hex(),rels))
    return sorted(rows,key=lambda r:repr(r[0]))


def record(obj):
    rows=object_view(obj,True)
    return dict(sections=len(rows),relocations=sum(len(r[3])for r in rows),
                sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest())


def loader_signature(obj):
    entry=obj.find_symbol(SYMBOL);raw,size,rels=obj.symbol_extent(entry);result=[]
    for r in rels:
        s=obj.symbol_by_index[r.symbol_index];key=s.name
        if s.section_number>0:
            sec=obj.section(s.section_number)
            if s.name.startswith('??_C@'):
                value=obj.data[sec.raw_ptr+s.value:sec.raw_ptr+sec.raw_size]
                key=('literal',value) if b'.cpp' not in value else 'validated loader filename'
            elif s.name.startswith('$L'):key=('EH',sec.name,s.value)
        result.append((r.virtual_address-entry.value,r.type,key))
    return raw,size,result


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class FollowCameraLoadFileTests(unittest.TestCase):
    def test_complete_loader_real_file_math_and_fault_order(self):
        import capstone
        image_path=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image=PEImage(image_path)
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        def decoded(va,size):return list(decoder.disasm(image.bytes_at_va(va,size),va))
        code=decoded(0x4650e0,1014)
        self.assertEqual(sum(i.size for i in code),1014)
        self.assertEqual([(i.address,bytes(i.bytes))for i in code if i.mnemonic=='ret'],[(0x4654d3,b'\xc2\x04\x00')])
        self.assertEqual(image.bytes_at_va(0x4654d6,10),b'\x90'*10)
        self.assertEqual({i.address:i.operands[0].imm for i in code if i.mnemonic=='call'},DIRECT)
        starts={i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):self.assertIn(i.operands[0].imm,starts)
        self.assertEqual(image.bytes_at_va(0x56b220,36).split(b'\0')[0],b'D:\\aardvark\\VC\\krusty2\\FollowCam.cpp')
        # Constructor-only unwind action; later allocation failures leak the stream.
        info=struct.unpack('<8I',image.bytes_at_va(0x560cf8,32))
        self.assertEqual(info[0:2],(0x19930520,1));self.assertEqual(info[3],0)
        self.assertEqual(struct.unpack('<iI',image.bytes_at_va(info[2],8)),(-1,0x54aba0))
        self.assertEqual([(i.address,i.op_str)for i in decoded(0x54aba0,26)if i.mnemonic=='call'],[(0x54abb1,'0x4a3060')])
        self.assertEqual([(i.address,i.op_str)for i in decoded(0x54abba,10)if i.mnemonic=='jmp'],[(0x54abbf,'0x53471a')])
        classes={c.name:c for c in parse_rtti(image)}
        for name,table in [('FollowCamera',0x55282c),('KrustyBikeCamera',0x554754)]:
            self.assertEqual([(r.object_offset,r.vtables)for r in classes[name].vtable_records],[(0,[table])])
            self.assertTrue(all(b.mdisp==0 and b.pdisp==-1 for b in classes[name].bases))
            col=struct.unpack('<5I',image.bytes_at_va(struct.unpack('<I',image.bytes_at_va(table-4,4))[0],20))
            self.assertEqual(col[1],0)
            self.assertEqual(image.bytes_at_va(col[3]+8,64).split(b'\0')[0],('.?AV'+name+'@@').encode())
        self.assertEqual(image.bytes_at_va(0x56d568,13),b'FlybyCam.vue\0')
        caller={i.address:(i.mnemonic,i.op_str)for i in decoded(0x497d90,81)}
        self.assertEqual(caller[0x497dc2],('call','0x416e80'))
        self.assertEqual(caller[0x497dcf],('push','0x56d568'))
        self.assertEqual(caller[0x497dd4],('mov','ecx, esi'))
        self.assertEqual(caller[0x497dd6],('call','0x4650e0'))
        # Constructor fields and order independently ground guarded real writes.
        ctor=decoded(0x460d10,77)
        ctor_stores=[(i.operands[0].mem.disp,i.operands[0].size)for i in ctor
                     if i.mnemonic=='mov' and i.operands[0].type==capstone.x86.X86_OP_MEM
                     and i.operands[0].mem.base==capstone.x86.X86_REG_EAX]
        self.assertEqual(ctor_stores,[(8,4),(16,4),(1,1),(20,4),(3,1),(28,4),(12,4),(304,4),
                                     (4,4),(2,1),(0,1),(32,1),(24,1),(300,4),(292,4),(296,4)])
        # Raw stores protect the independent progressive-output oracle.
        stores=[i.operands[0].mem.disp for i in code if 0x465425<=i.address<=0x46547b
                and i.mnemonic=='mov' and i.operands[0].type==capstone.x86.X86_OP_MEM
                and i.operands[0].mem.base==capstone.x86.X86_REG_ESI]
        self.assertEqual(stores,[0,4,16,20,8,24,32,12,28,36,40,44,48,52,56,60])
        compiler=Path(os.environ['VC6_ROOT']).resolve()
        linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-camera-load-') as directory:
            out=Path(directory)
            def compile_source(source,name,fixture=False,includes=True):
                path=out/(name+'.obj')
                command=[sys.executable,str(TOOL_ROOT/'tools/compile.py'),str(source),'-o',str(path),
                         '--vc6-root',str(compiler),'--profile','vc6_o2_mt']
                if includes:command+=['--include',str(ROOT/'src/reconstructed'),'--include',str(ROOT/'src/krusty2')]
                if fixture:command+=['--extra','/Od']
                run=subprocess.run(command,cwd=TOOL_ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(TOOL_ROOT)})
                self.assertEqual(run.returncode,0,run.stdout+run.stderr)
                return path,CoffObject(path)
            candidate_path,candidate=compile_source(SOURCE,'candidate',includes=False)
            _,aggregate=compile_source(AGGREGATE,'aggregate',includes=False)
            pins=json.loads((ROOT/'tests/fixtures/follow_camera_load_file_objects.json').read_text())
            self.assertEqual(record(candidate),pins['candidate']);self.assertEqual(record(aggregate),pins['aggregate'])
            self.assertEqual(loader_signature(candidate),loader_signature(aggregate))
            report=compare(image,candidate,SYMBOL,0x4650e0,1014,SOURCE.with_suffix('.bindings.json'))
            self.assertEqual((report['candidate_size'],report['strict_match']['matching_positions'],report['strict_exact']),(980,429,False))
            self.assertEqual({s.name for s in candidate.symbols if s.section_number>0 and s.storage_class==2 and s.type==32},{SYMBOL,'??_H@YGXPAXIHP6EX0@Z@Z'})
            self.assertEqual(signature(candidate,'??_H@YGXPAXIHP6EX0@Z@Z'),signature(aggregate,'??_H@YGXPAXIHP6EX0@Z@Z'))
            canonical={unit:compile_source(ROOT/f'src/reconstructed/{unit}.cpp',unit)[1]
                       for unit in ('FileStream','ResourceManager','EcoSystem','Math3D','CrossProduct','FastMath','KrustyBikeCamera')}
            stream=(ROOT/'src/reconstructed/FileStream.cpp').read_text()
            head=stream[stream.index('#include'):stream.index('// The stream that holds')]
            prefixes=['UnknownTextureStream::UnknownTextureStream(','UnknownTextureStream::~UnknownTextureStream(',
                      'UnknownTextureStream* UnknownTextureStream::FindArchive(','int UnknownTextureStream::IsValidFile(',
                      'int UnknownTextureStream::Open(','int UnknownTextureStream::SetBufferSize(','int UnknownTextureStream::Tell(']
            parts=[extract(stream,'static inline UnknownTextureStream* InnermostStream(')]+[extract(stream,p)for p in prefixes]
            for prefix in ['int UnknownTextureStream::ReadByte(','int UnknownTextureStream::ReadLine(']:
                # Validation extraction only; each full emitted body is dual-strict.
                parts.append('#pragma inline_depth(0)\n'+extract(stream,prefix)+'\n#pragma inline_depth()')
            helper_sources={'stream':head+'\n'.join(parts),
                'normalize':'#include "math/Vec3.h"\nfloat FastInvSqrt(float);\n'+extract((ROOT/'src/reconstructed/Math3D.cpp').read_text(),'Vec3 Vec3Normalize('),
                'cross':'#include "math/Vec3.h"\n'+extract((ROOT/'src/reconstructed/CrossProduct.cpp').read_text(),'Vec3 CrossProductCall('),
                'finditem':'#include <string.h>\n#include "TextureMap.h"\n#include "ResourceManager.h"\n'+extract((ROOT/'src/reconstructed/ResourceManager.cpp').read_text(),'ResourceItem* UnknownResourceManager::FindItem('),
                'readxor':'#include <stdio.h>\n'+extract((ROOT/'src/reconstructed/EcoSystem.cpp').read_text(),'int ReadXorDecoded(')}
            sample=(TOOL_ROOT/'samples/io/FileStream.cpp').read_text()
            helper_sources['stream_candidates']='#include <stdio.h>\n#include "FileStream.h"\nint ReadXorDecoded(unsigned char*,int,int,FILE*,unsigned char*);\n'+'\n'.join(extract(sample,p)for p in ['static inline int HeaderMatches(','int UnknownTextureStream::CheckHeader(','static inline void RestartKey(','int UnknownTextureStream::Seek('])
            helpers={};helper_paths=[]
            for name,text in helper_sources.items():
                path=out/(name+'.cpp');path.write_text(text+'\n');obj,o=compile_source(path,name);helpers[name]=o;helper_paths.append(obj)
            fast_path,fast=compile_source(ROOT/'src/reconstructed/FastMath.cpp','fast_native');helper_paths.append(fast_path)
            empty=out/'empty.json';empty.write_text('{}')
            strict=0
            for case in CASES:
                unit=Path(case['source']).stem
                if unit=='KrustyBikeCamera':selected=canonical[unit]
                elif unit in ('FileStream','ResourceManager','EcoSystem'):
                    selected=helpers['stream' if unit=='FileStream' else 'finditem' if unit=='ResourceManager' else 'readxor']
                    try:entry=selected.find_symbol(case['symbol'])
                    except ValueError:continue
                    if entry.section_number<=0:continue
                else:continue
                proof=compare(image,selected,case['symbol'],int(case['target_va'],0),case['target_size'],TOOL_ROOT/case['bindings'])
                self.assertTrue(proof['strict_exact'],proof);strict+=1
                if unit!='KrustyBikeCamera':self.assertEqual(signature(selected,case['symbol']),signature(canonical[unit],case['symbol']))
            for name,unit,symbol,va,size,bindings in [
                ('normalize','Math3D','?Vec3Normalize@@',0x5087b0,150,ROOT/'src/reconstructed/Math3D.bindings.json'),
                ('cross','CrossProduct','?CrossProductCall@@',0x515600,91,empty),
                ('fast','FastMath','?FastInvSqrt@@',0x460c00,107,ROOT/'src/reconstructed/FastMath.bindings.json'),
                ('fast','FastMath','?InitFastInvSqrtTable@@',0x460bb0,74,ROOT/'src/reconstructed/FastMath.bindings.json')]:
                selected=fast if name=='fast' else helpers[name]
                proof=compare(image,selected,symbol,va,size,bindings);self.assertTrue(proof['strict_exact'],proof);strict+=1
                self.assertEqual(signature(selected,symbol),signature(canonical[unit],symbol))
            _,whole_stream=compile_source(TOOL_ROOT/'samples/io/FileStream.cpp','whole_stream')
            for symbol in ('?CheckHeader@','?Seek@'):
                self.assertEqual(signature(helpers['stream_candidates'],symbol),signature(whole_stream,symbol))
            # Same actual command as the game build, including current includes/profile.
            spec=json.loads((TOOL_ROOT/'config/game_build.json').read_text())
            built=game_compile(SOURCE,out/'game-command',compiler,[TOOL_ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(TOOL_ROOT)})
            self.assertTrue(built['compiled'],built)
            self.assertEqual(object_view(candidate),object_view(CoffObject(Path(built['object']))))
            crt_bindings=out/'crt.json'
            crt_bindings.write_text(json.dumps({'_strlen':'0x53cc40','__input':'0x53c180',
                '___lc_handle':'0x68ae24','___unguarded_readlc_active':'0x68c49c',
                '___setlc_active':'0x68c498','__lock':'0x53821f',
                '__tolower_lk':'0x53ead3','__unlock':'0x538280'}))
            crt=[]
            for member,symbol,va,size in [('sscanf','_sscanf',0x53530b,52),('strnicmp','__strnicmp',0x5476f0,257)]:
                path=out/(member+'.obj');path.write_bytes(get_archive_member(find_child_ci(libraries,'libcmt.lib'),'build/intel/mt_obj/'+member+'.obj'))
                obj=CoffObject(path)
                proof=compare(image,obj,symbol,va,size,crt_bindings);self.assertTrue(proof['strict_exact'],proof);strict+=1
                crt.append((obj,symbol,size))
            fixture_path,_=compile_source(ROOT/'tests/fixtures/follow_camera_load_file_compare.cpp','fixture',True)
            def execute(obj,source,name):
                exe=out/(name+'.exe');mapfile=out/(name+'.map')
                linked=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{exe}',f'/map:{mapfile}',f'/libpath:{libraries}',str(obj),str(fixture_path),*map(str,helper_paths),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
                self.assertEqual(linked.returncode,0,linked.stdout+linked.stderr)
                linked_symbols=dict(re.findall(r'^[ ]+[0-9a-fA-F]{4}:[0-9a-fA-F]{8}[ ]+(\S+)[ ]+([0-9a-fA-F]{8})[ ]',mapfile.read_text(),re.M))
                native_bindings=out/(name+'-native.json');native_bindings.write_text(json.dumps({k:'0x'+v for k,v in linked_symbols.items()}))
                native_image=PEImage(exe)
                for crt_obj,crt_symbol,crt_size in crt:
                    proof=compare(native_image,crt_obj,crt_symbol,int(linked_symbols[crt_symbol],16),crt_size,native_bindings)
                    self.assertTrue(proof['strict_exact'],proof)
                return subprocess.run([str(exe),str(image_path),str(source)],cwd=out,capture_output=True,text=True,timeout=90)
            run=execute(candidate_path,SOURCE,'camera')
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            self.assertIn('CameraLoad: 1551 real-stream/math cases match',run.stdout)
            # Negative controls reconstruct the exact old assignment sequence; the
            # prior math control differs only by the observed first-cross rounding.
            text=SOURCE.read_text();start=text.index('            // Reconstructed access-order model:');end=text.index('            k++;',start)
            oldstores=''.join(f'            frame->m[{r}][{c}] = {value};\n'for r,c,value in [
                (0,0,'side.x'),(0,1,'side.y'),(0,2,'side.z'),(0,3,'0.0f'),
                (1,0,'up.x'),(1,1,'up.y'),(1,2,'up.z'),(1,3,'0.0f'),
                (2,0,'forward.x'),(2,1,'forward.y'),(2,2,'forward.z'),(2,3,'0.0f'),
                (3,0,'position.x'),(3,1,'position.y'),(3,2,'position.z'),(3,3,'1.0f')])
            control=text[:start]+oldstores+text[end:]
            for name,source_text,size,positions,diagnostic in [
                ('write_control',control,976,439,'GuardAllocation1 bytes4'),
                ('math_control',control.replace('Vec3 side = CrossProductCall(upHint, direction);','Vec3 side = FollowCameraCross(upHint, direction);'),952,519,'fullbitsbd180a5c/bd180a57')]:
                source_text=source_text.replace('"../../src/','"'+(ROOT/'src').as_posix()+'/')
                source=out/(name+'.cpp');source.write_text(source_text)
                obj,o=compile_source(source,name)
                proof=compare(image,o,SYMBOL,0x4650e0,1014,SOURCE.with_suffix('.bindings.json'))
                self.assertEqual((proof['candidate_size'],proof['strict_match']['matching_positions']),(size,positions))
                if name=='math_control':
                    raw,_,rels=loader_signature(o)
                    self.assertEqual(hashlib.sha256(raw).hexdigest(),pins['original_loader_raw_sha256'])
                negative=execute(obj,source,name)
                self.assertEqual(negative.returncode,3,negative.stdout+negative.stderr)
                self.assertIn(diagnostic,negative.stdout)
            self.assertEqual(strict,42)
            print(f'CameraLoad: 40 strict game helper/caller checks, 2 CRT bodies also checked in each linked executable; complete object pins; '+run.stdout.strip())


if __name__=='__main__':unittest.main()
