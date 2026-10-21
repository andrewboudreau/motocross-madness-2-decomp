"""The existing frozen camera uses its actual constructor, storage and cleanup."""
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
from test_visibility_connections import view

ROOT = Path(__file__).resolve().parents[1]
SOURCE = 'src/krusty2/visibility/VisibilityQuadTree.cpp'
CTOR = '??0PCCamera@@QAE@H@Z'
DTOR = '??1PCCamera@@UAE@XZ'
GLOBAL = '?g_frozenCamera@@3VPCCamera@@A'


def pin(rows):
    return dict(sections=len(rows), relocations=sum(len(row[3]) for row in rows),
                sha256=hashlib.sha256(json.dumps(rows).encode()).hexdigest())


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE'),
                     'requires authentic VC6 and hash-pinned private retail')
class FrozenCameraConnectionsTests(unittest.TestCase):
    def test_complete_lifecycle_crt_storage_and_preserved_callers(self):
        import capstone
        image = PEImage(Path(os.environ['MCM2_EXE']))
        self.assertEqual(hashlib.sha256(image.data).hexdigest(),
                         '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874')
        expected = json.loads((ROOT/'tests/fixtures/frozen_camera_objects.json').read_text())
        self.assertEqual(expected['renames'], {
            '??0VisibilityCamera@@QAE@H@Z': CTOR,
            '??1VisibilityCamera@@QAE@XZ': DTOR,
            '?g_frozenCamera@@3UVisibilityCamera@@A': GLOBAL,
        })
        with tempfile.TemporaryDirectory(prefix='mcm2-frozen-camera-') as directory:
            output = Path(directory)
            objects = {}
            for name, source in (
                ('consumer', SOURCE), ('provider', 'src/reconstructed/PCCamera.cpp'),
                ('layout', 'tests/fixtures/frozen_camera_layout.cpp'),
            ):
                path = output/(name+'.obj')
                run = subprocess.run([
                    sys.executable, str(ROOT/'tools/compile.py'), str(ROOT/source), '-o', str(path),
                    '--vc6-root', os.environ['VC6_ROOT'], '--profile', 'vc6_o2_mt',
                    '--include', str(ROOT/'src/krusty2'), '--include', str(ROOT/'src/reconstructed'),
                ], cwd=ROOT, capture_output=True, text=True, encoding='latin1')
                self.assertEqual(run.returncode, 0, run.stdout+run.stderr)
                objects[name] = CoffObject(path)
            consumer, provider, layout = (objects[n] for n in ('consumer', 'provider', 'layout'))
            rows, filenames = view(consumer, normalize_paths=(ROOT,))
            self.assertEqual([name for _, name in filenames], [SOURCE])
            self.assertEqual(pin(rows), expected['after'])
            self.assertEqual((len(rows), sum(len(row[3]) for row in rows)), (88, 342))
            indexed = {row[0]: row for row in rows}
            added = ('named', ((DTOR, 0),))
            destructor = indexed[added]
            section = consumer.section(consumer.find_symbol(DTOR).section_number)
            self.assertEqual((section.name, section.raw_size), ('.text', 16))
            self.assertEqual(destructor[2], 'e900000000'+'90'*11)
            self.assertEqual(destructor[3], [(1, 20, ('external', '??1Camera@@UAE@XZ', 0, 0))])
            cleanup = ('named', (('_$E5', 0),))
            old_cleanup = indexed[cleanup]
            self.assertEqual(len(old_cleanup[3]), 2)
            self.assertEqual(old_cleanup[3][1], (6, 20, ('defined', added, 0)))
            # Reverse only the independently verified definition resolution. No
            # bytes, addends or other target identities are omitted from the pin.
            indexed[cleanup] = old_cleanup[:3] + ([old_cleanup[3][0],
                (6, 20, ('external', DTOR, 0, 0))],)
            del indexed[added]
            original = sorted(indexed.values(), key=lambda row: repr(row[0]))
            self.assertEqual(pin(original), expected['before_joined'])
            self.assertEqual((len(original), sum(len(row[3]) for row in original)), (87, 341))
            bindings = ROOT/Path(SOURCE).with_suffix('.bindings.json')
            targets = json.loads((ROOT/'src/krusty2/visibility/targets.json').read_text())
            selected = [target for target in targets if target.get('source') == 'VisibilityQuadTree.cpp']
            self.assertEqual(len(selected), 41)
            strict = 0
            for target in selected:
                result = compare(image, consumer, target['candidate_symbol_contains'],
                                 int(target['target_va'], 0), target['target_size'], bindings)
                self.assertTrue(result['strict_exact'], result)
                strict += 1
            self.assertTrue(compare(image, consumer, DTOR, 0x4624d0, 5, bindings)['strict_exact'])
            strict += 1
            for symbol, va, size in ((CTOR, 0x4bed80, 25), (DTOR, 0x4624d0, 5)):
                result = compare(image, provider, symbol, va, size, ROOT/'src/reconstructed/Camera.bindings.json')
                self.assertTrue(result['strict_exact'], result)
                strict += 1
            for obj in (consumer, provider):
                data, size, rels = obj.symbol_extent(obj.find_symbol(DTOR))
                self.assertEqual((data, size), (b'\xe9\0\0\0\0', 5))
                self.assertEqual([(r.virtual_address, r.type, obj.symbol_by_index[r.symbol_index].name)
                                  for r in rels], [(1, 20, '??1Camera@@UAE@XZ')])
            imported = {s.name for s in consumer.symbols if s.storage_class == 2 and s.section_number == 0}
            self.assertIn(CTOR, imported)
            self.assertNotIn(DTOR, imported)
            self.assertFalse(set(expected['renames']) & imported)
            glob = consumer.find_symbol(GLOBAL)
            bss = consumer.section(glob.section_number)
            self.assertEqual((glob.value, bss.name, bss.raw_size, bss.raw_ptr), (64, '.bss', 620, 0))
            self.assertTrue(bss.characteristics & 0x80000000)
            self.assertLessEqual(glob.value+0x220, bss.raw_size)
            crt = next(s for s in consumer.sections if s.name == '.CRT$XCU')
            self.assertEqual((crt.raw_size, crt.reloc_count), (32, 8))
            crtrels = [r for r in consumer.relocations if r.section_number == crt.index]
            self.assertEqual([(r.virtual_address, r.type, consumer.symbol_by_index[r.symbol_index].name)
                              for r in crtrels], [(n*4, 6, name) for n, name in enumerate(
                                  ('_$E2', '_$E7', '_$E10', '_$E13', '_$E16', '_$E19', '_$E22', '_$E25'))])
            self.assertEqual(consumer.data[crt.raw_ptr:crt.raw_ptr+32], bytes(32))
            for symbol, expected_bytes in (
                ('?FrozenCameraPrimary@@', b'\x8b\x44\x24\x04\xc3'),
                ('?FrozenCameraLayout@@', b'\xb8\x04\x00\x00\x00\xc3'),
            ):
                data, size, rels = layout.symbol_extent(layout.find_symbol(symbol))
                self.assertEqual(data, expected_bytes)
                self.assertFalse(rels)
        word = lambda va: struct.unpack('<I', image.bytes_at_va(va, 4))[0]
        self.assertEqual(word(0x566558), 0x52d2e0)
        self.assertEqual(struct.unpack('<5I', image.bytes_at_va(0x55e190, 20)),
                         (0, 0, 0, 0x566b48, 0x55e180))
        self.assertEqual(image.bytes_at_va(0x566b50, 16).split(b'\0')[0], b'.?AVPCCamera@@')
        self.assertEqual(word(0x555da0), 0x55e190)
        self.assertEqual(word(0x55e188), 4)
        array = word(0x55e18c)
        for index, name in enumerate(('PCCamera', 'Camera', 'GameObject', 'BaseObject')):
            descriptor = word(array+4*index)
            td, count, mdisp, pdisp, vdisp, attributes = struct.unpack('<IIiiiI', image.bytes_at_va(descriptor, 24))
            self.assertEqual((mdisp, pdisp, vdisp, attributes), (0, -1, 0, 0))
            self.assertEqual(image.bytes_at_va(td+8, 40).split(b'\0')[0], ('.?AV'+name+'@@').encode())
        section = image.section_for_rva(0x68a968-image.image_base)
        offset = 0x68a968-image.image_base-section.virtual_address
        self.assertGreaterEqual(offset, section.raw_size)
        self.assertLessEqual(offset+0x220, section.virtual_size)
        self.assertTrue(section.characteristics & 0x80000000)
        # Exhaustive explicit base-address references, not a claim about indirect accesses.
        sites, start = [], 0
        while (found := image.data.find(struct.pack('<I', 0x68a968), start)) >= 0:
            sites.append(image.offset_to_va(found))
            start = found+1
        self.assertEqual(sites, [0x52d2f3, 0x52d311, 0x52ef2d, 0x52ef65])
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        instructions = {}
        for va, size in ((0x4bed80, 25), (0x4624d0, 5), (0x52d2e0, 10), (0x52d2f0, 13),
                         (0x52d300, 12), (0x52d310, 10), (0x52eec0, 135), (0x52ef50, 252)):
            decoded = list(decoder.disasm(image.bytes_at_va(va, size), va))
            self.assertEqual(sum(i.size for i in decoded), size)
            instructions.update({i.address: (i.mnemonic, i.op_str) for i in decoded})
        for va, expected_ins in {
            0x4bed88: ('call', '0x42e340'), 0x4bed8d: ('mov', 'dword ptr [esi], 0x555da4'),
            0x4bed96: ('ret', '4'), 0x4624d0: ('jmp', '0x42f020'),
            0x52d2e0: ('call', '0x52d2f0'), 0x52d2e5: ('jmp', '0x52d300'),
            0x52d2f0: ('push', '1'), 0x52d2f2: ('mov', 'ecx, 0x68a968'),
            0x52d2f7: ('call', '0x4bed80'), 0x52d300: ('push', '0x52d310'),
            0x52d305: ('call', '0x535430'), 0x52d310: ('mov', 'ecx, 0x68a968'),
            0x52d315: ('jmp', '0x4624d0'), 0x52ef27: ('mov', 'ecx, 0x88'),
            0x52ef2c: ('mov', 'edi, 0x68a968'),
            0x52ef34: ('rep movsd', 'dword ptr es:[edi], dword ptr [esi]'),
            0x52ef64: ('mov', 'eax, 0x68a968'),
        }.items():
            self.assertEqual(instructions[va], expected_ins)
        self.assertEqual(strict, 44)
        print('Frozen camera: 44 strict checks; 87 old sections/341 relocations; '
              'one exact implicit destructor; actual CRT/storage/primary-layout proof')


if __name__ == '__main__':
    unittest.main()
