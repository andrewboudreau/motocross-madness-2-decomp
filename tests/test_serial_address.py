"""Complete serial conversion, real menu caller, and native overlap/fault state."""
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
from tools.build_game import compile_source as game_compile_source
from tools.run_calibration import CASES
from test_steering_axis_from_points import complete_object, signature

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/SerialAddress.cpp'
SYMBOL = '?BuildComPortAddress@@YAXPAUSerialSettings@@PAUNetComPortAddress@@@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and hash-pinned private retail')
class SerialAddressTests(unittest.TestCase):
    def test_complete_helpers_actual_menu_call_and_native_conversion(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        body = list(decoder.disasm(retail.bytes_at_va(0x4ae410,69),0x4ae410))
        self.assertEqual(sum(i.size for i in body),69)
        self.assertEqual([(i.address,i.op_str)for i in body if i.mnemonic=='call'],
                         [(0x4ae423,'0x4ae2f0'),(0x4ae42f,'0x4ae370'),
                          (0x4ae43b,'0x4ae390'),(0x4ae447,'0x4ae3d0')])
        self.assertEqual([(i.address,i.op_str)for i in body if i.mnemonic=='ret'],[(0x4ae454,'')])
        self.assertEqual(retail.bytes_at_va(0x4ae455,11),b'\x90'*11)
        # The complete menu handler is strict below, including its dispatch data.
        caller = {i.address:(i.mnemonic,i.op_str)for i in
                  decoder.disasm(retail.bytes_at_va(0x452ae9,105),0x452ae9)}
        for va,ins in {
            0x452aeb:('lea','edx, [esp + 0x18]'),0x452af3:('push','edx'),
            0x452af8:('push','0x6886b8'),0x452b09:('call','0x4ae410'),
            0x452b0e:('add','esp, 8'),0x452b19:('lea','edx, [esp + 0x18]'),
            0x452b25:('push','edx'),
        }.items():self.assertEqual(caller[va],ins)
        compiler = Path(os.environ['VC6_ROOT']).resolve()
        with tempfile.TemporaryDirectory(prefix='mcm2-serial-address-') as directory:
            output = Path(directory)
            def compile_source(source,name):
                path = output/(name+'.obj')
                run = subprocess.run([sys.executable,str(ROOT/'tools/compile.py'),str(source),'-o',str(path),
                    '--vc6-root',str(compiler),'--profile','vc6_o2_mt','--include',str(ROOT/'src/krusty2'),
                    '--include',str(ROOT/'src/reconstructed')],cwd=ROOT,capture_output=True,text=True,
                    encoding='latin1',env={**os.environ,'PYTHONPATH':str(ROOT)})
                self.assertEqual(run.returncode,0,run.stdout+run.stderr)
                return path,CoffObject(path)
            provider_path,provider = compile_source(ROOT/SOURCE,'provider')
            self.assertFalse([s.name for s in provider.symbols if s.section_number==0])
            rows = complete_object(provider)
            self.assertEqual((len(rows),sum(len(r[3])for r in rows)),(6,26))
            self.assertEqual(hashlib.sha256(json.dumps(rows).encode()).hexdigest(),
                             '353de6e6677ee80041feb42594ea22ef33cfce3fadd95bdf1b4a517831f2d88b')
            self.assertFalse(any(s.name in ('.bss','.data','.CRT$XCU')for s in provider.sections))
            selected = [c for c in CASES if c['source']==SOURCE]
            self.assertEqual(len(selected),5)
            self.assertEqual({s.name for s in provider.symbols if s.type==32 and s.section_number>0},
                             {c['symbol']for c in selected})
            strict = 0
            for case in selected:
                proof = compare(retail,provider,case['symbol'],int(case['target_va'],0),case['target_size'],ROOT/case['bindings'])
                self.assertTrue(proof['strict_exact'],proof);strict += 1
            _,dialogs = compile_source(ROOT/'src/reconstructed/DlgProcs.cpp','dialogs')
            _,network = compile_source(ROOT/'src/reconstructed/NetProcs.cpp','network')
            for obj,query in ((dialogs,'?HandleMenuOk@@'),(network,'?HandleEvent@SerialPopupDlg@@')):
                case = next(c for c in CASES if c['symbol'].startswith(query))
                proof = compare(retail,obj,case['symbol'],int(case['target_va'],0),case['target_size'],ROOT/case['bindings'])
                self.assertTrue(proof['strict_exact'],proof);strict += 1
            self.assertEqual([(offset,kind)for offset,kind,name in signature(dialogs,'?HandleMenuOk@@')[2] if name==SYMBOL],[(474,20)])
            settings = network.find_symbol('?g_SerialSettings@@3USerialSettings@@A')
            section = network.section(settings.section_number)
            self.assertEqual(section.name,'.bss')
            self.assertTrue(section.characteristics & 0x80000000)
            spec = json.loads((ROOT/'config/game_build.json').read_text())
            result = game_compile_source(ROOT/SOURCE,output/'game-command',compiler,
                [ROOT/p for p in spec['includes']],spec['profile'],{**os.environ,'PYTHONPATH':str(ROOT)})
            self.assertTrue(result['compiled'],Path(result['log']).read_text())
            self.assertEqual(complete_object(provider),complete_object(CoffObject(result['object'])))
            fixture,_ = compile_source(ROOT/'tests/fixtures/serial_address_compare.cpp','fixture')
            executable = output/'serial.exe'
            run = subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')),'/nologo','/subsystem:console',
                '/base:0x100000','/fixed','/out:'+str(executable),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler),'lib')),str(provider_path),str(fixture),
                'kernel32.lib'],capture_output=True,text=True,encoding='latin1')
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            run = subprocess.run([str(executable),str(image)],capture_output=True,text=True,timeout=60)
            self.assertEqual(run.returncode,0,run.stdout+run.stderr)
            self.assertIn('Serial address: 28934 complete-state, overlap and fault cases match',run.stdout)
            print('%d strict comparisons; six complete sections; '%strict+run.stdout.strip())


if __name__=='__main__':unittest.main()
