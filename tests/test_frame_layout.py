"""tools/frame_layout.py: the parsing and prediction parts (no VC6 needed)."""
import struct
import unittest

from tools.frame_layout import (CvType, FrameSymbol, Insn, align, assign_offsets, callee_cleans,
                                equal_run_permutation, flatten, frame_operand, normalize,
                                parse_cv_procs, parse_cv_types, predict_layout,
                                suggest_adjustments, track_frame, type_size, vc6_quicksort)


def cv_record(kind: int, body: bytes) -> bytes:
    return struct.pack('<HH', len(body) + 2, kind) + body


def pascal(s: str) -> bytes:
    return bytes([len(s)]) + s.encode('latin1')


class CodeViewTests(unittest.TestCase):
    def test_types_arrays_structs_and_forward_references(self):
        raw = b'\x02\x00\x00\x00'
        # 0x1000 LF_ARRAY char[0x80]; 0x1001 LF_STRUCTURE fwd 'V'; 0x1002 LF_STRUCTURE 'V' size 12
        raw += cv_record(0x1003, struct.pack('<II', 0x70, 0x11) + struct.pack('<H', 0x80) + pascal(''))
        raw += cv_record(0x1005, struct.pack('<HHIII', 0, 0x80, 0, 0, 0) + struct.pack('<H', 0) + pascal('V'))
        raw += cv_record(0x1005, struct.pack('<HHIII', 3, 0, 0x1003, 0, 0) + struct.pack('<H', 12) + pascal('V'))
        raw += cv_record(0x1002, struct.pack('<II', 0x1002, 0x0a))          # 0x1003 LF_POINTER
        raw += cv_record(0x1003, struct.pack('<II', 0x74, 0x11) + struct.pack('<HI', 0x8004, 0x2800) + pascal(''))  # 0x1004 int[0xa00]
        types = parse_cv_types(raw)
        self.assertEqual(type_size(0x1000, types), 0x80)
        self.assertEqual(type_size(0x1001, types), 12)      # forward reference resolves by name
        self.assertEqual(type_size(0x1002, types), 12)
        self.assertEqual(type_size(0x1003, types), 4)
        self.assertEqual(type_size(0x1004, types), 0x2800)
        self.assertEqual(type_size(0x74, types), 4)          # T_INT4
        self.assertEqual(type_size(0x403, types), 4)         # 32-bit near pointer
        self.assertEqual(type_size(0x41, types), 8)          # T_REAL64
        self.assertIsNone(type_size(0x1fff, types))

    def test_procs_blocks_and_locals(self):
        raw = b'\x02\x00\x00\x00'
        proc_body = b'\0' * 12 + struct.pack('<I', 0x40) + b'\0' * 19 + pascal('f')
        raw += cv_record(0x100b, proc_body)
        raw += cv_record(0x1006, struct.pack('<iI', -0x80, 0x1000) + pascal('a'))
        raw += cv_record(0x207, struct.pack('<IIII', 0, 0, 0x10, 0x8) + b'\0' + pascal(''))
        raw += cv_record(0x1006, struct.pack('<iI', -0x100, 0x1000) + pascal('b'))
        raw += cv_record(0x6, b'')
        raw += cv_record(0x1001, struct.pack('<IH', 0x74, 18) + pascal('this'))
        raw += cv_record(0x6, b'')
        procs = parse_cv_procs(raw)
        self.assertEqual([p.name for p in procs], ['f'])
        p = procs[0]
        self.assertEqual(p.length, 0x40)
        self.assertEqual([(l.name, l.bprel, l.block_id) for l in p.locals if l.register is None],
                         [('a', -0x80, 0), ('b', -0x100, 1)])
        self.assertEqual([l.name for l in p.locals if l.register is not None], ['this'])
        self.assertEqual(p.blocks, [(1, 0, 0x8, 0x10)])


