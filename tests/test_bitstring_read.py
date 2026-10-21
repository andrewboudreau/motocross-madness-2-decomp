"""Complete global bit reader plus real-source LZW/allocator integration."""
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
from tools.build_game import compile_source as game_compile

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/render/BitStringRead.cpp'
CANONICAL = 'src/reconstructed/GR_BitString.cpp'
SYMBOL = '?ReadBits@GR_BitString@@QAEIH@Z'
GLOBALS = {'?g_LowBitMaskTable@@3PAIA','?g_BitReaderWord@@3IA',
           '?g_BitReaderBitsUsed@@3HA','?g_BitReaderCursor@@3PAIA'}


def complete_object(obj):
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    keys = {s.index: tuple(sorted((t.name,t.value) for t in obj.symbols
                                  if t.section_number==s.index and t.storage_class==2))
            or ('section',n,s.name) for n,s in enumerate(sections)}
    def target(symbol):
        return (keys[symbol.section_number],symbol.value) if symbol.section_number>0 else ('external',symbol.name,symbol.value,symbol.section_number)
    return [(keys[s.index],s.name,s.characteristics,
             (obj.data[s.raw_ptr:s.raw_ptr+s.raw_size] if s.raw_ptr else bytes(s.raw_size)).hex(),
             [(r.virtual_address,r.type,target(obj.symbol_by_index[r.symbol_index]))
              for r in obj.relocations if r.section_number==s.index]) for s in sections]


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail image')
class BitStringReadTests(unittest.TestCase):
    def test_complete_reader_actual_constructor_and_source_lzw(self):
        import capstone
        image=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail=PEImage(image)
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        decoder.detail=True
        # Complete reachable CFG, including the eager boundary load and RET4.
        todo=[0x423ef0];seen={}
        while todo:
            address=todo.pop()
            if address in seen:continue
            self.assertTrue(0x423ef0<=address<0x423f6d)
            ins=next(decoder.disasm(retail.bytes_at_va(address,15),address));seen[address]=ins
            self.assertNotEqual(ins.mnemonic,'call')
            if ins.mnemonic=='ret':
                self.assertEqual((address,ins.op_str),(0x423f6a,'4'));continue
            if ins.group(capstone.CS_GRP_JUMP):
                self.assertEqual(ins.operands[0].type,capstone.x86.X86_OP_IMM)
                todo.append(ins.operands[0].imm)
                if ins.mnemonic=='jmp':continue
            todo.append(address+ins.size)
        self.assertEqual(sum(i.size for i in seen.values()),125)
        self.assertEqual(max(a+i.size for a,i in seen.items()),0x423f6d)
        self.assertEqual(retail.bytes_at_va(0x423f6d,3),b'\x90'*3)
        self.assertEqual(seen[0x423f13].mnemonic,'jge')
        self.assertEqual(seen[0x423f30].op_str,'dword ptr [0x578e94], eax')
        self.assertEqual(seen[0x423f3a].op_str,'edx, dword ptr [edi]')
        self.assertEqual(seen[0x423f42].op_str,'dword ptr [0x578e98], edi')
        self.assertEqual(seen[0x423f48].op_str,'dword ptr [0x578e90], edx')
        # Raw primary identity and actual caller, independent of shared addresses.
        self.assertEqual(struct.unpack('<I',retail.bytes_at_va(0x550f4c,4))[0],0x55ae08)
        col=struct.unpack('<5I',retail.bytes_at_va(0x55ae08,20))
        self.assertEqual(col[:3],(0,0,0))
        self.assertEqual(retail.bytes_at_va(col[3]+8,19),b'.?AVGR_BitString@@\0')
        caller=list(decoder.disasm(retail.bytes_at_va(0x4a03d0,522),0x4a03d0))
        reader_calls=[i.address for i in caller if i.mnemonic=='call' and i.op_str=='0x423ef0']
        self.assertEqual(reader_calls,[0x4a041a,0x4a043b,0x4a0461,0x4a05a3])
        for va in (0x578e90,0x578e94,0x578e98):
            section=next(s for s in retail.sections if s.virtual_address<=va-retail.image_base<s.rva_end)
            offset=va-retail.image_base-section.virtual_address
            self.assertGreaterEqual(offset,section.raw_size)
            self.assertLessEqual(offset+4,section.virtual_size)
            self.assertTrue(section.characteristics&0x80000000)
        self.assertEqual(struct.unpack('<I',retail.bytes_at_va(0x568224,4))[0],0x5681a0)
        vc6=Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-bitstring-read-') as directory:
            output=Path(directory);objects={};paths={}
            sources=(SOURCE,CANONICAL,'samples/render/BitStringShifts.cpp',
                     'src/reconstructed/Lzw.cpp','src/reconstructed/MemTag.cpp',
                     'tests/fixtures/bitstring_read_compare.cpp','tests/fixtures/lzw_source_decode.cpp')
            for index,source in enumerate(sources):
                obj=output/('unit%d.obj'%index)
                command=[sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(obj),
                         '--vc6-root',str(vc6),'--profile','vc6_o2_mt']
                if source.startswith('tests/'):
                    command+=['--extra','/Od']
                result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                paths[source]=obj;objects[source]=CoffObject(obj)
            candidate=objects[SOURCE];entry=candidate.find_symbol(SYMBOL)
            self.assertEqual((entry.function_size,entry.extent_source),(157,'coff_function_aux'))
            body,size,relocs=candidate.symbol_extent(entry)
            self.assertEqual(size,157)
            self.assertEqual({candidate.symbol_by_index[r.symbol_index].name for r in relocs},GLOBALS)
            report=compare(retail,candidate,SYMBOL,0x423ef0,125,(ROOT/SOURCE).with_suffix('.bindings.json'))
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'],8)
            self.assertFalse(any(s.name.startswith('.CRT') for s in candidate.sections))
            strict=0
            for source in (CANONICAL,'src/reconstructed/Lzw.cpp','src/reconstructed/MemTag.cpp'):
                for case in CASES:
                    if case['source']!=source:continue
                    report=compare(retail,objects[source],case['symbol'],int(case['target_va'],16),
                                   case['target_size'],ROOT/case['bindings'])
                    self.assertTrue(report['strict_exact'],report);strict+=1
            self.assertEqual(strict,42)
            canonical=objects[CANONICAL]
            pin=json.loads((ROOT/'tests/fixtures/bitstring_read_canonical.json').read_text())
            rows=complete_object(canonical)
            self.assertEqual({'sections':len(rows),'relocations':sum(len(r[4])for r in rows),
                              'sha256':hashlib.sha256(json.dumps(rows).encode()).hexdigest()},pin)
            for name in GLOBALS-{'?g_LowBitMaskTable@@3PAIA'}:
                symbol=canonical.find_symbol(name);section=canonical.section(symbol.section_number)
                self.assertTrue(section.characteristics&0x80000000)
                self.assertTrue(section.characteristics&0x80);self.assertEqual(section.raw_ptr,0)
            symbol=canonical.find_symbol('?g_LowBitMasks@@3PAIA');section=canonical.section(symbol.section_number)
            data=canonical.data[section.raw_ptr+symbol.value:section.raw_ptr+symbol.value+132]
            self.assertEqual(data,retail.bytes_at_va(0x5681a0,132))
            self.assertEqual(data,struct.pack('<33I',*[(1<<n)-1 for n in range(33)]))
            self.assertTrue(section.characteristics&0x80000000)
            pointer=canonical.find_symbol('?g_LowBitMaskTable@@3PAIA')
            rs=[r for r in canonical.relocations if r.section_number==pointer.section_number and r.virtual_address==pointer.value]
            self.assertEqual(len(rs),1)
            self.assertEqual(canonical.symbol_by_index[rs[0].symbol_index].name,symbol.name)
            self.assertFalse(any(s.name.startswith('.CRT') for s in canonical.sections))
            spec=json.loads((ROOT/'config/game_build.json').read_text())
            build=game_compile(ROOT/SOURCE,output/'game',vc6,[ROOT/p for p in spec['includes']],
                               spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(build['compiled'],build)
            self.assertEqual(complete_object(candidate),complete_object(CoffObject(Path(build['object']))))
            linker=find_child_ci(find_vc6_bin(vc6),'link.exe')
            libraries=find_child_ci(infer_vc98_root(vc6),'lib')
            for fixture,extra,expected in (
                ('tests/fixtures/bitstring_read_compare.cpp',[], 'BitStringRead: 205632 scenarios PASS'),
                ('tests/fixtures/lzw_source_decode.cpp',['src/reconstructed/Lzw.cpp','src/reconstructed/MemTag.cpp'],
                 'LzwSourceDecode: 2051 packed-stream, dictionary, allocation and reader-state cases PASS')):
                executable=output/(Path(fixture).stem+'.exe')
                units=[SOURCE,CANONICAL,'samples/render/BitStringShifts.cpp',fixture]+extra
                result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                    f'/out:{executable}',f'/libpath:{libraries}',*[str(paths[s])for s in units],'kernel32.lib'],
                    cwd=ROOT,capture_output=True,text=True,encoding='latin1')
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                result=subprocess.run([str(executable),str(image)],capture_output=True,text=True,timeout=120)
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                self.assertIn(expected,result.stdout)
                print(result.stdout.strip())
            print('%d strict helper/initializer checks; canonical whole object and actual game compile preserved'%strict)


if __name__=='__main__':
    unittest.main()
