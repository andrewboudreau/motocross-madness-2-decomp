"""Synthetic tests: no game/SDK bytes are required or distributed."""
from pathlib import Path
import shutil
import struct
import tempfile
import unittest
from unittest.mock import patch

from mcm2tool.pe import PEImage, PEFormatError
from mcm2tool.provenance import (Instruction, build_map, checked_read, component_inventory,
                                cstring, decode, export_records, iat_reference,
                                import_records, instruction_sources, merge_ranges,
                                module_family, source_strings, version_fields, walk)


def synthetic_pe():
    data = bytearray(0x1000)
    data[:2] = b'MZ'
    struct.pack_into('<I', data, 0x3c, 0x80)
    data[0x80:0x84] = b'PE\0\0'
    struct.pack_into('<HHIIIHH', data, 0x84, 0x14c, 2, 0, 0, 0, 0xe0, 0x102)
    oh = 0x98
    struct.pack_into('<H', data, oh, 0x10b)
    for offset, value in [(16, 0x1000), (28, 0x400000), (32, 0x1000), (36, 0x200),
                          (56, 0x3000), (60, 0x200), (92, 16)]:
        struct.pack_into('<I', data, oh + offset, value)
    struct.pack_into('<II', data, oh + 104, 0x2000, 40)
    for i, (name, vs, va, rs, ro, flags) in enumerate([
        (b'.text', 0x100, 0x1000, 0x200, 0x200, 0x60000020),
        (b'.rdata', 0xc00, 0x2000, 0xc00, 0x400, 0x40000040),
    ]):
        at = oh + 0xe0 + i * 40
        data[at:at+len(name)] = name
        struct.pack_into('<IIIIIIHHI', data, at+8, vs, va, rs, ro, 0, 0, 0, 0, flags)
    struct.pack_into('<5I', data, 0x400, 0x2040, 0, 0, 0x2060, 0x2050)
    struct.pack_into('<3I', data, 0x440, 0x2080, 0x80000002, 0)
    struct.pack_into('<3I', data, 0x450, 0x2080, 0x80000002, 0)
    data[0x460:0x46b] = b'DSOUND.dll\0'
    struct.pack_into('<H', data, 0x480, 7)
    name = b'DirectSoundCreate\0'
    data[0x482:0x482+len(name)] = name
    source = b'D:\\aardvark\\VC\\krusty2\\Fixture.cpp\0'
    data[0x4a0:0x4a0+len(source)] = source
    data[0x200] = 0xc3
    return data


