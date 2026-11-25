"""tools/check_signatures.py: the MSVC parameter decoder, and (with the retail exe) every bound
signature agrees with retail's stack bytes and no const pointer parameter is stored through."""
import os
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))

import check_signatures as ccp  # noqa: E402


class DecoderTests(unittest.TestCase):
    def args(self, name):
        return ccp.Demangler(name).parse()

    def test_free_function_pointer_constness(self):
        args, hidden, thiscall = self.args('?TransformPoints@@YAXPAXPBXPBUMatrix4@@HHH@Z')
        self.assertEqual([(a[0], a[1]) for a in args],
                         [('ptr', False), ('ptr', True), ('ptr', True), ('val', False), ('val', False), ('val', False)])
        self.assertFalse(hidden)
        self.assertFalse(thiscall)

    def test_member_backreferences_keep_constness(self):
        # `1` repeats the second argument type (const Vec3*), not the pointee.
        args, _, thiscall = self.args('?SetAxesIn@SoultreeObject@@QAEXPAV1@PBUVec3@@1HH@Z')
        self.assertEqual([a[1] for a in args if a[0] == 'ptr'], [False, True, True])
        self.assertTrue(thiscall)

    def test_bool_parameters_register_as_backreferences(self):
        args, _, _ = self.args('?TrySettleOnPoint@VehicleCamera@@UAE_NABUVector3@@_N1@Z')
        self.assertEqual(len(args), 3)
        self.assertTrue(args[0][1])

    def test_constructor_has_no_return_type(self):
        args, hidden, thiscall = self.args('??0CollisionPoint@@QAE@MH@Z')
        self.assertEqual([a[0] for a in args], ['val', 'val'])
        self.assertFalse(hidden)
        self.assertTrue(thiscall)

    def test_class_return_takes_the_hidden_pointer(self):
        _, hidden, _ = self.args('?LocalToWorldPoint@SoultreeObject@@QAE?AUVec3@@ABU2@@Z')
        self.assertTrue(hidden)

    def test_data_and_string_symbols_are_rejected(self):
        for name in ('?g_pQuadTree@@3PAVQuadTree@@A', '??_C@_08EMDP@Quarry05?$AA@', '_GUID_ZAxis'):
            with self.assertRaises(Exception):
                self.args(name)


@unittest.skipUnless(os.environ.get('MCM2_EXE'), 'needs the retail executable (MCM2_EXE)')
class RetailTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        from mcm2tool.pe import PEImage
        cls.pe = PEImage(os.environ['MCM2_EXE'])

    def test_no_const_parameter_is_written(self):
        flagged, checked, _ = ccp.scan(self.pe)
        self.assertGreater(checked, 500)
        self.assertEqual([(hex(f[0]), f[1], f[2]) for f in flagged if f[3]], [])

    def test_declared_stack_bytes_match_retail(self):
        findings, checked = ccp.check_stack_bytes(self.pe)
        self.assertGreater(checked, 3000)
        self.assertEqual([(hex(f[0]), f[1], f[2], f[3]) for f in findings], [])

    def test_wrong_declarations_are_reported(self):
        # Terrain::QueryGround pops 16 bytes; a three-argument declaration must be caught, and
        # so must the retired view that declared its output normal const.
        short = '?QueryGround@Terrain@@QAEXPAUTerrainVec3@@0H@Z'
        findings, _ = ccp.check_stack_bytes(self.pe, {short: (0x00507c10, 'planted')})
        self.assertEqual([(f[2], f[3]) for f in findings], [(12, [16])])
        view = '?QueryGround@CollisionFrameHelper@@QAEXPAUCollisionVec3@@PBU2@HPAD@Z'
        flagged, _, _ = ccp.scan(self.pe, bindings={view: (0x00507c10, 'planted')})
        self.assertEqual([(f[2], f[3]) for f in flagged], [(1, True)])

    def test_virtual_base_constructors_take_the_flag(self):
        self.assertIn('SoultreePhysicsBaseObject', ccp.virtual_base_classes())
        findings, _ = ccp.check_stack_bytes(self.pe, {'??0SoultreePhysicsBaseObject@@QAE@H@Z': (0x00500aa0, 'x')})
        self.assertEqual(findings, [])


if __name__ == '__main__':
    unittest.main()
