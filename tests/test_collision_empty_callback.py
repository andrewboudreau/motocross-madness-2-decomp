"""Bind the empty provider to its actual installation and three cdecl call sites."""
import hashlib
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from mcm2tool.coff import CoffObject
from mcm2tool.pe import PEImage
from tools.match import compare

ROOT = Path(__file__).resolve().parents[1]
SYMBOL = '?CollisionEmptyCallback@@YAXPAVCollisionObject@@0@Z'


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and private retail')
class CollisionEmptyCallbackTests(unittest.TestCase):
    def test_complete_provider_installer_and_dispatch(self):
        import capstone
        image = Path(os.environ['MCM2_EXE']).resolve()
        self.assertEqual(hashlib.sha256(image.read_bytes()).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        retail = PEImage(image)
        # This is the whole return, followed by alignment, not a trimmed body.
        self.assertEqual(retail.bytes_at_va(0x464e90, 16), b'\xc3'+b'\x90'*15)
        self.assertEqual(retail.bytes_at_va(0x464e80, 3), b'\xc2\x04\x00')
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        installer = {i.address: (i.mnemonic, i.op_str)
                     for i in md.disasm(retail.bytes_at_va(0x43b9a0, 59), 0x43b9a0)}
        for address, expected in {
            0x43b9a5: ('mov', 'esi, ecx'),
            0x43b9ac: ('lea', 'edi, [esi - 0xc]'),
            0x43b9b2: ('mov', 'ecx, edi'),
            0x43b9b4: ('call', '0x4320f0'),
            0x43b9bc: ('mov', 'dword ptr [esi + 0x7c], 0x43b800'),
            0x43b9c5: ('mov', 'dword ptr [esi + 0x80], 0x464e90'),
            0x43b9cf: ('mov', 'dword ptr [esi + 0x54], edi'),
        }.items():
            self.assertEqual(installer[address], expected)
        query = {i.address: (i.mnemonic, i.op_str)
                 for i in md.disasm(retail.bytes_at_va(0x438e70, 1102), 0x438e70)}
        # The three broad-phase paths supply (hitObject, queryingObject).
        for load, push, call in ((0x438fcf, 0x438fe3, 0x438fe5),
                                (0x439100, 0x439110, 0x439112),
                                (0x439267, 0x43927b, 0x43927d)):
            self.assertEqual(query[load], ('mov', 'eax, dword ptr [esi + 0x9c]'))
            self.assertEqual(query[load+6], ('mov', 'ecx, dword ptr [eax + 0x8c]'))
            self.assertEqual(query[load+12], ('test', 'ecx, ecx'))
            self.assertEqual(query[load+14][0], 'je')
            self.assertEqual(query[push], ('push', 'esi'))
            self.assertEqual(query[push+1], ('push', 'eax'))
            self.assertEqual(query[call], ('call', 'ecx'))
            self.assertEqual(query[call+2], ('add', 'esp, 8'))
        with tempfile.TemporaryDirectory(prefix='mcm2-collision-callback-') as directory:
            out = Path(directory)
            def compile_source(source, name):
                path = out/(name+'.obj')
                result = subprocess.run([sys.executable, str(ROOT/'tools/compile.py'), str(source),
                    '-o', str(path), '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2')], cwd=ROOT, capture_output=True,
                    text=True, encoding='latin1', env={**os.environ, 'PYTHONPATH': str(ROOT)})
                self.assertEqual(result.returncode, 0, result.stdout+result.stderr)
                return CoffObject(path)
            targets = (
                ('src/reconstructed/CollisionCallbacks', SYMBOL, 0x464e90, 1),
                ('src/krusty2/constraint/ConstraintMethodCollisionModel',
                 'SetRenderContext@ConstraintMethodCollisionModel', 0x43b9a0, 59),
                ('src/krusty2/collision/CollisionObject', 'QueryCollisions@CollisionObject', 0x438e70, 1102))
            for index, (source, symbol, va, size) in enumerate(targets):
                obj = compile_source(ROOT/(source+'.cpp'), str(index))
                result = compare(retail, obj, symbol, va, size, ROOT/(source+'.bindings.json'))
                self.assertTrue(result['strict_exact'], result)
                if index == 0:
                    sections = [s for s in obj.sections if not s.name.startswith('.debug')]
                    self.assertEqual([(s.name, s.raw_size, s.characteristics) for s in sections],
                                     [('.drectve', 40, 0x00100a00), ('.text', 16, 0x60501020)])
                    code = sections[1]
                    self.assertEqual(obj.data[code.raw_ptr:code.raw_ptr+code.raw_size],
                                     retail.bytes_at_va(va, 16))
                    self.assertFalse([r for r in obj.relocations
                                      if not obj.section(r.section_number).name.startswith('.debug')])
                    self.assertEqual([(s.name, s.value) for s in obj.symbols
                                      if s.storage_class == 2 and s.section_number > 0], [(SYMBOL, 0)])
                    self.assertFalse([s for s in obj.symbols if s.storage_class == 2 and s.section_number == 0])
            layout = out/'layout.cpp'
            layout.write_text('#include "collision/CollisionObject.h"\n#include <stddef.h>\n'+
                '\n'.join('typedef char Check%d[(%s)?1:-1];' % (i, expression)
                    for i, expression in enumerate(('sizeof(CollisionObject)==0xb8',
                        'offsetof(CollisionObject,onHitCallback)==0x88',
                        'offsetof(CollisionObject,onHitByCallback)==0x8c',
                        'offsetof(CollisionObject,hitObject)==0x9c')))+'\n'+
                'void CollisionEmptyCallback(CollisionObject*, CollisionObject*);\n'+
                'CollisionCallback TypeCheck = CollisionEmptyCallback;\n')
            compile_source(layout, 'layout')
        print('Collision callback:3 full strict bodies; complete provider sections; installer and three cdecl dispatches')


if __name__ == '__main__':
    unittest.main()
