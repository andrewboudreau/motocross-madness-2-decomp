"""rank_near_misses classifies aligned instruction diffs; synthetic inputs only, no VC6."""
import unittest

from tools.frame_layout import Insn
from tools.rank_near_misses import compare, diff_counts, frame_size, lane_for, norm, parse_index, retail_extent


def insns(*lines):
    out = []
    for k, line in enumerate(lines):
        mnemonic, _, op = line.partition(' ')
        out.append(Insn(0x401000 + 4 * k, mnemonic, op, 4))
    return out


class FakePE:
    def __init__(self, base, data):
        self.base, self.data = base, data

    def bytes_at_va(self, va, size):
        return self.data[va - self.base:va - self.base + size]


class RankNearMissesTests(unittest.TestCase):
    def test_parse_index_row(self):
        text = ('| TU | VA | score | class | what differs |\n|---|---|---|---|---|\n'
                '| render/ResourceManager | `0x4e9030` | 610/802 | b | push order |\n')
        self.assertEqual(parse_index(text), [dict(tu='render/ResourceManager', va=0x4e9030,
                                                  score='610/802', index_class='b')])

    def test_lane_for(self):
        self.assertEqual(lane_for('render/Griddraw'), 'render')
        self.assertEqual(lane_for('camera/FollowCamera'), 'render')
        self.assertEqual(lane_for('ecosystem/EcoSystem'), 'render')
        self.assertEqual(lane_for('ui/GUIManager'), 'shell')
        self.assertEqual(lane_for('krusty2/vehicle/Bike'), 'rider')
        self.assertEqual(lane_for('krusty2/collision/CollisionObject'), 'body')
        self.assertEqual(lane_for('physics/tire/Tire'), 'rider')
        self.assertEqual(lane_for('physics/bvh/Quadtree'), 'body')

    def test_diff_counts(self):
        self.assertEqual(diff_counts(['a', 'b'], ['a', 'b']), (0, 0))
        self.assertEqual(diff_counts(['a', 'b', 'c'], ['a', 'x', 'c']), (1, 1))
        # one inserted instruction costs one, not every later position
        self.assertEqual(diff_counts(['a', 'b', 'c', 'd'], ['a', 'n', 'b', 'c', 'd']), (1, 1))

    def test_norm_masks(self):
        code = insns('call 0x401234', 'mov eax, dword ptr [0x5a1234]', 'mov dword ptr [esp + 0x10], ecx',
                     'call dword ptr [edx + 8]')
        self.assertEqual(norm(code), ['call X', 'mov eax, dword ptr [G]', 'mov dword ptr [esp + 0x10], ecx',
                                      'call dword ptr [edx + 8]'])
        self.assertEqual(norm(code, mask_frame=True)[2], 'mov dword ptr [esp + S], ecx')
        self.assertEqual(norm(code, mask_reg=True)[2], 'mov dword ptr [esp + 0x10], R')
        self.assertEqual(norm(code, mnemonic_only=True), ['call', 'mov', 'mov', 'call'])

    def test_compare_kinds(self):
        retail = insns('push esi', 'mov esi, ecx', 'mov eax, dword ptr [esp + 8]', 'ret 4')
        self.assertEqual(compare(retail, retail)['kind'], 'identical')
        frame = insns('push esi', 'mov esi, ecx', 'mov eax, dword ptr [esp + 0xc]', 'ret 4')
        self.assertEqual(compare(frame, retail)['kind'], 'frame-only')
        reg = insns('push edi', 'mov edi, ecx', 'mov eax, dword ptr [esp + 8]', 'ret 4')
        result = compare(reg, retail)
        self.assertEqual((result['kind'], result['diffs'], result['hunks']), ('reg-only', 2, 1))
        both = insns('push edi', 'mov edi, ecx', 'mov eax, dword ptr [esp + 0xc]', 'ret 4')
        self.assertEqual(compare(both, retail)['kind'], 'frame+reg')
        extra = insns('push esi', 'xor eax, eax', 'mov esi, ecx', 'mov eax, dword ptr [esp + 8]', 'ret 4')
        result = compare(extra, retail)
        self.assertEqual((result['kind'], result['diffs'], result['hunks']), ('structural', 1, 1))

    def test_frame_size(self):
        self.assertEqual(frame_size(insns('sub esp, 0x5c', 'push ebx')), 0x5c)
        self.assertEqual(frame_size(insns('push ecx', 'push esi')), 4)
        self.assertEqual(frame_size(insns('push esi', 'ret')), 0)

    def test_retail_extent_stops_at_padded_ret(self):
        base = 0x401000
        body = bytes([0x8b, 0xc1, 0xc3])            # mov eax, ecx; ret
        data = body + bytes([0xcc] * 13) + bytes([0x33, 0xc0, 0xc3])
        self.assertEqual(retail_extent(FakePE(base, data), base, [base, base + len(data)]), 3)


if __name__ == '__main__':
    unittest.main()
