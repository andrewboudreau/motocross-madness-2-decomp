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
    # The 53-bit 1-ulp boundary flip is fixed (see the case); the old shape needs n >= 600.
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
    # Call-record/replay cases (docs/GAME_BUILD.md): every call leaving the body is replayed.
    'vehicle_simulate_frame': 40,
    'kb_ai_step': 100,
    'kb_net_interpolate': 100,
    'game_tick': 60,
    # Third sweep (docs/GAME_BUILD.md).
    'tire_update_contact_patch': 100,
    'shadow_transform_projective': 60,
    'terrain_lookup_cell': 100,
    'grid_walk_visibility': 40,
    'grid_vertex_cache_get_vertex': 150,
    'shadow_caster_vertex_count': 100,
    'grid_light_vertices': 100,
    'net_receive_messages': 100,
    'net_handle_received_message': 120,
    'net_dispatch_messages': 60,
    'radar_draw_racers': 100,
    'stats_draw_standings': 80,
    'tire_update_suspension_probe': 60,
    'shock_rotating_solve_contact': 60,
    'wrecker_push_probes': 60,
    'wrecker_spray_particles': 60,
    'box_overlap': 40,
    'collision_test_hull_against': 100,
    'collision_test_model_against': 100,
    'collision_free_shape': 100,
    'gather_model_triangles': 60,
    'vegetation_test_distance': 60,
    'kb_simulate_frame': 60,
    'eco_build_collision_objects': 60,
    'camera_update_matrices': 60,
    'follow_camera_tick': 80,
    'follow_camera_process_input': 100,
    'follow_camera_constructor': 20,
    'follow_camera_load_file': 80,
    'krusty_bike_camera_tick': 50,
    'joystick_read_buffered_state': 80,
    'joystick_read_immediate_state': 50,
    'vcr_interface_tick': 100,
    'kb_set_followed_racer': 50,
    'kb_path_steering': 100,
    'kb_query_segment_obstacles': 60,
    'build_status_list': 100,
    'pay_out_race': 60,
    'track_game_mode_reset_defaults': 10,
    'pick_followed_racer': 100,
    'kb_select_rider_pose': 200,
    # Third sweep, bodies the replay cases also cover.
    'eco_generate_objects': 15,
    'bikerace_aim_camera': 100,
    'bikerace_next_camera_target': 100,
    'bikerace_write_relative_positions': 150,
    'visual_cue_tick': 100,
    'kb_simulate_look_ahead': 20,
    # Replay sweep (exact at both control words).
    'kb_look_ahead': 40,
    'morph_controller_angles': 40,
    'morph_modify_mesh': 40,
    # Gameplay sweep: linked bodies below 1.000 masked that no case covered.
    'net_create_address': 100,
    'race_sound_refill_stream': 100,
    'sound_set_primary_format': 100,
    'recorder_thread': 100,
    'bailout_dlg_handle_event': 100,
    'bunny_dlg_handle_event': 100,
    'sp_race_info_handle_event': 100,
    # Loading-dialog close path (EndDialog -> RenderPost3D -> MarkPendingRelease ->
    # ReleaseDetached, AdvanceFrame): the backdrop-stays-up report of 2026-10-30.
    'ui_dialog_end_dialog': 100,
    'ui_dialog_render_post3d': 100,
    'game_object_mark_pending_release': 100,
    'game_object_release_detached': 100,
    'render_target_advance_frame': 100,
    'loading_dlg_handle_event': 100,
    # Loader sweep: file loaders and parsers fed synthetic contents (edge lengths, unknown tokens).
    'eco_read_est': 60,
    'tga_write': 100,
    'grid_read_node': 60,
    'morph_load_parameter_file': 40,
    'bike_load_bike': 40,
    'overlay_load_font': 100,
    'mp_fill_bike_rider_lists': 100,
    'dlg_read_resource': 100,
    'projected_shadow_init': 100,
    'terrain_load': 100,
    'bike_race_create': 60,
    # Render/texture/text/UI sweep: pixel conversion, texture building, text layout and UI layout.
    'halve1555': 60,
    'dither_convert': 60,
    'grid_build_node_texture': 60,
    'grid_build_block_texture': 60,
    'grid_rebuild_block_buffers': 60,
    'overlay_draw_text': 30,
    'overlay_draw_text_1555': 40,
    'overlay_draw_text_4444': 40,
    'overlay_layout_text': 60,
    'dropdown_layout_parts': 60,
    'graphics_test_draw_circle': 60,
    'eco_render3d': 30,
    'soultree_prepare_geometry': 20,
    'soultree_draw_current_lod': 80,
}

