"""tools/audit_bindings.py: synthetic retail/build fixtures (no exe, VC6 or build needed)."""
import json
import struct
import tempfile
import unittest
from collections import defaultdict
from pathlib import Path

from tools.audit_bindings import (
    REL_DIR32, Auditor, BuildIndex, Defined, address_in_name, base_name, compare_data,
    guid_identifier, parse_define_guids, vtable_class)

TEXT_LO, TEXT_HI = 0x401000, 0x550000
DATA_LO, DATA_HI = 0x550000, 0x68d000

# DirectPlay's address types (dplobby.h), the pair the CreateAddress bindings swapped.
DPLOBBY = """
DEFINE_GUID(DPAID_Phone, 0x78ec89a0, 0xe0af, 0x11cf, 0x9c, 0x4e, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);
DEFINE_GUID(DPAID_Modem, 0xf6dcc200, 0xa2fe, 0x11d0, 0x9c, 0x4f, 0x0, 0xa0, 0xc9, 0x5, 0x42, 0x5e);
"""
UUIDS = 'OUR_GUID_ENTRY(MEDIATYPE_Video,\n0x73646976, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71)'
MIDL = '''EXTERN_C const IID IID_IAMMultiMediaStream;
    MIDL_INTERFACE("BEBE595C-9A6F-11d0-8FDE-00C04FD9189D")
    interface DECLSPEC_UUID("BEBE595C-9A6F-11d0-8FDE-00C04FD9189D") IAMMultiMediaStream'''


class FakeRetail:
    """The parts of ``Retail`` the auditor uses, from {va: bytes} blocks."""

    def __init__(self, blocks=None, atlas=None, vtables=None, bodies=None):
        self.blocks = blocks or {}
        self.text_lo, self.text_hi, self.data_lo, self.data_hi = TEXT_LO, TEXT_HI, DATA_LO, DATA_HI
        self.vtables = vtables or {}
        self.rtti = {}
        self.iat_by_va = {}
        self.atlas = defaultdict(set)
        self.atlas_names = defaultdict(set)
        for name, va in (atlas or {}).items():
            self.atlas[name].add(va)
            self.atlas_names[va].add(name)
        self.bodies = bodies or {}

    def read(self, va, size):
        out = bytearray(size)
        for base, blob in self.blocks.items():
            lo, hi = max(va, base), min(va + size, base + len(blob))
            if lo < hi:
                out[lo - va:hi - va] = blob[lo - base:hi - base]
        return bytes(out)

    def is_code(self, va):
        return TEXT_LO <= va < TEXT_HI

    def is_mapped(self, va):
        return TEXT_LO <= va < DATA_HI

    def is_function_start(self, va):
        return 'aligned' if va % 16 == 0 else None

    def body(self, va, limit=0x400):
        return self.bodies.get(va, b'\x90' * 32)


class FakeBuild(BuildIndex):
    def __init__(self, data=(), bodies=None):
        super().__init__(None)
        for d in data:
            self.data[d.name].append(d)
        self.bodies = bodies or {}

    def body(self, name):
        return self.bodies.get(name)

    def is_static(self, name):
        return False


def body(raw, mask=None):
    return raw, mask or bytes(len(raw))


def auditor(files, retail=None, build=None, guids=None, aliases=None):
    with tempfile.TemporaryDirectory() as tmp:     # the auditor reads the config at once
        root = Path(tmp)
        (root / 'config').mkdir()
        (root / 'config/game_build.json').write_text(json.dumps({'link_aliases': aliases or {}}))
        return Auditor(retail or FakeRetail(), build or FakeBuild(), guids or {}, files, root=root)


