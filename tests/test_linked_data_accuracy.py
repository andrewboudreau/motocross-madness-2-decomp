"""tools/linked_data_accuracy.py: synthetic map/COFF/retail fixtures (no exe or VC6 needed)."""
import os
import struct
import tempfile
import unittest
from pathlib import Path

from mcm2tool.coff import CoffObject
from tools.linked_data_accuracy import (
    REL_DIR32, Binding, DataSymbol, Reloc, bind_by_pointer, bind_symbols, comment_addresses,
    compare, content_copies, interpret, is_text, object_data_symbols, padding_only, parse_map,
    section_bases, uncovered_runs, unique_string)

ROOT = Path(__file__).resolve().parents[1]

MAP_TEXT = """\
  Address         Publics by Value              Rva+Base     Lib:Object

 0001:00000000       ?Tick@@YAXXZ               00401000 f   Game.obj
 0003:00000000       _g_a                       00566000     Game.obj
 0003:00000008       _g_c                       00566008     Game.obj
 0003:00000010       ?g_table@@3PAHA            00566010     Other.obj

 Static symbols

 0001:00000040       _$E1                       00401040 f   Game.obj
"""


class FakeImage:
    """Bytes by VA from {va: bytes}; zero elsewhere (like RetailImage past file data)."""

    def __init__(self, blocks):
        self.blocks = blocks

    def read(self, va, size):
        out = bytearray(size)
        for base, blob in self.blocks.items():
            lo, hi = max(va, base), min(va + size, base + len(blob))
            if lo < hi:
                out[lo - va:hi - va] = blob[lo - base:hi - base]
        return bytes(out)

    def word(self, va):
        return struct.unpack('<I', self.read(va, 4))[0]


def coff_bytes():
    """An i386 object: .data {_g_a = 1, static _s_ptr = &_s_buf, _g_c = 1.0f}, .bss {static _s_buf[16]}."""
    data = struct.pack('<IIf', 1, 0, 1.0)
    relocs = struct.pack('<IIH', 4, 7, REL_DIR32)       # _s_ptr -> symbol 7 (_s_buf)
    syms = []

    def sym(name, value, sec, sc, aux=0, typ=0):
        syms.append(name.encode().ljust(8, b'\0') + struct.pack('<IhHBB', value, sec, typ, sc, aux))
        if aux:
            syms.append(bytes(18))

    sym('.data', 0, 1, 3, aux=1)        # 0, 1
    sym('.bss', 0, 2, 3, aux=1)         # 2, 3
    sym('_g_a', 0, 1, 2)                # 4
    sym('_s_ptr', 4, 1, 3)              # 5
    sym('_g_c', 8, 1, 2)                # 6
    sym('_s_buf', 0, 2, 3)              # 7
    head = 20 + 2 * 40
    data_ptr = head
    reloc_ptr = data_ptr + len(data)
    sym_ptr = reloc_ptr + len(relocs)
    out = struct.pack('<HHIIIHH', 0x14c, 2, 0, sym_ptr, len(syms), 0, 0)
    out += b'.data\0\0\0' + struct.pack('<IIIIIIHHI', 0, 0, len(data), data_ptr, reloc_ptr, 0, 1, 0,
                                        0xC0300040)
    out += b'.bss\0\0\0\0' + struct.pack('<IIIIIIHHI', 0, 0, 16, 0, 0, 0, 0, 0, 0xC0300080)
    out += data + relocs + b''.join(syms) + struct.pack('<I', 4)
    return out


class MapTests(unittest.TestCase):
    def test_publics_and_statics(self):
        syms = {s.name: s for s in parse_map(MAP_TEXT)}
        self.assertEqual(syms['_g_a'].va, 0x566000)
        self.assertEqual(syms['_g_a'].obj, 'Game.obj')
        self.assertFalse(syms['_g_a'].is_func)
        self.assertTrue(syms['?Tick@@YAXXZ'].is_func)
        self.assertTrue(syms['_$E1'].is_func)
        self.assertEqual(syms['_g_c'].section, 3)
        self.assertEqual(len(syms), 5)


