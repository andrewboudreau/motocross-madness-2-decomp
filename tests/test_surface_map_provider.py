"""Verify SurfaceMap ownership, whole bodies, caller ABI, and native connections."""
from __future__ import annotations

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
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires native Windows, VC6_ROOT and private retail EXE')
class SurfaceMapProviderTests(unittest.TestCase):
    def test_full_bodies_layout_calls_and_native_ownership(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        pe = PEImage(EXE)
        self.assertEqual(struct.unpack('<I', pe.bytes_at_va(0x558244, 4))[0], 0x55f2e0)
        self.assertEqual(struct.unpack('<5I', pe.bytes_at_va(0x55f2e0, 20)),
                         (0, 0, 0, 0x574668, 0x55f2d0))
        self.assertEqual(pe.bytes_at_va(0x574670, 17), b'.?AVSurfaceMap@@\0')
        # The real caller requests 0x70 bytes, beyond the initialized 0x50 prefix.
        self.assertEqual(pe.bytes_at_va(0x4ff650, 2), b'\x6a\x70')
        call = pe.bytes_at_va(0x4ff66d, 5)
        self.assertEqual(call[0], 0xe8)
        self.assertEqual(0x4ff672 + struct.unpack('<i', call[1:])[0], 0x5051f0)
        # SurfaceMap overrides only the deleting destructor; inherited slots
        # retain BaseObject's actual implementations at object offset zero.
        self.assertEqual(struct.unpack('<4I', pe.bytes_at_va(0x558248, 16)),
                         (0x5052a0, 0x405160, 0x405170, 0x401940))
        compiler_root = Path(os.environ['VC6_ROOT'])
        linker = find_child_ci(find_vc6_bin(compiler_root), 'link.exe')
        libraries = find_child_ci(infer_vc98_root(compiler_root), 'lib')
        sources = {
            'SurfaceMap': 'src/reconstructed/SurfaceMap.cpp',
            'SoultreeMaterial': 'src/reconstructed/SoultreeMaterial.cpp',
            'BaseObject': 'src/reconstructed/BaseObject.cpp',
            'fixture': 'tests/fixtures/surface_map_smoke.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-surface-map-') as directory:
            output = Path(directory)
            objects = {}
            for name, source in sources.items():
                obj = output / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(obj), '--vc6-root', str(compiler_root), '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/reconstructed'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(obj)
            path = ROOT / 'samples/physics/motion/targets.json'
            cases = [row for row in json.loads(path.read_text()) if row.get('class') == 'SurfaceMap']
            self.assertEqual(len(cases), 6)
            for row in cases:
                self.assertEqual((path.parent / row['source']).resolve(), ROOT / sources['SurfaceMap'])
                result = compare(pe, objects['SurfaceMap'], row['candidate_symbol_contains'],
                                 int(row['target_va'], 0), row['target_size'], path.parent / row['bindings'])
                self.assertTrue(result['strict_exact'], result)
                self.assertEqual(result['candidate_size'], row['target_size'])
            consumer_cases = [case for case in CASES if case['source'] == sources['SoultreeMaterial']]
            self.assertEqual(len(consumer_cases), 11)
            for case in consumer_cases:
                result = compare(pe, objects['SoultreeMaterial'], case['symbol'],
                                 int(case['target_va'], 0), case['target_size'], ROOT / case['bindings'])
                self.assertTrue(result['strict_exact'], result)
            surface = objects['SurfaceMap']
            supplied = {s.name for s in surface.symbols if s.section_number > 0 and s.storage_class == 2}
            calls = {objects['SoultreeMaterial'].symbol_by_index[r.symbol_index].name
                     for r in objects['SoultreeMaterial'].relocations}
            connected = {
                '??0SurfaceMap@@QAE@PAVTextureMap@@PBUSurfaceMaterial@@@Z',
                '?SetTexture@SurfaceMap@@QAEXPAVTextureMap@@@Z',
                '?Select@SurfaceMap@@QAEXXZ',
            }
            self.assertTrue(connected <= supplied & calls)
            self.assertFalse(any('SurfaceBaseObject' in s.name or 'SurfaceTexture@@' in s.name
                                 for obj in objects.values() for s in obj.symbols))
            base_providers = {s.name for s in objects['BaseObject'].symbols
                              if s.section_number > 0 and s.storage_class == 2}
            base_calls = {surface.symbol_by_index[r.symbol_index].name for r in surface.relocations
                          if 'BaseObject@@' in surface.symbol_by_index[r.symbol_index].name
                          and surface.symbol_by_index[r.symbol_index].section_number == 0}
            self.assertEqual(len(base_calls), 5)
            self.assertTrue(base_calls <= base_providers)
            executable = output / 'surface_map.exe'
            result = subprocess.run([
                str(linker), '/nologo', '/subsystem:console', f'/out:{executable}',
                f'/libpath:{libraries}', str(output / 'SurfaceMap.obj'),
                str(output / 'BaseObject.obj'), str(output / 'fixture.obj'), 'kernel32.lib',
            ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('real BaseObject ownership, material data, virtual ABI and shared-pointer reload pass',
                          result.stdout)


if __name__ == '__main__':
    unittest.main()