class NameTests(unittest.TestCase):
    def test_sdk_guid_forms(self):
        guids = parse_define_guids(DPLOBBY + UUIDS + MIDL)
        self.assertEqual(guids['DPAID_Phone'][:4], struct.pack('<I', 0x78ec89a0))
        self.assertEqual(guids['MEDIATYPE_Video'][:4], b'vids')
        self.assertEqual(guids['IID_IAMMultiMediaStream'],
                         struct.pack('<IHH', 0xBEBE595C, 0x9A6F, 0x11d0) + bytes.fromhex('8FDE00C04FD9189D'))
        self.assertEqual(guid_identifier('_DPAID_Modem'), 'DPAID_Modem')
        self.assertEqual(guid_identifier('?IID_IFoo@@3U_GUID@@B'), 'IID_IFoo')
        self.assertIsNone(guid_identifier('_g_modem'))

    def test_names(self):
        self.assertEqual(base_name('?s_count@Grid@@3HA'), 'Grid::s_count')
        self.assertEqual(base_name('_g_value'), 'g_value')
        self.assertIsNone(base_name('??_7Tire@@6B@'))
        self.assertEqual(address_in_name('_s_gridCounter68a088', DATA_LO, DATA_HI), 0x68a088)
        self.assertEqual(address_in_name('?g_UnknownGlobal59af14@@3HA', DATA_LO, DATA_HI), 0x59af14)
        self.assertIsNone(address_in_name('_g_frameCount', DATA_LO, DATA_HI))
        self.assertEqual(vtable_class('??_7Tire@@6B@'), ('Tire', None))
        self.assertEqual(vtable_class('??_7Tire@@6BGraphicsTest@@@'), ('Tire', 'GraphicsTest'))
        self.assertEqual(vtable_class('??_7Inner@Outer@@6B@'), ('Outer::Inner', None))


class CompareTests(unittest.TestCase):
    def test_padding_after_the_object_is_equal(self):
        ours = Defined('_g', 'a.cpp', '.data', b'\1\2\3\0')
        self.assertEqual(compare_data(ours, b'\1\2\3\x7f', {})['diffs'], [])
        self.assertEqual(compare_data(ours, b'\1\5\3\0', {})['diffs'], [1])
        exact = Defined('_g', 'a.cpp', '<common>', b'\1\2\3\0', exact_size=True)
        self.assertEqual(compare_data(exact, b'\1\2\3\x7f', {})['diffs'], [3])

    def test_pointers_compare_by_target(self):
        ours = Defined('_p', 'a.cpp', '.data', b'\0' * 8, [(0, REL_DIR32, '_t', False, 0)])
        good = compare_data(ours, struct.pack('<II', 0x566010, 0), {0: 0x566010})
        self.assertEqual((good['diffs'], good['pointer_diffs']), ([], []))
        bad = compare_data(ours, struct.pack('<II', 0x566014, 0), {0: 0x566010})
        self.assertEqual(bad['pointer_diffs'][0]['retail'], '0x00566014')
        self.assertEqual(compare_data(ours, b'\0' * 8, {0: None})['unknown_pointers'], 1)


