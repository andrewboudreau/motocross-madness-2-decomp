"""Rider motion-start joins preserve full caller objects and the primary receiver."""
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
from tools.match import compare

ROOT=Path(__file__).resolve().parents[1]
START='?Method_0x004a8bf0@Character@@QAEXHM@Z'
RETIRED={'?Method_0x004a8bf0@BikeA5C4@@QAEXHM@Z','?Fn_004A8BF0@KbA5C4@@QAEXHM@Z'}
SOURCES={
 # Emitter joins preserve every prior byte/target under the verified constructor and Spark setter names.
 'src/krusty2/vehicle/Bike.cpp':(242,1089,3,'fbe939d9b6beb97aff44dc9c83cd5049917fbf5d1c508ac52bccf333d6fca6cd'),
 # Verified collision/debug-overlay connections change only three external names.
 'src/krusty2/vehicle/KrustyBike.cpp':(243,1054,2,'ac4bce1e5b9d6559b6be6c50d9f51d31b06d611090c921de5d479935f9ea054c'),
 'samples/physics/krustybike/KrustyBikeNearMisses.cpp':(271,1443,8,'d3ef0d93837a2b668ca6efbb9070adfb3f48aa88b67a32fdc251cb08c13eb7f4'),
}

def view(obj, normalize_paths=()):
    """Complete sections and destinations, with known source paths made portable."""
    sections = [section for section in obj.sections if not section.name.startswith('.debug')]
    prefixes = [str(path).replace('\\', '/') + '/' for path in normalize_paths]

    def filename(data):
        value, separator, tail = data.partition(b'\0')
        text = value.decode('latin1').replace('\\', '/')
        for prefix in prefixes:
            if separator and text.lower().startswith(prefix.lower()) and text[len(prefix):] in SOURCES:
                suffix = text[len(prefix):]
                return ('source:' + suffix).encode() + separator + tail, suffix
        return data, None

    def symbol_name(symbol):
        if symbol.name.startswith('??_C@') and symbol.section_number > 0:
            section = obj.sections[symbol.section_number - 1]
            data = obj.data[section.raw_ptr + symbol.value:section.raw_ptr + section.raw_size]
            normalized, suffix = filename(data)
            if suffix:
                return normalized.split(b'\0')[0].decode()
        return symbol.name

    keys, anonymous = {}, {}
    for section in sections:
        labels = tuple(sorted((symbol_name(symbol), symbol.value) for symbol in obj.symbols
                              if symbol.section_number == section.index and
                              (symbol.storage_class == 2 or symbol.type == 32)))
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

    rows, filenames = [], []
    for section in sections:
        data = (obj.data[section.raw_ptr:section.raw_ptr + section.raw_size]
                if section.raw_ptr else bytes(section.raw_size))
        data, suffix = filename(data)
        if suffix:
            filenames.append((keys[section.index], suffix))
        relocations = [(relocation.virtual_address, relocation.type,
                        target(obj.symbol_by_index[relocation.symbol_index]))
                       for relocation in obj.relocations if relocation.section_number == section.index]
        rows.append((keys[section.index], section.characteristics, data.hex(), relocations))
    return sorted(rows, key=lambda row: repr(row[0])), filenames


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class RiderMotionStartConnectionsTests(unittest.TestCase):
    def test_complete_callers_primary_receiver_and_real_import(self):
        import capstone
        image=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail=PEImage(image)
        word=lambda va:struct.unpack('<I',retail.bytes_at_va(va,4))[0]
        self.assertEqual(word(0x55152c),0x55b368)
        self.assertEqual(word(0x55b368+4),0)
        self.assertEqual(retail.bytes_at_va(word(0x55b368+12)+8,40).split(b'\0')[0],b'.?AVD3DIMSoultreeCharacter@@')
        character=next(c for c in parse_rtti(retail) if c.name=='D3DIMSoultreeCharacter')
        bases={b.name:(b.mdisp,b.pdisp,b.vdisp)for b in character.bases}
        self.assertEqual(bases['Character'],(0,-1,0))
        self.assertEqual(bases['GameObject'],(0,4,4))
        self.assertEqual(word(0x5514bc),0x55b328)
        self.assertEqual(word(0x55b328+4),532)
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        decoded={i.address:(i.mnemonic,i.op_str)for va,size in ((0x4079c0,6745),(0x48e280,339))
                 for i in decoder.disasm(retail.bytes_at_va(va,size),va)}
        for va,expected in {
            0x407c6c:('call','0x4455b0'),0x407c84:('mov','dword ptr [ebp + 0x5c4], eax'),
            0x4092f8:('mov','ecx, dword ptr [ebp + 0x670]'),0x4092fe:('push','0x3f000000'),
            0x409303:('push','ecx'),0x409304:('mov','ecx, dword ptr [ebp + 0x5c4]'),
            0x40930a:('call','0x4a8bf0'),0x48e399:('mov','ecx, dword ptr [esi + 0x6a4]'),
            0x48e39f:('push','0x3f000000'),0x48e3a4:('push','ecx'),
            0x48e3a5:('mov','ecx, dword ptr [esi + 0x5c4]'),0x48e3b5:('call','0x4a8bf0'),
        }.items():self.assertEqual(decoded[va],expected)
        with tempfile.TemporaryDirectory(prefix='mcm2-rider-start-') as directory:
            for index,(source,expected) in enumerate(SOURCES.items()):
                output=Path(directory)/('unit%d.obj'%index)
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(output),
                    '--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2')],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                obj=CoffObject(output);rows,filenames=view(obj,(ROOT,))
                self.assertTrue(all(suffix in SOURCES for _,suffix in filenames))
                self.assertEqual((len(rows),sum(len(r[3])for r in rows),
                                  hashlib.sha256(json.dumps(rows).encode()).hexdigest()),
                                 (expected[0],expected[1],expected[3]),source)
                imports={s.name for s in obj.symbols if s.section_number==0}
                self.assertIn(START,imports);self.assertFalse(imports&RETIRED)
                calls=[r for r in obj.relocations if not obj.sections[r.section_number-1].name.startswith('.debug')
                       and obj.symbol_by_index[r.symbol_index].name==START]
                self.assertEqual(len(calls),expected[2]);self.assertTrue(all(r.type==20 for r in calls))
                if source!='src/krusty2/vehicle/Bike.cpp':
                    self.assertTrue(compare(retail,obj,'Fn_0048E280',0x48e280,339,
                        ROOT/Path(source).with_suffix('.bindings.json'))['strict_exact'])
            print('Two strict339-byte callers; three complete objects,756 sections3586 relocations; primary rider imports preserved')


if __name__=='__main__':
    unittest.main()