# Pre-fix source shapes the differential cases caught: (case, [(text, pre-fix text)], runs).
# Each must still differ from retail, so the case keeps its power to see the bug.
PRE_FIX_SHAPES = [
    ('radar_line_circle', [('reference[1] - y', 'point[1] - y'), ('reference[0] - x', 'point[0] - x')], 60),
    ('track_place_near', [('behind = DistanceAlong(start, from);',
                           'behind = DistanceAlong(candidates->pos, start);')], 200),
    ('kb_find_placement', [('return *(int*)&outPosition->z;', 'return (int)outPosition;')], 200),
    # The smoother kept its target in a register (53-bit only): 1 ulp in smoothedForwardAccel.
    ('kb_ai_step', [('BikeAISmoothValue(VehicleSmoother* s, float target, float dt)',
                     'BikeAISmoothValue(VehicleSmoother* s, float dt, float target)'),
                    ('(scratchVector.z - prevLocalForwardVelocity) / frameTime, frameTime);',
                     'frameTime, (scratchVector.z - prevLocalForwardVelocity) / frameTime);'),
                    ('    s->blendFactor = (dt < timeConstant ? dt : timeConstant) / s->timeConstant;\n'
                     '    return s->smoothedValue = (target - s->smoothedValue) * s->blendFactor + s->smoothedValue;',
                     '    float step = dt;\n    if (!(step < timeConstant)) step = timeConstant;\n'
                     '    s->blendFactor = step / s->timeConstant;\n'
                     '    s->smoothedValue = (target - s->smoothedValue) * s->blendFactor + s->smoothedValue;\n'
                     '    return s->smoothedValue;')], 100),
    ('bikerace_write_relative_positions', [('float length = Vec3Dot(&v, &v);', 'float length = SquareMagnitude(v);'),
                                           ('return Vec3ScaleCall(v, FastInvSqrt(length));',
                                            'float scale = FastInvSqrt(length);\n    return Scale(v, scale);')], 200),
    ('bikerace_write_relative_positions', [('*a = AddCall(ScaleCall(gridSlotStep, (float)count), gridFirstSlot);',
                                           '*a = Scale(gridSlotStep, (float)count) + gridFirstSlot;')], 200),
    ('kb_simulate_look_ahead', [('SafeNormalize(BikeAICrossCtor(fwd, ', 'SafeNormalize((')], 60),
    ('kb_simulate_look_ahead', [('            groundR = back;', '            Vec3 groundR = back;')], 20),
    ('kb_simulate_look_ahead', [('sideV = Vec3(lean.x * k, lean.y * k, lean.z * k);', 'sideV = lean * k;')], 20),
    # The replay case sees the lean blend's rounding at 53-bit (13 of 30), which stays exact there.
    ('kb_look_ahead', [('sideV = Vec3(lean.x * k, lean.y * k, lean.z * k);', 'sideV = lean * k;')], 30),
    # ControllerAngles: the flat four-term sum of the controller product (row 0, column 0 alone
    # shows at 24-bit), and the x block's rotation with t*y and t*z kept unrounded.
    ('morph_controller_angles',
     [('(world(0, 1) * inverse(1, 0) + (world(0, 2) * inverse(2, 0) + world(0, 3) * inverse(3, 0)))',
       'world(0, 1) * inverse(1, 0) + world(0, 2) * inverse(2, 0) + world(0, 3) * inverse(3, 0)')], 60),
    ('morph_controller_angles',
     [('    float ty = t * y;\n    rotation.m[0][1] = x * ty + s * z;\n    float tz = t * z;\n'
       '    rotation.m[0][2] = x * tz - s * y;\n',
       '    rotation.m[0][1] = t * x * y + s * z;\n    rotation.m[0][2] = t * x * z - s * y;\n'),
      ('    rotation.m[1][0] = x * ty - s * z;\n    rotation.m[1][1] = y * ty + c;\n'
       '    rotation.m[1][2] = z * ty + s * x;\n',
       '    rotation.m[1][0] = t * x * y - s * z;\n    rotation.m[1][1] = t * y * y + c;\n'
       '    rotation.m[1][2] = t * y * z + s * x;\n'),
      ('    rotation.m[2][0] = x * tz + s * y;\n    rotation.m[2][1] = z * ty - s * x;\n'
       '    rotation.m[2][2] = z * tz + c;\n',
       '    rotation.m[2][0] = t * x * z + s * y;\n    rotation.m[2][1] = t * y * z - s * x;\n'
       '    rotation.m[2][2] = t * z * z + c;\n')], 60),
    # CreateAddress tagged the modem name DPAID_Phone and the phone number DPAID_Modem.
    ('net_create_address', [('elements[1].type = DPAID_Modem;', 'elements[1].type = DPAID_Phone;'),
                            ('elements[count].type = DPAID_Phone;', 'elements[count].type = DPAID_Modem;')], 60),
    # SimulateLookAhead's dots summed flat ((x + z) + y; retail (x + y) + z) and the lateral
    # acceleration's dot merged into the outer sum: both differ only at 24-bit.
    ('kb_look_ahead', [('return a.z * b.z + (a.x * b.x + a.y * b.y);',
                        'return a.x * b.x + a.y * b.y + a.z * b.z;')], 20),
    ('kb_simulate_look_ahead', [('return a.z * b.z + (a.x * b.x + a.y * b.y);',
                                 'return a.x * b.x + a.y * b.y + a.z * b.z;')], 20),
    ('kb_look_ahead', [('* invDt + (BikeAILookDot(accR, right));', '* invDt + BikeAILookDot(accR, right);')], 40),
    # LoadParameterFile's channel inverse summed the y translation's x product first (24-bit only).
    ('morph_load_parameter_file',
     [('-((m->m[3][1] * m->m[1][1] + m->m[3][2] * m->m[2][1]) + m->m[3][0] * m->m[0][1]),',
       '-(m->m[3][0] * m->m[0][1] + m->m[3][1] * m->m[1][1] + m->m[3][2] * m->m[2][1]),')], 40),
    # ReadDialogResource read a Set_Default section's FontName into the control's own name.
    ('dlg_read_resource', [('GetString("FontName", defaultFontName, isDefault ? defaultFontName : fontName, -1);',
                            'GetString("FontName", defaultFontName, fontName, -1);')], 40),
    # Terrain::Load gave the height field its host pointer instead of the load-children flag.
    ('terrain_load', [('(this->stream), loadChildren,', '(this->stream), host,')], 60),
    # BikeRace::Create's vector length summed x*x + (y*y + z*z); retail adds a float z*z to
    # x*x + y*y. Only 24-bit shows it, on the flag girl's and box's offsets (2 of 400).
    ('bike_race_create',
     [('    float zz = v.z * v.z;\n    float lengthSquared = v.x * v.x + v.y * v.y + zz;\n',
       '    float xx = v.x * v.x;\n    float yy = v.y * v.y;\n    float lengthSquared = xx + yy + v.z * v.z;\n')],
     400),
    # PrepareGeometry summed row 1 of the tree's view product flat; retail groups _21, _23 and
    # _24 (24-bit only).
    ('soultree_prepare_geometry',
     [('view._21 = ((world.m[1][2] * projection.m[2][0] + world.m[1][3] * projection.m[3][0]) + world.m[1][0] * projection.m[0][0]) + world.m[1][1] * projection.m[1][0];',
       'view._21 = world.m[1][0] * projection.m[0][0] + world.m[1][1] * projection.m[1][0] + world.m[1][2] * projection.m[2][0] + world.m[1][3] * projection.m[3][0];'),
      ('view._23 = ((world.m[1][0] * projection.m[0][2] + world.m[1][1] * projection.m[1][2]) + world.m[1][3] * projection.m[3][2]) + world.m[1][2] * projection.m[2][2];',
       'view._23 = world.m[1][0] * projection.m[0][2] + world.m[1][1] * projection.m[1][2] + world.m[1][2] * projection.m[2][2] + world.m[1][3] * projection.m[3][2];'),
      ('view._24 = ((world.m[1][2] * projection.m[2][3] + world.m[1][3] * projection.m[3][3]) + world.m[1][1] * projection.m[1][3]) + world.m[1][0] * projection.m[0][3];',
       'view._24 = world.m[1][0] * projection.m[0][3] + world.m[1][1] * projection.m[1][3] + world.m[1][2] * projection.m[2][3] + world.m[1][3] * projection.m[3][3];')], 20),
    # DrawCurrentLod's sum orders at 24 bits: modes 3/4 tu, mode 2's world normal and facing,
    # mode 5's world normal; and mode 5's ray, whose dx/dz retail keeps on the x87 stack (53-bit).
    ('soultree_draw_current_lod',
     [('                        const Vector3& normal = group->normals[v];\n'
       '                        group->sourceVertices[v].tu =\n'
       '                            1.0f - ((direction.y * normal.y + direction.x * normal.x) +\n'
       '                                    direction.z * normal.z + 1.0f) * 0.5f;\n',
       '                        group->sourceVertices[v].tu =\n'
       '                            1.0f - (direction.y * group->normals[v].y + direction.x * group->normals[v].x +\n'
       '                                    direction.z * group->normals[v].z + 1.0f) * 0.5f;\n')], 30),
    ('soultree_draw_current_lod',
     [('                        Vector3 worldNormal;\n'
       '                        worldNormal.x = normal.x * world->m[0][0] + normal.y * world->m[1][0] + normal.z * world->m[2][0];\n'
       '                        worldNormal.y = normal.x * world->m[0][1] + normal.y * world->m[1][1] + normal.z * world->m[2][1];\n'
       '                        worldNormal.z = normal.x * world->m[0][2] + normal.y * world->m[1][2] + normal.z * world->m[2][2];\n',
       '                        Vector3 worldNormal(\n'
       '                            normal.x * world->m[0][0] + normal.y * world->m[1][0] + normal.z * world->m[2][0],\n'
       '                            normal.x * world->m[0][1] + normal.y * world->m[1][1] + normal.z * world->m[2][1],\n'
       '                            normal.x * world->m[0][2] + normal.y * world->m[1][2] + normal.z * world->m[2][2]);\n'),
      ('float facing = (direction.x * worldNormal.x + direction.y * worldNormal.y)\n',
       'float facing = direction.x * worldNormal.x + direction.y * worldNormal.y\n')], 30),
    ('soultree_draw_current_lod',
     [('(normal.x * world->m[0][0] + normal.z * world->m[2][0]) + normal.y * world->m[1][0],\n'
       '                            (normal.y * world->m[1][1] + normal.z * world->m[2][1]) + normal.x * world->m[0][1],\n'
       '                            (normal.y * world->m[1][2] + normal.z * world->m[2][2]) + normal.x * world->m[0][2]);\n',
       'normal.x * world->m[0][0] + normal.z * world->m[2][0] + normal.y * world->m[1][0],\n'
       '                            normal.x * world->m[0][1] + normal.y * world->m[1][1] + normal.z * world->m[2][1],\n'
       '                            normal.x * world->m[0][2] + normal.y * world->m[1][2] + normal.z * world->m[2][2]);\n')], 80),
    ('soultree_draw_current_lod',
     [('float dz = target.z - position.z;\n', 'float dy = target.y - position.y;\n'
       '                        float dz = target.z - position.z;\n'),
      ('hit = position + (target - position) * t;', 'hit = position + Vector3(dx * t, dy * t, dz * t);')], 30),
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

    def test_demangle_signature(self):
        sig = emu_diff.demangle_signature('?Add@@YGHHH@Z')            # int __stdcall Add(int, int)
        self.assertEqual((sig.convention, sig.member, sig.ret.kind, sig.stack_words()), ('stdcall', False, 'int', 2))
        sig = emu_diff.demangle_signature('?M@C@@QAEMM@Z')            # float __thiscall C::M(float)
        self.assertEqual((sig.convention, sig.member, sig.ret.kind, sig.stack_words()), ('thiscall', True, 'float', 1))
        # Vec3 __thiscall C::W(const Vec3&): a member returning a struct takes a hidden buffer.
        sig = emu_diff.demangle_signature('?W@C@@QAE?AUVec3@@ABU2@@Z')
        self.assertTrue(sig.hidden_return())
        self.assertEqual(sig.stack_words(), 2)
        self.assertTrue(sig.params[0].const)
        self.assertEqual(sig.params[0].pointee.size, 12)
        self.assertIsNone(emu_diff.demangle_signature('_strncat'))

    def test_synthetic_results_depend_only_on_seed_target_and_occurrence(self):
        def value(seed, target, occ, kind):
            return emu_diff.synthetic_value(emu_diff.synthetic_rng(seed, target, occ), kind)
        self.assertEqual(value((1, 2), 0x401000, 0, 'float'), value((1, 2), 0x401000, 0, 'float'))
        self.assertNotEqual(value((1, 2), 0x401000, 0, 'int') + value((1, 2), 0x401000, 1, 'int')
                            + value((1, 2), 0x401000, 2, 'int'),
                            value((1, 3), 0x401000, 0, 'int') + value((1, 3), 0x401000, 1, 'int')
                            + value((1, 3), 0x401000, 2, 'int'))
        eax, fval = value(7, 0x401000, 0, 'ptr')
        self.assertTrue(emu_diff.ARENA <= eax < emu_diff.ARENA + emu_diff.ARENA_SIZE)
        self.assertIsNone(fval)

    @unittest.skipUnless(importlib.util.find_spec('unicorn') is not None
                         and importlib.util.find_spec('capstone') is not None, 'requires unicorn and capstone')
    def test_replay_skips_logs_and_returns_synthetic_results(self):
        import struct
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
        from unicorn.x86_const import UC_X86_REG_ESP
        add, get, count = 0x00401000, 0x00402000, 0x00403000
        uc = Uc(UC_ARCH_X86, UC_MODE_32)
        for base, size in ((emu_diff.CAND, 0x1000), (emu_diff.STUB, emu_diff.STUB_SIZE),
                           (emu_diff.HEAP, emu_diff.HEAP_SIZE),
                           (emu_diff.STACK_TOP - emu_diff.STACK_SIZE, emu_diff.STACK_SIZE)):
            uc.mem_map(base, size)
        ctx = emu_diff.Context(uc, random.Random(1))
        out = ctx.alloc(16)
        native = ctx.alloc(6, bytes.fromhex('b82a000000c3'))     # mov eax, 42; ret

        code = bytearray()

        def call(target):
            code.extend(b'\xe8' + struct.pack('<i', target - (emu_diff.CAND + len(code) + 5)))
        code += bytes.fromhex('6a076a05')                        # push 7; push 5
        call(add)                                                # stdcall: replay pops 8
        code += b'\xa3' + struct.pack('<I', out)                 # mov [out], eax
        call(get)                                                # float return through ST0
        code += b'\xd9\x1d' + struct.pack('<I', out + 4)         # fstp dword [out+4]
        call(count)                                              # replayed, not logged
        call(native)                                             # passthrough: runs natively
        code += b'\xa3' + struct.pack('<I', out + 8)             # mov [out+8], eax
        end = emu_diff.CAND + len(code)
        code += b'\xc3'
        uc.mem_write(emu_diff.CAND, bytes(code))

        def effect(ctx, rec):
            ctx.pack(out + 12, 'I', rec.args[0] + rec.args[1])
        replay = emu_diff.Replay(dict(unlogged=('?Count@@YAXXZ',), passthrough=(native,),
                                      effects={'?Add@@YGHHH@Z': effect}),
                                 names={add: ['?Add@@YGHHH@Z'], get: ['?Get@@YAMXZ'], count: ['?Count@@YAXXZ']})
        uc.hook_add(UC_HOOK_CODE, emu_diff.ReplayRun(replay, ctx, 'seed', emu_diff.CAND).hook)
        esp = emu_diff.STACK_TOP - 0x100
        uc.reg_write(UC_X86_REG_ESP, esp)
        uc.emu_start(emu_diff.CAND, end)

        self.assertEqual(ctx.calls, [('?Add@@YGHHH@Z', None, (5, 7), ()), ('?Get@@YAMXZ', None, (), ())])
        self.assertEqual(uc.reg_read(UC_X86_REG_ESP), esp)
        expect_eax, _ = emu_diff.synthetic_value(emu_diff.synthetic_rng('seed', add, 0), 'int')
        _, expect_f = emu_diff.synthetic_value(emu_diff.synthetic_rng('seed', get, 0), 'float')
        self.assertEqual(ctx.unpack(out, 'IfII'), (expect_eax, expect_f, 42, 12))

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
                    # The copy keeps its directory's headers and its configured flags (/QIfist ...).
                    flags = ['/I' + str(path.parent)] + emu_diff.configured_flags(module.SOURCE)
                    obj = emu_diff.compile_source(str(source), Path(tmp) / 'candidate.obj', flags)
                    summary = emu_diff.run_diff(module, obj, n=n, retail=Path(os.environ['MCM2_EXE']),
                                                show=1)
                self.assertGreater(summary.mismatches, 0)


class ReplayPointeeAndToleranceTests(unittest.TestCase):
    @unittest.skipUnless(importlib.util.find_spec('unicorn') is not None
                         and importlib.util.find_spec('capstone') is not None, 'requires unicorn and capstone')
    def test_inout_pointees_are_logged_and_left_for_the_effect(self):
        import struct
        from unicorn import Uc, UC_ARCH_X86, UC_MODE_32, UC_HOOK_CODE
        from unicorn.x86_const import UC_X86_REG_ESP
        query = 0x00401000                       # void __cdecl Q(Vec3*)

        def run(spec):
            uc = Uc(UC_ARCH_X86, UC_MODE_32)
            for base, size in ((emu_diff.CAND, 0x1000), (emu_diff.STUB, emu_diff.STUB_SIZE),
                               (emu_diff.HEAP, emu_diff.HEAP_SIZE),
                               (emu_diff.STACK_TOP - emu_diff.STACK_SIZE, emu_diff.STACK_SIZE)):
                uc.mem_map(base, size)
            ctx = emu_diff.Context(uc, random.Random(1))
            out = ctx.alloc(12)
            code = bytearray(bytes.fromhex('83ec0c'))                 # sub esp, 12 (a Vec3 local)
            for k, value in enumerate((1.0, 2.0, 3.0)):
                code += bytes.fromhex('c74424') + bytes([4 * k]) + struct.pack('<f', value)
            code += b'\x54'                                           # push esp (&local)
            code += b'\xe8' + struct.pack('<i', query - (emu_diff.CAND + len(code) + 5))
            code += bytes.fromhex('83c404')                           # add esp, 4
            for k in range(3):                                        # copy the local to out
                code += bytes.fromhex('8b4424') + bytes([4 * k]) + b'\xa3' + struct.pack('<I', out + 4 * k)
            code += bytes.fromhex('83c40c')
            end = emu_diff.CAND + len(code)
            code += b'\xc3'
            uc.mem_write(emu_diff.CAND, bytes(code))
            seen = []

            def effect(ctx, rec):
                seen.append(ctx.unpack(rec.args[0], '3f'))
                ctx.pack(rec.args[0] + 4, 'f', 9.0)                  # keeps x and z, writes y
            replay = emu_diff.Replay(dict(spec, effects={query: effect}),
                                     names={query: ['?Q@@YAXPAUVec3@@@Z']})
            uc.hook_add(UC_HOOK_CODE, emu_diff.ReplayRun(replay, ctx, 'seed', emu_diff.CAND).hook)
            uc.reg_write(UC_X86_REG_ESP, emu_diff.STACK_TOP - 0x100)
            uc.emu_start(emu_diff.CAND, end)
            return ctx.calls, seen, ctx.unpack(out, '3f')

        passed = struct.pack('<3f', 1.0, 2.0, 3.0).hex()
        calls, seen, result = run({'inout': {query: (0,)}})
        self.assertEqual(calls[0][3], (('*arg0', passed),))
        self.assertEqual(seen, [(1.0, 2.0, 3.0)])
        self.assertEqual(result, (1.0, 9.0, 3.0))
        # A plain stack out-pointer is neither logged nor kept: synthetic bytes replace it.
        calls, seen, result = run({})
        self.assertEqual(calls[0][3], ())
        self.assertNotEqual(seen, [(1.0, 2.0, 3.0)])
        # pointees with an explicit size log that many bytes.
        calls, _, _ = run({'pointees': {query: ((0, 8),)}})
        self.assertEqual(calls[0][3], (('*arg0', passed[:16]),))

    def test_tolerance_is_per_control_word(self):
        import types
        case = types.SimpleNamespace(FLOAT_ULPS={0x007f: 64}, FLOAT_ABS=1e-5)
        self.assertEqual(emu_diff.tolerance_for(case, 0x007f), (64, 1e-5))
        self.assertEqual(emu_diff.tolerance_for(case, 0x027f), (0, 0.0))
        case = types.SimpleNamespace(FLOAT_ULPS=8)
        self.assertEqual(emu_diff.tolerance_for(case, 0x027f), (8, 0.0))
        self.assertEqual(emu_diff.tolerance_for(types.SimpleNamespace(), 0x027f), (0, 0.0))

    def test_outcomes_within_ulps(self):
        import struct

        def word(f):
            return struct.unpack('<I', struct.pack('<f', f))[0]

        def outcome(x, arg, status='ret'):
            return (status, ('eax', 1), ('calls', (('?F@@YAXM@Z', None, (arg,), ()),)),
                    ('writes', ((0x40000000, struct.pack('<f', x).hex()),)))
        one = word(1.0)
        self.assertEqual(emu_diff.ulp_distance(one, one + 3), 3)
        self.assertEqual(emu_diff.ulp_distance(word(-0.0), word(0.0)), 0)
        self.assertEqual(emu_diff.ulp_distance(word(1e-45), word(-1e-45)), 2)
        a = outcome(1.0, one)
        self.assertEqual(emu_diff.outcomes_within_ulps(a, a, 4), 0)
        b = outcome(struct.unpack('<f', struct.pack('<I', one + 3))[0], one + 2)
        self.assertEqual(emu_diff.outcomes_within_ulps(a, b, 4), 3)
        self.assertIsNone(emu_diff.outcomes_within_ulps(a, b, 2))
        self.assertIsNone(emu_diff.outcomes_within_ulps(a, b, 0))
        self.assertIsNone(emu_diff.outcomes_within_ulps(a, outcome(1.0, one, 'limit'), 4))
        # Near zero an absolute bound also agrees; far apart values never do.
        self.assertIsNone(emu_diff.outcomes_within_ulps(outcome(1e-7, one), outcome(-1e-7, one), 4))
        self.assertEqual(emu_diff.outcomes_within_ulps(outcome(1e-7, one), outcome(-1e-7, one), 4, 1e-6), 0)
        self.assertIsNone(emu_diff.outcomes_within_ulps(outcome(1.0, one), outcome(1.5, one), 4, 1e-6))


if __name__ == '__main__':
    unittest.main()
