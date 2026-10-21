"""Check the race's chat pointer against retail calls and real COFF providers."""
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
                     'requires VC6_ROOT and the private retail EXE')
class ChatOverlayConnectionTests(unittest.TestCase):
    def test_race_pointer_layout_and_chat_providers(self):
        self.assertEqual(sha256_file(EXE),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(EXE)
        for call, target in ((0x41f5b1, 0x51da30), (0x41f675, 0x51dce0),
                             (0x41f731, 0x51dd10), (0x41e135, 0x51dd40),
                             (0x420440, 0x51dd70),
                             (0x4e090a, 0x51cda0)):
            with self.subTest(call=hex(call)):
                instruction = retail.bytes_at_va(call, 5)
                self.assertEqual(instruction[0], 0xe8)
                self.assertEqual(call + 5 + struct.unpack('<i', instruction[1:])[0], target)
        # Constructor installs ChatOverlay's primary vtable. The event allocates
        # 0x3e0 bytes; the race reads this pointer at +0x19c and writes +0x3dc.
        self.assertEqual(retail.bytes_at_va(0x51cdfc, 6), bytes.fromhex('c70654885500'))
        self.assertEqual(retail.bytes_at_va(0x4e08e4, 5), bytes.fromhex('68e0030000'))
        self.assertEqual(retail.bytes_at_va(0x41f59d, 6), bytes.fromhex('8b8e9c010000'))
        self.assertEqual(retail.bytes_at_va(0x41f658, 6), bytes.fromhex('8999dc030000'))

        with tempfile.TemporaryDirectory(prefix='mcm2-chat-connections-') as directory:
            directory = Path(directory)
            fixture = directory / 'layout.cpp'
            fixture.write_text('''#include "BikeRace.h"
#include "TrackOverlay.h"
#include <stddef.h>
typedef char ChatSize[(sizeof(ChatOverlay) == 0x3e0) ? 1 : -1];
typedef char TypingOffset[(offsetof(ChatOverlay, typingEnabled) == 0x3dc) ? 1 : -1];
typedef char RaceChatOffset[(offsetof(BikeRace, chatOverlay) == 0x19c) ? 1 : -1];
ChatOverlay* RaceChat(BikeRace* race) { return race->chatOverlay; }
''')
            objects = {}
            for name, source in (
                    ('layout', fixture),
                    ('race', ROOT / 'src/reconstructed/BikeRace.cpp'),
                    ('overlay', ROOT / 'src/reconstructed/TrackOverlay.cpp')):
                output = directory / (name + '.obj')
                result = subprocess.run([
                    sys.executable, str(ROOT / 'tools/compile.py'), str(source),
                    '-o', str(output), '--vc6-root', os.environ['VC6_ROOT'],
                    '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/krusty2'),
                    '--include', str(ROOT / 'src/reconstructed'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin-1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                objects[name] = CoffObject(output)
            race = objects['race']
            imports = {race.symbol_by_index[r.symbol_index].name for r in race.relocations
                       if race.symbol_by_index[r.symbol_index].section_number == 0}
            providers = {s.name for s in objects['overlay'].symbols
                         if s.storage_class == 2 and s.section_number > 0}
            methods = {
                '?HandleChatKey@ChatOverlay@@QAEHHPAH@Z',
                '?HandleChatInput@ChatOverlay@@QAEHPAUUnknownControlEvent@@PAUUnknownInputEntry@@PAH@Z',
                '?ShowInputLine@ChatOverlay@@QAEXXZ',
                '?HideInputLine@ChatOverlay@@QAEXXZ',
                '?AddMessage@ChatOverlay@@QAEXPBDHH@Z',
            }
            self.assertTrue(methods <= imports)
            self.assertTrue(methods <= providers)
            self.assertFalse(any('UnknownBikeRaceOverlay19c' in s.name for s in race.symbols))


if __name__ == '__main__':
    unittest.main()
