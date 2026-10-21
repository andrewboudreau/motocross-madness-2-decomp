"""Shared EcoSystem tree interfaces and finite actual Insert/allocator behavior."""
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
from mcm2tool.toolchain import find_child_ci,find_vc6_bin,infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES
ROOT=Path(__file__).resolve().parents[1]
SOURCES=('src/reconstructed/EcoSystem.cpp','samples/ecosystem/EcoSystemNearMisses.cpp',
         'src/krusty2/broadphase/Quadtree.cpp','src/krusty2/visibility/VisibilityQuadTree.cpp')
FILENAMES=SOURCES
ALLOCATOR='src/reconstructed/BlockAllocator.cpp'
INSERT='?Insert@QuadTree@@QAEXPAVQuadTreeObject@@IMM@Z'

def whole_object(obj, roots=(ROOT,)):
    """Keep all nondebug contents; normalize only the enumerated source paths."""
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    prefixes = [str(root).replace('\\', '/') + '/' for root in roots]

    def filename(data):
        value, nul, tail = data.partition(b'\0')
        path = value.decode('latin1').replace('\\', '/')
        for prefix in prefixes:
            if nul and path.lower().startswith(prefix.lower()) and path[len(prefix):] in FILENAMES:
                return ('source:' + path[len(prefix):]).encode() + nul + tail
        return data

    def name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.section(symbol.section_number)
            data = obj.data[section.raw_ptr + symbol.value:section.raw_ptr + section.raw_size]
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
            ordinal = anonymous.get(section.name, 0)
            anonymous[section.name] = ordinal + 1
            keys[section.index] = ('anonymous', section.name, ordinal)
    assert len(set(keys.values())) == len(keys)

    def target(symbol):
        if symbol.section_number > 0:
            return ('defined', keys[symbol.section_number], symbol.value)
        return ('external', symbol.name, symbol.value, symbol.section_number)

    rows = []
    for section in sections:
        data = obj.data[section.raw_ptr:section.raw_ptr + section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        relocations = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                       for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.characteristics, filename(data).hex(), relocations))
    return sorted(rows, key=lambda row: repr(row[0]))


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail')
class EcoQuadTreeConnectionsTests(unittest.TestCase):
    def test_complete_callers_primary_receivers_and_actual_insert_closure(self):
        import capstone
        exe=Path(os.environ['MCM2_EXE']);image=PEImage(exe)
        self.assertEqual(hashlib.sha256(exe.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);md.detail=True
        def decode(va,size):
            ins=list(md.disasm(image.bytes_at_va(va,size),va))
            self.assertEqual(sum(i.size for i in ins),size)
            return {i.address:(i.mnemonic,i.op_str)for i in ins}
        classes={c.name:c for c in parse_rtti(image)}
        for name,table,col,offset in (
            ('QuadTree',0x55763c,0x55ea58,0),('QuadTreeObject',0x5511c4,0x55b038,0),
            ('Vegetation',0x5524f8,0x55bef0,0),('VisibilityQuadTree',0x558dfc,0x55fac8,0),
            ('VisibilityQuadTree',0x558d8c,0x55fa40,0x874)):
            self.assertEqual(struct.unpack('<I',image.bytes_at_va(table-4,4))[0],col)
            words=struct.unpack('<5I',image.bytes_at_va(col,20));self.assertEqual(words[1],offset)
            self.assertEqual(image.bytes_at_va(words[3]+8,80).split(b'\0')[0],('.?AV'+name+'@@').encode())
            self.assertTrue(any(r.object_offset==offset and table in r.vtables for r in classes[name].vtable_records))
        for cls,base,offset in [('VisibilityQuadTree','QuadTree',0),('VisibilityQuadTree','GameObject',0x874),('Vegetation','QuadTreeObject',0)]:
            self.assertTrue(any(b.name==base and b.mdisp==offset and b.pdisp==-1 for b in classes[cls].bases))
        instructions={}
        for va,size in [(0x52d340,138),(0x52d3f0,103),(0x456650,58),(0x4598d0,455),(0x45aad0,771)]:
            instructions.update(decode(va,size))
        expected={
            0x52d37d:('mov','dword ptr [esi], 0x558dfc'),
            0x52d369:('lea','edi, [esi + 0x874]'),
            0x52d383:('mov','dword ptr [edi], 0x558d8c'),
            0x52d389:('mov','dword ptr [0x68aba4], esi'),
            0x52d425:('mov','dword ptr [0x68aba4], ecx'),
            0x456654:('mov','word ptr [eax + 4], cx'),
            0x456658:('mov','byte ptr [eax + 8], 0xff'),
            0x45665c:('mov','dword ptr [eax], 0x5524f8'),
            0x456680:('mov','dl, byte ptr [ecx + 0x599]'),
            0x456686:('mov','byte ptr [eax + 8], dl'),
            0x45994e:('mov','edx, dword ptr [esi + 0x38]'),
            0x459954:('mov','ecx, ebp'),0x459956:('add','ecx, edx'),
            0x459958:('call','0x4567a0'),
            0x4599eb:('mov','ecx, dword ptr [0x68aba4]'),
            0x4599f8:('call','0x4dc8c0'),0x459a08:('mov','edi, eax'),
            0x459a21:('push','eax'),0x459a22:('push','edi'),
            0x459a23:('add','ecx, dword ptr [esi + 0x38]'),0x459a26:('push','ecx'),
            0x459a27:('mov','ecx, dword ptr [0x68aba4]'),0x459a2d:('call','0x4dcac0'),
            0x45aba4:('mov','ecx, dword ptr [0x68aba4]'),0x45abaa:('call','0x4dd750'),
            0x45abaf:('mov','ecx, dword ptr [0x68aba4]'),0x45abb5:('call','0x4dd600'),
            0x45abba:('mov','edi, eax'),0x45ad91:('mov','ecx, dword ptr [0x68aba4]'),
            0x45ad97:('call','0x4dd600'),
            0x45abc6:('mov','al, byte ptr [edi + 8]'),0x45abc9:('mov','cl, byte ptr [esi + 0x599]'),
            0x45abcf:('cmp','al, cl'),0x45abd1:('jne','0x45ad91'),
        }
        for va,op in expected.items():self.assertEqual(instructions[va],op,hex(va))
        insert=decode(0x4dcac0,627)
        self.assertEqual({at:op[1]for at,op in insert.items()if op[0]=='call'},
                         {0x4dcc25:'dword ptr [eax + 4]',0x4dccbb:'0x4dd130',0x4dcd05:'0x4ddce0',0x4dcd11:'0x4ddce0'})
        self.assertEqual(insert[0x4dccf7],('ret','0x10'))
        self.assertEqual(insert[0x4dcd2e],('jmp','0x4dcc21'))
        self.assertEqual(insert[0x4dcaf0],('mov','word ptr [edx + 4], si'))
        pins=json.loads((ROOT/'tests/fixtures/eco_quadtree_objects.json').read_text())
        vc=Path(os.environ['VC6_ROOT']);link=find_child_ci(find_vc6_bin(vc),'link.exe')
        with tempfile.TemporaryDirectory(prefix='mcm2-eco-tree-')as directory:
            output=Path(directory);objects={};paths={}
            def compile_source(source,name,fixture=False):
                path=output/(name+'.obj')
                args=[sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),
                      '--vc6-root',str(vc),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2')]
                if fixture:args+=['--extra','/Od']
                r=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(r.returncode,0,r.stdout+r.stderr)
                return path,CoffObject(path)
            for index,source in enumerate((*SOURCES,ALLOCATOR)):
                paths[source],objects[source]=compile_source(ROOT/source,str(index))
                if source in SOURCES:
                    view=whole_object(objects[source]);pin=pins['objects'][source]
                    self.assertEqual((len(view),sum(len(r[-1])for r in view)),(pin['sections'],pin['relocations']))
                    self.assertEqual(hashlib.sha256(json.dumps(view,separators=(',',':')).encode()).hexdigest(),pin['sha256'],source)
            cases=[(c['source'],c['symbol'],int(c['target_va'],0),c['target_size'],ROOT/c['bindings'])for c in CASES if c['source']in (*SOURCES,ALLOCATOR)]
            for parent in ('src/krusty2','samples/physics'):
                for manifest in (ROOT/parent).rglob('targets.json'):
                    for target in json.loads(manifest.read_text()):
                        if 'source'not in target or target.get('expect')=='partial' or not target.get('bindings'):continue
                        source=(manifest.parent/target['source']).resolve().relative_to(ROOT).as_posix()
                        if source in SOURCES:
                            cases.append((source,target['candidate_symbol_contains'],int(target['target_va'],0),target['target_size'],manifest.parent/target['bindings']))
            cases=list(dict.fromkeys(cases))
            self.assertEqual(len(cases),120)
            for source,symbol,va,size,bindings in cases:
                self.assertTrue(compare(image,objects[source],symbol,va,size,bindings)['strict_exact'],(source,symbol))
            result=compare(image,objects[SOURCES[2]],INSERT,0x4dcac0,627,ROOT/'src/krusty2/broadphase/Quadtree.bindings.json')
            self.assertEqual((result['candidate_size'],result['strict_exact'],result['strict_match']['matching_positions']),(627,False,620))
            self.assertEqual([m['offset']for m in result['strict_match']['mismatches']],[99,103,513,515,517,519,521])
            inventory=[]
            for source in SOURCES[:2]:
                obj=objects[source]
                for r in obj.relocations:
                    if obj.section(r.section_number).name.startswith('.debug'):continue
                    name=obj.symbol_by_index[r.symbol_index].name
                    self.assertNotIn(name,pins['renames'])
                    if name in pins['renames'].values():
                        syms=[s for s in obj.symbols if s.section_number==r.section_number and s.type==32 and s.value<=r.virtual_address]
                        s=max(syms,key=lambda s:s.value)
                        inventory.append([source,s.name,name,r.virtual_address-s.value,r.type])
            self.assertEqual(sorted(inventory),sorted(pins['inventory']))
            layout=output/'layout.cpp'
            layout.write_text('#include "'+(ROOT/'src/krusty2/broadphase/Quadtree.h').as_posix()+'"\n#include <stddef.h>\n'+''.join(
                f'typedef char A{i}[({condition})?1:-1];\n'for i,condition in enumerate([
                    'sizeof(QuadTree)==0x874','sizeof(QuadTreeNode)==20','sizeof(QuadTreeItemLink)==20','sizeof(BlockAllocator)==40',
                    'offsetof(QuadTreeObject,queryStamp)==4','offsetof(QuadTreeObject,sortKey)==6','offsetof(QuadTreeObject,objectTypeId)==8',
                    'offsetof(QuadTree,rootNode)==0x5c','offsetof(QuadTree,itemPool)==0x60',
                    'offsetof(QuadTree,liveNodeCount)==0x854','offsetof(QuadTree,objectCount)==0x85c',
                    'offsetof(QuadTreeNode,children)==4','offsetof(QuadTreeNode,itemList)==8',
                    'offsetof(QuadTreeNode,yCenter)==12','offsetof(QuadTreeNode,yHalfExtent)==16',
                    'sizeof(void(QuadTree::*)(QuadTreeObject*,unsigned int,float,float))==4'])))
            compile_source(layout,'layout')
            eco_layout=output/'eco_layout.cpp'
            eco_layout.write_text('#include "'+(ROOT/'src/reconstructed/EcoSystem.h').as_posix()+'"\n#include <stddef.h>\n'+''.join(
                f'typedef char B{i}[({condition})?1:-1];\n'for i,condition in enumerate([
                    'sizeof(Vegetation)==28','sizeof(UnknownEcoQuadTreeObject)==12',
                    'offsetof(Vegetation,queryStamp)==4','offsetof(Vegetation,sortKey)==6',
                    'offsetof(Vegetation,typeId)==8','offsetof(Vegetation,geometryBlock)==24',
                    'offsetof(EcoSystem,vegetation)==0x38'])))
            compile_source(eco_layout,'eco_layout')
            fixture,_=compile_source(ROOT/'tests/fixtures/eco_quadtree_insert_compare.cpp','fixture',True)
            executable=output/'tree.exe'
            r=subprocess.run([str(link),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                              f'/out:{executable}',f'/libpath:{find_child_ci(infer_vc98_root(vc),"lib")}',
                              str(paths[SOURCES[2]]),str(paths[ALLOCATOR]),str(fixture),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            r=subprocess.run([str(executable),str(exe)],capture_output=True,text=True,timeout=120)
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)
            self.assertIn('QuadTree Insert: 13347 complete-body scenarios match',r.stdout)
            print(f'Eco QuadTree: {len(cases)} strict; four complete objects; {len(inventory)} import sites; '+r.stdout.strip())


if __name__=='__main__':unittest.main()