class CoffPlacementTests(unittest.TestCase):
    def setUp(self):
        fd, name = tempfile.mkstemp(suffix='.obj')
        with os.fdopen(fd, 'wb') as f:
            f.write(coff_bytes())
        self.addCleanup(os.unlink, name)
        self.obj = CoffObject(name)

    def test_static_section_placed_through_relocation(self):
        built = FakeImage({0x566004: struct.pack('<I', 0x680010)})
        bases = section_bases(self.obj, {'_g_a': 0x566000}, built.word)
        self.assertEqual(bases, {1: 0x566000, 2: 0x680010})

    def test_data_symbols_sized_to_next_symbol(self):
        syms = {s.name: s for s in object_data_symbols(self.obj, 'Game.obj', {1: 0x566000, 2: 0x680010})}
        self.assertEqual(set(syms), {'_g_a', '_s_ptr', '_g_c', '_s_buf'})
        self.assertEqual((syms['_g_a'].va, syms['_g_a'].size, syms['_g_a'].data), (0x566000, 4, b'\1\0\0\0'))
        self.assertTrue(syms['_s_ptr'].static)
        self.assertEqual([(r.offset, r.type, r.target, r.target_static) for r in syms['_s_ptr'].relocs],
                         [(0, REL_DIR32, '_s_buf', True)])
        self.assertEqual(syms['_g_c'].data, struct.pack('<f', 1.0))
        self.assertTrue(syms['_s_buf'].bss)
        self.assertEqual((syms['_s_buf'].va, syms['_s_buf'].size), (0x680010, 16))

    def test_unplaced_section_is_skipped(self):
        names = {s.name for s in object_data_symbols(self.obj, 'Game.obj', {1: 0x566000})}
        self.assertEqual(names, {'_g_a', '_s_ptr', '_g_c'})


class CompareTests(unittest.TestCase):
    def test_plain_difference_and_interpretation(self):
        res = compare(b'\0\0\0\0\0\0\x80\x3f', [], struct.pack('<If', 1, 1.0))
        self.assertEqual((res['equal'], res['different']), (7, 1))
        self.assertEqual(res['diff_words'], [0])
        self.assertEqual(interpret(b'\1\0\0\0'), '0x00000001 (1)')
        self.assertEqual(interpret(struct.pack('<f', 0.5)), '0x3f000000 (1056964608, 0.5f)')

    def test_relocated_words(self):
        retail = struct.pack('<III', 0x68acd4, 0x12345678, 7)
        res = compare(bytes(8) + b'\7\0\0\0', [(0, 4, 0x68acd4), (4, 4, None)], retail)
        self.assertEqual((res['equal'], res['different'], res['unknown_pointer']), (8, 0, 4))
        res = compare(bytes(8), [(0, 4, 0x68a000)], retail[:8])
        self.assertEqual(res['pointer_diffs'][0]['retail'], 'd4ac6800')

    def test_padding_only(self):
        self.assertTrue(padding_only([7], b'abc\0\0\0\0\0'))
        self.assertFalse(padding_only([1], b'abc\0\0\0\0\0'))


class CommentBindingTests(unittest.TestCase):
    def test_same_line_and_block_above(self):
        text = ('int g_x = 3;   // 0x0056d060\n'
                '\n'
                '// 0x0057471c: quality flag.\n'
                '// more text\n'
                'int g_quality = 1;\n'
                '\n'
                '// 0x00566000: describes the pair, not one of them\n'
                'int g_first;\n'
                'int g_second;\n'
                'void Code()   // 0x00401000 is code\n')
        self.assertEqual(comment_addresses(text), {'g_x': 0x56d060, 'g_quality': 0x57471c})


class StringTests(unittest.TestCase):
    def test_unique_string_prefers_clean_starts(self):
        self.assertEqual(unique_string([(0x566000, True), (0x566100, False)]), 0x566000)
        self.assertIsNone(unique_string([(0x566000, True), (0x566100, True)]))
        self.assertEqual(unique_string([(0x566104, False)]), 0x566104)

    def test_is_text(self):
        self.assertTrue(is_text(b'D:\\aardvark\\VC\\krusty2\\Lzw.cpp\0\0'))
        self.assertFalse(is_text(b'\x01\0\0\0\x03\0\0\0'))
        self.assertFalse(is_text(b'abc\0'))


def dsym(name, va, data, obj='Game.obj', relocs=(), source=None):
    return DataSymbol(name, obj, '.data', va, len(data), data, list(relocs), source=source)