class AuditTests(unittest.TestCase):
    def test_swapped_guid_bindings_are_mismatches(self):
        guids = {k: {v} for k, v in parse_define_guids(DPLOBBY).items()}
        retail = FakeRetail({0x556900: guids['DPAID_Phone'].copy().pop(),
                             0x556920: guids['DPAID_Modem'].copy().pop()})
        files = {'src/Net.bindings.json': {'_DPAID_Modem': 0x556900, '_DPAID_Phone': 0x556920}}
        rows = {r['symbol']: r for r in auditor(files, retail, guids=guids).run()['rows']}
        self.assertEqual(rows['_DPAID_Modem']['status'], 'mismatch')
        self.assertIn('DPAID_Phone', rows['_DPAID_Modem']['detail'])
        files = {'src/Net.bindings.json': {'_DPAID_Phone': 0x556900, '_DPAID_Modem': 0x556920}}
        report = auditor(files, retail, guids=guids).run()
        self.assertEqual(report['summary']['findings'], 0)

    def test_crt_names_must_match_the_atlas(self):
        retail = FakeRetail(atlas={'__stricmp': 0x536070, '_strcmp': 0x5382a0, '_strrchr': 0x534a60})
        files = {'src/Bike.bindings.json': {'_strcmp': 0x536070},
                 'src/Ui.bindings.json': {'_strchr': 0x534a60},
                 'src/Ok.bindings.json': {'__stricmp': 0x536070}}
        report = auditor(files, retail).run()
        rows = {(r['file'], r['symbol']): r for r in report['rows']}
        self.assertIn('0x005382a0', rows['src/Bike.bindings.json', '_strcmp']['detail'])
        self.assertIn('_strrchr', rows['src/Ui.bindings.json', '_strchr']['detail'])
        self.assertEqual(rows['src/Ok.bindings.json', '__stricmp']['status'], 'match')
        self.assertEqual({c['class'] for c in report['va_collisions']}, {'collision'})

    def test_zero_initialised_definition_of_initialised_retail_data(self):
        build = FakeBuild([Defined('_g_quality', 'src/T.cpp', '.bss', b'\0' * 4, bss=True)])
        retail = FakeRetail({0x57471c: struct.pack('<I', 1)})
        rows = auditor({'src/T.bindings.json': {'_g_quality': 0x57471c}}, retail, build).run()['rows']
        self.assertEqual(rows[0]['status'], 'mismatch')
        rows = auditor({'src/T.bindings.json': {'_g_quality': 0x57471c}}, FakeRetail(), build).run()['rows']
        self.assertEqual(rows[0]['status'], 'uninitialised')

    def test_link_alias_takes_the_provider_definition(self):
        build = FakeBuild([Defined('_g_shared', 'src/G.cpp', '.data', struct.pack('<I', 7))])
        retail = FakeRetail({0x575a98: struct.pack('<I', 7)})
        files = {'samples/S.bindings.json': {'_g_view': 0x575a98}, 'src/G.bindings.json': {'_g_shared': 0x575a98}}
        report = auditor(files, retail, build, aliases={'_g_view': '_g_shared'}).run()
        rows = {r['symbol']: r for r in report['rows']}
        self.assertEqual(rows['_g_view']['status'], 'match')
        self.assertEqual(rows['_g_view']['evidence'], 'link alias public')
        files['src/G.bindings.json'] = {'_g_shared': 0x575a9c}
        report = auditor(files, retail, build, aliases={'_g_view': '_g_shared'}).run()
        self.assertEqual(report['summary']['link_alias_conflicts'], 1)

    def test_vtable_subobject_comes_from_our_col(self):
        col = Defined('??_R4Tire@@6B@', 'src/Tire.cpp', '.rdata$r', struct.pack('<III', 0, 184, 0))
        vtables = {0x5584fc: {'class': 'Tire', 'object_offset': 184},
                   0x558578: {'class': 'Tire', 'object_offset': 0}}
        files = {'src/Tire.bindings.json': {'??_7Tire@@6B@': 0x5584fc}}
        rows = auditor(files, FakeRetail(vtables=vtables), FakeBuild([col])).run()['rows']
        self.assertEqual((rows[0]['status'], rows[0]['evidence']), ('match', 'COL object offset'))
        files = {'src/Tire.bindings.json': {'??_7Tire@@6B@': 0x558578}}
        rows = auditor(files, FakeRetail(vtables=vtables), FakeBuild([col])).run()['rows']
        self.assertEqual(rows[0]['status'], 'mismatch')

    def test_collision_classes(self):
        same, other = body(b'\x8b\xc1\xc3' * 4), body(b'\x33\xc0\xc3' * 4)
        build = FakeBuild(
            [Defined('_kOne', 'a.cpp', '.rdata', struct.pack('<f', 1.0)),
             Defined('_kAlsoOne', 'b.cpp', '.rdata', struct.pack('<f', 1.0)),
             Defined('_kTwo', 'c.cpp', '.rdata', struct.pack('<f', 2.0))],
            {'?A@X@@QAEXXZ': same, '?A@Y@@QAEXXZ': same,
             '?B@X@@QAEXXZ': same, '?B@Stub@@QAEXXZ': other})
        files = {'src/a.bindings.json': {'_kOne': 0x550100, '_kAlsoOne': 0x550100,
                                         '_kTwo': 0x550200, '_kOneAgain': 0x550200,
                                         '?A@X@@QAEXXZ': 0x401000, '?A@Y@@QAEXXZ': 0x401000,
                                         '?B@X@@QAEXXZ': 0x401100, '?B@Stub@@QAEXXZ': 0x401100,
                                         '?C@X@@QAEXXZ': 0x401200, '?C@View@@QAEXXZ': 0x401200,
                                         '??_EX@@UAEPAXI@Z': 0x401300, '??_GX@@UAEPAXI@Z': 0x401300}}
        files['src/b.bindings.json'] = {'_kTwo': 0x550100, '?C@X@@QAEXXZ': 0x401200}
        report = auditor(files, build=build).run()
        classes = {c['va']: c['class'] for c in report['va_collisions']}
        self.assertEqual(classes['0x00550100'], 'collision')    # 1.0 and 2.0 at one address
        self.assertEqual(classes['0x00550200'], 'alias')        # one definition
        self.assertEqual(classes['0x00401000'], 'fold')
        self.assertEqual(classes['0x00401100'], 'distinct-bodies')
        self.assertEqual(classes['0x00401200'], 'alias')        # no bodies: callers' declarations
        self.assertEqual(classes['0x00401300'], 'deleting-dtor')
        conflicts = {c['symbol']: c['class'] for c in report['symbol_conflicts']}
        self.assertEqual(conflicts, {'_kTwo': 'conflict'})


if __name__ == '__main__':
    unittest.main()
