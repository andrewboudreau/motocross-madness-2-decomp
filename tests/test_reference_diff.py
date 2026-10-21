"""reference_diff counts references on both sides as multisets; synthetic inputs only, no VC6."""
import io
import struct
import unittest
from contextlib import redirect_stderr
from pathlib import Path
from types import SimpleNamespace

from mcm2tool.coff import CoffRelocation, CoffSymbol
from tools.reference_diff import (candidate_refs, compile_failures, exit_status, float_key, match_refs,
                                  render_md, retail_refs, score, string_key, table_entries)

BASE = 0x401000


class FakeRetail:
    """The parts of reference_diff.Retail that retail_refs uses, over one flat image."""

    def __init__(self, image: bytes, lo=0x400000, hi=0x700000):
        self.image, self.lo, self.hi = image, lo, hi

    def is_address(self, v):
        return self.lo <= v < self.hi

    def read(self, va, n):
        off = va - 0x400000
        return self.image[off:off + n] if 0 <= off < len(self.image) else b''

    def alt_key(self, va, width):
        if width:
            return float_key(self.read(va, width), width)
        return string_key(self.read(va, 64))


def image_with(patches):
    data = bytearray(0x3000)
    for va, b in patches.items():
        data[va - 0x400000:va - 0x400000 + len(b)] = b
    return bytes(data)


class FakeObject:
    def __init__(self, sections, symbols, relocations):
        self.sections, self.symbols, self.relocations = sections, symbols, relocations
        self.symbol_by_index = {s.index: s for s in symbols}
        self.data = b''.join(sections[n]['bytes'] for n in sorted(sections))
        ptr = 0
        for n in sorted(sections):
            sections[n]['ptr'] = ptr
            ptr += len(sections[n]['bytes'])

    def section(self, n):
        s = self.sections[n]
        return SimpleNamespace(raw_ptr=s['ptr'], raw_size=len(s['bytes']))

    def symbol_extent(self, s):
        code = self.sections[s.section_number]['bytes']
        return code, len(code), [r for r in self.relocations if r.section_number == s.section_number]


