"""CollisionObject reads its existing TU-local initialized vector storage."""
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

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/krusty2/collision/CollisionObject.cpp',
           'samples/physics/collision/CollisionObjectNearMisses.cpp')
OLD = '?g_CollisionVec3_5797b0@@3UCollisionVec3@@A'


def object_view(obj, root=None, connect_old=False):
    """Keep every nondebug byte and destination; optional exact old storage join."""
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    files = {}
    if root is not None:
        prefix = str(root).replace('\\', '/') + '/'
        for symbol in obj.symbols:
            if symbol.name.startswith('??_C@') and symbol.section_number > 0:
                section = obj.section(symbol.section_number)
                raw = obj.data[section.raw_ptr+symbol.value:section.raw_ptr+section.raw_size]
                value, nul, tail = raw.partition(b'\0')
                path = value.decode('latin1').replace('\\', '/')
                if nul and path.lower().startswith(prefix.lower()) and path[len(prefix):] in SOURCES:
                    suffix = path[len(prefix):]
                    files[section.index] = ('source:'+suffix, ('source:'+suffix).encode()+nul+tail)
    def name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number in files:
            return files[symbol.section_number][0]
        return symbol.name
    keys, anonymous = {}, {}
    for section in sections:
        labels = tuple(sorted((name(s), s.value) for s in obj.symbols
                              if s.section_number == section.index and (s.storage_class == 2 or s.type == 32)))
        if labels:
            keys[section.index] = ('named', labels)
        else:
            index = anonymous.get(section.name, 0)
            anonymous[section.name] = index+1
            keys[section.index] = ('anonymous', section.name, index)
    assert len(set(keys.values())) == len(keys)
    zero = obj.find_symbol('_kVec3Zero')
    def target(symbol):
        if connect_old and symbol.name == OLD and symbol.section_number == 0:
            assert symbol.value == 0 and symbol.storage_class == 2
            return ('defined', keys[zero.section_number], zero.value)
        if symbol.section_number > 0:
            return ('defined', keys[symbol.section_number], symbol.value)
        return ('external', symbol.name, symbol.value, symbol.section_number)
    rows = []
    for section in sections:
        data = obj.data[section.raw_ptr:section.raw_ptr+section.raw_size] if section.raw_ptr else bytes(section.raw_size)
        if section.index in files:
            data = files[section.index][1]
        rows.append((keys[section.index], section.characteristics, data.hex(),
                     [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                      for r in obj.relocations if r.section_number == section.index]))
    return sorted(rows, key=lambda row: repr(row[0]))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail')
