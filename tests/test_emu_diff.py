"""tools/emu_diff.py: case-module sanity everywhere; differential runs where the private inputs exist.

The behavioural tests compile each case's SOURCE with authentic VC6 and its configured flags
(config/game_build.json), run retail and the candidate under unicorn at 53- and 24-bit precision
and require identical outcomes. Negative controls show that the cases see the rounding: the
four-pose blend without /Op, the trick pick without /QIfist and the radar clip with its old
two-hit reference must differ.
"""
from __future__ import annotations

import importlib.util
import os
import pickle
import random
import tempfile
import unittest
from pathlib import Path

from tools import emu_diff

ROOT = Path(__file__).resolve().parents[1]

# Case -> random cases per control word (and module attributes to override). Sized so that each
# previously fixed difference (TestBox 24-bit corners, ScoreBoard 53-bit eye distance,
# DropTextOverlay::Setup 24-bit y, Render3D light frame) shows up with the default seed when the
# old source shape returns.
CLEAN_CASES = {
    'four_pose': 60,
    'visibility_testbox': 400,
    'scoreboard_show_value': 400,
    'drop_text_setup': 400,
    'drop_text_tick': 400,
    'krusty_trick_pick': 400,
    'scene_tick': 400,
    'motion_start': 400,
    'motion_blend': 200,
    'particle_render3d': 200,
    'projected_shadow_render': 200,
    'projected_shadow_tint': 200,
    'terrain_shadow_slot30': 200,
    'd3dim_shadow_slot30': 200,
    # Out-of-int focal lengths differ by design (plain helper, see the case docstring).
    'terrain_prepare_geometry': (400, {'EXTREME_EVERY': 0}),
    # Small counts that still catch, with the default seed: the corner-3 normal and height
    # add order before d3c99491 (QueryGround), a reassociated transform-point row
    # (SetTransform) and round-half-up at the midpoint fistp ties (IntersectSegment).
    'terrain_query_ground': 40,
    'collision_set_transform': 40,
    'gridnode_intersect_segment': 80,
    # Second differential sweep (docs/GAME_BUILD.md): non-exact linked bodies that agree.
    'segment_segment_distance': 100,
    'rotate_pose': 100,
    'pose_rotation': 100,
    'character_slot7': 100,
    'transform_points': 100,
    'halve555': 60,
    'halve565': 60,
    'convert555_to8': 40,
    'bitstring_read': 200,
    'radar_line_circle': 200,
    'halve8888': 60,
    'texture_channel_difference': 60,
    'shadow_fill_triangle': 100,
    'car_position_at_time': 100,
    'grid_viewer_detail': 100,
    'bitstring_shift_front': 100,
    'bitstring_shift_back': 100,
    'texture_has_color': 100,
    'button_contains_point': 60,
    'multistate_contains_point': 60,
    'd3dim_scale': 60,
    'projected_shadow_bounds': 60,
    'terrain_shadow_slot28': 100,
    # Fixed by the second sweep: sized to show the old difference with the default seed.
    'bike_steer_torque': 100,
    'collision_hull_vs_hull': 300,
    'segment_triangle_query': 300,
    'soultree_set_matrix_in': 60,
    'soultree_set_axes_in': 60,
    'resolve_contact_impulse': 60,
    'contact_solve_impulse': 60,
    'constraint_contact_impulse': 60,
    'vehicle_wheel_impulse': 60,
    'vehicle_lean_from_normal': 60,
    'vehicle_body_contact_impulse': 60,
    'vehicle_ctor': 60,
    'collision_relative_frame': 60,
    'collision_hull_vs_hull_swept': 60,
    'collision_hull_vs_model': 40,
    'collision_hull_vs_model_swept': 40,
    'collision_model_vs_hull': 40,
    'collision_model_vs_hull_swept': 40,
    'collision_model_vs_model': 20,
    'collision_model_vs_model_swept': 20,
    'collision_update_model_bounds': 60,
    'collision_segment_touches_object': 60,
    'box_capsule_overlap': 30,
    # One open 1-ulp boundary flip at 53-bit (about 1 in 300, see the case); 0 at this size.
    'box_triangle_query': 60,
    'tree_tree_query': 60,
    'quadtree_insert': 100,
    'quadtree_remove': 100,
    'visibility_traverse': 150,
    'kb_speed_gain': 200,
    'kb_find_placement': 200,
    'kb_update_rider_pose': 500,
    'kb_send_delta': 500,
    'kb_ai_racing_line': 150,
    'vegetation_collision_object': 200,
    'track_place_near': 200,
    'car_procedural_tick': 200,
}

# Pre-fix source shapes the differential cases caught: (case, [(text, pre-fix text)], runs).
# Each must still differ from retail, so the case keeps its power to see the bug.
PRE_FIX_SHAPES = [
    ('radar_line_circle', [('reference[1] - y', 'point[1] - y'), ('reference[0] - x', 'point[0] - x')], 60),
    ('track_place_near', [('behind = DistanceAlong(start, from);',
                           'behind = DistanceAlong(candidates->pos, start);')], 200),
    ('kb_find_placement', [('return *(int*)&outPosition->z;', 'return (int)outPosition;')], 200),
]