class BindTests(unittest.TestCase):
    def test_binding_sources_and_order(self):
        syms = [dsym('?g_a@@3HA', 0x401000, b'\1\0\0\0'),
                dsym('_g_UnknownGlobal56a740', 0x401004, bytes(4)),
                dsym('_s_frame', 0x401008, bytes(4)),
                dsym('??_C@_02JAFN@?$CFi?$AA@', 0x40100c, b'%i\0'),
                dsym('_kVec3XAxis', 0x401010, bytes(12), source='src/Bike.cpp'),
                dsym('_kVec3YAxis', 0x40101c, bytes(12), source='src/Other.cpp')]
        names = {'?g_a@@3HA': ({0x566000}, 'bindings'),
                 '_kVec3XAxis': ({0x577ab0, 0x67c508}, 'bindings'),
                 '_kVec3YAxis': ({0x577ac0, 0x67c518}, 'bindings')}
        comments = {'Game.obj': {'s_frame': 0x56d060}}
        strings = {b'%i\0': [(0x56f834, True)]}
        units = {'src/bike': {'_kVec3XAxis': 0x67c508}}
        bound, why = bind_symbols(syms, names, comments, strings, units)
        self.assertEqual(bound[0], Binding(0x566000, 'bindings'))
        self.assertEqual(bound[1], Binding(0x56a740, 'name'))
        self.assertEqual(bound[2], Binding(0x56d060, 'comment'))
        self.assertEqual(bound[3], Binding(0x56f834, 'content'))
        self.assertEqual(bound[4], Binding(0x67c508, 'bindings'))
        self.assertNotIn(5, bound)
        self.assertEqual(why[5], 'bindings: 2 retail VAs')

    def test_pointer_binding(self):
        holder = dsym('?g_clipper@@3PAVClipper@@A', 0x401000, bytes(4),
                      relocs=[Reloc(0, REL_DIR32, '_s_object', True, 0)])
        target = DataSymbol('_s_object', 'Game.obj', '.bss', 0x402000, 64, bytes(64), bss=True)
        syms = [holder, target]
        bound = {0: Binding(0x575a98, 'bindings')}
        built = FakeImage({0x401000: struct.pack('<I', 0x402000)})
        retail = FakeImage({0x575a98: struct.pack('<I', 0x68acd4)})
        self.assertEqual(bind_by_pointer(syms, bound, built.word, retail), 1)
        self.assertEqual(bound[1], Binding(0x68acd4, 'pointer'))


class RetailGapTests(unittest.TestCase):
    def test_uncovered_runs_split_at_anchors(self):
        retail = FakeImage({0x566000: struct.pack('<IIII', 1, 0, 2, 3) + b'abcd\0\0\0\0'})
        gaps = uncovered_runs(retail, [(0x566000, 0x566004)], [(0x566000, 0x566018)],
                              [0x566008, 0x566010], {0x56600c: [0x401234]})
        self.assertEqual([(g['va'], g['size'], g['nonzero'], g['text'], g['code_refs']) for g in gaps],
                         [('0x00566008', 8, 2, False, ['0x00401234']),
                          ('0x00566010', 8, 4, True, [])])

    def test_content_copies_are_aligned(self):
        table = struct.pack('<4I', 0, 0x10, 0x20, 0x30)
        retail = FakeImage({0x566000: b'\0\0' + table + b'\0\0' + table})
        self.assertEqual(content_copies(retail, [table], [(0x566000, 0x566030)]),
                         [(0x566014, 0x566024)])


@unittest.skipUnless(os.environ.get('MCM2_EXE') and Path(os.environ['MCM2_EXE']).is_file(),
                     'MCM2_EXE (retail image) not set')
class IntegrationTests(unittest.TestCase):
    def test_latest_build_binds_and_matches(self):
        builds = sorted((p for p in (ROOT / 'work' / 'game-build').glob('build-*')
                         if (p / 'mcm2.exe').is_file() and (p / 'mcm2.map').is_file()),
                        key=lambda p: p.stat().st_mtime)
        if not builds:
            self.skipTest('no linked build under work/game-build (tools/build_game.py)')
        from tools.linked_data_accuracy import analyse
        report = analyse(builds[-1], Path(os.environ['MCM2_EXE']), ROOT, code_refs=False)
        s = report['summary']
        game = s['by_category']['game']
        self.assertEqual(s['unplaced_symbols'], 0)
        self.assertGreater(game['equal_bytes'], 1_000_000)
        self.assertGreater(s['by_category']['string']['equal_bytes'], 30_000)
        # Every remaining difference is small: gameplay tables compare equal.
        self.assertLess(game.get('different_bytes', 0), 64)


if __name__ == '__main__':
    unittest.main()