class CollisionObjectZeroTests(unittest.TestCase):
    def test_existing_vector_full_initializers_and_both_constructor_copies(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        section = retail.section_for_rva(0x5797b0-retail.image_base)
        offset = 0x5797b0-retail.image_base-section.virtual_address
        self.assertTrue(section.characteristics & 0x80000000)
        self.assertGreaterEqual(offset, section.raw_size)
        self.assertLessEqual(offset+12, section.virtual_size)
        refs = {0x5797b0: [0x431f77, 0x431f9d, 0x435728],
                0x5797b4: [0x431f85, 0x431fa5, 0x43572e],
                0x5797b8: [0x431f8e, 0x431fae, 0x435734]}
        for address, expected in refs.items():
            offsets, start = [], 0
            while True:
                at = retail.data.find(struct.pack('<I', address), start)
                if at < 0:
                    break
                offsets.append(retail.offset_to_va(at)); start = at+1
            self.assertEqual(offsets, expected)
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        constructor = {i.address: (i.mnemonic, i.op_str) for i in md.disasm(retail.bytes_at_va(0x431e70, 346), 0x431e70)}
        for address, expected in {
            0x431e91: ('lea', 'ebp, [esi + 0xc]'),
            0x431eae: ('mov', 'dword ptr [esi], 0x5511b8'),
            0x431eb4: ('mov', 'dword ptr [ebp], 0x551148'),
            0x431f7b: ('lea', 'ecx, [esi + 0xa0]'),
            0x431f92: ('lea', 'eax, [esi + 0xac]'),
            0x431f82: ('mov', 'dword ptr [ecx], edx'),
            0x431f89: ('mov', 'dword ptr [ecx + 4], eax'),
            0x431f98: ('mov', 'dword ptr [ecx + 8], edx'),
            0x431fa1: ('mov', 'dword ptr [eax], ecx'),
            0x431fa9: ('mov', 'dword ptr [eax + 4], edx'),
            0x431fb2: ('mov', 'dword ptr [eax + 8], ecx'),
        }.items():
            self.assertEqual(constructor[address], expected)
        self.assertEqual(retail.bytes_at_va(0x5660ec, 4), struct.pack('<I', 0x4356f0))
        self.assertEqual(retail.bytes_at_va(0x4356f0, 5), b'\xe9\x0b\0\0\0')
        pins = json.loads((ROOT/'tests/fixtures/collision_object_zero_objects.json').read_text())
        targets = [('??0CollisionObject', 0x431e70, 346)]
        targets += [(symbol, va, n) for symbol, va, n in (
            ('_$E2',0x4356f0,5),('_$E1',0x435700,60),('_$E5',0x435740,5),('_$E4',0x435750,60),
            ('_$E8',0x435790,5),('_$E7',0x4357a0,60),('_$E11',0x4357e0,5),('_$E10',0x4357f0,60))]
        strict = 0
        with tempfile.TemporaryDirectory(prefix='mcm2-collision-zero-') as directory:
            out = Path(directory)
            def compile_source(source, name):
                obj = out/(name+'.obj')
                result = subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(obj),
                    '--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2')],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                return CoffObject(obj)
            for index, source in enumerate(SOURCES):
                obj = compile_source(ROOT/source, str(index))
                rows = object_view(obj, ROOT)
                actual = dict(sections=len(rows),relocations=sum(len(r[3]) for r in rows),
                              sha256=hashlib.sha256(json.dumps(rows).encode()).hexdigest())
                self.assertEqual(actual, pins['objects'][source])
                self.assertFalse(any(s.name == OLD for s in obj.symbols))
                zero = obj.find_symbol('_kVec3Zero'); bss = obj.section(zero.section_number)
                self.assertEqual((zero.value,zero.storage_class,bss.name,bss.raw_size,bss.raw_ptr,bss.characteristics),
                                 (0,3,'.bss',60,0,0xc0400080))
                crt = [s for s in obj.sections if s.name == '.CRT$XCU']
                self.assertEqual(len(crt),1)
                self.assertEqual((crt[0].raw_size,crt[0].characteristics),(16,0xc0300040))
                self.assertEqual(obj.data[crt[0].raw_ptr:crt[0].raw_ptr+16],bytes(16))
                self.assertEqual([(r.virtual_address,r.type,obj.symbol_by_index[r.symbol_index].name)
                                  for r in obj.relocations if r.section_number == crt[0].index],
                                 [(0,6,'_$E2'),(4,6,'_$E5'),(8,6,'_$E8'),(12,6,'_$E11')])
                for symbol, expected_addends in (('_$E1',[0,4,8]),('??0CollisionObject',[0,4,8,0,4,8])):
                    entry = obj.find_symbol(symbol); body, size, rels = obj.symbol_extent(entry)
                    zero_rels = [r for r in rels if obj.symbol_by_index[r.symbol_index].name == '_kVec3Zero']
                    self.assertEqual([r.type for r in zero_rels],[6]*len(expected_addends))
                    self.assertEqual([struct.unpack_from('<I',body,r.virtual_address-entry.value)[0]
                                      for r in zero_rels],expected_addends)
                all_refs = [r for r in obj.relocations if not obj.section(r.section_number).name.startswith('.debug')
                            and obj.symbol_by_index[r.symbol_index].name == '_kVec3Zero']
                self.assertEqual(len(all_refs),9)
                for symbol, va, size in targets:
                    result = compare(retail,obj,symbol,va,size,(ROOT/SOURCES[0]).with_suffix('.bindings.json'))
                    self.assertTrue(result['strict_exact'],result); strict += 1
            layout = out/'layout.cpp'
            layout.write_text('#include "collision/CollisionObject.h"\n#include "math/Math3D.h"\n#include <stddef.h>\n'+
                '\n'.join('typedef char Check%d[(%s)?1:-1];' % (i, expression) for i, expression in enumerate((
                    'sizeof(Vec3)==12','sizeof(CollisionVec3)==12','sizeof(CollisionObject)==0xb8',
                    'offsetof(Vec3,x)==0','offsetof(Vec3,y)==4','offsetof(Vec3,z)==8',
                    'offsetof(CollisionVec3,x)==0','offsetof(CollisionVec3,y)==4','offsetof(CollisionVec3,z)==8',
                    'offsetof(CollisionObject,hitPoint)==0xa0','offsetof(CollisionObject,hitNormal)==0xac')))+'\n')
            compile_source(layout,'layout')
        self.assertEqual(strict,18)
        print('CollisionObject zero:18 strict checks; two full objects; existing private BSS and CRT; nine references')


if __name__ == '__main__':
    unittest.main()