def version_block(key, value=b'', kind=1, children=()):
    out = bytearray(6) + key.encode('utf-16le') + b'\0\0'
    out += b'\0' * (-len(out) % 4)
    out += value
    for child in children:
        out += b'\0' * (-len(out) % 4)
        out += child
    struct.pack_into('<HHH', out, 0, len(out), len(value) // (2 if kind else 1), kind)
    return out


class ProvenanceTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.path = Path(self.temp.name) / 'fixture.exe'
        self.data = synthetic_pe()
        self.pe = self.load()

    def load(self):
        self.path.write_bytes(self.data)
        return PEImage(self.path)

    def test_named_and_ordinal_imports(self):
        rows = import_records(self.pe)
        self.assertEqual(len(rows), 2)
        self.assertEqual(rows[0]['name'], 'DirectSoundCreate')
        self.assertEqual(rows[0]['hint'], 7)
        self.assertIsNone(rows[1]['name'])
        self.assertEqual(rows[1]['ordinal'], 2)
        self.assertEqual(rows[0]['iat_va'], '0x00402050')
        self.assertEqual(rows[0]['iat_rva'], '0x00002050')

    def test_unbound_iat_fallback(self):
        struct.pack_into('<I', self.data, 0x400, 0)
        self.assertEqual(import_records(self.load())[0]['name'], 'DirectSoundCreate')

    def test_bound_iat_does_not_invent_symbol(self):
        struct.pack_into('<II', self.data, 0x400, 0, 1)
        struct.pack_into('<3I', self.data, 0x450, 0x70001000, 0x70002000, 0)
        rows = import_records(self.load())
        self.assertIsNone(rows[0]['name'])
        self.assertIsNone(rows[0]['ordinal'])
        self.assertEqual(rows[0]['resolution'], 'bound_iat_without_lookup')

    def test_import_descriptor_terminator_required(self):
        struct.pack_into('<I', self.data, 0x98 + 108, 20)
        with self.assertRaises(PEFormatError):
            import_records(self.load())

    def test_reads_do_not_cross_file_backing(self):
        with self.assertRaises(PEFormatError):
            checked_read(self.pe, 0x11ff, 2)
        with self.assertRaises(PEFormatError):
            checked_read(self.pe, -1, 4)

    def test_truncated_input_detected(self):
        self.data = self.data[:0x485]
        with self.assertRaises(PEFormatError):
            import_records(self.load())

    def test_source_paths_are_literal_anchors(self):
        rows = source_strings(self.pe)
        self.assertEqual(rows[0x4020a0], 'D:\\aardvark\\VC\\krusty2\\Fixture.cpp')
        ins = Instruction(0x401000, bytes(5), 'push', '0x4020a0')
        self.assertEqual(instruction_sources(ins, rows)[0]['string_va'], '0x004020a0')
        self.assertEqual(instruction_sources(Instruction(0, bytes(5), 'cmp', 'eax,0x4020a0'), rows), [])
        self.assertEqual(instruction_sources(Instruction(0, bytes(5), 'mov', 'eax,DWORD PTR ds:0x4020a0'), rows), [])

    def test_named_ordinal_exports_and_forwarders(self):
        struct.pack_into('<II', self.data, 0x98 + 96, 0x2200, 0x100)
        struct.pack_into('<IIHH7I', self.data, 0x600, 0, 0, 0, 0, 0x2260, 10, 3, 1, 0x2240, 0x2250, 0x2258)
        struct.pack_into('<3I', self.data, 0x640, 0x1000, 0x2270, 0)
        struct.pack_into('<I', self.data, 0x650, 0x2280)
        struct.pack_into('<H', self.data, 0x658, 0)
        for at, value in [(0x660, b'x.dll\0'), (0x670, b'Y.Target\0'), (0x680, b'Entry\0')]:
            self.data[at:at+len(value)] = value
        rows = export_records(self.load())
        self.assertEqual(len(rows), 2)
        self.assertEqual(rows[0]['ordinal'], 10)
        self.assertEqual(rows[0]['names'], ['Entry'])
        self.assertEqual(rows[1]['names'], [])
        self.assertEqual(rows[1]['forwarder'], 'Y.Target')
        self.assertIsNone(rows[1]['preferred_va'])

    def test_version_resource_hierarchy(self):
        company = version_block('CompanyName', 'Fixture Co\0'.encode('utf-16le'))
        table = version_block('040904b0', children=[company])
        strings = version_block('StringFileInfo', children=[table])
        version = version_block('VS_VERSION_INFO', bytes(52), 0, [strings])
        struct.pack_into('<II', self.data, 0x98+112, 0x2300, 0x500)
        for offset, key, value in [(0, 16, 0x80000018), (24, 1, 0x80000030), (48, 1033, 72)]:
            struct.pack_into('<H', self.data, 0x700 + offset + 14, 1)
            struct.pack_into('<II', self.data, 0x700 + offset + 16, key, value)
        struct.pack_into('<4I', self.data, 0x748, 0x2400, len(version), 0, 0)
        self.data[0x800:0x800+len(version)] = version
        self.assertEqual(version_fields(self.load()), {'CompanyName': ['Fixture Co']})

    def test_no_version_metadata_from_arbitrary_string_pairs(self):
        self.data[0x800:0x82a] = 'CompanyName\0Bogus Co\0'.encode('utf-16le')
        self.assertEqual(version_fields(self.load()), {})

    def test_unknown_dll_not_called_windows_or_rainbow(self):
        self.assertEqual(module_family('mystery.dll'), 'unclassified_external')
        self.assertEqual(module_family('blade.dll'), 'unclassified_external')
        self.assertEqual(module_family('KERNEL32.dll'), 'windows')
        self.assertEqual(module_family('d3drm.dll'), 'directx')

    def test_import_calls_identified_at_local_iat(self):
        imports = {int(x['iat_va'], 16): x for x in import_records(self.pe)}
        ins = Instruction(0x401000, bytes(6), 'call', 'DWORD PTR ds:0x402050')
        self.assertEqual(iat_reference(ins, imports)['module'], 'DSOUND.dll')
        self.assertIsNone(iat_reference(Instruction(0, bytes(2), 'call', 'DWORD PTR [eax+0x10]'), imports))

    def test_cfg_never_includes_callee_body(self):
        ins = {1: Instruction(1, bytes(5), 'call', '10'),
               6: Instruction(6, bytes(1), 'ret', ''),
               16: Instruction(16, bytes(1), 'ret', '')}
        reached, _ = walk(1, ins, {1, 16})
        self.assertEqual(reached, [1, 6])

    def test_cfg_stops_at_another_entry(self):
        ins = {1: Instruction(1, bytes(1), 'nop', ''), 2: Instruction(2, bytes(1), 'ret', '')}
        reached, stops = walk(1, ins, {1, 2})
        self.assertEqual(reached, [1])
        self.assertIn('other_candidate_entry', stops)

    def test_cfg_records_unresolved_indirect_jump(self):
        ins = {1: Instruction(1, bytes(2), 'jmp', 'eax')}
        self.assertEqual(walk(1, ins, {1})[1], ['indirect_jump'])

    def test_cfg_terminates_on_a_loop(self):
        ins = {1: Instruction(1, bytes(2), 'jne', '1'), 3: Instruction(3, bytes(1), 'ret', '')}
        self.assertEqual(walk(1, ins, {1})[0], [1, 3])

    def test_coverage_union_not_double_counted(self):
        self.assertEqual(merge_ranges([(1, 4), (3, 6), (8, 10), (9, 9)]), [[1, 6], [8, 10]])

    def test_importing_directx_does_not_reclassify_caller(self):
        ins = {0x401000: Instruction(0x401000, bytes(6), 'call', 'DWORD PTR ds:0x402050'),
               0x401006: Instruction(0x401006, bytes(1), 'ret', '')}
        with patch('mcm2tool.provenance.parse_rtti', return_value=[]):
            result = build_map(self.pe, ins, import_records(self.pe))
        row = result['functions'][0]
        self.assertEqual(row['owner'], 'unknown')
        self.assertEqual(row['external_calls'][0]['module'], 'DSOUND.dll')
        self.assertIsNone(row['original_translation_unit'])
        self.assertFalse(result['summary']['coverage_is_decomp_progress'])

    def test_source_reference_does_not_become_original_translation_unit(self):
        ins = {0x401000: Instruction(0x401000, bytes(5), 'push', '0x4020a0'),
               0x401005: Instruction(0x401005, bytes(1), 'ret', '')}
        with patch('mcm2tool.provenance.parse_rtti', return_value=[]):
            row = build_map(self.pe, ins, [])['functions'][0]
        self.assertEqual(row['owner'], 'rainbow_project_associated')
        self.assertIsNone(row['original_translation_unit'])

    def test_packaged_is_not_the_same_as_direct_import(self):
        dll = self.path.parent / 'packaged.dll'
        dll.write_bytes(self.data)
        rows = component_inventory(self.path.parent, import_records(self.pe))
        packaged = next(r for r in rows if r['module'] == 'packaged.dll')
        self.assertFalse(packaged['direct_import'])
        self.assertEqual(packaged['family'], 'unclassified_external')
        missing = next(r for r in rows if r['module'] == 'dsound.dll')
        self.assertEqual(missing['availability'], 'not_in_supplied_package')

    @unittest.skipUnless(shutil.which('objdump'), 'GNU binutils not installed')
    def test_real_objdump_parsing_of_synthetic_file(self):
        rows, version = decode(self.pe)
        self.assertEqual(rows[0x401000].mnemonic, 'ret')
        self.assertTrue(version)


if __name__ == '__main__':
    unittest.main()
