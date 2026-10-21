"""One existing light-lookup signature correction, complete callers and native bodies."""
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
from tools.run_calibration import CASES
from test_edit_box_on_char import extract
from test_edit_box_draw import signature
import test_eco_quadtree_connections as objects

ROOT = Path(__file__).resolve().parents[1]
OLD = '?FindLightOfType@LightManager@@QAEPAVUnknownEcoLight@@H@Z'
GET = '?FindLightOfType@LightManager@@QAEPAVLightEmitter@@H@Z'
ADD = '?AddLight@LightManager@@QAEXPAVLightEmitter@@@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and hash-pinned retail')
class EcoLightConnections(unittest.TestCase):
    def test_complete_objects_receivers_return_prefix_and_actual_helpers(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        pe = PEImage(image)
        self.assertEqual(hashlib.sha256(pe.data).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        def instruction(address, mnemonic, operands):
            row = next(decoder.disasm(pe.bytes_at_va(address, 16), address))
            self.assertEqual((row.mnemonic, row.op_str), (mnemonic, operands), hex(address))
        for table, col, name in ((0x555144, 0x55d6e0, b'.?AVLightManager@@\0'),
                                  (0x5550cc, 0x55d690, b'.?AVLightEmitter@@\0')):
            self.assertEqual(struct.unpack('<I', pe.bytes_at_va(table-4, 4))[0], col)
            values = struct.unpack('<5I', pe.bytes_at_va(col, 20))
            self.assertEqual(values[1], 0)
            self.assertEqual(pe.bytes_at_va(values[3]+8, len(name)), name)
        # Actual manager allocation -> primary constructor -> event field -> Create arg4.
        for args in (
            (0x4deee0, 'push', '0x11c'), (0x4deee5, 'call', '0x4a3010'),
            (0x4def03, 'mov', 'ecx, eax'), (0x4def05, 'call', '0x49e390'),
            (0x4def11, 'mov', 'dword ptr [ebp + 0x7c], eax'),
            (0x4df963, 'mov', 'ecx, dword ptr [ebp + 0x7c]'),
            (0x4df966, 'push', 'ecx'), (0x4df972, 'call', '0x4594d0'),
            (0x45952f, 'mov', 'edx, dword ptr [esp + 0x2c]'),
            (0x45953d, 'mov', 'dword ptr [esi + 0x4c], edx'),
            (0x49e395, 'mov', 'esi, ecx'),
            (0x49e39f, 'mov', 'dword ptr [esi], 0x555144'),
            (0x49e3d8, 'mov', 'eax, esi'),
            # Actual emitter allocation/creation and unchanged pointer passed to AddLight.
            (0x4efe02, 'mov', 'ecx, eax'), (0x4efe04, 'call', '0x49deb0'),
            (0x49deb9, 'mov', 'esi, ecx'),
            (0x49df1b, 'mov', 'dword ptr [esi], 0x5550cc'),
            (0x49df7e, 'mov', 'eax, esi'),
            (0x4efe49, 'mov', 'ecx, eax'), (0x4efe4b, 'call', '0x49e230'),
            (0x4efe56, 'mov', 'dword ptr [edi + edx], eax'),
            (0x4efe5f, 'mov', 'eax, dword ptr [edi + eax]'),
            (0x4efe6a, 'mov', 'ecx, dword ptr [esp + 0x134]'),
            (0x4efe71, 'push', 'eax'), (0x4efe72, 'call', '0x49e470'),
            # The actual provider stores the input pointer and returns it unchanged.
            (0x49e477, 'mov', 'dword ptr [ecx + edx*4 + 0x30], eax'),
            (0x49e47b, 'mov', 'dword ptr [eax + 0xa4], ecx'),
            (0x4a01a5, 'cmp', 'dword ptr [ebx + 0x2c], edi'),
            (0x4a01ba, 'mov', 'eax, dword ptr [ecx + eax*4 + 0x30]'),
        ):
            instruction(*args)
        pins = json.loads((ROOT/'tests/fixtures/eco_light_objects.json').read_text())
        self.assertEqual(pins['renames'], {OLD: GET})
        with tempfile.TemporaryDirectory(prefix='mcm2-eco-light-') as temporary:
            out = Path(temporary)
            compiled, paths = {}, {}
            def compile_source(key, source, extra=()):
                path = out/('unit%d.obj' % len(paths))
                command = [sys.executable, str(ROOT/'tools/compile.py'), str(source), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'), '--include', str(ROOT/'src/reconstructed')]
                result = subprocess.run(command+list(extra), cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH':str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                paths[key] = path
                compiled[key] = CoffObject(path)
                return compiled[key]
            for source, pin in pins['objects'].items():
                obj = compile_source(source, ROOT/source)
                with patch.object(objects, 'FILENAMES',
                                  tuple(pins['objects']) + ('src/reconstructed/ContainerList.h',)):
                    rows = objects.whole_object(obj, (ROOT,))
                actual = dict(sections=len(rows), relocations=sum(len(x[-1]) for x in rows),
                    sha256=hashlib.sha256(json.dumps(rows,separators=(',',':')).encode()).hexdigest())
                self.assertEqual(actual, pin, source)
            strict = 0
            for case in pins['strict_cases']:
                result = compare(pe, compiled[case['source']], case['symbol'], int(case['va'],0),
                    case['size'], (ROOT/case['source']).with_suffix('.bindings.json'))
                self.assertTrue(result['strict_exact'], result)
                strict += 1
            scene = next(c for c in CASES if c['source']=='src/reconstructed/SceneManager.cpp'
                         and c['target_va']=='0x004efb20')
            self.assertTrue(compare(pe, compiled[scene['source']], scene['symbol'], 0x4efb20, 1040,
                                    ROOT/scene['bindings'])['strict_exact'])
            strict += 1
            caller = compiled['src/reconstructed/EcoSystem.cpp']
            entry = caller.find_symbol('?LightingChanged@EcoSystem@@QAEXXZ')
            raw, size, relocs = caller.symbol_extent(entry)
            self.assertEqual([(r.virtual_address-entry.value, r.type) for r in relocs
                if caller.symbol_by_index[r.symbol_index].name==GET], [(12,20),(75,20),(89,20)])
            self.assertNotIn(OLD, {s.name for s in caller.symbols})
            for address in (0x45a9ab,0x45a9ea,0x45a9f8):
                instruction(address, 'call', '0x4a0190')
            # The two existing declarations are compiled separately: only their accessed
            # primary prefixes agree; these are not interchangeable full class definitions.
            layout = out/'eco_layout.cpp'
            layout.write_text('#include "EcoSystem.h"\n#include <stddef.h>\n'
                'typedef char E[(offsetof(EcoSystem,lightManager)==0x4c && '
                'offsetof(UnknownEcoLight,colour)==0x54 && '
                'offsetof(UnknownEcoLight,direction)==0x70)?1:-1];\n')
            compile_source('layout', layout)
            code = (ROOT/'src/reconstructed/LightEmitter.cpp').read_text()
            helpers = out/'helpers.cpp'
            helpers.write_text('#include "LightEmitter.h"\n'+
                extract(code,'LightEmitter* LightManager::FindLightOfType(')+'\n'+
                extract(code,'void LightManager::AddLight(')+'\n')
            obj = compile_source('helpers', helpers)
            for symbol, va, extent in ((GET,0x4a0190,52),(ADD,0x49e470,23)):
                self.assertEqual(signature(obj,symbol),signature(compiled['src/reconstructed/LightEmitter.cpp'],symbol))
                self.assertTrue(compare(pe,obj,symbol,va,extent,
                    ROOT/'src/reconstructed/LightEmitter.bindings.json')['strict_exact'])
                self.assertFalse(obj.symbol_extent(obj.find_symbol(symbol))[2])
                strict += 1
            self.assertFalse([s for s in obj.symbols if s.storage_class==2 and s.section_number==0])
            fixture = compile_source('fixture', ROOT/'tests/fixtures/eco_light_compare.cpp')
            for symbol in (GET,ADD):
                self.assertEqual(sum(fixture.symbol_by_index[r.symbol_index].name==symbol
                                     for r in fixture.relocations if r.type==20),1)
            compiler = Path(os.environ['VC6_ROOT'])
            executable = out/'eco-light.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler),'link.exe')),
                '/nologo','/subsystem:console','/base:0x100000','/fixed','/opt:ref',
                '/out:'+str(executable),'/libpath:'+str(find_child_ci(infer_vc98_root(compiler),'lib')),
                str(paths['helpers']),str(paths['fixture']),'kernel32.lib'],
                capture_output=True,text=True,encoding='latin1')
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            result = subprocess.run([str(executable),str(image)],capture_output=True,text=True,timeout=30)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertEqual(result.stdout.strip(),'Eco light: 4480 cases, 192 independently modeled AV')
            self.assertEqual(strict,75)
        print('75 strict checks; 13 complete objects; three calls; '+result.stdout.strip())


if __name__=='__main__':
    unittest.main()
