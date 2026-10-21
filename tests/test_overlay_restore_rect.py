"""Overlay restore: full extent, ordered accesses, actual helpers and finite COM state."""
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
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.build_game import compile_source as game_compile
from test_edit_box_on_char import extract
from test_edit_box_draw import complete_object, signature

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT/'samples/render/OverlayNearMisses.cpp'
SYMBOL = '?RestoreRect@Overlay@@QAEHPBUUnknownOverlayRect@@@Z'
HELPERS = [('PCTextureMap','void* PCTextureMap::LockSurface(',0x4c79e0,99),
           ('PCTextureMap','int PCTextureMap::UnlockSurface(',0x4c7a50,25),
           ('Tgafile','int PixelFormatBytesPerPixel(',0x511970,78)]
CALLERS = [
    ('?DrawViewModePanel@StatsOverlay@@QAEHXZ',0x519ef0,1418,[0x51a302]),
    ('?DrawRacePanel@StatsOverlay@@QAEHXZ',0x51a560,1243,[0x51a8dc]),
    ('?DrawTagStandings@StatsOverlay@@QAEHXZ',0x51aa40,1086,[0x51ad4e]),
    ('?RenderPre3D@ChatOverlay@@UAEHXZ',0x51e240,326,[0x51e2a0]),
    ('?RedrawChat@ChatOverlay@@QAEHXZ',0x51de10,1000,[0x51de23]),
    ('?RedrawNameLine@ChatOverlay@@QAEXXZ',0x51e800,257,[0x51e84a]),
    ('?RedrawNameTags@ChatOverlay@@QAEXH@Z',0x51e910,308,[0x51e949,0x51e971]),
]
PRODUCERS = [('PCTextureMap','??0PCTextureMap@@',0x4c5f00,46),
             ('Overlay','??0Overlay@@',0x4b5e20,120),
             ('Overlay','?Attach@Overlay@@',0x4b5f50,583),
             ('TrackOverlay','?LoadPanel@StatsOverlay@@',0x5194b0,976)]