class CaseModuleTests(unittest.TestCase):
    def test_every_case_module_declares_a_target_and_yields_cases(self):
        modules = sorted(emu_diff.CASES_DIR.glob('*.py'))
        self.assertTrue(modules)
        for path in modules:
            with self.subTest(case=path.stem):
                module = emu_diff.load_case(str(path))
                self.assertIsInstance(module.RETAIL_VA, int)
                self.assertTrue(module.SYMBOL.startswith('?'))
                self.assertTrue((ROOT / module.SOURCE).is_file(), module.SOURCE)
                cases = list(module.cases(random.Random(1), 4))
                self.assertTrue(cases)
                pickle.dumps(cases)
                self.assertTrue(callable(module.setup))

    def test_clean_cases_exist(self):
        for name in CLEAN_CASES:
            self.assertTrue((emu_diff.CASES_DIR / (name + '.py')).is_file(), name)

    @unittest.skipUnless(importlib.util.find_spec('unicorn') is not None, 'requires unicorn')
    def test_context_call_runs_code_to_its_return(self):
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        uc.mem_map(emu_diff.STUB, emu_diff.STUB_SIZE)
        uc.mem_map(emu_diff.HEAP, emu_diff.HEAP_SIZE)
        ctx = emu_diff.Context(uc, random.Random(1))
        self.assertEqual(ctx.alloc(16), emu_diff.HEAP)
        # mov eax, [esp+4]; add eax, ecx; mov [0x40000000], eax; ret 4
        code = bytes.fromhex('8b44240401c8a300000040c20400')
        at = ctx.alloc(len(code), code)
        self.assertEqual(ctx.call(at, [5], ecx=7), 12)
        self.assertEqual(ctx.unpack(emu_diff.HEAP, 'I'), (12,))

    def test_pack_args(self):
        packed = emu_diff.pack_args([1, -1, 0.5, emu_diff.Double(0.25), b'ab'])
        self.assertEqual(packed[:8], b'\x01\0\0\0\xff\xff\xff\xff')
        self.assertEqual(packed[8:12], b'\0\0\0\x3f')
        self.assertEqual(packed[12:20], b'\0\0\0\0\0\0\xd0\x3f')
        self.assertEqual(packed[20:], b'ab')


@unittest.skipUnless(os.name == 'nt' and os.environ.get('VC6_ROOT') and os.environ.get('MCM2_EXE')
                     and importlib.util.find_spec('unicorn') is not None,
                     'requires native Windows, VC6_ROOT, private MCM2_EXE and unicorn')
class DifferentialTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory(prefix='emu-diff-test-')
        cls.objects: dict[tuple, Path] = {}

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def compiled(self, source: str, flags: list[str] | None) -> Path:
        # Several cases share a source; compile each (source, flags) once.
        key = (source, None if flags is None else tuple(flags))
        if key not in self.objects:
            out = Path(self.tmp.name) / f'candidate{len(self.objects)}.obj'
            self.objects[key] = emu_diff.compile_source(source, out, flags)
        return self.objects[key]

    def run_case(self, name: str, n: int, flags: list[str] | None = None,
                 overrides: dict | None = None) -> emu_diff.Summary:
        module = emu_diff.load_case(name)
        for key, value in (overrides or {}).items():
            setattr(module, key, value)
        obj = self.compiled(module.SOURCE, flags)
        return emu_diff.run_diff(module, obj, n=n, retail=Path(os.environ['MCM2_EXE']), show=3)

    def test_linked_fistp_bodies_agree_with_retail(self):
        for name, spec in CLEAN_CASES.items():
            n, overrides = spec if isinstance(spec, tuple) else (spec, {})
            with self.subTest(case=name):
                summary = self.run_case(name, n, overrides=overrides)
                self.assertEqual(summary.mismatches, 0, summary.examples)
                self.assertEqual(summary.retail_faults, 0)
                for va, hits in summary.probes.items():
                    self.assertGreater(hits, 0, f'probe {va:#x} never reached')

    def test_four_pose_without_op_differs(self):
        # Without /Op VC6 keeps 80-bit intermediates that retail rounds to binary32.
        summary = self.run_case('four_pose', 20, ['/QIfist'])
        self.assertGreater(summary.mismatches, 0)

    def test_trick_pick_without_qifist_differs(self):
        # __ftol truncates where retail's fistp rounds to nearest.
        summary = self.run_case('krusty_trick_pick', 200, [])
        self.assertGreater(summary.mismatches, 0)

    def test_pre_fix_shapes_differ(self):
        for name, edits, n in PRE_FIX_SHAPES:
            with self.subTest(case=name):
                module = emu_diff.load_case(name)
                path = ROOT / module.SOURCE
                text = path.read_text(encoding='latin-1')
                for current, previous in edits:
                    self.assertIn(current, text)
                    text = text.replace(current, previous)
                with tempfile.TemporaryDirectory(prefix='emu-diff-test-') as tmp:
                    source = Path(tmp) / path.name
                    source.write_text(text, encoding='latin-1')
                    obj = emu_diff.compile_source(str(source), Path(tmp) / 'candidate.obj',
                                                  ['/I' + str(path.parent)])
                    summary = emu_diff.run_diff(module, obj, n=n, retail=Path(os.environ['MCM2_EXE']),
                                                show=1)
                self.assertGreater(summary.mismatches, 0)


if __name__ == '__main__':
    unittest.main()
