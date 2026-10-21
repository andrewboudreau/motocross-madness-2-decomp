"""Shared cache-manager ABI, strict callers and native production lifecycle."""
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
SOURCES = (
    'src/krusty2/broadphase/Terrain.cpp', 'src/reconstructed/TerrainSupport.cpp',
    'src/reconstructed/Griddraw.cpp', 'src/reconstructed/Grid1.cpp',
    'samples/render/GriddrawNearMisses.cpp', 'samples/render/Grid1NearMisses.cpp',
    'samples/physics/shadow/TerrainShadow.cpp', 'src/reconstructed/AgeManager.cpp',
    'src/reconstructed/EcoSystem.cpp',
)
REGISTER = '?Register@AgeManager@@QAEXPAUAgeEntry@@P6AHPAXH@Z1HH@Z'
UNREGISTER = '?Unregister@AgeManager@@QAEXPAUAgeEntry@@@Z'
MARK = '?MarkUsed@AgeManager@@QAEXPAUAgeEntry@@@Z'
EVICT = '?EvictStale@AgeManager@@QAEHH@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class AgeManagerConnectionsTests(unittest.TestCase):
    def test_full_providers_callers_layout_and_native_lifecycle(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(reference)
        with tempfile.TemporaryDirectory(prefix='mcm2-age-manager-') as directory:
            output = Path(directory)
            layout = output / 'layout.cpp'
            layout.write_text('\n'.join([
                '#include "' + (ROOT / path).as_posix() + '"' for path in (
                    'src/krusty2/broadphase/Terrain.h', 'src/reconstructed/Grid1.h')
            ]) + '\n#include <stddef.h>\n' + '\n'.join(
                'typedef char Check%d[(%s) ? 1 : -1];' % (n, condition)
                for n, condition in enumerate((
                    'sizeof(AgeManager) == 0x14', 'sizeof(AgeEntry) == 0x14',
                    'offsetof(Terrain, textureAgeManager) == 0xc84',
                    'offsetof(Terrain, meshAgeManager) == 0xc88',
                    'offsetof(Grid1Terrain, textureAgeManager) == 0xc84',
                    'offsetof(GridTerrain, ageManager) == 0xc88',
                    'offsetof(GridNodeDrawData, ageEntry) == 0x144',
                    'offsetof(Grid1NodeExtra, ageEntry) == 0x14',
                    'offsetof(Grid1BlockTexture, ageEntry) == 8',
                ))) + '\ntypedef int (*Callback)(void*, int);\n'
                'Callback bufferCallback = EvictNodeBuffers;\nCallback textureCallback = EvictNodeTexture;\n')
            fixture = 'tests/fixtures/age_manager_callbacks.cpp'
            paths, objects = {}, {}
            for source in (*SOURCES, fixture, str(layout)):
                path = output / (Path(source).stem + '.obj')
                args = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                        '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if source == SOURCES[6]:
                    args += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source], paths[source] = CoffObject(path), path
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            for base in ('src/krusty2', 'samples/physics'):
                for manifest in (ROOT / base).rglob('targets.json'):
                    for target in json.loads(manifest.read_text()):
                        if 'source' not in target or target.get('expect') == 'partial':
                            continue
                        source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                        if source in SOURCES and target.get('bindings'):
                            cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                          target['target_size'], manifest.parent / target['bindings']))
            cases = list(dict.fromkeys(cases))
            for source, symbol, va, size, bindings in cases:
                self.assertTrue(compare(retail, objects[source], symbol, va, size, bindings)['strict_exact'],
                                (source, symbol))
            self.assertGreaterEqual(len(cases), 170)
            provider = objects[SOURCES[7]]
            provider_cases = [c for c in CASES if c['source'] == SOURCES[7]]
            self.assertEqual(len(provider_cases), 9)
            for case in provider_cases:
                entry = provider.find_symbol(case['symbol'])
                self.assertIn(entry.extent_source, ('coff_function_aux', 'codeview_proc'))
                self.assertEqual(provider.symbol_extent(entry)[1], case['target_size'])
            imports = {s.name for s in provider.symbols if s.storage_class == 2 and s.section_number == 0}
            self.assertEqual(imports, {'?DebugRealloc@@YAPAXPAXIPBDH@Z', '?DebugFree@@YAXPAXPBDH@Z', '_qsort'})
            required = {
                SOURCES[0]: {'??0AgeManager@@QAE@XZ', '??1AgeManager@@QAE@XZ'},
                SOURCES[1]: {EVICT}, SOURCES[2]: {REGISTER, UNREGISTER, MARK},
                SOURCES[3]: {MARK}, SOURCES[4]: {REGISTER}, SOURCES[5]: {REGISTER},
                SOURCES[6]: {MARK},
            }
            for source, names in required.items():
                obj = objects[source]
                undefined = {s.name for s in obj.symbols if s.storage_class == 2 and s.section_number == 0}
                self.assertTrue(names <= undefined, (source, names - undefined))
                self.assertFalse(any('@TerrainOwned@@' in name or '@GridAgeManager@@' in name
                                     or '@GridAgeEntry@@' in name for name in undefined), source)
                for name in names:
                    self.assertGreater(provider.find_symbol(name).section_number, 0)
            # Both changed callbacks are ordinary cdecl definitions. Their
            # unused context argument requires no new instruction or wrapper.
            for source, symbol, va, size in (
                (SOURCES[2], '?EvictNodeBuffers@@YAHPAXH@Z', 0x47ecc0, 231),
                (SOURCES[3], '?EvictNodeTexture@@YAHPAXH@Z', 0x47ca00, 56),
            ):
                self.assertEqual(objects[source].find_symbol(symbol).function_size, size)
                self.assertTrue(compare(retail, objects[source], symbol, va, size,
                                       ROOT / Path(source).with_suffix('.bindings.json'))['strict_exact'])
            decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
            decoder.detail = True
            code = list(decoder.disasm(retail.bytes_at_va(0x4011b0, 150), 0x4011b0))
            indirect = [(n, i) for n, i in enumerate(code) if i.mnemonic == 'call'
                        and i.operands[0].type == capstone.x86.X86_OP_MEM]
            self.assertEqual(len(indirect), 1)
            n, call = indirect[0]
            self.assertEqual(call.operands[0].mem.disp, 4)
            # The native callback asserts both actual owner and all context
            # bits (including negative values), and observes accounting before
            # the manager applies the saved-size debit.
            self.assertEqual([i.mnemonic for i in code[n-2:n]], ['push', 'push'])
            compiler = Path(os.environ['VC6_ROOT'])
            linker = find_child_ci(find_vc6_bin(compiler), 'link.exe')
            libraries = find_child_ci(infer_vc98_root(compiler), 'lib')
            executable = output / 'age.exe'
            result = subprocess.run([str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(paths[SOURCES[7]]), str(paths[fixture]), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('AgeManager: 2308 native allocation, lifetime, callback and accounting cases passed', result.stdout)
            print('%d full strict caller/provider checks; %s' % (len(cases), result.stdout.strip()))


if __name__ == '__main__':
    unittest.main()
