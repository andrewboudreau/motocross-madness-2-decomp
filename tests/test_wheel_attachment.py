"""Wheel attachment provider with real scene transforms and whole aggregate preservation."""
import hashlib
import json
import os
from pathlib import Path
import re
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.build_game import compile_source as build_compile_source

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/VehicleWheelAttachment.cpp'
SAMPLE = 'samples/physics/tire/Tire.cpp'
CANONICAL = 'src/krusty2/vehicle/Bike.cpp'
TREE = 'src/krusty2/soultree/soultree.cpp'
MATRIX = 'src/krusty2/bvh/BoundingBoxTreeQuery.cpp'
POSITION = 'src/reconstructed/SoultreePositionValue.cpp'
SYMBOL = '?UpdateAttachment@VehicleWheel@@QAEXXZ'
HELPERS = (('InvalidateWorldMatrix',0x4fb850,45),('InvalidateSiblingChain',0x4fb880,50),
           ('MarkSubtreeDirty',0x4fdab0,38),('SetLocalMatrix',0x4fca30,33),('Rotate',0x4fceb0,465))


def signature(obj, query):
    symbol = obj.find_symbol(query)
    data, size, relocations = obj.symbol_extent(symbol)
    return data, size, [(r.virtual_address-symbol.value, r.type,
                        obj.symbol_by_index[r.symbol_index].name) for r in relocations]


def complete_object(obj):
    """Every nondebug byte/flag/destination; only a known checkout prefix varies."""
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    prefix = str(ROOT).replace('\\', '/')+'/'
    def filename(data):
        text, nul, tail = data.partition(b'\0')
        value = text.decode('latin1').replace('\\', '/')
        if nul and value.lower().startswith(prefix.lower()) and value[len(prefix):] == CANONICAL:
            return ('source:'+CANONICAL).encode()+nul+tail
        return data
    def name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.section(symbol.section_number)
            data = obj.data[section.raw_ptr+symbol.value:section.raw_ptr+section.raw_size]
            normalized = filename(data)
            if normalized != data:
                return normalized.split(b'\0')[0].decode()
        return symbol.name
    keys, anonymous = {}, {}
    for section in sections:
        labels = tuple(sorted((name(s), s.value) for s in obj.symbols
                       if s.section_number == section.index and (s.storage_class == 2 or s.type == 32)))
        if labels:
            keys[section.index] = ('named', labels)
        else:
            count = anonymous.get(section.name, 0)
            anonymous[section.name] = count+1
            keys[section.index] = ('anonymous', section.name, count)
    assert len(set(keys.values())) == len(keys)
    def target(symbol):
        if symbol.section_number > 0:
            return ('defined', keys[symbol.section_number], symbol.value)
        return ('external', symbol.name, symbol.value, symbol.section_number)
    rows = []
    for section in sections:
        data = obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        relocations = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                       for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.characteristics, filename(data).hex(), relocations))
    return sorted(rows, key=lambda r: repr(r[0]))