@unittest.skipUnless(os.name=='nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and private retail')
class OverlayRestoreTests(unittest.TestCase):
    def test_complete_restore_order_actual_helpers_and_finite_surfaces(self):
        import capstone
        image_path=Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image_path.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image=PEImage(image_path)
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
        def decoded(va,size):return list(decoder.disasm(image.bytes_at_va(va,size),va))
        code=decoded(0x4b6710,365)
        self.assertEqual(sum(i.size for i in code),365)
        self.assertEqual([(i.address,bytes(i.bytes)) for i in code if i.mnemonic=='ret'],
                         [(0x4b686e,b'\xc2\x04\0'),(0x4b687a,b'\xc2\x04\0')])
        self.assertEqual(image.bytes_at_va(0x4b687d,3),b'\x90'*3)
        self.assertEqual({i.address:i.operands[0].imm for i in code if i.mnemonic=='call' and i.operands[0].type==capstone.x86.X86_OP_IMM},
                         {0x4b6740:0x511970,0x4b6766:0x511970,0x4b67fe:0x511970})
        self.assertEqual({i.address:i.op_str for i in code if i.mnemonic=='call' and i.operands[0].type!=capstone.x86.X86_OP_IMM},
                         {0x4b6790:'dword ptr [eax + 0x34]',0x4b67a6:'dword ptr [eax + 0x34]',0x4b6853:'dword ptr [eax + 0x38]',0x4b685f:'dword ptr [edx + 0x38]'})
        addresses={i.address for i in code}
        for i in code:
            if i.group(capstone.CS_GRP_JUMP):self.assertIn(i.operands[0].imm,addresses)
        self.assertFalse(any('fs:' in i.op_str for i in code))
        classes={c.name:c for c in parse_rtti(image)}
        for name,table,base in [('Overlay',0x555a28,'GameObject'),('StatsOverlay',0x558680,'Overlay'),('ChatOverlay',0x558854,'Overlay'),('PCTextureMap',0x555fec,'TextureMap')]:
            self.assertEqual([(r.object_offset,r.vtables) for r in classes[name].vtable_records],[(0,[table])])
            self.assertTrue(any(b.name==base and b.mdisp==0 and b.pdisp==-1 for b in classes[name].bases))
            col=struct.unpack('<I',image.bytes_at_va(table-4,4))[0]
            words=struct.unpack('<5I',image.bytes_at_va(col,20))
            self.assertEqual(words[1],0)
            self.assertEqual(image.bytes_at_va(words[3]+8,64).split(b'\0')[0],('.?AV'+name+'@@').encode())
        self.assertEqual(struct.unpack('<2I',image.bytes_at_va(0x555fec+13*4,8)),(0x4c79e0,0x4c7a50))
        producer_code={i.address:(i.mnemonic,i.op_str) for _,_,va,size in PRODUCERS for i in decoded(va,size)}
        for va,mnemonic,operand in [
            (0x4c5f16,'mov','dword ptr [esi], 0x555fec'),
            (0x4b5e3a,'mov','dword ptr [esi], 0x555a28'),
            (0x4b5e40,'mov','dword ptr [esi + 0x2c], edx'),
            (0x4b5e43,'mov','dword ptr [esi + 0x30], edx'),
            (0x4b5f77,'mov','edx, dword ptr [esp + 0x1c]'),
            (0x4b5f88,'mov','dword ptr [esi + 0x2c], edx'),
            (0x51954d,'call','0x50a590'),
            (0x519557,'mov','dword ptr [ebp + 0x30], eax'),
            (0x519573,'mov','edx, dword ptr [eax]'),
            (0x519575,'mov','ecx, eax'),
            (0x519577,'call','dword ptr [edx + 0x18]'),
            (0x51957a,'mov','dword ptr [ebp + 0x2c], eax')]:
            self.assertEqual(producer_code[va],(mnemonic,operand))
        compiler=Path(os.environ['VC6_ROOT'])
        linker=find_child_ci(find_vc6_bin(compiler),'link.exe')
        libraries=find_child_ci(infer_vc98_root(compiler),'lib')
        with tempfile.TemporaryDirectory(prefix='mcm2-overlay-restore-') as directory:
            output=Path(directory)
            def compile_source(source,name,fixture=False):
                path=output/(name+'.obj')
                includes=[] if name=='candidate' else ['--include',str(ROOT/'src/reconstructed'),'--include',str(ROOT/'src/krusty2')]
                result=subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),'--vc6-root',str(compiler),'--profile','vc6_o2_mt',*includes,*(['--extra','/Od'] if fixture else [])],cwd=ROOT,capture_output=True,text=True,encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                return path,CoffObject(path)
            candidate_path,candidate=compile_source(SOURCE,'candidate')
            result=compare(image,candidate,SYMBOL,0x4b6710,365,SOURCE.with_suffix('.bindings.json'))
            self.assertEqual((result['candidate_size'],result['strict_match']['matching_positions'],result['strict_exact']),(365,77,False))
            self.assertEqual(hashlib.sha256(signature(candidate,SYMBOL)[0]).hexdigest(),'8e4dc3cc8b5796d421d5c7f4253f620002a1297324617b3382830f7da40e208d')
            self.assertEqual([(s.name,s.raw_size,s.characteristics) for s in candidate.sections if not s.name.startswith('.debug')],[('.drectve',40,0x100a00),('.text',368,0x60501020)])
            self.assertEqual([s.name for s in candidate.symbols if s.storage_class==2 and s.section_number==0],['?PixelFormatBytesPerPixel@@YAHH@Z'])
            self.assertEqual([s.name for s in candidate.symbols if s.storage_class==2 and s.section_number>0],[SYMBOL])
            # Ordinary sequenced locals are optimized back into the old fault order.
            ordinary=output/'ordinary.cpp'
            ordinary.write_text(SOURCE.read_text().replace('const volatile UnknownOverlayRect* initial','const UnknownOverlayRect* initial').replace('../../src/reconstructed/',''))
            old_path,old=compile_source(ordinary,'ordinary')
            old_result=compare(image,old,SYMBOL,0x4b6710,365,SOURCE.with_suffix('.bindings.json'))
            self.assertEqual((old_result['candidate_size'],old_result['strict_match']['matching_positions']),(350,63))
            self.assertEqual(hashlib.sha256(signature(old,SYMBOL)[0]).hexdigest(),'ad0578915615ec27d40d139fd534635d60832a22aec3f22fdb14c6c36acc9cbd')
            self.assertEqual([(r[0],r[1],r[2]) for r in signature(old,SYMBOL)[2]],[(49,20,'?PixelFormatBytesPerPixel@@YAHH@Z'),(80,20,'?PixelFormatBytesPerPixel@@YAHH@Z'),(218,20,'?PixelFormatBytesPerPixel@@YAHH@Z')])
            canonical={unit:compile_source(ROOT/f'src/reconstructed/{unit}.cpp',unit)[1] for unit in ('PCTextureMap','Tgafile','TrackOverlay','Overlay')}
            helper_source=output/'helpers.cpp'
            helper_source.write_text('#include <string.h>\n#include "PCTextureMap.h"\n#include "Tgafile.h"\n'+'\n'.join(extract((ROOT/f'src/reconstructed/{unit}.cpp').read_text(),sig) for unit,sig,_,_ in HELPERS)+'\n')
            helper_path,helpers=compile_source(helper_source,'helpers');strict=0
            for unit,sig,va,size in HELPERS:
                name=sig.split(' ')[-1].rstrip('(')
                symbol='?'+'@'.join(reversed(name.split('::')))+'@@'
                for obj in (helpers,canonical[unit]):
                    report=compare(image,obj,symbol,va,size,ROOT/f'src/reconstructed/{unit}.bindings.json')
                    self.assertTrue(report['strict_exact'],report);strict+=1
                self.assertEqual(signature(helpers,symbol),signature(canonical[unit],symbol))
            for unit,symbol,va,size in PRODUCERS:
                report=compare(image,canonical[unit],symbol,va,size,ROOT/f'src/reconstructed/{unit}.bindings.json')
                self.assertTrue(report['strict_exact'],report);strict+=1
            for symbol,va,size,calls in CALLERS:
                obj=canonical['TrackOverlay'];report=compare(image,obj,symbol,va,size,ROOT/'src/reconstructed/TrackOverlay.bindings.json')
                self.assertTrue(report['strict_exact'],report);strict+=1
                self.assertEqual([i.address for i in decoded(va,size) if i.mnemonic=='call' and i.op_str=='0x4b6710'],calls)
                entry=obj.find_symbol(symbol);_,extent,relocs=obj.symbol_extent(entry)
                self.assertEqual(extent,size)
                self.assertEqual([va+r.virtual_address-entry.value-1 for r in relocs if obj.symbol_by_index[r.symbol_index].name==SYMBOL],calls)
            layout=output/'layout.cpp'
            layout.write_text('#include "Overlay.h"\n#include "PCTextureMap.h"\n#include <stddef.h>\n'
                'typedef char Layout[(sizeof(Overlay)==0x11c && sizeof(PCTextureMap)==0x80 && sizeof(UnknownOverlayRect)==16 && '
                'sizeof(int(Overlay::*)(const UnknownOverlayRect*))==4 && '
                'offsetof(Overlay,overlayTexture)==0x2c && offsetof(Overlay,backingTexture)==0x30 && '
                'offsetof(TextureMap,textureWidth)==0x14 && offsetof(TextureMap,mapFormat)==0x20 && '
                'offsetof(PCTextureMap,systemSurface)==0x70 && sizeof(UnknownSurfaceDesc)==124)?1:-1];\n')
            compile_source(layout,'layout')
            spec=json.loads((ROOT/'config/game_build.json').read_text())
            built=game_compile(SOURCE,output/'game-command',compiler,[ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(built['compiled'],built)
            self.assertEqual(complete_object(candidate),complete_object(CoffObject(Path(built['object']))))
            fixture_path,_=compile_source(ROOT/'tests/fixtures/overlay_restore_rect_compare.cpp','fixture',True)
            executable=output/'overlay.exe'
            result=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{executable}',f'/libpath:{libraries}',str(candidate_path),str(helper_path),str(fixture_path),'kernel32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(executable),str(image_path)],cwd=ROOT,capture_output=True,text=True,timeout=120)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('Overlay RestoreRect: 12970 real-helper finite cases match',result.stdout)
            # Same current matrix, but the old ordinary-read object must fail at
            # the explicit rect=1 oracle after the expanded ordinary cases.
            negative=output/'ordinary.exe'
            linked=subprocess.run([str(linker),'/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',f'/out:{negative}',f'/libpath:{libraries}',str(old_path),str(helper_path),str(fixture_path),'kernel32.lib'],cwd=ROOT,capture_output=True,text=True,encoding='latin1')
            self.assertEqual(linked.returncode,0,linked.stdout+linked.stderr)
            failed=subprocess.run([str(negative),str(image_path)],cwd=ROOT,capture_output=True,text=True,timeout=120)
            self.assertEqual(failed.returncode,7,failed.stdout+failed.stderr)
            self.assertIn('Invalid rect=1: retail AV 0:1, candidate AV 0:5',failed.stdout)
            self.assertNotIn('cases match',failed.stdout)
            self.assertEqual(strict,17)
            print(f'Overlay RestoreRect: {strict} strict helper/producer/caller checks; eight calls; '+result.stdout.strip())


if __name__=='__main__':unittest.main()
