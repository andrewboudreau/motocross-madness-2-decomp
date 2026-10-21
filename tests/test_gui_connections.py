"""Check shared GUI layouts, real caller symbols, and retail setup ABI."""
from __future__ import annotations

import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.toolchain import sha256_file

ROOT = Path(__file__).resolve().parents[1]
EXE = Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe'))


@unittest.skipUnless(os.environ.get('VC6_ROOT') and EXE.is_file(),
                     'requires VC6_ROOT and private retail EXE')
class GuiConnectionTests(unittest.TestCase):
    def test_shared_layouts_callers_and_setup_boundary(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        # Create allocates the full manager, then calls SetUp and AppendSibling.
        self.assertEqual(retail.bytes_at_va(0x498a07, 5), bytes.fromhex('68e0030000'))
        for site, target in ((0x498a29, 0x485190), (0x498a6b, 0x4853b0),
                             (0x498a73, 0x469130)):
            instruction = retail.bytes_at_va(site, 5)
            self.assertEqual(instruction[0], 0xe8)
            self.assertEqual(site + 5 + struct.unpack('<i', instruction[1:])[0], target)
        # SetUp pops ten arguments; AppendSibling consumes the returned object
        # and the -1 pushed before the SetUp argument list.
        self.assertEqual(retail.bytes_at_va(0x4857ea, 3), bytes.fromhex('c22800'))
        sources = {name: f'src/reconstructed/{name}.cpp' for name in (
            'GUIManager', 'KrustyUI', 'BikeRace', 'EventManager', 'TrackGame',
            'DlgProcs', 'OptionProcs', 'SelectGamePicProcs', 'TrackOverlay')}
        sources.update({'startup_sample': 'samples/ui/KrustyUINearMisses.cpp',
                        'event_sample': 'samples/game/EventManagerNearMisses.cpp',
                        'layout': 'tests/fixtures/gui_layout.cpp'})
        with tempfile.TemporaryDirectory(prefix='mcm2-gui-') as directory:
            objects = {}
            for name, source in sources.items():
                path = Path(directory) / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'),
                    str(ROOT / source), '-o', str(path), '--vc6-root',
                    os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt'],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(path)
            def imports(obj):
                return {obj.symbol_by_index[r.symbol_index].name for r in obj.relocations
                        if obj.symbol_by_index[r.symbol_index].section_number == 0}
            providers = {s.name for s in objects['GUIManager'].symbols
                         if s.storage_class == 2 and s.section_number > 0}
            production = set().union(*(imports(obj) for name, obj in objects.items()
                                      if name not in ('GUIManager', 'startup_sample',
                                                      'event_sample', 'layout')))
            connected = {
                '?ShowCursors@GUIManager@@QAEXH@Z',
                '?GetUser@GUIManager@@QAEPAVGUIUser@@H@Z',
                '?CreateCursors@GUIManager@@QAEXPBDH@Z',
                '?ReleaseBackground@GUIManager@@QAEXXZ',
                '?EnableWindowClipper@GUIManager@@QAEHH@Z',
                '?FindInputDialog@GUIManager@@QAEPAVUIDialog@@XZ',
                '?CloseDialogResource@GUIManager@@QAEXXZ',
                '?OpenDialogResource@GUIManager@@QAEHPBD@Z',
            }
            self.assertTrue(connected <= production)
            self.assertTrue(connected <= providers)
            # The canonical provider now supplies the shared dialog entry.
            self.assertIn('?ShowDialog@GUIManager@@QAEPAVUIDialog@@PAV2@HHH0HHH@Z', production)
            self.assertIn('?ShowDialog@GUIManager@@QAEPAVUIDialog@@PAV2@HHH0HHH@Z', providers)
            startup = {
                '??0GUIManager@@QAE@H@Z',
                '?SetUp@GUIManager@@QAEPAV1@PAXPAVPalette8@@PAVTextureMapManager@@PAVBackgroundImage@@HPAVSoundGroup@@PBDF5H@Z',
                '?AcceptDevice@GUIUser@@QAEHPAVGUIInputDevice@@@Z',
                '?ForgetDevices@GUIUser@@QAEXXZ',
            }
            self.assertTrue(startup <= imports(objects['startup_sample']))
            self.assertTrue(startup <= providers)
            self.assertIn('?AppendSibling@GameObject@@QAEHPAV1@H@Z',
                          imports(objects['startup_sample']))
            for obj in objects.values():
                self.assertFalse(any('UnknownKrustyUIGui' in name for name in imports(obj)))


if __name__ == '__main__':
    unittest.main()
