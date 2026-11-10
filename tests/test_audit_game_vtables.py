import unittest

from tools.audit_game_vtables import (Side, classify, parse_map, render, rtti_scope,
                                      symbol_owner, thunk_target)


def image(classes, vtables):
    """classes: {scope: [bases]}, vtables: {(scope, offset): [entries]}."""
    return {'classes': {s: {'name': s.split('@')[0], 'bases': b} for s, b in classes.items()},
            'vtables': {k: {'class': k[0].split('@')[0], 'scope': k[0], 'object_offset': k[1],
                            'vtable_va': 0x1000 + 0x100 * i, 'entries': e}
                        for i, (k, e) in enumerate(vtables.items())}}


CLASSES = {'Base': [], 'Derived': ['Base'], 'Leaf': ['Derived', 'Base']}
# Retail: Base slot 1 = 0x100, Derived overrides slot 1 with 0x200, Leaf inherits it.
RETAIL = image(CLASSES, {('Base', 0): [0x10, 0x100, 0x300],
                         ('Derived', 0): [0x20, 0x200, 0x300],
                         ('Leaf', 0): [0x30, 0x200, 0x300]})
SYMBOLS = {0x5010: ['??_EBase@@UAEPAXI@Z'], 0x5020: ['??_EDerived@@UAEPAXI@Z'],
           0x5030: ['??_ELeaf@@UAEPAXI@Z'], 0x5100: ['?Tick@Base@@UAEXXZ'],
           0x5200: ['?Tick@Derived@@UAEXXZ'], 0x5300: ['?Draw@Base@@UAEXXZ'],
           0x5400: ['__purecall'], 0x5500: ['?Fold@Other@@UAEXXZ']}


def kinds(report):
    return {(f['class'], f['object_offset'], f['slot']): f['kind'] for f in report['findings']}


class SymbolTests(unittest.TestCase):
    def test_owner_of_methods_specials_thunks_and_free_functions(self):
        self.assertEqual(symbol_owner('?Tick@NameOverlay@@UAEXXZ'), ('NameOverlay', 'Tick'))
        self.assertEqual(symbol_owner('?Tick@Inner@Outer@@UAEXXZ')[0], 'Inner@Outer')
        self.assertEqual(symbol_owner('??_GShadowCamera@@UAEPAXI@Z')[0], 'ShadowCamera')
        self.assertEqual(symbol_owner('??1Camera@@UAE@XZ')[0], 'Camera')
        self.assertEqual(symbol_owner('?Disable@D3DIMSoultreeCharacter@@WDAI@AEXXZ')[0],
                         'D3DIMSoultreeCharacter')
        self.assertEqual(symbol_owner('?FastInvSqrt@@YAMM@Z'), (None, 'FastInvSqrt'))
        self.assertEqual(symbol_owner('__purecall'), (None, '__purecall'))
        self.assertEqual(rtti_scope('.?AVInner@Outer@@'), 'Inner@Outer')

    def test_map_lists_every_symbol_at_a_folded_address(self):
        text = ('  Address         Publics by Value              Rva+Base     Lib:Object\n'
                ' 0001:00103dd0       ??_GShadowCamera@@UAEPAXI@Z 00504dd0 f i ShadowCamera.obj\n'
                ' 0001:00103dd0       ??_GPCCamera@@UAEPAXI@Z    00504dd0 f i ShadowCamera.obj\n'
                ' 0002:00000414       ??_7ArcadeObject@@6B@      00556414     ArcadeObject.obj\n')
        self.assertEqual(parse_map(text), {
            0x504dd0: ['??_GShadowCamera@@UAEPAXI@Z', '??_GPCCamera@@UAEPAXI@Z'],
            0x556414: ['??_7ArcadeObject@@6B@']})

    def test_thunks_and_forwarding_jumps_resolve_to_their_targets(self):
        # sub ecx,0x308; jmp +0x10
        self.assertEqual(thunk_target(bytes.fromhex('81e908030000e910000000'), 0x1000),
                         (0x1000 + 11 + 0x10, bytes.fromhex('81e908030000')))
        # vtordisp: sub ecx,[ecx-4]; sub ecx,0x54; jmp
        self.assertEqual(thunk_target(bytes.fromhex('2b49fc83e954e900000000'), 0x2000)[1],
                         bytes.fromhex('2b49fc83e954'))
        # Bare forwarding jmp: same target, no adjustment.
        self.assertEqual(thunk_target(bytes.fromhex('e9fbffffff'), 0x3000), (0x3000, None))
        self.assertIsNone(thunk_target(bytes.fromhex('33c0c3'), 0x4000))