class TrackerTests(unittest.TestCase):
    def test_frame_operand(self):
        self.assertEqual(frame_operand('eax, dword ptr [esp + 0x10]', 'mov'), ('esp', 0x10, 4))
        self.assertEqual(frame_operand('ecx, [esp + 0x2cc]', 'lea'), ('esp', 0x2cc, 0))
        self.assertEqual(frame_operand('byte ptr [esp], 0', 'mov'), ('esp', 0, 1))
        self.assertEqual(frame_operand('eax, dword ptr [ebp - 0x14]', 'mov'), ('ebp', -0x14, 4))
        self.assertIsNone(frame_operand('eax, dword ptr [esp + eax*4 + 0x10]', 'mov'))
        self.assertIsNone(frame_operand('eax, dword ptr [ecx + 0x28]', 'mov'))

    def test_callee_cleans(self):
        self.assertFalse(callee_cleans('?f@@YAXXZ'))
        self.assertTrue(callee_cleans('?f@@YGXH@Z'))
        self.assertTrue(callee_cleans('?Init@UIDialog@@QAEHPAXH@Z'))
        self.assertTrue(callee_cleans('?Slot@UIDialog@@UAEXH@Z'))
        self.assertFalse(callee_cleans('?Make@UIDialog@@SAPAVUIDialog@@XZ'))
        self.assertFalse(callee_cleans('??2@YAPAXIPBDH@Z'))
        self.assertTrue(callee_cleans('??0UIButton@@QAE@HPAUCameraRect@@PAVUIDialog@@@Z'))
        self.assertFalse(callee_cleans('_strcpy'))
        self.assertTrue(callee_cleans('_LoadStringA@16'))
        self.assertTrue(callee_cleans('__imp__LoadStringA@16'))
        self.assertIsNone(callee_cleans('$L1234'))

    def test_esp_frame_depths(self):
        # sub esp,0x10; push esi; lea eax,[esp+8]; push eax; call X; add esp,4; push 1; push 2;
        # call Y (stdcall); mov [esp+4],eax; pop esi; add esp,0x10; ret
        code = (b'\x83\xec\x10' b'\x56' b'\x8d\x44\x24\x08' b'\x50' b'\xe8\x00\x00\x00\x00' b'\x83\xc4\x04'
                b'\x6a\x01' b'\x6a\x02' b'\xe8\x00\x00\x00\x00' b'\x89\x44\x24\x04' b'\x5e' b'\x83\xc4\x10' b'\xc3')
        from tools.frame_layout import disassemble
        insns = disassemble(code)
        info = track_frame(insns, {8: True})
        self.assertEqual(info.frame_size, 0x10)
        self.assertFalse(info.ebp_based)
        self.assertEqual([(a.index, a.offset, a.lea) for a in info.accesses], [(2, 4, True), (9, 0, False)])
        # the same without the stdcall hint: the look-ahead sees no add esp and resets
        info2 = track_frame(insns)
        self.assertEqual([(a.index, a.offset) for a in info2.accesses], [(2, 4), (9, 0)])
        self.assertEqual(info2.depth_resets, 1)

    def test_chkstk_and_eh_prologue(self):
        # push -1; push 0; mov eax,fs:[0]; push eax; mov eax,0x1000; mov fs:[0],esp; call chkstk;
        # push ebx; push esi; mov byte ptr [esp+0x18], 0; ret
        code = (b'\x6a\xff' b'\x6a\x00' b'\x64\xa1\x00\x00\x00\x00' b'\x50' b'\xb8\x00\x10\x00\x00'
                b'\x64\x89\x25\x00\x00\x00\x00' b'\xe8\x00\x00\x00\x00' b'\x53' b'\x56' b'\xc6\x44\x24\x18\x00' b'\xc3')
        from tools.frame_layout import disassemble
        info = track_frame(disassemble(code))
        self.assertEqual(info.frame_size, 0x1000)
        self.assertEqual(info.saved_regs, 0xc)
        self.assertEqual([(a.offset, a.width) for a in info.accesses], [(0x10, 1)])

    def test_ebp_frame(self):
        # push ebp; mov ebp,esp; sub esp,0x20; mov eax,[ebp-4]; mov [ebp-0x20],eax; leave; ret
        code = b'\x55' b'\x8b\xec' b'\x83\xec\x20' b'\x8b\x45\xfc' b'\x89\x45\xe0' b'\xc9' b'\xc3'
        from tools.frame_layout import disassemble
        info = track_frame(disassemble(code))
        self.assertTrue(info.ebp_based)
        self.assertEqual(info.frame_size, 0x20)
        self.assertEqual(info.ebp_delta, 0x20)
        self.assertEqual([a.offset for a in info.accesses], [0x1c, 0])

    def test_normalize_and_align(self):
        a = [Insn(0, 'lea', 'eax, [esp + 0x80]', 7), Insn(7, 'push', 'eax', 1), Insn(8, 'call', '0x100', 5),
             Insn(13, 'mov', 'dword ptr [esp + 0x24], 1', 8)]
        b = [Insn(0, 'lea', 'eax, [esp + 0x100]', 7), Insn(7, 'push', 'eax', 1), Insn(8, 'call', '0x4a0000', 5),
             Insn(13, 'mov', 'eax, ecx', 2), Insn(15, 'mov', 'dword ptr [esp + 0x28], 1', 8)]
        self.assertEqual(normalize(a[0]), normalize(b[0]))
        self.assertEqual(normalize(a[2]), 'call X')
        self.assertEqual(align(a, b), [(0, 0), (1, 1), (2, 2), (3, 4)])

    def test_align_keeps_repeated_blocks_in_sync(self):
        block = [Insn(0, 'lea', 'edx, [esp + 0x80]', 7), Insn(7, 'rep movsd', 'dword ptr es:[edi], dword ptr [esi]', 2)]
        a = block * 4
        b = block * 4 + [Insn(0, 'ret', '', 1)]
        self.assertEqual(align(a, b), [(i, i) for i in range(8)])