def definition(text, start):
    at = text.index(start)
    end, depth = text.index('{', at)+1, 1
    while depth:
        depth += (text[end] == '{')-(text[end] == '}')
        end += 1
    return text[at:end]


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and hash-pinned retail')
class WheelAttachmentTests(unittest.TestCase):
    def test_full_provider_primary_receiver_actual_helpers_and_faults(self):
        import capstone
        from mcm2tool.rtti import parse_rtti
        image=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        pe=PEImage(image);decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        def decode(va,size):
            code=list(decoder.disasm(pe.bytes_at_va(va,size),va));self.assertEqual(sum(i.size for i in code),size);return code
        body=decode(0x515660,195)
        self.assertEqual([(i.address,i.op_str)for i in body if i.mnemonic=='ret'],[(0x5156bd,''),(0x515722,'')])
        self.assertEqual({i.address:i.op_str for i in body if i.mnemonic=='call'},
                         {0x5156b4:'0x4fc630',0x5156de:'0x534d07',0x5156f1:'0x4fca30',0x515719:'0x4fceb0'})
        for i in body:
            if i.group(capstone.CS_GRP_JUMP):self.assertIn(i.operands[0].imm,{j.address for j in body})
        self.assertEqual(pe.bytes_at_va(0x515723,13),b'\x90'*13)
        # Distinguish the primary wheel array from the separately adjusted contact array.
        table=0x558578;col,=struct.unpack('<I',pe.bytes_at_va(table-4,4));self.assertEqual(col,0x55f5e0)
        _,offset,_,td,_=struct.unpack('<5I',pe.bytes_at_va(col,20));self.assertEqual(offset,0)
        self.assertEqual(pe.bytes_at_va(td+8,40).split(b'\0')[0],b'.?AVTire@@')
        tire=next(c for c in parse_rtti(pe) if c.name=='Tire')
        self.assertEqual([(b.name,b.mdisp,b.pdisp,b.vdisp)for b in tire.bases],
            [('Tire',0,-1,0),('CollisionObject',0,-1,0),('QuadTreeObject',0,-1,0),
             ('GraphicsTest',12,-1,0),('GameObject',12,-1,0),('BaseObject',12,-1,0),
             ('MovingPart',380,-1,0),('CollisionPoint',184,-1,0)])
        actual={i.address:(i.mnemonic,i.op_str)for va,n in ((0x512f10,1168),(0x4079c0,6745),(0x525d30,160),(0x40cbd0,486))for i in decode(va,n)}
        for va,want in {
            0x513120:('mov','dword ptr [esi], 0x558578'),
            0x51312a:('mov','dword ptr [esi + 0xc], 0x558508'),
            0x513135:('mov','dword ptr [esi + 0xb8], 0x5584fc'),
            0x4084d7:('call','0x512f10'),0x4084dc:('mov','esi, eax'),
            0x408547:('push','esi'),0x408548:('call','0x525d30'),
            0x525d58:('mov','dword ptr [eax + 0x2b0], ecx'),
            0x525d5e:('mov','dword ptr [eax + 0x2ac], edx'),
            0x525d78:('mov','dword ptr [edx + ecx*4], eax'),
            0x525d7d:('add','eax, 0xb8'),0x525db2:('mov','dword ptr [esi + 0x544], ecx'),
            0x40cd98:('mov','edx, dword ptr [esi + 0x53c]'),
            0x40cd9e:('mov','ecx, dword ptr [edx + edi*4]'),0x40cda1:('call','0x515660'),
        }.items():self.assertEqual(actual[va],want)
        compiler=Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-wheel-attachment-') as directory:
            output=Path(directory)
            text=(ROOT/TREE).read_text();helper=output/'helpers.cpp'
            starts=['static inline void AxisAngleMatrix(']+['void SoultreeObject::'+name+'(' for name,_,_ in HELPERS]
            helper.write_text('#include "math/Math3D.h"\n#include <math.h>\n'+'\n'.join(definition(text,s)for s in starts)+'\n')
            matrix=output/'matrix.cpp';matrix.write_text('#include "math/Math3D.h"\n'+definition((ROOT/MATRIX).read_text(),'void MatrixMultiply(')+'\n')
            layout=output/'layout.cpp';layout.write_text('#include <stddef.h>\n#include "TireAttachmentViews.h"\n#include "vehicle/Vehicle.h"\n'
                'typedef char Layout[(sizeof(VehicleWheel)==0x2c0 && offsetof(VehicleWheel,primaryAux)==0x2b0 && offsetof(VehicleWheel,secondaryAux)==0x2ac && '
                'sizeof(CollisionVec3)==sizeof(Vec3) && sizeof(CollisionMatrix4)==sizeof(Matrix4) && offsetof(TireAttachA,node)==0x44 && '
                'offsetof(TireAttachA,position)==0x98 && offsetof(TireAttachA,direction)==0xc0 && offsetof(TireAttachB,restMatrix)==4 && '
                'offsetof(TireAttachB,node)==0x44 && offsetof(TireAttachB,position)==0x98 && offsetof(TireAttachB,scale)==0xc8 && offsetof(TireAttachB,axis)==0xd4)?1:-1];\n')
            jobs=[(s,ROOT/s)for s in (SOURCE,SAMPLE,CANONICAL,TREE,MATRIX,POSITION,'src/krusty2/vehicle/Vehicle.cpp','tests/fixtures/wheel_attachment_compare.cpp')]
            jobs += [('helpers',helper),('matrix',matrix),('layout',layout)]
            paths={};objects={}
            for number,(key,source) in enumerate(jobs):
                path=output/('unit%d.obj'%number)
                r=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),'--include',str(ROOT/'src/reconstructed')],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(r.returncode,0,r.stdout+r.stderr);paths[key]=path;objects[key]=CoffObject(path)
            baseline=json.loads((ROOT/'tests/fixtures/wheel_attachment_objects.json').read_text())
            for source in (SAMPLE,CANONICAL):
                rows=complete_object(objects[source]);fingerprint={'sections':len(rows),'relocations':sum(len(r[3])for r in rows),
                    'sha256':hashlib.sha256(json.dumps(rows).encode()).hexdigest()}
                self.assertEqual(fingerprint,baseline[source],source)
            provider=objects[SOURCE];entry=provider.find_symbol(SYMBOL)
            self.assertEqual((entry.function_size,entry.extent_source),(195,'coff_function_aux'))
            self.assertEqual({s.name for s in provider.symbols if s.storage_class==2 and s.section_number==0},
                {'__fltused','__finite','?SetPositionVec3@SoultreeObject@@QAEXUVec3@@@Z',
                 '?SetLocalMatrix@SoultreeObject@@QAEXPBUMatrix4@@@Z','?Rotate@SoultreeObject@@QAEXUVec3@@M@Z'})
            strict=0
            for source,symbol,va,size,bindings in ((SOURCE,SYMBOL,0x515660,195,(ROOT/SOURCE).with_suffix('.bindings.json')),
                (CANONICAL,'?UnknownVirtualSlot41@Bike@@',0x40cbd0,486,(ROOT/CANONICAL).with_suffix('.bindings.json')),
                ('src/krusty2/vehicle/Vehicle.cpp','?AddWheel@Vehicle@@',0x525d30,160,ROOT/'src/krusty2/vehicle/Vehicle.bindings.json'),
                (POSITION,'?SetPositionVec3@SoultreeObject@@',0x4fc630,38,(ROOT/POSITION).with_suffix('.bindings.json'))):
                result=compare(pe,objects[source],symbol,va,size,bindings);self.assertTrue(result['strict_exact'],result);strict+=1
            for t in json.loads((ROOT/'samples/physics/tire/targets.json').read_text()):
                if t['source']=='Tire.cpp' and t['expect']=='exact':
                    result=compare(pe,objects[SAMPLE],t['candidate_symbol_contains'],int(t['target_va'],0),t['target_size'],ROOT/'samples/physics/tire'/t.get('bindings','Tire.bindings.json'))
                    self.assertTrue(result['strict_exact'],result);strict+=1
            for name,va,size in HELPERS:
                symbol='?'+name+'@SoultreeObject@@'
                for source in ('helpers',TREE):
                    result=compare(pe,objects[source],symbol,va,size,(ROOT/TREE).with_suffix('.bindings.json'))
                    self.assertTrue(result['strict_exact'],result);strict+=1
                self.assertEqual(signature(objects['helpers'],symbol),signature(objects[TREE],symbol))
            for source in (MATRIX,'matrix'):
                result=compare(pe,objects[source],'?MatrixMultiply@@',0x42a1a0,684,(ROOT/MATRIX).with_suffix('.bindings.json'))
                self.assertTrue(result['strict_exact'],result);strict+=1
            self.assertEqual(signature(objects[MATRIX],'?MatrixMultiply@@'),signature(objects['matrix'],'?MatrixMultiply@@'))
            # Ordinary Math3D header overhead: private vectors and exact initializer chain.
            inits=(('_$E2','_$E1','_kVec3Zero',0),('_$E5','_$E4','_kVec3XAxis',32),
                   ('_$E8','_$E7','_kVec3YAxis',48),('_$E11','_$E10','_kVec3ZAxis',16))
            bss=next(s for s in provider.sections if s.name=='.bss');crt=next(s for s in provider.sections if s.name=='.CRT$XCU')
            self.assertEqual((bss.raw_size,bss.raw_ptr,bss.characteristics),(60,0,0xc0400080))
            self.assertEqual({(s.name,s.value,s.storage_class)for s in provider.symbols if s.section_number==bss.index and s.name!='.bss'},
                             {(name,offset,3)for _,_,name,offset in inits})
            self.assertEqual((crt.raw_size,crt.characteristics),(16,0xc0300040));self.assertEqual(provider.data[crt.raw_ptr:crt.raw_ptr+16],bytes(16))
            self.assertEqual([(r.virtual_address,r.type,provider.symbol_by_index[r.symbol_index].name)for r in provider.relocations if r.section_number==crt.index],
                             [(4*i,6,t[0])for i,t in enumerate(inits)])
            tree_targets=json.loads((ROOT/'src/krusty2/soultree/targets.json').read_text())
            for thunk,body,_,_ in inits:
                for name in (thunk,body):
                    self.assertEqual(signature(provider,name),signature(objects[TREE],name))
                    target=next(t for t in tree_targets if t.get('source')=='soultree.cpp' and t['candidate_symbol_contains']==name)
                    result=compare(pe,objects[TREE],name,int(target['target_va'],0),target['target_size'],(ROOT/TREE).with_suffix('.bindings.json'))
                    self.assertTrue(result['strict_exact'],result);strict+=1
            self.assertEqual({s.name for s in provider.symbols if s.type==32 and s.section_number>0},
                             {n for thunk,body,_,_ in inits for n in (thunk,body)}|{SYMBOL})
            self.assertEqual(len([s for s in provider.sections if not s.name.startswith('.debug')]),12)
            spec=json.loads((ROOT/'config/game_build.json').read_text())
            built=build_compile_source(ROOT/SOURCE,output/'game-command',compiler,[ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(built['compiled'],Path(built['log']).read_text());self.assertEqual(complete_object(provider),complete_object(CoffObject(Path(built['object']))))
            linker=find_child_ci(find_vc6_bin(compiler),'link.exe');libraries=find_child_ci(infer_vc98_root(compiler),'lib')
            exe=output/'attachment.exe';mapping=output/'attachment.map'
            r=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{exe}',f'/map:{mapping}',
                f'/libpath:{libraries}',*[str(paths[s])for s in (SOURCE,POSITION,'helpers','matrix','tests/fixtures/wheel_attachment_compare.cpp')],'kernel32.lib'],
                capture_output=True,text=True,encoding='latin1');self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            finite=re.search(r'\s__finite\s+([0-9a-fA-F]{8})\s',mapping.read_text());self.assertIsNotNone(finite)
            self.assertEqual(PEImage(exe).bytes_at_va(int(finite.group(1),16),20),pe.bytes_at_va(0x534d07,20))
            r=subprocess.run([str(exe),str(image)],capture_output=True,text=True,timeout=120);self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            self.assertEqual(r.stdout.strip(),'Wheel attachment: 47006 complete-state, actual-helper and partial-fault scenarios match')
            print('%d strict checks; two complete objects; real CRT/scene helpers; %s'%(strict,r.stdout.strip()))


if __name__=='__main__':
    unittest.main()
