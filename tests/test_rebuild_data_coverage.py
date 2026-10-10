"""tools/rebuild_data_coverage.py: pure helpers (no retail image or VC6 needed)."""
import struct
import unittest

from tools.rebuild_data_coverage import (
    Coverage, defines_variable, derive_link_options, extern_variables, mangled_scalar_size,
    parse_resource_tree, real_size, scan_c_strings, short_import_name, string_literal_size,
    symbol_base_name, symbol_kind, x87_memory_operand)


class SymbolTests(unittest.TestCase):
    def test_string_literal_sizes(self):
        self.assertEqual(string_literal_size('??_C@_04MHIC@NONE?$AA@'), 5)
        self.assertEqual(string_literal_size('??_C@_0BB@HLJF@Object?5Hierarchy?$AA@'), 17)
        self.assertEqual(string_literal_size('??_C@_00A@?$AA@'), 1)
        self.assertIsNone(string_literal_size('?g_x@@3HA'))

    def test_real_and_scalar_sizes(self):
        self.assertEqual(real_size('__real@4@3fff8000000000000000'), 4)
        self.assertEqual(real_size('__real@8@00000000000000000000'), 8)
        self.assertEqual(mangled_scalar_size('?g_soultreeNodeCount@@3HA'), 4)
        self.assertEqual(mangled_scalar_size('?g_quadTree@@3PAVQuadTree@@A'), 4)
        self.assertEqual(mangled_scalar_size('?s_x@Foo@@2NA'), 8)
        self.assertIsNone(mangled_scalar_size('?kVec3XAxis@@3UTreeVec3@@B'))

    def test_names_and_kinds(self):
        self.assertEqual(symbol_base_name('?g_TypeRegistry@@3PAVTypeRegistry@@A'), 'g_TypeRegistry')
        self.assertEqual(symbol_base_name('?s_m@FontTexture@@2VMgr@@A'), 'FontTexture::s_m')
        self.assertEqual(symbol_base_name('_g_taskSwitchAllowed'), 'g_taskSwitchAllowed')
        self.assertEqual(symbol_kind('__imp__Sleep@4'), 'import')
        self.assertEqual(symbol_kind('_IID_IDirectDraw7'), 'guid')
        self.assertEqual(symbol_kind('?g_NetApplicationGuid@@3U_GUID@@B'), 'guid')
        self.assertEqual(symbol_kind('_c_dfDIMouse'), 'library-data')
        self.assertEqual(symbol_kind('___argc'), 'crt-global')
        self.assertEqual(symbol_kind('_?$S13@?1??F@K@@QAEXH@Z@4EA'), 'local-static')
        self.assertEqual(symbol_kind('??_7Foo@@6B@'), 'vtable')
        self.assertEqual(symbol_kind('?g_x@@3HA'), 'global')

    def test_short_import_names(self):
        self.assertEqual(short_import_name('_Sleep@4', 3), 'Sleep')
        self.assertEqual(short_import_name('_foo', 2), 'foo')
        self.assertEqual(short_import_name('_foo@8', 1), '_foo@8')


class ScanTests(unittest.TestCase):
    def test_c_strings_are_aligned_and_terminated(self):
        data = b'abcd\0\0\0\0xy\0\0hello world\0\x01\x02'
        self.assertEqual(scan_c_strings(data, 0x1000), [(0x1000, 5), (0x100c, 12)])

    def test_x87_operand(self):
        self.assertEqual(x87_memory_operand(bytes.fromhex('d9 05 84 04 55 00')), (0x550484, 4))
        self.assertEqual(x87_memory_operand(bytes.fromhex('dc 0d e8 07 55 00')), (0x5507e8, 8))
        self.assertIsNone(x87_memory_operand(bytes.fromhex('d9 45 08')))
        self.assertIsNone(x87_memory_operand(bytes.fromhex('8b 05 00 00 55 00')))