class LayoutRuleTests(unittest.TestCase):
    def test_quicksort_equal_keys_permutation(self):
        # the probed permutations of 2..16 equally used 0x80-byte buffers, nearest esp first
        expected = {2: [0, 1], 3: [1, 0, 2], 4: [1, 2, 0, 3], 5: [2, 0, 3, 1, 4], 6: [2, 3, 1, 4, 0, 5],
                    7: [3, 0, 4, 2, 5, 1, 6], 8: [3, 4, 1, 5, 0, 6, 2, 7], 10: [4, 5, 1, 6, 3, 7, 2, 8, 0, 9],
                    16: [7, 8, 1, 9, 3, 10, 5, 11, 0, 12, 4, 13, 2, 14, 6, 15]}
        for n, order in expected.items():
            self.assertEqual(equal_run_permutation(n), order, n)

    def test_quicksort_with_keys(self):
        self.assertEqual(vc6_quicksort([3, 1, 2], lambda x, y: x > y), [3, 2, 1])
        self.assertEqual(vc6_quicksort([], lambda x, y: x > y), [])

    def test_counts_then_last_use(self):
        # probes p (3), q (1), r (3), s (1), t (3) in last-use order p q r s t -> t p r q s
        syms = [FrameSymbol(n, c, 0x80, i) for i, (n, c) in enumerate([('p', 3), ('q', 1), ('r', 3), ('s', 1), ('t', 3)])]
        self.assertEqual(flatten(predict_layout(syms)), ['t', 'p', 'r', 'q', 's'])
        # two equal maxima among five: the earlier one first; among four: the later one
        syms = [FrameSymbol(n, c, 0x80, i) for i, (n, c) in enumerate([('a', 1), ('b', 1), ('c', 1), ('d', 2), ('e', 2)])]
        self.assertEqual(flatten(predict_layout(syms)), ['d', 'e', 'a', 'b', 'c'])
        syms = [FrameSymbol(n, c, 0x80, i) for i, (n, c) in enumerate([('a', 1), ('b', 1), ('c', 2), ('d', 2)])]
        self.assertEqual(flatten(predict_layout(syms)), ['d', 'c', 'a', 'b'])

    def test_density_and_sizes(self):
        # a[0x80] used 4 times ties with b[0x40] used twice: the smaller first; 5 uses win
        self.assertEqual(flatten(predict_layout([FrameSymbol('a', 4, 0x80, 0), FrameSymbol('b', 2, 0x40, 1)])), ['b', 'a'])
        self.assertEqual(flatten(predict_layout([FrameSymbol('a', 5, 0x80, 0), FrameSymbol('b', 2, 0x40, 1)])), ['a', 'b'])
        # an int used once outranks a 0x80 buffer used 32 times and loses to 35 uses
        self.assertEqual(flatten(predict_layout([FrameSymbol('y', 1, 4, 2), FrameSymbol('a', 32, 0x80, 30)])), ['y', 'a'])
        self.assertEqual(flatten(predict_layout([FrameSymbol('y', 1, 4, 2), FrameSymbol('a', 35, 0x80, 30)])), ['a', 'y'])
        # equal density, different sizes, four symbols (probe h1): a c b x
        syms = [FrameSymbol('a', 4, 0x40, 3), FrameSymbol('b', 1, 0x20, 4), FrameSymbol('c', 1, 0x10, 5), FrameSymbol('x', 1, 0x80, 6)]
        self.assertEqual(flatten(predict_layout(syms)), ['a', 'c', 'b', 'x'])
        # a 0x400 buffer with four uses sits between a 0x80 one and 0x1900 tables
        syms = [FrameSymbol('a', 4, 0x400, 0), FrameSymbol('b', 5, 0x1900, 1), FrameSymbol('c', 3, 0x1900, 2), FrameSymbol('d', 2, 0x80, 3)]
        self.assertEqual(flatten(predict_layout(syms)), ['d', 'a', 'b', 'c'])

    def test_small_frames_rank_by_size(self):
        # frame <= 0x80: a[0x40] used ten times still sits above b[0x20] and an int
        syms = [FrameSymbol('a', 10, 0x40, 9), FrameSymbol('b', 1, 0x20, 10), FrameSymbol('y', 1, 4, 11)]
        self.assertEqual(flatten(predict_layout(syms)), ['y', 'b', 'a'])
        # the same with a 0x80 buffer added (frame 0xe0): density order
        syms.append(FrameSymbol('x', 1, 0x80, 12))
        self.assertEqual(flatten(predict_layout(syms))[:2], ['y', 'a'])

    def test_regions_merge(self):
        # c, d, e used once each, then sibling blocks {a} {b}: the shared slot first, then d e c
        syms = [FrameSymbol('c', 1, 0x80, 0), FrameSymbol('d', 1, 0x80, 1), FrameSymbol('e', 1, 0x80, 2),
                FrameSymbol('a', 1, 0x80, 3, 1), FrameSymbol('b', 1, 0x80, 4, 1)]
        self.assertEqual(flatten(predict_layout(syms)), ['a', 'b', 'd', 'e', 'c'])

    def test_offsets_and_adjustments(self):
        order = [FrameSymbol('t', 5, 0x80, 0), FrameSymbol('n', 1, 4, 1), FrameSymbol('q', 1, 0x80, 2)]
        self.assertEqual(assign_offsets(order), {'t': 0, 'n': 0x80, 'q': 0x84})
        syms = [FrameSymbol('p', 1, 0x80, 0), FrameSymbol('q', 1, 0x80, 1), FrameSymbol('r', 1, 0x80, 2)]
        self.assertEqual(flatten(predict_layout(syms)), ['q', 'p', 'r'])
        delta = suggest_adjustments(syms, ['r', 'q', 'p'])
        self.assertIsNotNone(delta)
        for name, d in delta.items():
            next(s for s in syms if s.name == name).count += d
        self.assertEqual(flatten(predict_layout(syms)), ['r', 'q', 'p'])


if __name__ == '__main__':
    unittest.main()
