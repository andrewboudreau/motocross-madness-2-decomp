"""Two independently evidenced empty roles; complete providers and caller ABIs."""
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import sha256_file
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
GUI = '?UnknownFunction464e90@GUIManager@@QAEXXZ'
FINDER = '??1UnknownTrackGameObject574@@QAE@XZ'
PROVIDERS = {'GUIManagerHooks': GUI, 'SceneArchiveFinderLifetime': FINDER}
CALLERS = ('GUIManager', 'GameUi', 'TrackGame', 'SceneManager')


@unittest.skipUnless(os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail image')
class EmptyLifecycleProviderTests(unittest.TestCase):
    def test_independent_call_roles_complete_extents_and_callers(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        self.assertEqual(sha256_file(reference),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        self.assertEqual(retail.bytes_at_va(0x464e90, 16), b'\xc3' + b'\x90' * 15)
        self.assertEqual(next(decoder.disasm(retail.bytes_at_va(0x464ea0, 5), 0x464ea0)).mnemonic, 'mov')
        # Decode each complete body from its true entry before selecting sites.
        decoded = {}
        for address, extent, sites in ((0x485870, 83, [0x4858a8]),
                                       (0x46a070, 649, [0x46a0dc]),
                                       (0x521a70, 103, [0x521a80])):
            instructions = list(decoder.disasm(retail.bytes_at_va(address, extent), address))
            self.assertEqual(sum(i.size for i in instructions), extent)
            decoded.update({i.address: i for i in instructions})
            calls = [i for i in instructions if i.mnemonic == 'call' and
                     i.operands[0].type == capstone.x86.X86_OP_IMM and i.operands[0].imm == 0x464e90]
            self.assertEqual([i.address for i in calls], sites)
        self.assertEqual(decoded[0x48589c].op_str, 'ecx, esi')
        self.assertEqual(decoded[0x48589e].op_str, 'dword ptr [esi + 0x1f0], 0')
        self.assertEqual(decoded[0x46a0d9].op_str, 'ecx, dword ptr [esi + 0x30]')
        # The destructor's zero-manager arm also reaches the empty call.
        self.assertEqual(decoded[0x46a0ce].mnemonic, 'je')
        self.assertEqual(decoded[0x46a0ce].operands[0].imm, 0x46a0d9)
        self.assertEqual(decoded[0x521a74].op_str, 'edi, dword ptr [esi + 0x574]')
        self.assertEqual(decoded[0x521a7e].op_str, 'ecx, edi')
        self.assertEqual((decoded[0x521a85].mnemonic, decoded[0x521a85].op_str), ('push', 'edi'))
        self.assertEqual(decoded[0x521a86].operands[0].imm, 0x4a30c0)
        self.assertEqual(decoded[0x521a8e].op_str, 'dword ptr [esi + 0x574], 0')

        with tempfile.TemporaryDirectory(prefix='mcm2-empty-lifecycle-') as directory:
            objects = {}
            sources = ['src/reconstructed/' + name + '.cpp' for name in (*PROVIDERS, *CALLERS)]
            fixture = 'tests/fixtures/empty_lifecycle_calls.cpp'
            for index, source in enumerate((*sources, fixture)):
                path = Path(directory) / (str(index) + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin-1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(path)
            for stem, symbol in PROVIDERS.items():
                obj = objects['src/reconstructed/' + stem + '.cpp']
                self.assertEqual({s.name for s in obj.symbols if s.storage_class == 2 and s.section_number > 0}, {symbol})
                self.assertFalse([s for s in obj.symbols if s.storage_class == 2 and s.section_number == 0])
                self.assertFalse([s for s in obj.sections if s.name.startswith('.CRT')])
                self.assertFalse([r for r in obj.relocations if not obj.section(r.section_number).name.startswith('.debug')])
                entry = obj.find_symbol(symbol)
                self.assertEqual((entry.function_size, entry.extent_source), (1, 'coff_function_aux'))
                body, extent, relocations = obj.symbol_extent(entry)
                self.assertEqual((body, extent, relocations), (b'\xc3', 1, []))
                self.assertTrue(compare(retail, obj, symbol, 0x464e90, 1,
                    ROOT / ('src/reconstructed/' + stem + '.bindings.json'))['strict_exact'])
            # Actual complete caller TUs retain calls to separate role-specific
            # symbols. No provider is an alias of the other or a wrapper.
            for caller, symbol in (('GUIManager', GUI), ('GameUi', GUI), ('TrackGame', FINDER)):
                obj = objects['src/reconstructed/' + caller + '.cpp']
                self.assertIn(symbol, {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations if r.type == 0x14})
            probe = objects[fixture]
            for name, target in (('?InvokeGuiEmptyHook@@YAXPAVGUIManager@@@Z', GUI),
                                 ('?DestroySceneFinder@@YAXPAVUnknownTrackGameObject574@@@Z', FINDER)):
                entry = probe.find_symbol(name)
                body, extent, relocations = probe.symbol_extent(entry)
                self.assertEqual(body, bytes.fromhex('8b4c2404e900000000'))
                self.assertEqual(extent, 9)
                self.assertEqual([(r.virtual_address - entry.value, r.type,
                                   probe.symbol_by_index[r.symbol_index].name) for r in relocations], [(5, 0x14, target)])
            checked = []
            for case in CASES:
                if case['source'] not in objects:
                    continue
                proof = compare(retail, objects[case['source']], case['symbol'], int(case['target_va'], 0),
                                case['target_size'], ROOT / case['bindings'])
                self.assertTrue(proof['strict_exact'], (case['source'], case['symbol'], proof))
                checked.append(case)
            self.assertEqual(len(checked), 501)
            self.assertTrue(any(c['symbol'] == '??0UnknownTrackGameObject574@@QAE@XZ' for c in checked))


if __name__ == '__main__':
    unittest.main()
