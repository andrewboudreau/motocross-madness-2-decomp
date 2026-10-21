"""Two actual startup list constructors, identified through complete producers."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.provenance import import_records
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare
from tools.run_calibration import CASES
import test_eco_quadtree_connections as objects

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/reconstructed/StartupListConstructors.cpp'
SOURCES = ('src/reconstructed/TrackGame.cpp', 'src/reconstructed/ZoneReport.cpp',
           'src/reconstructed/Parser.cpp', 'src/reconstructed/DirectoryList.cpp',
           'src/reconstructed/UiInfo.cpp')
PROVIDERS = (('??0UnknownParserList@@QAE@XZ', 0x4aae20, 9),
             ('??0UnknownDriveList@@QAE@XZ', 0x4676a0, 16))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail')
class StartupListConstructorTests(unittest.TestCase):
    def test_complete_producers_storage_abi_and_guarded_stores(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        self.assertEqual(retail.bytes_at_va(0x4aae20, 16),
                         bytes.fromhex('8bc1c70000000000c3') + b'\x90'*7)
        self.assertEqual(retail.bytes_at_va(0x4676a0, 16),
                         bytes.fromhex('8bc1c70000000000c7400400000000c3'))
        imports = {int(r['iat_va'], 0): r for r in import_records(retail)}
        self.assertEqual(imports[0x5501dc]['name'], 'lstrlenA')
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoded = {}
        for address, size in ((0x522060, 960), (0x49c2f0, 763),
                              (0x49c600, 355), (0x4b87f0, 408)):
            rows = list(md.disasm(retail.bytes_at_va(address, size), address))
            self.assertEqual(sum(i.size for i in rows), size)
            decoded.update((i.address, (i.mnemonic, i.op_str)) for i in rows)
        expected = {
            0x52212e: ('push', '0x47'), 0x522133: ('push', '0x575780'),
            0x522138: ('push', '8'), 0x5221bb: ('call', '0x4a3010'),
            0x5221c0: ('add', 'esp, 0xc'), 0x5221c7: ('cmp', 'eax, ebx'),
            0x5221ce: ('je', '0x5221d9'), 0x5221d0: ('mov', 'ecx, eax'),
            0x5221d2: ('call', '0x4676a0'), 0x5221d9: ('xor', 'eax, eax'),
            0x5221db: ('mov', 'ecx, eax'),
            0x5221e1: ('mov', 'dword ptr [ebp + 0x25dc], eax'),
            0x5221e7: ('call', '0x449e60'),
            # lstrlenA@4 consumes its sole pushed argument. Parse's two
            # arguments then move ESP by eight: +0x18 becomes the original
            # +0x10 receiver. The constructor's +0x14 is pairs at receiver+4.
            0x49c334: ('lea', 'ecx, [esp + 0x14]'), 0x49c338: ('call', '0x4aae20'),
            0x49c34c: ('push', 'ecx'), 0x49c34d: ('call', 'dword ptr [0x5501dc]'),
            0x49c357: ('push', 'eax'), 0x49c358: ('push', 'edx'),
            0x49c359: ('lea', 'ecx, [esp + 0x18]'), 0x49c35d: ('call', '0x4b87f0'),
            0x49c5c2: ('lea', 'ecx, [esp + 0x14]'), 0x49c5c6: ('call', '0x4b86e0'),
            0x49c63f: ('mov', 'esi, dword ptr [0x5501dc]'),
            0x49c666: ('lea', 'ecx, [esp + 0x14]'), 0x49c66a: ('call', '0x4aae20'),
            0x49c67e: ('push', 'ecx'), 0x49c67f: ('call', 'esi'),
            0x49c685: ('push', 'eax'), 0x49c686: ('push', 'edx'),
            0x49c687: ('lea', 'ecx, [esp + 0x18]'), 0x49c68b: ('call', '0x4b87f0'),
            0x49c6ec: ('lea', 'ecx, [esp + 0x14]'), 0x49c6fb: ('call', '0x4b86e0'),
            0x49c72b: ('lea', 'ecx, [esp + 0x14]'), 0x49c72f: ('call', '0x4b86e0'),
            0x4b8816: ('ret', '8'), 0x4b8985: ('ret', '8'),
        }
        for address, row in expected.items():
            self.assertEqual(decoded[address], row, hex(address))
        # The three allocator arguments remain live across the string-copy
        # sequence. There are no calls or stack-pointer changes in between.
        for address, (mnemonic, operand) in decoded.items():
            if 0x52213a <= address < 0x5221bb:
                self.assertNotIn(mnemonic, ('push', 'pop', 'call', 'ret'))
                self.assertFalse(operand == 'esp' or operand.startswith('esp,'))
        self.assertEqual(0x14 - (0x18 - 8), 4)
        pins = json.loads((ROOT/'tests/fixtures/startup_list_objects.json').read_text())['objects']
        with tempfile.TemporaryDirectory(prefix='mcm2-startup-list-') as directory:
            out = Path(directory)
            paths, compiled = {}, {}
            def compile_source(key, source, extra=()):
                path = out/('unit%d.obj' % len(paths))
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    *extra], cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                paths[key], compiled[key] = path, CoffObject(path)
                return compiled[key]
            for source in (SOURCE, *SOURCES):
                compile_source(source, ROOT/source)
            strict = 0
            for symbol, address, size in PROVIDERS:
                obj = compiled[SOURCE]
                entry = obj.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (size, 'coff_function_aux'))
                self.assertTrue(compare(retail, obj, symbol, address, size,
                                        (ROOT/SOURCE).with_suffix('.bindings.json'))['strict_exact'])
                strict += 1
            provider = compiled[SOURCE]
            sections = [s for s in provider.sections if not s.name.startswith('.debug')]
            self.assertEqual([(s.name, s.raw_size, s.characteristics) for s in sections],
                             [('.drectve', 82, 0x00100a00), ('.text', 16, 0x60501020),
                              ('.text', 16, 0x60501020)])
            self.assertFalse([r for r in provider.relocations
                              if not provider.section(r.section_number).name.startswith('.debug')])
            self.assertEqual({s.name for s in provider.symbols if s.storage_class == 2 and s.section_number > 0},
                             {p[0] for p in PROVIDERS})
            self.assertFalse([s for s in provider.symbols if s.storage_class == 2 and s.section_number == 0])
            for source in SOURCES:
                with patch.object(objects, 'FILENAMES', SOURCES):
                    rows = objects.whole_object(compiled[source], roots=(ROOT,))
                actual = dict(sections=len(rows), relocations=sum(len(r[-1]) for r in rows),
                              sha256=hashlib.sha256(json.dumps(rows, separators=(',', ':')).encode()).hexdigest())
                self.assertEqual(actual, pins[source], source)
            for case in CASES:
                if case['source'] not in SOURCES or case.get('expect') == 'near_miss':
                    continue
                result = compare(retail, compiled[case['source']], case['symbol'], int(case['target_va'], 0),
                                 case['target_size'], ROOT/case['bindings'])
                self.assertTrue(result['strict_exact'], (case, result))
                strict += 1
            self.assertEqual(strict, 102)
            self.assertTrue(any(s.name == '__imp__lstrlenA@4' for s in compiled[SOURCES[1]].symbols))
            fixture = 'tests/fixtures/startup_list_constructors.cpp'
            compile_source(fixture, ROOT/fixture, ('--include', str(ROOT/'src/reconstructed'), '--extra', '/Od'))
            for symbol, _, _ in PROVIDERS:
                self.assertEqual(sum(compiled[fixture].symbol_by_index[r.symbol_index].name == symbol
                                     for r in compiled[fixture].relocations if r.type == 20), 1)
            layout = out/'layout.cpp'
            layout.write_text('#include "TrackGame.h"\n#include <stddef.h>\n'
                              'typedef char Check[(offsetof(TrackGameMode,driveList)==0x25dc)?1:-1];\n')
            compile_source('layout', layout, ('--include', str(ROOT/'src/reconstructed')))
            compiler = Path(os.environ['VC6_ROOT'])
            exe = out/'startup.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', '/out:'+str(exe),
                '/libpath:'+str(find_child_ci(infer_vc98_root(compiler), 'lib')),
                str(paths[SOURCE]), str(paths[fixture]), 'kernel32.lib'],
                capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(), 'Startup list constructors: 1344 cases, 192 exact write AV')
            print('%d strict checks; five complete old objects; %s' % (strict, result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
