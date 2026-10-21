"""Complete bit-string shift bodies, unchanged aggregate, and real wrapper integration."""
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

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'samples/render/BitStringShifts.cpp'
AGGREGATE = 'samples/render/GR_BitStringNearMisses.cpp'
CANONICAL = 'src/reconstructed/GR_BitString.cpp'
FIXTURE = 'tests/fixtures/bitstring_shifts_compare.cpp'
MASK = '?g_WordHighBitMasks@@3PAIA'
TARGETS = (
 ('?ShiftBitsTowardFront@GR_BitString@@QAEXH@Z',0x4239c0,420,420,343,(0x423b48,0x423b61)),
 ('?ShiftBitsTowardBack@GR_BitString@@QAEXH@Z',0x423b70,442,438,93,(0x423d0c,0x423d27)),
)

def object_fingerprint(obj, renames=None):
    renames = renames or {}
    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
    keys = {s.index: tuple(sorted(t.name for t in obj.symbols
                                  if t.section_number == s.index and t.storage_class == 2))
            or ('section', n, s.name) for n, s in enumerate(sections)}
    def target(symbol):
        if symbol.section_number > 0:
            return (keys[symbol.section_number], symbol.value)
        return ('external', renames.get(symbol.name, symbol.name), symbol.value, symbol.section_number)
    rows = []
    for section in sections:
        data = (obj.data[section.raw_ptr:section.raw_ptr + section.raw_size]
                if section.raw_ptr else bytes(section.raw_size))
        relocations = [(r.virtual_address, r.type, target(obj.symbol_by_index[r.symbol_index]))
                       for r in obj.relocations if r.section_number == section.index]
        rows.append((keys[section.index], section.name, section.characteristics, data.hex(), relocations))
    return {'sections': len(rows), 'relocations': sum(len(row[4]) for row in rows),
            'sha256': hashlib.sha256(json.dumps(rows).encode()).hexdigest()}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and hash-pinned private retail')
