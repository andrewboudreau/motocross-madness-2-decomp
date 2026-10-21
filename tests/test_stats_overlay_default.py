"""The quarry cue call uses the allocated primary StatsOverlay and full RET 4."""
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from mcm2tool.rtti import parse_rtti
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?UnknownFunction464e80@StatsOverlay@@QAEXH@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail')
class StatsOverlayDefaultTests(unittest.TestCase):
    def test_complete_default_primary_producer_and_caller(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        self.assertEqual(retail.bytes_at_va(0x464e80, 16), b'\xc2\x04\0'+b'\x90'*13)
        self.assertEqual(retail.bytes_at_va(0x464e90, 1), b'\xc3')
        cls = next(c for c in parse_rtti(retail) if c.name == 'StatsOverlay')
        self.assertEqual([(t.complete_object_locator_va, t.object_offset, t.vtables)
                          for t in cls.vtable_records], [(0x55f6e8, 0, [0x558680])])
        self.assertEqual([(b.name, b.mdisp, b.pdisp) for b in cls.bases],
                         [('StatsOverlay', 0, -1), ('Overlay', 0, -1),
                          ('GameObject', 0, -1), ('BaseObject', 0, -1)])
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        instructions = {}
        for address, size in ((0x4de410, 358), (0x4e07c0, 1049),
                              (0x519370, 133), (0x5194b0, 976)):
            instructions.update((i.address, (i.mnemonic, i.op_str))
                                for i in md.disasm(retail.bytes_at_va(address, size), address))
        for address, expected in {
            # Allocate, construct and retain the primary object at event +0x5c.
            0x4e0a39: ('push', '0x9b8'),
            0x4e0a3e: ('call', '0x4a3010'),
            0x4e0a5b: ('mov', 'ecx, eax'),
            0x4e0a5d: ('call', '0x519370'),
            0x519379: ('mov', 'esi, ecx'),
            0x5193a8: ('mov', 'dword ptr [esi], 0x558680'),
            0x5193ed: ('mov', 'eax, esi'),
            0x4e0a6f: ('mov', 'dword ptr [esi + 0x5c], eax'),
            0x4e0a9a: ('mov', 'ecx, eax'),
            0x4e0a9c: ('call', '0x5194b0'),
            0x5194bf: ('mov', 'ebp, ecx'),
            0x519843: ('mov', 'eax, ebp'),
            0x4e0aa3: ('mov', 'dword ptr [esi + 0x5c], eax'),
            # The cue index is one stack argument; ECX is the primary receiver.
            0x4de415: ('mov', 'esi, ecx'),
            0x4de48c: ('call', '0x48bc30'),
            0x4de491: ('cmp', 'eax, -1'),
            0x4de494: ('je', '0x4de4a8'),
            0x4de499: ('push', 'eax'),
            0x4de4a0: ('mov', 'ecx, dword ptr [esi + 0x5c]'),
            0x4de4a3: ('call', '0x464e80'),
            0x4de4a8: ('mov', 'ecx, dword ptr [esi + 0x38]'),
        }.items():
            self.assertEqual(instructions[address], expected)
        with tempfile.TemporaryDirectory(prefix='mcm2-stats-default-') as directory:
            out = Path(directory)
            def compile_source(source, name):
                path = out/(name+'.obj')
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'), '--include', str(ROOT/'src/reconstructed')],
                    cwd=ROOT, capture_output=True, text=True, encoding='latin1',
                    env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                return CoffObject(path)
            targets = (
                ('src/reconstructed/StatsOverlayDefaults', ((SYMBOL, 0x464e80, 3),)),
                ('src/reconstructed/QuarryStuntEvent', (
                    ('?Tick@BaseQuarryEvent@@UAEHM@Z', 0x4de410, 358),
                    ('?CreateOverlays@BaseQuarryEvent@@UAEXXZ', 0x4e07c0, 1049))),
                ('src/reconstructed/TrackOverlay', (
                    ('??0StatsOverlay@@QAE@H@Z', 0x519370, 133),
                    ('?LoadPanel@StatsOverlay@@QAEPAV1@PAVRenderTarget@@PAVTextureMapManager@@PAXUUnknownOverlayRect@@@Z',
                     0x5194b0, 976))))
            for index, (source, cases) in enumerate(targets):
                obj = compile_source(ROOT/(source+'.cpp'), str(index))
                for symbol, address, size in cases:
                    result = compare(retail, obj, symbol, address, size, ROOT/(source+'.bindings.json'))
                    self.assertTrue(result['strict_exact'], result)
                if index == 0:
                    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
                    self.assertEqual([(s.name, s.raw_size, s.characteristics) for s in sections],
                                     [('.drectve', 40, 0x00100a00), ('.text', 16, 0x60501020)])
                    expected = (b'-defaultlib:LIBCMT -defaultlib:OLDNAMES ', retail.bytes_at_va(0x464e80, 16))
                    # VC6 pads the directives to four-byte alignment with spaces.
                    for section, data in zip(sections, expected):
                        self.assertEqual(obj.data[section.raw_ptr:section.raw_ptr+section.raw_size],
                                         data.ljust(section.raw_size, b' '))
                    self.assertFalse([r for r in obj.relocations
                                      if not obj.section(r.section_number).name.startswith('.debug')])
                    self.assertEqual([(s.name, s.value) for s in obj.symbols
                                      if s.storage_class == 2 and s.section_number > 0], [(SYMBOL, 0)])
                    self.assertFalse([s for s in obj.symbols if s.storage_class == 2 and s.section_number == 0])
            layout = out/'layout.cpp'
            layout.write_text('#include "TrackOverlay.h"\n#include "BaseQuarryEvent.h"\n#include <stddef.h>\n'+
                '\n'.join('typedef char Check%d[(%s)?1:-1];' % (i, expression)
                    for i, expression in enumerate(('sizeof(StatsOverlay)==0x9b8',
                        'offsetof(BaseQuarryEvent,statsOverlay)==0x5c',
                        'sizeof(void (StatsOverlay::*)(int))==4')))+'\n')
            compile_source(layout, 'layout')
        print('StatsOverlay default:5 full strict bodies; primary RTTI/allocation/caller chain; complete provider object')


if __name__ == '__main__':
    unittest.main()
