"""Verify motion loaders import the supplied stream and resource implementations."""
from __future__ import annotations

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(os.environ.get('VC6_ROOT'), 'requires VC6_ROOT')
class MotionStreamConnectionTests(unittest.TestCase):
    def test_motion_loaders_reference_existing_providers(self):
        sources = {
            'motion': 'src/krusty2/motion/Motnctrl.cpp',
            'd3d': 'src/krusty2/motion/D3DIMSoultreeMotnctrl.cpp',
            'partial_motion': 'samples/physics/motion/Motnctrl.cpp',
            'stream': 'src/reconstructed/FileStream.cpp',
            'partial_stream': 'samples/io/FileStream.cpp',
            'resources': 'src/reconstructed/ResourceManager.cpp',
            'archive_candidate': 'samples/render/ResourceManagerNearMisses.cpp',
            'parameters': 'src/reconstructed/Parameterblocks.cpp',
        }
        with tempfile.TemporaryDirectory(prefix='mcm2-motion-stream-') as directory:
            objects = {}
            for name, source in sources.items():
                path = Path(directory) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(path)

            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name
                        for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}

            stream_calls = {
                '??0FileStream@@QAE@PAVResourceManager@@@Z',
                '??1FileStream@@QAE@XZ',
                '?Open@FileStream@@QAEHPBD0H@Z',
                '?SetBufferSize@FileStream@@QAEHH@Z',
                '?Read@FileStream@@QAEHPAXHH@Z',
                '?ReadLine@FileStream@@QAEHPADH@Z',
            }
            resource_calls = {
                '?g_ResourceManager@@3PAVResourceManager@@A',
                '?FindItem@ResourceManager@@QAEPAVResourceItem@@PBDH@Z',
                '?FindItemByObject@ResourceManager@@QAEPAXPAX@Z',
                '?SetItemObject@ResourceManager@@QAEXPAX0@Z',
                '?AddLooseFile@ResourceManager@@QAEHPBD0@Z',
            }
            parameter_calls = {
                '??0ParameterBlock@@QAE@XZ',
                '??1ParameterBlock@@QAE@XZ',
                '?Attach@ParameterBlock@@QAEXPAVFileStream@@HH@Z',
                '?SelectSection@ParameterBlock@@QAEHPBD@Z',
                '?ReadString@ParameterBlock@@QAEHPBDPADH@Z',
                '?GetString@ParameterBlock@@QAEHPBD0PADH@Z',
                '?GetInt@ParameterBlock@@QAEHPBDHPAH@Z',
                '?GetFloat@ParameterBlock@@QAEHPBDMPAM@Z',
            }
            motion_imports = imports(objects['motion']) | imports(objects['d3d'])
            self.assertTrue(stream_calls | resource_calls | parameter_calls <= motion_imports)
            supplied = {s.name for name in ('stream', 'partial_stream', 'resources',
                                           'archive_candidate', 'parameters')
                        for s in objects[name].symbols
                        if s.storage_class == 2 and s.section_number > 0}
            self.assertTrue(stream_calls | resource_calls | parameter_calls <= supplied)
            seek = '?Seek@FileStream@@QAEHHHH@Z'
            self.assertIn(seek, imports(objects['partial_motion']))
            self.assertIn(seek, supplied)
            parameter_dependencies = {
                seek,
                '?Tell@FileStream@@QAEHXZ',
                '?ReadLine@FileStream@@QAEHPADH@Z',
                '?FindItem@ResourceManager@@QAEPAVResourceItem@@PBDH@Z',
                '?AddArchive@ResourceManager@@QAEPAVFileStream@@PBDH@Z',
                '?AddLooseFile@ResourceManager@@QAEHPBD0@Z',
            }
            self.assertTrue(parameter_dependencies <= imports(objects['parameters']))
            # AddArchive is supplied by the explicitly reviewed nonexact
            # candidate; the other resource methods have canonical providers.
            self.assertTrue(parameter_dependencies <= supplied)
            # SelectSection naturally emits the recursive accessor's shared
            # out-of-line copy, in addition to inlining its outer steps.
            reset = '?ResetTextMode@FileStream@@QAEXXZ'
            self.assertTrue(any(s.name == reset and s.section_number > 0
                                for s in objects['parameters'].symbols))
            self.assertFalse(any('UnknownParameterStream' in s.name or
                                 'UnknownParameterArchive' in s.name
                                 for s in objects['parameters'].symbols))
            for name in ('motion', 'd3d', 'partial_motion'):
                self.assertFalse(any(alias in symbol.name
                                     for symbol in objects[name].symbols
                                     for alias in ('ArchiveFile@@', 'OwnerRegistry@@',
                                                   'OwnerEntry@@', 'g_pOwnerRegistry@@',
                                                   'SltFile@@')))


if __name__ == '__main__':
    unittest.main()