class BitStringShiftsTests(unittest.TestCase):
    def test_complete_candidates_callers_masks_and_native_behavior(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        decoder.detail = True
        for symbol, va, size, emitted, score, returns in TARGETS:
            todo, seen = [va], {}
            while todo:
                address = todo.pop()
                if address in seen:
                    continue
                self.assertTrue(va <= address < va+size)
                instruction = next(decoder.disasm(retail.bytes_at_va(address,15),address))
                seen[address] = instruction
                self.assertNotEqual(instruction.mnemonic,'call')
                if instruction.mnemonic == 'ret':
                    self.assertEqual(instruction.op_str,'4')
                    continue
                if instruction.group(capstone.CS_GRP_JUMP):
                    self.assertEqual(instruction.operands[0].type,capstone.x86.X86_OP_IMM)
                    todo.append(instruction.operands[0].imm)
                    if instruction.mnemonic == 'jmp':
                        continue
                todo.append(address+instruction.size)
            self.assertEqual(tuple(a for a,i in sorted(seen.items()) if i.mnemonic=='ret'),returns)
            self.assertEqual(sum(i.size for i in seen.values()),size)
            self.assertEqual(max(a+i.size for a,i in seen.items()),va+size)
        # Raw primary RTTI and slot2, not shared-address identity.
        self.assertEqual(struct.unpack('<I',retail.bytes_at_va(0x550f4c,4))[0],0x55ae08)
        col=struct.unpack('<5I',retail.bytes_at_va(0x55ae08,20))
        self.assertEqual(col[:3],(0,0,0))
        self.assertEqual(retail.bytes_at_va(col[3]+8,19),b'.?AVGR_BitString@@\0')
        self.assertEqual(struct.unpack('<5I',retail.bytes_at_va(0x550f50,20)),
                         (0x423e70,0x423df0,0x423e90,0x423ec0,0x423d30))
        # Strict wrappers establish that each direct helper sees only remainder1..31.
        for va,size,target in ((0x4238c0,121,0x4239c0),(0x423940,125,0x423b70)):
            ins=list(decoder.disasm(retail.bytes_at_va(va,size),va))
            self.assertEqual(sum(i.size for i in ins),size)
            self.assertEqual(sum(i.mnemonic in ('call','jmp') and i.op_str==hex(target) for i in ins),1)
            self.assertTrue(any(i.mnemonic=='cmp' and '0x20' in i.op_str for i in ins))
        compiler=Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-bitstring-shifts-') as directory:
            output=Path(directory);objects={};paths={}
            for index,source in enumerate((SOURCE,AGGREGATE,CANONICAL,FIXTURE)):
                path=output/('unit%d.obj'%index)
                command=[sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(path),
                         '--vc6-root',str(compiler),'--profile','vc6_o2_mt']
                if source==FIXTURE:
                    command+=['--extra','/Od']
                result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,encoding='latin1',
                                      env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                objects[source],paths[source]=CoffObject(path),path
            # Captured same-path, authentic pre-extraction aggregate. All nondebug
            # sections, flags, bytes, relocation addends and destinations are kept.
            self.assertEqual(object_fingerprint(objects[AGGREGATE]),{
                'sections':22,'relocations':38,
                'sha256':'312d3f5904cabc988e9257d15482b9e79c3a3f3477ba77d59592c8557c0de088'})
            candidate=objects[SOURCE]
            self.assertEqual({s.name for s in candidate.symbols if s.section_number==0},{MASK})
            self.assertEqual({s.name for s in candidate.symbols if s.section_number>0 and s.storage_class==2},
                             {row[0] for row in TARGETS})
            self.assertFalse(any(s.name.startswith('.CRT') or s.characteristics&0xc0
                                 for s in candidate.sections if not s.name.startswith('.debug')))
            for symbol,va,size,emitted,score,returns in TARGETS:
                records=[]
                for source in (SOURCE,AGGREGATE):
                    obj=objects[source];entry=obj.find_symbol(symbol)
                    self.assertEqual((entry.function_size,entry.extent_source),(emitted,'coff_function_aux'))
                    body,extent,relocs=obj.symbol_extent(entry)
                    records.append((body,[(r.virtual_address-entry.value,r.type,
                                          obj.symbol_by_index[r.symbol_index].name) for r in relocs]))
                    proof=compare(retail,obj,symbol,va,size,(ROOT/SOURCE).with_suffix('.bindings.json'))
                    self.assertEqual(proof['strict_match']['matching_positions'],score)
                    self.assertFalse(proof['strict_exact'])
                self.assertEqual(records[0],records[1])
                self.assertEqual({r[2] for r in records[0][1]},{MASK})
            strict=0
            cases=[c for c in CASES if c['source']==CANONICAL]
            self.assertEqual(len(cases),9)
            for case in cases:
                for source in (CANONICAL,AGGREGATE):
                    proof=compare(retail,objects[source],case['symbol'],int(case['target_va'],16),
                                  case['target_size'],ROOT/case['bindings'])
                    self.assertTrue(proof['strict_exact'],proof)
                    strict+=1
            obj=objects[CANONICAL];entry=obj.find_symbol(MASK);section=obj.sections[entry.section_number-1]
            self.assertTrue(section.characteristics&0x80000000)
            raw=obj.data[section.raw_ptr+entry.value:section.raw_ptr+entry.value+132]
            self.assertEqual(raw,retail.bytes_at_va(0x568098,132))
            self.assertEqual(raw,b''.join(((0xffffffff<<(32-n))&0xffffffff).to_bytes(4,'big') for n in range(33)))
            self.assertFalse(any(s.name.startswith('.CRT') for s in obj.sections))
            linker=find_child_ci(find_vc6_bin(compiler),'link.exe')
            libraries=find_child_ci(infer_vc98_root(compiler),'lib');executable=output/'compare.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                f'/out:{executable}',f'/libpath:{libraries}',str(paths[SOURCE]),str(paths[CANONICAL]),
                str(paths[FIXTURE]),'kernel32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image)],capture_output=True,text=True,timeout=90)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('BitStringShifts: 422838 complete-body, independent bit-oracle, wrapper, alias and fault cases match',result.stdout)
            print('%d strict checks; full aggregate preserved; '%strict+result.stdout.strip())


if __name__=='__main__':
    unittest.main()