class ClassifyTests(unittest.TestCase):
    def built(self, leaf_slot1=0x5200, derived_slot1=0x5200, base_slot2=0x5300):
        return image(CLASSES, {('Base', 0): [0x5010, 0x5100, base_slot2],
                               ('Derived', 0): [0x5020, derived_slot1, 0x5300],
                               ('Leaf', 0): [0x5030, leaf_slot1, 0x5300]})

    def test_matching_build_is_clean(self):
        report = classify(RETAIL, self.built(), SYMBOLS, {})
        self.assertEqual(report['findings'], [])
        self.assertEqual(report['summary'], {'ok': 9})

    def test_missing_override_runs_the_base_implementation(self):
        # The NameOverlay case: retail Derived overrides slot 1, build inherits Base.
        report = classify(RETAIL, self.built(derived_slot1=0x5100, leaf_slot1=0x5100), SYMBOLS, {})
        self.assertEqual(kinds(report), {('Derived', 0, 1): 'missing_override',
                                         ('Leaf', 0, 1): 'inherited_mismatch'})
        row = report['findings'][0]
        self.assertEqual(row['built_symbol'], '?Tick@Base@@UAEXXZ')
        self.assertTrue(row['owner_is_base'])
        self.assertIn('missing_override', render(report))

    def test_purecall_where_retail_has_a_body(self):
        report = classify(RETAIL, self.built(derived_slot1=0x5400, leaf_slot1=0x5400), SYMBOLS, {})
        self.assertEqual(set(kinds(report).values()), {'purecall'})
        # Both sides pure is fine.
        report = classify(RETAIL, self.built(base_slot2=0x5400), SYMBOLS, {},
                          retail_purecall={0x300})
        self.assertNotIn(('Base', 0, 2), kinds(report))

    def test_bindings_confirm_or_contradict_the_built_symbol(self):
        built = self.built(derived_slot1=0x5500, leaf_slot1=0x5500)
        ok = classify(RETAIL, built, SYMBOLS, {'?Fold@Other@@UAEXXZ': {0x200}})
        self.assertEqual(ok['findings'], [])
        bad = classify(RETAIL, built, SYMBOLS, {'?Fold@Other@@UAEXXZ': {0x999}})
        self.assertEqual(kinds(bad)[('Derived', 0, 1)], 'missing_override')

    def test_identical_tiny_bodies_are_folding_not_missing_overrides(self):
        bodies = {0x200: b'\x33\xc0\xc3', 0x5100: b'\x33\xc0\xc3'}
        side = Side(size=lambda va: len(bodies[va]) if va in bodies else None,
                    body=lambda va: bodies.get(va))
        built = self.built(derived_slot1=0x5100, leaf_slot1=0x5100)
        self.assertEqual(classify(RETAIL, built, SYMBOLS, {}, side, side)['findings'], [])
        bodies[0x5100] = b'\xb0\x01\xc3'
        report = classify(RETAIL, built, SYMBOLS, {}, side, side)
        self.assertTrue(report['findings'][0]['low_priority'])

    def test_extra_override_stub_and_thunk_differences(self):
        symbols = {**SYMBOLS, 0x5600: ['?Draw@Leaf@@UAEXXZ']}
        built = image(CLASSES, {('Base', 0): [0x5010, 0x5100, 0x5300],
                                ('Derived', 0): [0x5020, 0x5200, 0x5300],
                                ('Leaf', 0): [0x5030, 0x5200, 0x5600]})
        self.assertEqual(kinds(classify(RETAIL, built, symbols, {})),
                         {('Leaf', 0, 2): 'extra_override'})
        sizes = {0x200: 64, 0x5200: 1}
        side = Side(size=sizes.get)
        self.assertEqual(kinds(classify(RETAIL, self.built(), SYMBOLS, {}, side, side)),
                         {('Derived', 0, 1): 'stub', ('Leaf', 0, 1): 'stub'})
        retail_side = Side(resolve=lambda va: (va, b'\x83\xe9\x08') if va == 0x30 else (va, None))
        report = classify(RETAIL, self.built(), SYMBOLS, {}, retail_side, Side())
        self.assertEqual(kinds(report), {('Leaf', 0, 0): 'thunk_mismatch'})

    def test_absent_classes_and_slot_counts(self):
        built = image({'Base': [], 'Derived': ['Base']},
                      {('Base', 0): [0x5010, 0x5100], ('Derived', 0): [0x5020, 0x5200, 0x5300]})
        report = classify(RETAIL, built, SYMBOLS, {})
        self.assertEqual(report['summary']['class_absent'], 1)
        self.assertEqual(report['summary']['slot_count'], 1)
        self.assertEqual(report['slot_counts'][0]['built_slots'], 2)
        self.assertEqual(report['missing_vtables'][0]['class'], 'Leaf')


if __name__ == '__main__':
    unittest.main()
