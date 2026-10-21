"""UIEditBox's independently identified folded callbacks and real helper closure."""
import hashlib
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
METHODS = [('?ButtonUp@UIEditBox@@UAEHHPAH@Z', 0x474b10, 144, 56),
           ('?UpdateNeedsRedraw@UIEditBox@@UAEXXZ', 0x479100, 107, 38)]
HELPERS = [
    ('int UIControl::ButtonUp(', '?ButtonUp@UIControl@@UAEHHPAH@Z', 0x472250, 84),
    ('UIControl* UIControl::FindLastAnchor(', '?FindLastAnchor@UIControl@@QAEPAV1@PAV1@@Z', 0x470850, 32),
    ('char* UIControl::GetName(', '?GetName@UIControl@@QAEPADXZ', 0x470df0, 7),
    ('void UIControl::SetState(', '?SetState@UIControl@@UAEXH@Z', 0x470970, 18),
    ('void UIControl::RequestRedraw(', '?RequestRedraw@UIControl@@UAEXXZ', 0x470640, 21),
]


def function_text(source, signature):
    start = source.index(signature)
    end, depth = source.index('{', start) + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic native VC6 and the private retail image')
class UIEditEventsTests(unittest.TestCase):
    def test_complete_methods_real_helpers_slots_and_native_events(self):
        reference = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(reference.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        image = PEImage(reference)
        word = lambda va: struct.unpack('<I', image.bytes_at_va(va, 4))[0]
        table = 0x552de8
        locator = word(table - 4)
        self.assertEqual(word(locator + 4), 0)
        self.assertEqual(image.bytes_at_va(word(locator + 12) + 8, 16), b'.?AVUIEditBox@@\0')
        cls = next(c for c in parse_rtti(image) if c.name == 'UIEditBox')
        self.assertEqual([(b.name, b.mdisp, b.pdisp, b.vdisp) for b in cls.bases],
                         [(n, 0, -1, 0) for n in ('UIEditBox', 'UIControl', 'GameObject', 'BaseObject')])
        for _, va, _, slot in METHODS:
            self.assertEqual(word(table + 4 * slot), va)
        with tempfile.TemporaryDirectory(prefix='mcm2-ui-edit-events-') as directory:
            output = Path(directory)
            helper = output / 'helpers.cpp'
            canonical = (ROOT / 'src/reconstructed/GameUi.cpp').read_text()
            helper.write_text('#include "GameUi.h"\n#include "DialogProc.h"\n' + '\n'.join(
                function_text(canonical, signature) for signature, _, _, _ in HELPERS))
            paths, objects = {}, {}
            for source in ('src/reconstructed/UIEditBoxEvents.cpp', 'src/reconstructed/GameUi.cpp',
                           str(helper), 'tests/fixtures/ui_edit_events.cpp'):
                path = output / (Path(source).stem + '.obj')
                result = subprocess.run([sys.executable, str(ROOT / 'tools/compile.py'), str(ROOT / source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT / 'src/reconstructed')], cwd=ROOT,
                    capture_output=True, text=True, encoding='latin1')
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                paths[Path(source).stem], objects[Path(source).stem] = path, CoffObject(path)
            provider = objects['UIEditBoxEvents']
            self.assertEqual({s.name for s in provider.symbols if s.storage_class == 2
                              and s.section_number > 0 and s.type == 0x20}, {r[0] for r in METHODS})
            self.assertEqual({s.name for s in provider.symbols if s.storage_class == 2 and s.section_number == 0},
                             {r[1] for r in HELPERS[:3]})
            for symbol, address, extent, _ in METHODS:
                self.assertEqual(provider.symbol_extent(provider.find_symbol(symbol))[1], extent)
                self.assertTrue(compare(image, provider, symbol, address, extent,
                    ROOT / 'src/reconstructed/UIEditBoxEvents.bindings.json')['strict_exact'])
            for _, symbol, address, extent in HELPERS:
                for obj in (objects['GameUi'], objects['helpers']):
                    self.assertEqual(obj.symbol_extent(obj.find_symbol(symbol))[1], extent)
                    self.assertTrue(compare(image, obj, symbol, address, extent,
                        ROOT / 'src/reconstructed/GameUi.bindings.json')['strict_exact'], symbol)
            obj = objects['GameUi']
            entry = obj.find_symbol('??_7UIEditBox@@6B@')
            relocs = {r.virtual_address: obj.symbol_by_index[r.symbol_index] for r in obj.relocations
                      if r.section_number == entry.section_number}
            col = relocs[entry.value - 4]
            section = obj.sections[col.section_number - 1]
            self.assertEqual(struct.unpack_from('<I', obj.data, section.raw_ptr + col.value + 4)[0], 0)
            for symbol, _, _, slot in METHODS:
                self.assertEqual(relocs[entry.value + 4 * slot].name, symbol)
            compiler = Path(os.environ['VC6_ROOT'])
            executable = output / 'events.exe'
            result = subprocess.run([str(find_child_ci(find_vc6_bin(compiler), 'link.exe')), '/nologo',
                '/subsystem:console', f'/out:{executable}', f'/libpath:{find_child_ci(infer_vc98_root(compiler), "lib")}',
                *(str(paths[key]) for key in ('UIEditBoxEvents', 'helpers', 'ui_edit_events')), 'kernel32.lib'],
                cwd=ROOT, capture_output=True, text=True, encoding='latin1')
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            result = subprocess.run([str(executable)], capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout.splitlines(), [
                'UI edit release: 3840 event, helper, mutation and exception cases passed',
                'UI edit redraw: 1152 text, anchor, counter and whole-storage cases passed'])
            print('Two strict methods, five real strict helpers, two primary slots; ' + result.stdout.strip())


if __name__ == '__main__':
    unittest.main()
