"""Existing CollisionPoint storage keeps all bytes, CRT writes and consumer targets."""
from collections import Counter
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
from tools.match import compare

ROOT=Path(__file__).resolve().parents[1]
POINT='src/krusty2/collision/CollisionPoint.cpp'
SAMPLE='samples/physics/collision/CollisionPointNearMisses.cpp'
ZERO='?kCollisionPointZero@@3UCollisionPointConstVec3@@B'
AXIS='?kCollisionPointYAxis@@3UCollisionPointConstVec3@@B'
OLD_ZERO='?g_CollisionZeroVec3@@3UCollisionVec3@@A'
OLD_AXIS='?g_CollisionVec3_579810@@3UCollisionVec3@@A'


def object_rows(obj, root):
    """Stable complete sections; only the two evidenced storage joins normalize.

    Public storage moves the BSS section earlier in COFF. Defined symbol identity
    and exact offsets retain destinations independently of disposable section IDs.
    Initializer function names remain exact; anonymous sections retain kind/order.
    Complete recognized __FILE__ prefixes are checkout-relative, never code bytes.
    """
    sections=[s for s in obj.sections if not s.name.startswith('.debug')]
    names={ZERO:'_kCollisionPointZero',AXIS:'_kCollisionPointYAxis'}
    prefix=str(root).replace('\\','/')+'/'
    def filename(data):
        value,separator,tail=data.partition(b'\0')
        name=value.decode('latin1').replace('\\','/')
        if separator and name.lower().startswith(prefix.lower()):
            suffix=name[len(prefix):]
            if suffix==POINT:
                return ('source:'+suffix).encode()+separator+tail
        return data
    def name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number>0:
            sec=obj.sections[symbol.section_number-1]
            data=obj.data[sec.raw_ptr+symbol.value:sec.raw_ptr+sec.raw_size]
            converted=filename(data)
            if converted!=data:
                return converted.split(b'\0')[0].decode()
        return names.get(symbol.name,symbol.name)
    keys={};anonymous={}
    for section in sections:
        labels=tuple(sorted((name(s),s.value) for s in obj.symbols if s.section_number==section.index and
                     (s.storage_class==2 or s.type==32 or s.name.startswith('_kCollisionPoint'))))
        if labels:
            keys[section.index]=('named',labels)
        else:
            count=anonymous.get(section.name,0);anonymous[section.name]=count+1
            keys[section.index]=('anonymous',section.name,count)
    assert len(set(keys.values()))==len(keys)
    bss=next(s for s in sections if s.name=='.bss')
    def target(symbol):
        if symbol.name in (OLD_ZERO,OLD_AXIS) and symbol.section_number==0:
            return ('defined',keys[bss.index],32 if symbol.name==OLD_ZERO else 0)
        if symbol.section_number>0:
            return ('defined',keys[symbol.section_number],symbol.value)
        return ('external',symbol.name,symbol.value,symbol.section_number)
    rows=[]
    for section in sections:
        data=obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        relocations=[(r.virtual_address,r.type,target(obj.symbol_by_index[r.symbol_index]))
                     for r in obj.relocations if r.section_number==section.index]
        rows.append((keys[section.index],section.characteristics,filename(data).hex(),relocations))
    return sorted(rows,key=lambda r:repr(r[0]))


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail')
class CollisionPointStorageTests(unittest.TestCase):
    def test_full_objects_existing_storage_and_initializers(self):
        image=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail=PEImage(image)
        # Both objects occupy writable, unbacked PE storage before CRT startup.
        for va in (0x5797f0,0x579810):
            rva=va-retail.image_base
            section=next(s for s in retail.sections if s.virtual_address<=rva<s.rva_end)
            self.assertTrue(section.characteristics&0x80000000)
            self.assertGreaterEqual(rva-section.virtual_address,section.raw_size)
            self.assertLessEqual(rva-section.virtual_address+12,section.virtual_size)
        targets=[t for t in json.loads((ROOT/'src/krusty2/collision/targets.json').read_text())
                 if t.get('source')=='CollisionPoint.cpp']
        self.assertEqual(len(targets),16)
        baseline={POINT:(30,111,'b12c78bc10b13113f1f4a29ee39b8b6b68cf162928567171d769a4a8fbd08046'),
                  SAMPLE:(32,128,'40f2d5bcda818a1cb92d8cea44c1aaf4abff65c2eb6b9e40c9cfcabc35b92ff9')}
        with tempfile.TemporaryDirectory(prefix='mcm2-contact-storage-') as directory:
            for index,source in enumerate((POINT,SAMPLE)):
                output=Path(directory)/('unit%d.obj'%index)
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(output),
                    '--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),
                    '--include',str(ROOT/'src/reconstructed')],cwd=ROOT,capture_output=True,text=True,encoding='latin1',
                    env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                obj=CoffObject(output);rows=object_rows(obj,ROOT)
                self.assertEqual((len(rows),sum(len(x[3]) for x in rows),
                                  hashlib.sha256(json.dumps(rows).encode()).hexdigest()),baseline[source])
                self.assertFalse(any(s.name in (OLD_ZERO,OLD_AXIS) for s in obj.symbols))
                self.assertTrue(any(s.name=='?Vec3Normalize@@YAPAUCollisionVec3@@PAU1@PBU1@@Z'
                                    and s.section_number==0 for s in obj.symbols))
                bss=next(s for s in obj.sections if s.name=='.bss')
                self.assertEqual((bss.raw_size,bss.raw_ptr,bss.characteristics),(60,0,0xc0400080))
                for symbol,offset,storage in ((ZERO,32,2),(AXIS,0,2),('_kCollisionPointXAxis',48,3),
                                             ('_kCollisionPointZAxis',16,3)):
                    entry=obj.find_symbol(symbol)
                    self.assertEqual((entry.section_number,entry.value,entry.storage_class),(bss.index,offset,storage))
                crt=[s for s in obj.sections if s.name=='.CRT$XCU']
                self.assertEqual(len(crt),1)
                self.assertEqual((crt[0].raw_size,crt[0].characteristics),(16,0xc0300040))
                self.assertEqual(obj.data[crt[0].raw_ptr:crt[0].raw_ptr+16],bytes(16))
                self.assertEqual([(r.virtual_address,r.type,obj.symbol_by_index[r.symbol_index].name)
                                  for r in obj.relocations if r.section_number==crt[0].index],
                                 [(0,6,'_$E2'),(4,6,'_$E5'),(8,6,'_$E8'),(12,6,'_$E11')])
                for initializer,symbol in (('_$E1',ZERO),('_$E4','_kCollisionPointXAxis'),
                                          ('_$E7',AXIS),('_$E10','_kCollisionPointZAxis')):
                    entry=obj.find_symbol(initializer);body,size,relocations=obj.symbol_extent(entry)
                    self.assertEqual(size,60)
                    self.assertEqual([(r.type,obj.symbol_by_index[r.symbol_index].name,
                                       struct.unpack_from('<I',body,r.virtual_address-entry.value)[0]) for r in relocations],
                                     [(6,symbol,0),(6,symbol,4),(6,symbol,8)])
                references=Counter(obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                                   if not obj.sections[r.section_number-1].name.startswith('.debug'))
                self.assertEqual((references[ZERO],references[AXIS]),(39 if source==POINT else 47,6))
                for target in targets:
                    proof=compare(retail,obj,target['candidate_symbol_contains'],int(target['target_va'],0),
                                  target['target_size'],(ROOT/POINT).with_suffix('.bindings.json'))
                    self.assertTrue(proof['strict_exact'],proof)
            print('32 strict checks; two complete objects,62 sections,239 relocations; actual shared BSS/CRT preserved')


if __name__=='__main__':
    unittest.main()