class CoverageTests(unittest.TestCase):
    def test_first_claim_wins_and_summary(self):
        cov = Coverage({'.rdata': (0x1000, 0x1020), '.data': (0x2000, 0x2010)})
        self.assertEqual(cov.claim(0x1000, 8, 'vtables', 'confirmed', 'vt'), 8)
        self.assertEqual(cov.claim(0x1004, 8, 'float-constants', 'strong', 'f'), 4)
        self.assertEqual(cov.conflicts[('vtables', 'float-constants')], 4)
        cov.claim(0x2000, 0x20, 'game-globals', 'heuristic', 'g')  # clipped to the range
        s = cov.summary()
        self.assertEqual(s['.rdata']['categories']['vtables']['bytes'], 8)
        self.assertEqual(s['.rdata']['categories']['unattributed']['bytes'], 0x20 - 12)
        self.assertEqual(s['.data']['categories']['game-globals']['by_tier'], {'heuristic': 0x10})
        self.assertEqual(list(cov.runs('.rdata')), [(0x100c, 0x14)])


class LinkOptionTests(unittest.TestCase):
    def test_retail_like_header(self):
        h = {'subsystem': 2, 'subsystem_version': '4.0', 'image_base': 0x400000,
             'characteristics': 0x10f, 'reloc_dir_size': 0, 'file_alignment': 0x1000,
             'debug_dir_size': 0, 'has_idata_section': False, 'iat_section': '.rdata',
             'stack': (0x100000, 0x1000), 'heap': (0x100000, 0x1000), 'checksum': 0,
             'dll_characteristics': 0, 'linker_version': '6.00', 'entry_symbol': None}
        options = [r['option'] for r in derive_link_options(h)]
        self.assertIn('/SUBSYSTEM:WINDOWS', options)
        self.assertTrue(any(o.startswith('/FIXED') for o in options))
        self.assertTrue(any(o.startswith('/OPT:WIN98') for o in options))
        h['file_alignment'] = 0x200
        self.assertIn('/OPT:NOWIN98', [r['option'] for r in derive_link_options(h)])


class ResourceTests(unittest.TestCase):
    def test_tree_walk(self):
        # root -> type 3 -> id 1 -> lang 1033 -> data entry
        def directory(entries):
            return struct.pack('<IIHHHH', 0, 0, 0, 0, 0, len(entries)) + b''.join(
                struct.pack('<II', k, v) for k, v in entries)
        root = directory([(3, 0x80000000 | 24)])
        types = directory([(1, 0x80000000 | 48)])
        names = directory([(1033, 72)])
        data = struct.pack('<IIII', 0x5000 + 88, 4, 0, 0) + b'ICON'
        leaves = parse_resource_tree(root + types + names + data, 0x5000)
        self.assertEqual(leaves, [{'path': [3, 1, 1033], 'data_rva': 0x5058, 'size': 4,
                                   'codepage': 0, 'data_offset': 88}])


class GlobalsTests(unittest.TestCase):
    def test_extern_and_definitions(self):
        header = ('extern int g_a;          // 0x1\n'
                  'extern "C" const GUID IID_X;\n'
                  'extern Foo* g_list[];\n'
                  'extern int Function(int a);\n'
                  'extern "C" void* memset(void* d, int c, unsigned n);\n')
        self.assertEqual(extern_variables(header), {'g_a', 'IID_X', 'g_list'})
        self.assertTrue(defines_variable('int g_a = 3;\n', 'g_a'))
        self.assertTrue(defines_variable('Foo* g_list[4];\n', 'g_list'))
        self.assertTrue(defines_variable('static Mgr Holder::s_m(1, 2);\n', 'Holder::s_m'))
        self.assertTrue(defines_variable('TrackGame* g_TrackGame = &g_x;\n', 'g_TrackGame'))
        self.assertFalse(defines_variable('extern int g_a;\n', 'g_a'))
        self.assertFalse(defines_variable('    int g_a = 3;\n', 'g_a'))  # function scope
        self.assertFalse(defines_variable('int g_a(int x);\n', 'g_a'))
        self.assertFalse(defines_variable('// int g_a = 3;\n', 'g_a'))


if __name__ == '__main__':
    unittest.main()