class ReferenceDiffTests(unittest.TestCase):
    def test_string_key(self):
        self.assertEqual(string_key(b'%s: %s\0junk'), 'str:%s: %s')
        self.assertEqual(string_key(b'D:\\aardvark\\VC\\krusty2\\bikerace.cpp\0'), 'str:__FILE__')
        self.assertEqual(string_key(b'C:\\tmp\\x\\samples\\race\\BikeRaceNearMisses.cpp\0'), 'str:__FILE__')
        self.assertIsNone(string_key(b'\0'))            # zero-filled data is not an empty string
        self.assertIsNone(string_key(b'\x01\x02\0'))
        self.assertIsNone(string_key(b'abc'))           # unterminated

    def test_float_key(self):
        self.assertEqual(float_key(struct.pack('<f', 20.0), 4), 'flt:20.0')
        self.assertEqual(float_key(struct.pack('<d', 0.5), 8), 'flt:0.5')
        self.assertIsNone(float_key(b'\0\0', 4))

    def test_table_entries(self):
        table = {0x1000: 0x10, 0x1004: 0x20, 0x1008: 0x10, 0x100c: 0x9999}
        self.assertEqual(table_entries(table.get, 0x1000, 0x0, 0x100, 16), [0x10, 0x20, 0x10])

    def test_retail_refs(self):
        code = (b'\x6a\xff'                          # push -1
                b'\x68\x00\x30\x50\x00'              # push 0x503000  (EH handler: skipped)
                b'\x68\x00\x20\x40\x00'              # push 0x402000  (string)
                b'\xe8' + struct.pack('<i', 0x500000 - (BASE + 17)) +   # call 0x500000
                b'\xd9\x05\x10\x20\x40\x00'          # fld dword ptr [0x402010]
                b'\x6a\x05'                          # push 5
                b'\x83\xec\x10'                      # sub esp, 0x10 (frame: skipped)
                b'\xff\x24\x85\x30\x10\x40\x00'      # jmp [eax*4 + 0x401030]
                b'\xc3')
        table = struct.pack('<III', BASE + 0x20, BASE + 0x24, BASE + 0x20)
        image = image_with({BASE: code, BASE + 0x30: table, 0x402000: b'%s: %s\0',
                            0x402010: struct.pack('<f', 20.0)})
        got = retail_refs(FakeRetail(image), BASE, len(code), BASE + 0x40)
        self.assertEqual([(r['kind'], r.get('va'), r.get('alt'), r.get('value')) for r in got['refs']], [
            ('imm', None, None, 0xffffffff),
            ('addr', 0x402000, 'str:%s: %s', None),
            ('call', 0x500000, None, None),
            ('addr', 0x402010, 'flt:20.0', None),
            ('imm', None, None, 5)])
        self.assertEqual(got['tables'], [(3, 2)])

    def test_retail_refs_stop_at_the_tables(self):
        # a byte index table after the jump table: its 0x04 bytes would decode as `add al, 4`
        code = (b'\x0f\xb6\x80' + struct.pack('<I', BASE + 0x18) +   # movzx eax, byte ptr [eax + index]
                b'\xff\x24\x85' + struct.pack('<I', BASE + 0x10) +   # jmp [eax*4 + table]
                b'\xc3\xc3')
        table = struct.pack('<II', BASE + 0xe, BASE + 0xf)
        index = b'\x04\x04\x04\x04\x01\x00\x04\x04'
        image = image_with({BASE: code, BASE + 0x10: table, BASE + 0x18: index})
        got = retail_refs(FakeRetail(image), BASE, 0x20, BASE + 0x40)
        self.assertEqual(got['refs'], [])
        self.assertEqual((got['tables'], got['byte_tables']), ([(2, 2)], 1))

    def test_compile_failures_are_reported(self):
        ok, bad = Path('samples/race/Ok.cpp').resolve(), Path('samples/race/Bad.cpp').resolve()
        by_va = {0x401000: [(bad, 'F', False)], 0x402000: [(ok, 'G', False), (bad, 'G', False)],
                 0x403000: [(ok, 'H', False)]}
        errors = {ok: None, bad: 'Bad.cpp(3) : error C2027: undefined type'}
        rows = []
        for va in sorted(by_va):
            r = dict(tu='race/X', va=va, lane='shell', kind='unmeasured')
            r['compile_failures'] = compile_failures(r, by_va, errors)
            rows.append(r)
        self.assertEqual([len(r['compile_failures']) for r in rows], [1, 1, 0])
        self.assertTrue(rows[0]['compile_failures'][0][0].endswith('samples/race/Bad.cpp'))
        rows[0]['kind'] = 'compile_failure'
        md = render_md(rows)
        self.assertIn('| race/X | `0x00401000` | compile failure |', md)
        self.assertIn('## compile failures', md)
        self.assertEqual(md.count('error C2027'), 2)       # both rows that name the broken source
        with redirect_stderr(io.StringIO()) as err:
            self.assertEqual(exit_status(rows), 1)
        self.assertIn('0x00402000', err.getvalue())
        self.assertEqual(exit_status([rows[2]]), 0)

    def test_candidate_refs(self):
        code = (b'\x68\x00\x00\x00\x00'              # push offset ??_C@... (DIR32)
                b'\xe8\x00\x00\x00\x00'              # call _f (REL32)
                b'\xa1\x04\x00\x00\x00'              # mov eax, [_g + 4] (DIR32, addend 4)
                b'\x6a\x05'                          # push 5
                b'\xc3')
        func = CoffSymbol(0, '?F@@YAXXZ', 0, 1, 0x20, 2, 0)
        lit = CoffSymbol(1, '??_C@_06HNNJ@?$CFs?3?5?$CFs?$AA@', 0, 2, 0, 3, 0)
        f = CoffSymbol(2, '_f', 0, 0, 0x20, 2, 0)
        g = CoffSymbol(3, '_g', 0, 0, 0, 2, 0)
        relocs = [CoffRelocation(1, 1, 1, 0x0006), CoffRelocation(1, 6, 2, 0x0014), CoffRelocation(1, 11, 3, 0x0006)]
        o = FakeObject({1: dict(bytes=code), 2: dict(bytes=b'%s: %s\0')}, [func, lit, f, g], relocs)
        got = candidate_refs(o, func, {'_f': 0x500000, '_g': 0x600000}.get, None)
        self.assertEqual([(r['kind'], r.get('va'), r.get('alt'), r.get('value')) for r in got['refs']], [
            ('addr', None, 'str:%s: %s', None),
            ('call', 0x500000, None, None),
            ('addr', 0x600004, None, None),
            ('imm', None, None, 5)])

    def test_match_refs_and_score(self):
        retail = dict(refs=[dict(kind='call', va=0x500000, alt=None), dict(kind='call', va=0x500000, alt=None),
                            dict(kind='call', va=0x510000, alt=None),
                            dict(kind='addr', va=0x402000, alt='str:%s: %s'),
                            dict(kind='addr', va=0x402010, alt='flt:20.0'),
                            dict(kind='imm', value=5), dict(kind='imm', value=7)],
                      tables=[(12, 8)], byte_tables=0)
        cand = dict(refs=[dict(kind='call', va=0x500000, alt=None, name=None),
                          dict(kind='call', va=None, alt=None, name='_unbound'),
                          dict(kind='addr', va=None, alt='str:%s: %s', name='??_C@x'),
                          dict(kind='addr', va=0x403000, alt=None, name=None),
                          dict(kind='imm', value=5), dict(kind='imm', value=9)],
                    tables=[(12, 7)], byte_tables=0)
        m = match_refs(retail, cand)
        self.assertEqual(sorted(x['va'] for x in m['call']['missing']), [0x500000, 0x510000])
        self.assertEqual([x['name'] for x in m['call']['unbound']], ['_unbound'])
        self.assertEqual([x['va'] for x in m['addr']['missing']], [0x402010])   # the string paired by content
        self.assertEqual([x['va'] for x in m['addr']['extra']], [0x403000])
        self.assertEqual((m['imm']['missing'], m['imm']['extra']), ([7], [9]))
        s = score(m)
        self.assertEqual((s['missing_calls'], s['missing_addrs'], s['missing_switch']), (1, 1, 1))
        self.assertEqual(s['missing'], 3)
        # 0x510000 and the float are used nowhere in the candidate; one 0x500000 call is only a shortfall
        self.assertEqual(sorted(x['va'] for x in m['call']['absent']), [0x510000])
        self.assertEqual(s['absent'], 0 + 1)   # one absent call less one unbound call, plus the absent float


if __name__ == '__main__':
    unittest.main()
