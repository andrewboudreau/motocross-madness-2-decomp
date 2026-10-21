"""Whole MediaControl Open candidate with native COM and failure boundaries."""
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
SYMBOL = '?Open@MediaControl@@QAEPAV1@PAXPBDP6AXPAVUIDialog@@@Z2@Z'
IMPORTS = {'__imp__CoCreateInstance@20': 0, '__imp__MultiByteToWideChar@24': 1}
GUIDS = {'_CLSID_AMMultiMediaStream': 0x558ec0, '_IID_IAMMultiMediaStream': 0x558ed0,
         '_MSPID_PrimaryVideo': 0x558eb0, '_MSPID_PrimaryAudio': 0x558ea0,
         '_IID_IDirectDrawMediaStream': 0x558e90, '_IID_IDirectDraw': 0x556080,
         '_IID_IDirectDrawSurface4': 0x5560f0}
HELPERS = {'?SetRenderContext@GameObject@@UAEPAV1@PAX@Z': 0x4692f0,
           '?IsDirectShowInstalled@MediaControl@@SAHXZ': 0x4a23d0}
INDIRECT = {
    0x4a25b8: 'dword ptr [0x5503fc]', 0x4a25d1: 'dword ptr [ecx + 0x30]',
    0x4a25ee: 'dword ptr [edx + 0x3c]', 0x4a2609: 'dword ptr [edx + 0x3c]',
    0x4a2620: 'dword ptr [ecx + 4]', 0x4a263e: 'dword ptr [0x5501a0]',
    0x4a2653: 'dword ptr [ecx + 0x40]', 0x4a266c: 'dword ptr [eax + 0x10]',
    0x4a2685: 'dword ptr [ecx]', 0x4a26bd: 'dword ptr [edx + 0x24]',
    0x4a26f8: 'dword ptr [ecx]', 0x4a2715: 'dword ptr [edx + 0x18]',
    0x4a2732: 'dword ptr [edx + 0x34]', 0x4a2749: 'dword ptr [edx]',
    0x4a2770: 'dword ptr [ecx + 0x58]', 0x4a27a8: 'dword ptr [edx + 0x18]',
    0x4a27b3: 'dword ptr [edx + 8]',
}


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and private retail image')
class MediaControlOpenBehaviorTests(unittest.TestCase):
    def test_complete_movie_open_com_outputs_and_failures(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(sha256_file(reference), '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        instructions = list(decoder.disasm(image.bytes_at_va(0x4a2560,646),0x4a2560))
        self.assertEqual(sum(i.size for i in instructions),646)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in instructions if i.mnemonic == 'ret'],
                         [(0x4a27c2,b'\xc2\x10\0'),(0x4a27e3,b'\xc2\x10\0')])
        self.assertEqual(image.bytes_at_va(0x4a27e6,10),b'\x90'*10)
        self.assertFalse(any('fs:' in i.op_str for i in instructions))
        boundaries={i.address for i in instructions}
        patches=[];indirect={};targets=set()
        for i in instructions:
            if i.group(capstone.CS_GRP_JUMP):
                self.assertEqual(i.operands[0].type,capstone.x86.X86_OP_IMM)
                self.assertIn(i.operands[0].imm,boundaries)
            if i.mnemonic != 'call':continue
            operand=i.operands[0]
            if operand.type==capstone.x86.X86_OP_IMM:
                self.assertEqual(i.bytes[0],0xe8)
                self.assertIn(operand.imm,HELPERS.values())
                targets.add(operand.imm)
                patches.append(struct.pack('<III',0,i.address,operand.imm))
            else:indirect[i.address]=i.op_str
        self.assertEqual(targets,set(HELPERS.values()))
        self.assertEqual(indirect,INDIRECT)
        for address in GUIDS.values():
            section=next(s for s in image.sections if image.image_base+s.virtual_address <= address < image.image_base+s.virtual_address+s.virtual_size)
            self.assertFalse(section.characteristics & 0x80000000)
        compiler=Path(os.environ['VC6_ROOT']).resolve()
        linker=find_child_ci(find_vc6_bin(compiler),'link.exe')
        libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-media-open-') as directory:
            output=Path(directory)
            def compile_source(source):
                obj=output/(Path(source).stem+'.obj')
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(ROOT/source),'-o',str(obj),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),
                    *(['--extra','/Od'] if source.startswith('tests/') else [])],cwd=ROOT,capture_output=True,text=True,
                    encoding='latin-1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return obj,CoffObject(obj)
            candidate_path,candidate=compile_source('samples/gameui/MediaControlNearMisses.cpp')
            entry=candidate.find_symbol(SYMBOL)
            self.assertEqual(entry.function_size,656)
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0 and s.type==0x20},{SYMBOL})
            self.assertEqual({s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0},{*HELPERS,*GUIDS,*IMPORTS})
            self.assertFalse(any(s.name.startswith('.CRT') for s in candidate.sections))
            report=compare(image,candidate,SYMBOL,0x4a2560,646,ROOT/'samples/gameui/MediaControlNearMisses.bindings.json')
            self.assertFalse(report['strict_exact'])
            self.assertEqual(report['strict_match']['matching_positions'],53)
            body,size,relocations=candidate.symbol_extent(entry)
            for r in relocations:
                name=candidate.symbol_by_index[r.symbol_index].name
                if name in IMPORTS:
                    offset=r.virtual_address-entry.value
                    self.assertEqual(r.type,6)
                    self.assertEqual(body[offset:offset+4],b'\0'*4)
                    patches.append(struct.pack('<III',1,offset,IMPORTS[name]))
            self.assertEqual(len(patches),4)
            # Use actual GUID providers, not test-only constants. The native
            # fixture checks all seven linked values against retail as well.
            guid_path,guid_object=compile_source('src/reconstructed/DirectXGuids.cpp')
            guid=guid_object.find_symbol('_IID_IDirectDrawSurface4')
            section=guid_object.section(guid.section_number)
            self.assertFalse(section.characteristics & 0x80000000)
            self.assertEqual(guid_object.data[section.raw_ptr+guid.value:section.raw_ptr+guid.value+16],image.bytes_at_va(0x5560f0,16))
            fixture_path,_=compile_source('tests/fixtures/media_control_open_compare.cpp')
            patch_file=output/'calls.bin';patch_file.write_bytes(b''.join(patches))
            executable=output/'open.exe';map_path=output/'open.map'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                f'/out:{executable}',f'/map:{map_path}',f'/libpath:{libraries}',str(candidate_path),str(fixture_path),str(guid_path),
                'kernel32.lib','ole32.lib','dxguid.lib','strmiids.lib','amstrmid.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin-1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            linked=PEImage(executable)
            lines=map_path.read_text().splitlines()
            for name,address in GUIDS.items():
                line=next(line for line in lines if name in line.split())
                linked_address=int(line.split()[2],16)
                section=next(s for s in linked.sections if linked.image_base+s.virtual_address <= linked_address < linked.image_base+s.virtual_address+s.virtual_size)
                self.assertFalse(section.characteristics & 0x80000000,name)
                self.assertEqual(linked.bytes_at_va(linked_address,16),image.bytes_at_va(address,16),name)
            result=subprocess.run([str(executable),str(reference),str(patch_file)],capture_output=True,text=True,timeout=180)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('MediaControlOpen: 11125 complete COM, descriptor, state, alias and failure scenarios match',result.stdout)
            print(result.stdout.strip())


if __name__=='__main__':
    unittest.main()
