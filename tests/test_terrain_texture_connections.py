"""Terrain texture construction uses complete existing canonical providers."""
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
from tools.match import compare
from tools.run_calibration import CASES

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('src/krusty2/broadphase/Terrain.cpp', 'src/reconstructed/TerrainSupport.cpp',
           'src/reconstructed/PCTextureMap.cpp', 'src/reconstructed/ManagedTexture.cpp',
           'src/reconstructed/ManagedTextureGroup.cpp')
PROVIDERS = (
    ('??0PCTextureMap@@QAE@PAVTextureMapManager@@H@Z', 0x4c5f00, 46, SOURCES[2]),
    ('??0ManagedTexture@@QAE@PAVTextureMapManager@@@Z', 0x510500, 123, SOURCES[3]),
    ('?AddTexture@ManagedTextureGroup@@QAEXPAVManagedTexture@@@Z', 0x50c6c0, 145, SOURCES[4]),
)


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires native VC6 and the private retail image')
class TerrainTextureConnectionsTests(unittest.TestCase):
    def test_constructor_identity_layout_and_complete_callers(self):
        import capstone
        reference = Path(os.environ['MCM2_EXE'])
        expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(), expected['mcm2_sha256'])
        retail = PEImage(reference)
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        # Decode each actual COL and base descriptor, rather than accepting
        # an address shared with a short constructor as class identity.
        for vtable, col, names in (
            (0x555fec, 0x55e440, ('PCTextureMap', 'TextureMap', 'BaseObject')),
            (0x558430, 0x55f4c8, ('ManagedTexture', 'PCTextureMap', 'TextureMap', 'BaseObject')),
            (0x5583c0, 0x55f418, ('ManagedTextureGroup', 'BaseObject')),
        ):
            self.assertEqual(struct.unpack('<I', retail.bytes_at_va(vtable - 4, 4))[0], col)
            _, offset, cd_offset, _, hierarchy = struct.unpack('<5I', retail.bytes_at_va(col, 20))
            self.assertEqual((offset, cd_offset), (0, 0))
            _, _, count, array = struct.unpack('<4I', retail.bytes_at_va(hierarchy, 16))
            self.assertEqual(count, len(names))
            for pointer, name in zip(struct.unpack('<' + 'I' * count, retail.bytes_at_va(array, 4 * count)), names):
                descriptor, _, mdisp, pdisp, vdisp, _ = struct.unpack('<2I3iI', retail.bytes_at_va(pointer, 24))
                self.assertEqual((mdisp, pdisp, vdisp), (0, -1, 0))
                self.assertEqual(retail.bytes_at_va(descriptor + 8, len(name) + 7),
                                 ('.?AV' + name + '@@\0').encode())
        for address, size, vtable in ((0x4c5f00, 46, 0x555fec), (0x510500, 123, 0x558430)):
            code = list(decoder.disasm(retail.bytes_at_va(address, size), address))
            self.assertEqual(sum(i.size for i in code), size)
            self.assertTrue(any(i.mnemonic == 'mov' and len(i.operands) == 2
                and i.operands[0].type == capstone.x86.X86_OP_MEM and i.operands[0].mem.disp == 0
                and i.operands[1].type == capstone.x86.X86_OP_IMM and i.operands[1].imm == vtable
                for i in code))
        for address, size in ((0x505600, 455), (0x5059d0, 2116)):
            code = list(decoder.disasm(retail.bytes_at_va(address, size), address))
            self.assertEqual(sum(i.size for i in code), size)
            calls = [i.operands[0].imm for i in code if i.mnemonic == 'call'
                     and i.operands[0].type == capstone.x86.X86_OP_IMM]
            for _, provider, _, _ in PROVIDERS:
                self.assertEqual(calls.count(provider), 1)
            for allocation in (0x80, 0xbc):
                self.assertTrue(any(i.mnemonic == 'push' and i.operands[0].type == capstone.x86.X86_OP_IMM
                                    and i.operands[0].imm == allocation for i in code))
        with tempfile.TemporaryDirectory(prefix='mcm2-terrain-textures-') as directory:
            output = Path(directory)
            layout = output / 'layout.cpp'
            layout.write_text('\n'.join('#include "' + (ROOT / path).as_posix() + '"' for path in (
                'src/krusty2/broadphase/Terrain.h', 'src/reconstructed/ManagedTexture.h'))
                + '\n#include <stddef.h>\n' + '\n'.join('typedef char Check%d[(%s) ? 1 : -1];' % (n, condition)
                for n, condition in enumerate((
                    'sizeof(PCTextureMap) == 0x80 && sizeof(TerrainSurfaceBase) == 0x80',
                    'sizeof(ManagedTexture) == 0xbc && sizeof(TerrainSurface) == 0xbc',
                    'offsetof(PCTextureMap, averageColor) == offsetof(TerrainSurfaceBase, averageColor)',
                    'offsetof(PCTextureMap, textureSurface) == offsetof(TerrainSurfaceBase, textureSurface)',
                    'sizeof(UnknownTextureFormatChoice) == sizeof(TerrainSurfaceDesc)',
                    'offsetof(UnknownTextureFormatChoice, manager) == offsetof(TerrainSurfaceDesc, manager)',
                    'offsetof(UnknownTextureFormatChoice, opaqueGroup) == offsetof(TerrainSurfaceDesc, registry)',
                    'offsetof(ManagedTexture, group) == 0x90',
                    'offsetof(ManagedTextureGroup, managedTextures) == 0x44',
                ))))
            objects = {}
            for source in (*SOURCES, str(layout)):
                path = output / (Path(source).stem + '.obj')
                command = [sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source), '-o', str(path),
                           '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt']
                if source == SOURCES[0]:
                    command += ['--include', str(ROOT / 'src/krusty2')]
                result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                                        env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[source] = CoffObject(path)
            for symbol, va, size, source in PROVIDERS:
                obj = objects[source]
                self.assertEqual(obj.find_symbol(symbol).function_size, size)
                self.assertTrue(compare(retail, obj, symbol, va, size,
                                       ROOT / Path(source).with_suffix('.bindings.json'))['strict_exact'])
            for source in SOURCES[:2]:
                obj = objects[source]
                imports = {s.name for s in obj.symbols if s.section_number == 0 and s.storage_class == 2}
                for symbol, _, _, _ in PROVIDERS:
                    self.assertIn(symbol, imports)
                self.assertFalse(any('@TerrainSurface' in name for name in imports))
            cases = [(c['source'], c['symbol'], int(c['target_va'], 0), c['target_size'], ROOT / c['bindings'])
                     for c in CASES if c['source'] in SOURCES]
            manifest = ROOT / 'src/krusty2/broadphase/targets.json'
            for target in json.loads(manifest.read_text()):
                if 'source' not in target or target.get('expect') == 'partial':
                    continue
                source = (manifest.parent / target['source']).resolve().relative_to(ROOT).as_posix()
                if source in SOURCES:
                    cases.append((source, target['candidate_symbol_contains'], int(target['target_va'], 0),
                                  target['target_size'], manifest.parent / target['bindings']))
            for source, symbol, va, size, bindings in dict.fromkeys(cases):
                self.assertTrue(compare(retail, objects[source], symbol, va, size, bindings)['strict_exact'],
                                (source, symbol))
            self.assertEqual(len(dict.fromkeys(cases)), 105)
            print('%d strict terrain texture caller/provider comparisons preserved' % len(dict.fromkeys(cases)))


if __name__ == '__main__':
    unittest.main()
