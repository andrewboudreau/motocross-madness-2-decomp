"""Existing primary palette objects retain their scalar lookup-table call boundaries."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES
from test_edit_box_on_char import extract

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/reconstructed/Pixtrans.cpp', 'samples/render/PixtransNearMisses.cpp')
FILENAMES = SOURCES
PROOFS = ('src/reconstructed/Quantize.cpp', 'src/reconstructed/Palette8.cpp',
          'src/reconstructed/TextureMap.cpp', 'src/reconstructed/PCTextureMap.cpp')
RENAMES = {'?GetIndexFrom%s@UnknownTexturePalette@@QAEPAEXZ' % n:
           '?GetIndexFrom%s@ColorMapper@@QAEPAEXZ' % n for n in ('555', '565')}
NATIVE = ((SOURCES[0], 'int Convert24To8(', '?Convert24To8@@', 0x4d0aa0, 232),
          (SOURCES[0], 'int Convert565To8(', '?Convert565To8@@', 0x4d0b90, 171),
          (PROOFS[0], 'unsigned char* ColorMapper::GetIndexFrom555(', '?GetIndexFrom555@ColorMapper@@', 0x4de280, 7),
          (PROOFS[0], 'unsigned char* ColorMapper::GetIndexFrom565(', '?GetIndexFrom565@ColorMapper@@', 0x4de290, 7))


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


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and hash-pinned private retail')
class PaletteGetterConnections(unittest.TestCase):
    def test_complete_objects_primary_producers_and_scalar_converters(self):
        import capstone
        image_path = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(image_path)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        def decode(va, size):
            code = list(decoder.disasm(image.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in code), size)
            return {i.address:(i.mnemonic, i.op_str) for i in code}
        for vt, col, td, name in ((0x557658,0x55eaf0,0x5720a8,b'.?AVColorMapper@@'),
                                  (0x555aa4,0x55ded0,0x56f7f0,b'.?AVPalette8@@')):
            self.assertEqual(struct.unpack('<I',image.bytes_at_va(vt-4,4))[0],col)
            self.assertEqual(struct.unpack('<5I',image.bytes_at_va(col,20))[1:4],(0,0,td))
            self.assertEqual(image.bytes_at_va(td+8,len(name)+1),name+b'\0')
        # Allocation -> complete primary ColorMapper -> Palette8's mapper field.
        expected = {
            0x4dde4b:('push','0x18b10'), 0x4dde6a:('call','0x4ddf40'),
            0x4ddf7d:('mov','dword ptr [esi], 0x557658'),
            0x4b6e30:('call','0x4dddd0'), 0x4b6e38:('mov','dword ptr [esi + 0x708], eax'),
            0x50a758:('mov','ecx, dword ptr [ebp + 0x708]'),
            0x50a90f:('mov','edx, dword ptr [ebp + 0x708]'),
            0x50aa68:('mov','ecx, dword ptr [esp + 0x3c]'), 0x50aa6c:('push','ecx'),
            0x50aa85:('call','dword ptr [edx + 0x10]'),
            0x50aaca:('mov','edx, dword ptr [esp + 0x30]'), 0x50aad3:('push','edx'),
            0x50aaee:('call','dword ptr [eax + 0x14]'),
            0x4c64f3:('mov','edx, dword ptr [esp + 0x114]'), 0x4c64ff:('push','edx'),
            0x4c651c:('call','0x4d1d20'), 0x4c6523:('add','esp, 0x34'),
            0x4d1dce:('mov','eax, dword ptr [esp + 0x28]'), 0x4d1dda:('push','eax'),
            0x4d1df6:('call','0x4d0b90'),
            0x4d1f0c:('mov','ecx, dword ptr [esp + 0x28]'), 0x4d1f18:('push','ecx'),
            0x4d1f34:('call','0x4d0c40'),
            0x4d212e:('mov','edx, dword ptr [esp + 0x28]'), 0x4d213a:('push','edx'),
            0x4d2156:('call','0x4d0aa0'),
        }
        instructions = {}
        for va,size in ((0x4dddd0,201),(0x4ddf40,689),(0x4b6c00,630),
                        (0x50a590,1442),(0x4c6080,2129),(0x4d1d20,1964)):
            instructions.update(decode(va,size))
        for at,instruction in expected.items():
            self.assertEqual(instructions[at],instruction,(hex(at),instructions[at]))
        self.assertEqual(struct.unpack('<2I',image.bytes_at_va(0x555fec+4*4,8)),(0x4c69c0,0x4c6080))
        # The reviewed-nonexact CreateSurface is only decoded here, not counted strict.
        self.assertEqual(decode(0x4c69ca,7)[0x4c69ca],('mov','edx, dword ptr [esp + 0xe4]'))
        self.assertEqual(decode(0x4c69fe,3)[0x4c69fe],('mov','dword ptr [ebp + 0x2c], edx'))
        callers = (('?Convert24To8@@',0x4d0aa0,232,0x4de280,[0x4d0afa]),
                   ('?Convert565To8@@',0x4d0b90,171,0x4de290,[0x4d0be4]),
                   ('?Convert555To8@@',0x4d0c40,242,0x4de280,[0x4d0cac,0x4d0ce1]))
        for symbol,va,size,target,sites in callers:
            code=decode(va,size)
            self.assertEqual([at for at,op in code.items() if op==('call',hex(target))],sites)
            self.assertEqual(code[va+4],('mov','ecx, dword ptr [esp + 0x20]'))
        # Sample receivers are the unchanged seventh/eleventh caller arguments.
        for va,size,expected_ops in (
            (0x4cee30,41,{0x4cee34:('mov','ebx, dword ptr [esp + 0x68]'),
                           0x4cee52:('mov','ecx, ebx'),0x4cee54:('call','0x4de280')}),
            (0x4cf2a0,39,{0x4cf2b0:('mov','esi, dword ptr [esp + 0x90]'),
                           0x4cf2c0:('mov','ecx, esi'),0x4cf2c2:('call','0x4de280')})):
            code=decode(va,size)
            for at,op in expected_ops.items():self.assertEqual(code[at],op)
        for va,offset in ((0x4de280,0x710),(0x4de290,0x8710)):
            self.assertEqual(list(decode(va,7).values()),[('lea','eax, [ecx + %s]'%hex(offset)),('ret','')])

        compiler=Path(os.environ['VC6_ROOT'])
        pins=json.loads((ROOT/'tests/fixtures/palette_getter_objects.json').read_text())
        strict=0
        with tempfile.TemporaryDirectory(prefix='mcm2-palette-') as directory:
            output=Path(directory)
            def compile_source(source,stem,fixture=False):
                path=output/(stem+'.obj')
                command=[sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),
                         '--vc6-root',str(compiler),'--profile','vc6_o2_mt']
                if fixture:command+=['--include',str(ROOT/'src/reconstructed')]
                result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,encoding='latin1',
                                      env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return path,CoffObject(path)
            objects={}
            for i,source in enumerate(SOURCES+PROOFS):
                _,obj=compile_source(ROOT/source,'unit%d'%i);objects[source]=obj
                if source in SOURCES:
                    rows=whole_object(obj)
                    self.assertEqual((len(rows),sum(len(r[3])for r in rows)),
                                     (pins['objects'][source]['sections'],pins['objects'][source]['relocations']))
                    self.assertEqual(hashlib.sha256(json.dumps(rows).encode()).hexdigest(),pins['objects'][source]['sha256'])
                    self.assertFalse(any(s.name in RENAMES for s in obj.symbols))
                for case in CASES:
                    if case['source']!=source:continue
                    proof=compare(image,obj,case['symbol'],int(case['target_va'],0),case['target_size'],ROOT/case['bindings'])
                    self.assertTrue(proof['strict_exact'],proof);strict+=1
            for source,rows in pins['getter_calls'].items():
                obj=objects[source]
                found=[]
                for row in rows:
                    entry=obj.find_symbol(row['caller']);body,size,relocs=obj.symbol_extent(entry)
                    found.extend((entry.name,r.virtual_address-entry.value,obj.symbol_by_index[r.symbol_index].name)
                                 for r in relocs if obj.symbol_by_index[r.symbol_index].name in RENAMES.values())
                self.assertEqual(sorted(set(found)),sorted((r['caller'],r['offset'],r['callee'])for r in rows))
            # Whole sample pins above include every complete nonexact body, not selected windows.
            native=output/'providers.cpp'
            native.write_text('#include "Pixtrans.h"\n#include "TextureMap.h"\n#include "Quantize.h"\n'+
                              '\n'.join(extract((ROOT/src).read_text(),sig)for src,sig,_,_,_ in NATIVE)+'\n')
            native_path,obj=compile_source(native,'providers',True)
            def function(obj,query):
                entry=obj.find_symbol(query);body,size,rels=obj.symbol_extent(entry)
                return body,size,[(r.virtual_address-entry.value,r.type,obj.symbol_by_index[r.symbol_index].name)for r in rels]
            for source,_,query,va,size in NATIVE:
                self.assertEqual(function(obj,query),function(objects[source],query))
                self.assertTrue(compare(image,obj,query,va,size,ROOT/Path(source).with_suffix('.bindings.json'))['strict_exact'])
                strict+=1
            self.assertEqual({s.name for s in obj.symbols if s.section_number==0 and s.storage_class==2},
                             {'?DitherConvert@@YAHPAXHHHHH0000PAUUnknownTexturePalette@@@Z'})
            self.assertFalse(any(s.raw_size and (s.name.startswith('.CRT')or s.characteristics&0xc0)
                                 for s in obj.sections if not s.name.startswith('.debug')))
            fixture_path,_=compile_source(ROOT/'tests/fixtures/palette_getter_compare.cpp','fixture',True)
            executable=output/'palette.exe'
            result=subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')),
                '/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref','/out:'+str(executable),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler),'lib')),
                str(native_path),str(fixture_path),'kernel32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image_path)],capture_output=True,text=True,timeout=90)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),'Palette getters: 8139 cases, 22 access faults, scalar buffers and aliases match')
            self.assertEqual(strict, 103)
            print('%d strict checks; two complete objects; six calls; '%strict+result.stdout.strip())


if __name__=='__main__':
    unittest.main()
