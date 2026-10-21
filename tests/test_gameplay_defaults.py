"""Separately identified gameplay defaults and their exact receiver boundaries."""
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
import test_eco_quadtree_connections as views

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/GameplayDefaults.cpp'
PROVIDERS = (('??1BikeA60C@@QAE@XZ', 0x464e90, 1),
             ('??1VehicleSpeedState@@QAE@XZ', 0x464e90, 1),
             ('?Fn_004DE580@KbRaceHandler@@QAEXPAXMH@Z', 0x4de580, 3))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail')
class GameplayDefaultTests(unittest.TestCase):
    def test_complete_callers_lifetimes_and_no_access_abi(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        pe = PEImage(image)
        self.assertEqual(pe.bytes_at_va(0x464e90, 16), b'\xc3'+b'\x90'*15)
        self.assertEqual(pe.bytes_at_va(0x4de580, 16), b'\xc2\x0c\0'+b'\x90'*13)
        word = lambda address: struct.unpack('<I', pe.bytes_at_va(address, 4))[0]
        # Owner primary/virtual bases are separate. The two destructors take
        # the virtual GameObject address, then load primary-owned subobjects.
        for name, table, offset in (('Bike',0x550900,0),('Bike',0x55085c,0x738),
                                    ('Vehicle',0x558a84,0),('Vehicle',0x5589e0,0x5c0),
                                    ('KrustyBike',0x554548,0)):
            col = word(table-4)
            self.assertEqual(word(col+4), offset)
            self.assertEqual(pe.bytes_at_va(word(col+12)+8, 80).split(b'\0')[0],
                             ('.?AV'+name+'@@').encode())
        self.assertEqual((word(0x554548+3*4),word(0x554548+4*4)),(0x48dbf0,0x48dc60))
        md = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        instructions = {}
        for address,size in ((0x409a10,276),(0x526380,319),(0x48dbf0,109),(0x48dc60,112),
                             (0x4079c0,6745),(0x525e20,1373),(0x48fc80,5384),(0x4a23a0,48)):
            instructions.update((i.address,(i.mnemonic,i.op_str))
                                for i in md.disasm(pe.bytes_at_va(address,size),address))
        expected = {
            0x409a27:('mov','esi, ecx'),0x409a2d:('mov','dword ptr [esi - 0x738], 0x550900'),
            0x409ac0:('mov','edi, dword ptr [esi - 0x12c]'),0x409ac6:('test','edi, edi'),
            0x409ac8:('je','0x409ada'),0x409aca:('mov','ecx, edi'),0x409acc:('call','0x464e90'),
            0x409ad1:('push','edi'),0x409ad2:('call','0x4a30c0'),
            0x526398:('mov','esi, ecx'),0x52639e:('mov','dword ptr [esi - 0x5c0], 0x558a84'),
            0x526403:('mov','edi, dword ptr [esi - 0x140]'),0x52640b:('je','0x52641d'),
            0x52640d:('mov','ecx, edi'),0x52640f:('call','0x464e90'),
            0x526414:('push','edi'),0x526415:('call','0x4a30c0'),
            0x407d9a:('push','0x4c'),0x407da2:('call','0x4a3010'),
            0x407dc5:('push','ecx'),0x407dc6:('push','edx'),0x407dc7:('push','0x566e38'),
            0x407dcc:('mov','ecx, eax'),0x407dce:('call','0x4a23a0'),
            0x407dee:('mov','dword ptr [ebp + 0x60c], eax'),
            0x4a23b4:('mov','dword ptr [esi + 0x40], eax'),
            0x4a23c0:('mov','dword ptr [esi + 0x48], 0'),
            0x4a23c7:('mov','dword ptr [esi + 0x44], ecx'),0x4a23ca:('mov','eax, esi'),
            0x526080:('push','0x1e4'),0x526085:('call','0x4a3010'),
            0x5260dd:('mov','ecx, eax'),0x5260df:('call','0x4d2940'),
            0x5260f8:('mov','dword ptr [esi + 0x480], eax'),
            0x48fca4:('mov','ebp, ecx'),0x48fd5e:('mov','ecx, dword ptr [esp + 0x128]'),
            0x48fd7f:('mov','dword ptr [ebp + 0x740], ecx'),
            0x48dbfd:('mov','esi, ecx'),0x48dc01:('mov','eax, dword ptr [esi + 0x740]'),
            0x48dc07:('push','edi'),0x48dc08:('push','0'),0x48dc0a:('push','esi'),
            0x48dc0b:('mov','ecx, dword ptr [eax + 0x44]'),0x48dc0e:('call','0x4de580'),
            0x48dc44:('mov','eax, dword ptr [ebx]'),0x48dc46:('mov','ecx, dword ptr [esi + 0x740]'),
            0x48dc4c:('push','edi'),0x48dc4d:('push','eax'),0x48dc4e:('mov','ecx, dword ptr [ecx + 0x44]'),
            0x48dc51:('push','esi'),0x48dc52:('call','0x4de580'),
            0x48dc6f:('mov','esi, ecx'),0x48dcb7:('mov','ecx, dword ptr [edi]'),
            0x48dcb9:('mov','edx, dword ptr [esi + 0x740]'),0x48dcbf:('push','ebx'),
            0x48dcc0:('push','ecx'),0x48dcc1:('mov','ecx, dword ptr [edx + 0x44]'),
            0x48dcc4:('push','esi'),0x48dcc5:('call','0x4de580'),
        }
        for address,row in expected.items():self.assertEqual(instructions[address],row,hex(address))
        self.assertEqual((0x738-0x12c,0x5c0-0x140),(0x60c,0x480))
        self.assertEqual(pe.bytes_at_va(0x566e38,11),b'Handlebars\0')
        pins=json.loads((ROOT/'tests/fixtures/gameplay_defaults_objects.json').read_text())['objects']
        with tempfile.TemporaryDirectory(prefix='mcm2-gameplay-defaults-') as directory:
            out=Path(directory);compiled={};paths={}
            def compile_source(key,path,include=True,extra=()):
                obj=out/('unit%d.obj'%len(paths))
                cmd=[sys.executable,str(ROOT/'tools/compile.py'),str(path),'-o',str(obj),
                     '--vc6-root',os.environ['VC6_ROOT'],'--profile','vc6_o2_mt']
                if include:cmd+=['--include',str(ROOT/'src/krusty2')]
                result=subprocess.run(cmd+list(extra),cwd=ROOT,capture_output=True,text=True,encoding='latin1',
                                      env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode,0,result.stdout+result.stderr)
                paths[key]=obj;compiled[key]=CoffObject(obj);return compiled[key]
            for source in pins:compile_source(source,ROOT/source)
            obj=compile_source(SOURCE,ROOT/SOURCE,False)
            self.assertEqual([(s.name,s.raw_size,s.characteristics)for s in obj.sections if not s.name.startswith('.debug')],
                             [('.drectve',40,0x00100a00)]+[('.text',16,0x60501020)]*3)
            self.assertFalse([s for s in obj.symbols if s.storage_class==2 and s.section_number==0])
            self.assertEqual({s.name for s in obj.symbols if s.storage_class==2 and s.section_number>0},{p[0]for p in PROVIDERS})
            self.assertFalse([r for r in obj.relocations if not obj.section(r.section_number).name.startswith('.debug')])
            strict=0
            for symbol,address,size in PROVIDERS:
                entry=obj.find_symbol(symbol);raw,extent,rels=obj.symbol_extent(entry)
                self.assertEqual((extent,entry.extent_source),(size,'coff_function_aux'))
                self.assertEqual(raw,pe.bytes_at_va(address,size));self.assertFalse(rels);strict+=1
            for source,pin in pins.items():
                with patch.object(views,'FILENAMES',tuple(pins)):rows=views.whole_object(compiled[source],(ROOT,))
                self.assertEqual({'sections':len(rows),'relocations':sum(len(x[-1])for x in rows),
                    'sha256':hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest()},pin,source)
            for target in json.loads((ROOT/'src/krusty2/vehicle/targets.json').read_text()):
                if target.get('expect')!='exact' or not target.get('source'):continue
                source='src/krusty2/vehicle/'+target['source']
                if source not in compiled:continue
                proof=compare(pe,compiled[source],target['candidate_symbol_contains'],int(target['target_va'],0),
                              target['target_size'],ROOT/'src/krusty2/vehicle'/target['bindings'])
                self.assertTrue(proof['strict_exact'],(source,target,proof));strict+=1
            for source,symbol,address,size in (
                ('src/reconstructed/MovingPart.cpp','??0MovingPart@@',0x4a23a0,48),
                ('src/reconstructed/GearRatios.cpp','??0UnknownEngineGearbox@@',0x4d2940,1542)):
                provider=compile_source(source,ROOT/source)
                self.assertTrue(compare(pe,provider,symbol,address,size,(ROOT/source).with_suffix('.bindings.json'))['strict_exact'])
                strict+=1
            self.assertEqual(strict,211)
            for source,query,start,destination,sites in (
                ('Bike','??1Bike@@',0x409a10,PROVIDERS[0][0],[0x409acc]),
                ('Vehicle','??1Vehicle@@',0x526380,PROVIDERS[1][0],[0x52640f]),
                ('KrustyBike','?UnknownVirtualSlot3@KrustyBike@@',0x48dbf0,PROVIDERS[2][0],[0x48dc0e,0x48dc52]),
                ('KrustyBike','?UnknownVirtualSlot4@KrustyBike@@',0x48dc60,PROVIDERS[2][0],[0x48dcc5])):
                caller=compiled['src/krusty2/vehicle/'+source+'.cpp'];entry=caller.find_symbol(query)
                _,_,relocs=caller.symbol_extent(entry)
                self.assertEqual([(r.type,start+r.virtual_address-entry.value-1)for r in relocs
                                  if caller.symbol_by_index[r.symbol_index].name==destination],[(20,s)for s in sites])
            # Compiled declaration layout is independent of the decoded vbase arithmetic.
            layout=out/'layout.cpp';layout.write_text('#include "vehicle/KrustyBike.h"\n#include <stddef.h>\n'
                'typedef char A[(offsetof(Bike,handlebars)==0x60c)?1:-1];\n'
                'typedef char B[(offsetof(Vehicle,engineState)==0x480)?1:-1];\n'
                'typedef char C[(offsetof(KrustyBike,race)==0x740&&offsetof(KbRace,handler)==0x44)?1:-1];\n')
            compile_source('layout',layout)
            fixture='tests/fixtures/gameplay_defaults.cpp';f=compile_source(fixture,ROOT/fixture,False,('--extra','/Od'))
            for symbol,_,_ in PROVIDERS:self.assertEqual(sum(f.symbol_by_index[r.symbol_index].name==symbol for r in f.relocations if r.type==20),1)
            compiler=Path(os.environ['VC6_ROOT']);exe=out/'defaults.exe'
            result=subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')),'/nologo','/subsystem:console',
                '/out:'+str(exe),'/libpath:'+str(find_child_ci(infer_vc98_root(compiler),'lib')),
                str(paths[SOURCE]),str(paths[fixture]),'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=30)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),'Gameplay defaults: 3072 unchanged-page and inaccessible-pointer calls')
            print('%d strict checks; nine complete old objects; %s'%(strict,result.stdout.strip()))


if __name__=='__main__':unittest.main()
