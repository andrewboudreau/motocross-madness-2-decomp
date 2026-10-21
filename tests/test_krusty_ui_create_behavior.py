"""Private complete GUI startup operation and its two allocation unwind states."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root, sha256_file
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?Create@KrustyUI@@QAEPAVGameObject@@PAVRenderTarget@@H@Z'
RANGES = {0x4988a0: 1104, 0x54be40: 23, 0x54be57: 26, 0x54be71: 10}
DEPENDENCIES = {0x4692f0,0x521970,0x534b10,0x536340,0x4a3010,0x485190,0x4853b0,
                0x469130,0x5476f0,0x4866c0,0x536057,0x485ef0,0x485d70,0x486560,
                0x486540,0x487d60,0x487c30,0x486630,0x4e9030,0x523e50,0x5231f0,
                0x4c3ae0,0x49a8b0,0x49a540,0x469db0,0x485a70,0x4a3060,0x53471a}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class KrustyUICreateBehaviorTests(unittest.TestCase):
    def test_complete_startup_state_allocations_and_unwind(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        patches, seen, indirect = [], set(), []
        for start, size in RANGES.items():
            instructions = list(decoder.disasm(image.bytes_at_va(start,size),start))
            self.assertEqual(sum(i.size for i in instructions),size)
            boundaries = {i.address for i in instructions}
            self.assertEqual(instructions[-1].mnemonic,'jmp' if start==0x54be71 else 'ret')
            if start==0x4988a0:
                self.assertEqual((instructions[-1].address,bytes(instructions[-1].bytes)),(0x498ced,b'\xc2\x08\0'))
            for instruction in instructions:
                if instruction.mnemonic!='call' and not instruction.group(capstone.CS_GRP_JUMP):
                    continue
                operand=instruction.operands[0]
                if operand.type!=capstone.x86.X86_OP_IMM:
                    self.assertEqual(instruction.mnemonic,'call')
                    self.assertEqual(operand.type,capstone.x86.X86_OP_MEM)
                    indirect.append((instruction.address,operand.mem.disp))
                    continue
                target=operand.imm
                if start<=target<start+size:
                    self.assertIn(target,boundaries)
                    continue
                self.assertIn(target,DEPENDENCIES)
                self.assertIn(instruction.bytes[0],(0xe8,0xe9))
                patches.append(struct.pack('<III',instruction.address,target,instruction.bytes[0]))
                seen.add(target)
        self.assertEqual(seen,DEPENDENCIES)
        self.assertEqual(len(patches),36)
        self.assertEqual(indirect,[(0x498a80,8),(0x498ba0,0x5c),(0x498bfe,0x58)])
        self.assertEqual(struct.unpack('<4I',image.bytes_at_va(0x561d60,16)),(0x19930520,2,0x561d80,0))
        self.assertEqual(struct.unpack('<iIiI',image.bytes_at_va(0x561d80,16)),(-1,0x54be40,-1,0x54be57))
        compiler=Path(os.environ['VC6_ROOT']).resolve()
        linker=find_child_ci(find_vc6_bin(compiler),'link.exe')
        libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-ui-create-') as directory:
            output=Path(directory);objects=[]
            for source in ('samples/ui/KrustyUINearMisses.cpp','tests/fixtures/krusty_ui_create_compare.cpp'):
                obj=output/(Path(source).stem+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(obj),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),
                    *(['--extra','/Od'] if source.startswith('tests/') else [])],cwd=ROOT,capture_output=True,text=True,
                    encoding='latin-1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr);objects.append(str(obj))
                if source.startswith('samples/'):
                    candidate=CoffObject(obj);entry=candidate.find_symbol(SYMBOL)
                    self.assertEqual(entry.function_size,1102)
                    self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==0x20},
                                     {SYMBOL,'??_GIntro1Dlg@@UAEPAXI@Z','??1Intro1Dlg@@UAE@XZ'})
                    report=compare(image,candidate,SYMBOL,0x4988a0,1104,ROOT/'samples/ui/KrustyUINearMisses.bindings.json')
                    self.assertFalse(report['strict_exact'])
                    self.assertEqual(report['strict_match']['matching_positions'],872)
                    self.assertFalse(any(s.name.startswith('.CRT') for s in candidate.sections))
                    def relocated(section,offset):
                        rows=[r for r in candidate.relocations if r.section_number==section and r.virtual_address==offset]
                        self.assertEqual(len(rows),1)
                        return candidate.symbol_by_index[rows[0].symbol_index]
                    def data(symbol,size):
                        section=candidate.section(symbol.section_number)
                        return candidate.data[section.raw_ptr+symbol.value:section.raw_ptr+symbol.value+size]
                    handler=relocated(entry.section_number,entry.value+3)
                    info=relocated(handler.section_number,handler.value+1)
                    words=struct.unpack('<4I',data(info,16))
                    self.assertEqual((words[0],words[1],words[3]),(0x19930520,2,0))
                    unwind=relocated(info.section_number,info.value+8)
                    for index,offset in ((0,15),(1,18)):
                        self.assertEqual(struct.unpack('<i',data(unwind,16)[index*8:index*8+4])[0],-1)
                        action=relocated(unwind.section_number,unwind.value+index*8+4)
                        self.assertEqual(relocated(action.section_number,action.value+offset).name,'??3@YAXPAXPBDH@Z')
                    # The inline Intro1Dlg construction also emits a deleting
                    # wrapper and destructor. Their distinct class identity is
                    # supplied by the primary table and its direct call, not by
                    # the shared five-byte destructor address alone.
                    binding_path=ROOT/'samples/ui/KrustyUINearMisses.bindings.json'
                    for name,address,size in (('??_GIntro1Dlg@@UAEPAXI@Z',0x49bdb0,30),
                                              ('??1Intro1Dlg@@UAE@XZ',0x450fc0,5)):
                        self.assertTrue(compare(image,candidate,name,address,size,binding_path)['strict_exact'])
                    table=candidate.find_symbol('??_7Intro1Dlg@@6B@')
                    all_bindings={}
                    for name in ('BaseObject','GameObject','GameUi','DlgProcs'):
                        all_bindings.update(json.loads((ROOT/f'src/reconstructed/{name}.bindings.json').read_text()))
                    for slot in range(32):
                        target=relocated(table.section_number,table.value+slot*4)
                        address=struct.unpack('<I',image.bytes_at_va(0x5548fc+slot*4,4))[0]
                        if slot==0:
                            self.assertEqual(target.name,'??_EIntro1Dlg@@UAEPAXI@Z')
                            self.assertEqual(address,0x49bdb0)
                        else:self.assertEqual(int(all_bindings[target.name],0),address)
                    col=relocated(table.section_number,table.value-4)
                    self.assertEqual(struct.unpack('<I',data(col,8)[4:8])[0],0)
                    self.assertEqual(struct.unpack('<I',image.bytes_at_va(0x55d18c,4))[0],0)
            patch_file=output/'calls.bin';patch_file.write_bytes(b''.join(patches))
            executable=output/'create.exe';map_path=output/'create.map'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                f'/out:{executable}',f'/map:{map_path}',f'/libpath:{libraries}',*objects,'kernel32.lib'],
                cwd=ROOT,capture_output=True,text=True,encoding='latin-1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            intro_table=next(line.split()[2] for line in map_path.read_text().splitlines() if '??_7Intro1Dlg@@6B@' in line)
            result=subprocess.run([str(executable),str(reference),str(patch_file),intro_table],capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('KrustyUICreate: 13540 complete startup, allocation, profile and callback scenarios match',result.stdout)
            print(result.stdout.strip())


if __name__=='__main__':
    unittest.main()
