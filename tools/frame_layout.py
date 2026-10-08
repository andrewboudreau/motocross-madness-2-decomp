#!/usr/bin/env python3
"""Compare a VC6 candidate's stack-frame layout with the retail function's.

VC6 SP3 lays a function's locals out by static use count (docs/VC6_FRAME_LAYOUT.md):
the list of frame symbols is sorted by (use count descending, size ascending,
last use ascending) and then run through a quicksort whose comparator never
returns 0, which permutes equal-key runs in a fixed way. The nearest-esp slot
goes to the first symbol of the result. Two functions that differ only in slot
assignment therefore differ in those counts, and this tool reports which
candidate locals need more or fewer references to land in retail's slot.

Usage:
  python3 tools/frame_layout.py OBJ SYMBOL --va 0xVA [--size N] [--exe mcm2.exe]
         [--cv-name SUBSTR] [--json]
  python3 tools/frame_layout.py OBJ [PROCSUBSTR] --dump      # locals of a probe object

OBJ is a VC6 object compiled with /Z7 (C11 CodeView), SYMBOL a substring of the
COFF function symbol, --va the retail function, --cv-name the (unmangled)
CodeView procedure name when it differs from the symbol's. The report lists:
1. the candidate's locals (CodeView S_BPREL32 records, sizes from .debug$T),
2. the candidate's and retail's frame accesses (``[esp+N]``/``[ebp-N]`` with the
   push depth tracked), aligned instruction by instruction (difflib),
3. the local -> retail-offset map voted from the aligned pairs, the retail
   slot order, retail slots no candidate local maps to (dead buffers),
4. the per-local reference counts the disassembly shows, the layout the rule
   predicts from them (a self-check of the proxy count), and the +k/-k use
   changes that make the rule reproduce retail's order.

The parsing and prediction parts have no VC6 dependency (tests/test_frame_layout.py).
"""
from __future__ import annotations

import argparse
import difflib
import json
import os
import re
import struct
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

# --------------------------------------------------------------------------
# CodeView (C11, /Z7) parsing
# --------------------------------------------------------------------------

CV_SIGNATURE_C11 = b'\x02\x00\x00\x00'

# CodeView primitive type sizes (the low byte is the kind, 0x04xx a 32-bit near pointer).
_PRIMITIVE_SIZES = {
    0x10: 1, 0x20: 1, 0x70: 1, 0x30: 1, 0x68: 1, 0x69: 1,   # char, uchar, rchar, bool08, int1, uint1
    0x11: 2, 0x21: 2, 0x31: 2, 0x71: 2, 0x72: 2,            # short, ushort, bool16, wchar, int2
    0x12: 4, 0x22: 4, 0x32: 4, 0x40: 4, 0x74: 4, 0x75: 4,   # long, ulong, bool32, real32, int4, uint4
    0x13: 8, 0x23: 8, 0x41: 8, 0x76: 8, 0x77: 8,            # quad, uquad, real64, int8, uint8
    0x42: 10, 0x03: 0,                                      # real80, void
}


def primitive_size(ti: int) -> int | None:
    """Size of a CodeView primitive type index, None when unknown."""
    if ti >= 0x1000:
        return None
    mode = (ti >> 8) & 0x7
    if mode in (2, 4, 6):   # far/near 32-bit pointer modes
        return 4
    if mode != 0:
        return None
    return _PRIMITIVE_SIZES.get(ti & 0xff)


def _numeric_leaf(body: bytes, pos: int) -> tuple[int, int]:
    """Decode a CodeView numeric leaf at ``pos``: (value, next position)."""
    v = struct.unpack_from('<H', body, pos)[0]
    if v < 0x8000:
        return v, pos + 2
    if v == 0x8000:   # LF_CHAR
        return struct.unpack_from('<b', body, pos + 2)[0], pos + 3
    if v == 0x8001:   # LF_SHORT
        return struct.unpack_from('<h', body, pos + 2)[0], pos + 4
    if v == 0x8002:   # LF_USHORT
        return struct.unpack_from('<H', body, pos + 2)[0], pos + 4
    if v == 0x8003:   # LF_LONG
        return struct.unpack_from('<i', body, pos + 2)[0], pos + 6
    if v == 0x8004:   # LF_ULONG
        return struct.unpack_from('<I', body, pos + 2)[0], pos + 6
    raise ValueError(f'unsupported numeric leaf {v:#x}')


def _pascal(body: bytes, pos: int) -> str:
    n = body[pos]
    return body[pos + 1:pos + 1 + n].decode('latin1')


@dataclass
class CvType:
    leaf: int
    size: int | None = None
    name: str = ''
    forward: bool = False
    base: int | None = None     # underlying type for modifiers


def parse_cv_types(raw: bytes) -> dict[int, CvType]:
    """Parse a .debug$T section (C11 / CV4 leaves) into {type index: CvType}."""
    types: dict[int, CvType] = {}
    pos = 4 if raw[:4] == CV_SIGNATURE_C11 else 0
    index = 0x1000
    while pos + 4 <= len(raw):
        length, leaf = struct.unpack_from('<HH', raw, pos)
        if length < 2:
            break
        body = raw[pos + 4:pos + 2 + length]
        t = CvType(leaf)
        try:
            if leaf == 0x1003:        # LF_ARRAY_ST: elemtype, idxtype, numeric length, name
                size, p = _numeric_leaf(body, 8)
                t.size = size
                t.name = _pascal(body, p)
            elif leaf in (0x1004, 0x1005):   # LF_CLASS_ST / LF_STRUCTURE_ST
                count, prop = struct.unpack_from('<HH', body, 0)
                size, p = _numeric_leaf(body, 16)
                t.size = size
                t.name = _pascal(body, p)
                t.forward = bool(prop & 0x80)
            elif leaf == 0x1006:      # LF_UNION_ST: count, property, field, numeric length, name
                count, prop = struct.unpack_from('<HH', body, 0)
                size, p = _numeric_leaf(body, 8)
                t.size = size
                t.name = _pascal(body, p)
                t.forward = bool(prop & 0x80)
            elif leaf == 0x1002:      # LF_POINTER
                t.size = 4
            elif leaf == 0x1007:      # LF_ENUM_ST
                t.size = 4
                t.name = _pascal(body, 12)
            elif leaf == 0x1001:      # LF_MODIFIER: type, attr
                t.base = struct.unpack_from('<I', body, 0)[0]
        except (struct.error, IndexError, ValueError):
            pass
        types[index] = t
        index += 1
        pos += 2 + length
    return types


def type_size(ti: int, types: dict[int, CvType]) -> int | None:
    """Byte size of CodeView type ``ti``; forward references resolve by name."""
    p = primitive_size(ti)
    if p is not None:
        return p
    t = types.get(ti)
    if t is None:
        return None
    if t.leaf == 0x1001 and t.base is not None:
        return type_size(t.base, types)
    if t.forward or (t.size == 0 and t.leaf in (0x1004, 0x1005, 0x1006)):
        for other in types.values():
            if other.leaf == t.leaf and other.name == t.name and not other.forward and other.size:
                return other.size
        return None
    return t.size


@dataclass
class CvLocal:
    name: str
    bprel: int
    type_index: int
    block: int          # nesting depth: 0 = function scope, 1+ = S_BLOCK32 nesting
    block_id: int       # id of the innermost enclosing block (0 = function scope)
    size: int | None = None
    register: int | None = None   # S_REGISTER locals (not on the frame)


@dataclass
class CvProc:
    name: str
    length: int
    locals: list[CvLocal] = field(default_factory=list)
    blocks: list[tuple[int, int, int, int]] = field(default_factory=list)   # (id, parent, offset, length)


def parse_cv_procs(raw: bytes) -> list[CvProc]:
    """Parse a .debug$S section (C11) into procedures with their frame locals.

    Only the records the frame layout needs are read: S_GPROC32_ST/S_LPROC32_ST
    (0x100a/0x100b), S_BPREL32_ST (0x1006), S_REGISTER_ST (0x1001), S_BLOCK32
    (0x207) and S_END (0x6).
    """
    procs: list[CvProc] = []
    pos = 4 if raw[:4] == CV_SIGNATURE_C11 else 0
    cur: CvProc | None = None
    block_stack: list[int] = []
    next_block = 0
    while pos + 4 <= len(raw):
        length, kind = struct.unpack_from('<HH', raw, pos)
        if length < 2:
            break
        body = raw[pos + 4:pos + 2 + length]
        if kind in (0x100a, 0x100b):
            plen = struct.unpack_from('<I', body, 12)[0]
            cur = CvProc(_pascal(body, 35), plen)
            procs.append(cur)
            block_stack = []
        elif kind == 0x6:
            if block_stack:
                block_stack.pop()
            else:
                cur = None
        elif kind == 0x207 and cur is not None:
            blen, boff = struct.unpack_from('<II', body, 8)
            next_block += 1
            parent = block_stack[-1] if block_stack else 0
            cur.blocks.append((next_block, parent, boff, blen))
            block_stack.append(next_block)
        elif kind == 0x1006 and cur is not None:
            off, ti = struct.unpack_from('<iI', body, 0)
            cur.locals.append(CvLocal(_pascal(body, 8), off, ti, len(block_stack),
                                      block_stack[-1] if block_stack else 0))
        elif kind == 0x1001 and cur is not None:
            ti, reg = struct.unpack_from('<IH', body, 0)
            cur.locals.append(CvLocal(_pascal(body, 6), 0, ti, len(block_stack),
                                      block_stack[-1] if block_stack else 0, register=reg))
        pos += 2 + length
    return procs


def load_codeview(obj, proc_substr: str) -> tuple[CvProc, dict[int, CvType]]:
    """Find the CodeView procedure whose name contains ``proc_substr`` in a CoffObject."""
    types: dict[int, CvType] = {}
    found: list[CvProc] = []
    for sec in obj.sections:
        raw = obj.data[sec.raw_ptr:sec.raw_ptr + sec.raw_size]
        if sec.name == '.debug$T':
            types.update(parse_cv_types(raw))
        elif sec.name == '.debug$S':
            for proc in parse_cv_procs(raw):
                if proc_substr in proc.name:
                    found.append(proc)
    exact = [p for p in found if p.name == proc_substr]
    if exact:
        found = exact
    if not found:
        raise SystemExit(f'no CodeView procedure matches {proc_substr!r}')
    if len(found) > 1:
        raise SystemExit('ambiguous CodeView procedure: ' + ', '.join(p.name for p in found))
    proc = found[0]
    for loc in proc.locals:
        loc.size = type_size(loc.type_index, types)
    return proc, types


# --------------------------------------------------------------------------
# Frame access tracking
# --------------------------------------------------------------------------

@dataclass
class Insn:
    address: int
    mnemonic: str
    op_str: str
    size: int
    bytes: bytes = b''


@dataclass
class FrameAccess:
    index: int          # instruction index
    offset: int         # byte offset from the frame base (esp after the prologue)
    width: int          # operand width in bytes (0 when unknown, e.g. lea)
    lea: bool           # address taken (lea) rather than a load/store
    text: str


@dataclass
class FrameInfo:
    frame_size: int = 0          # bytes of locals (sub esp / chkstk)
    saved_regs: int = 0          # bytes of callee-saved pushes in the prologue
    ebp_based: bool = False      # push ebp; mov ebp, esp frame
    ebp_delta: int = 0           # ebp - frame base (valid when ebp_based)
    prologue_end: int = 0        # instruction index after the prologue
    accesses: list[FrameAccess] = field(default_factory=list)
    depth_resets: int = 0        # calls whose cleanup the look-ahead heuristic decided
    depths: list = field(default_factory=list)   # (instruction index, depth after it)


def disassemble(code: bytes, base: int = 0) -> list[Insn]:
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    return [Insn(i.address, i.mnemonic, i.op_str, i.size, bytes(i.bytes)) for i in md.disasm(code, base)]


_MEM_RE = re.compile(r'(?:(byte|word|dword|qword|xmmword|tbyte) ptr )?\[([^\]]*)\]')
_WIDTHS = {'byte': 1, 'word': 2, 'dword': 4, 'qword': 8, 'tbyte': 10, 'xmmword': 16, None: 0}


def _imm(s: str) -> int:
    s = s.strip()
    neg = s.startswith('-')
    v = int(s.lstrip('-'), 0)
    return -v if neg else v


def frame_operand(op_str: str, mnemonic: str) -> tuple[str, int, int] | None:
    """Return (reg, displacement, width) for an ``[esp+N]``/``[ebp+N]`` operand.

    Only operands whose base register is esp or ebp and that carry no other
    register count (``[esp+eax*4+N]`` is a table index, not a slot access).
    """
    m = _MEM_RE.search(op_str)
    if not m:
        return None
    inner = m.group(2).replace(' ', '')
    parts = re.split(r'(?=[+-])', inner)
    parts = [p for p in parts if p]
    reg = None
    disp = 0
    for p in parts:
        core = p.lstrip('+-')
        if core in ('esp', 'ebp') and not p.startswith('-'):
            if reg is not None:
                return None
            reg = core
        elif re.fullmatch(r'0x[0-9a-f]+|\d+', core):
            disp += _imm(p if p[0] in '+-' else '+' + p)
        else:
            return None   # another register (index), segment, etc.
    if reg is None:
        return None
    width = _WIDTHS[m.group(1)]
    if mnemonic == 'lea':
        width = 0
    return reg, disp, width


def callee_cleans(name: str) -> bool | None:
    """Does the callee pop its arguments? Decoded from the mangled name.

    ``?f@@YA..`` is __cdecl (caller cleans), ``YG`` __stdcall, ``YI`` __fastcall;
    member functions carry the convention after the access and ``this``
    qualifier letters (``QAE`` = public __thiscall). C symbols: ``_f@N`` is
    stdcall, ``@f@N`` fastcall, plain ``_f`` cdecl. None when undecidable.
    """
    if name.startswith('__imp_'):
        name = name[6:]
    if name.startswith('?'):
        if name.startswith('??') and len(name) > 4 and name[3] == '@' and name[4] != '@':
            t = name[4:]            # global operator: ??2@YAPAXI@Z
        else:
            tail = name.split('@@', 1)
            if len(tail) < 2 or not tail[1]:
                return None
            t = tail[1]
        if t[0] == 'Y':
            conv = t[1:2]
        elif t[0] in 'CDKLST':        # static members: convention follows directly
            conv = t[1:2]
        elif t[0] in 'ABEFIJMNQRUVWX':  # non-static members: this-qualifier, then convention
            conv = t[2:3]
        elif t[0] == '$':             # templates/thunks: not decoded
            return None
        else:
            return None
        if conv in ('E', 'G', 'I'):
            return True
        if conv == 'A':
            return False
        return None
    if name.startswith('@'):
        return True
    if name.startswith('_'):
        return bool(re.search(r'@\d+$', name))
    return None


def call_conventions(obj, sym, insns: list[Insn]) -> dict[int, bool]:
    """{instruction index: callee cleans} for the direct calls of a COFF function."""
    raw, _, relocs = obj.symbol_extent(sym)
    by_addr = {}
    for r in relocs:
        by_addr[r.virtual_address - sym.value] = r
    out: dict[int, bool] = {}
    for idx, ins in enumerate(insns):
        if ins.mnemonic != 'call':
            continue
        r = by_addr.get(ins.address + 1)
        if r is None:
            if ins.op_str.startswith('dword ptr [e') and '+' in ins.op_str:
                out[idx] = True      # virtual call through a vtable: __thiscall
            continue
        target = obj.symbol_by_index.get(r.symbol_index)
        if target is None:
            continue
        c = callee_cleans(target.name)
        if c is not None:
            out[idx] = c
    return out


def track_frame(insns: list[Insn], conventions: dict[int, bool] | None = None) -> FrameInfo:
    """Track the esp depth through a function and collect its frame accesses.

    The prologue is the run of ``push reg/imm``, ``mov ebp, esp``, the EH
    registration (``mov eax, fs:[0]`` / ``mov fs:[0], esp``) and the frame
    allocation (``sub esp, N`` or ``mov eax, N; call __chkstk``), which ends
    it. ``conventions`` says per call index whether the callee pops its
    arguments (from the callee's mangled name, ``call_conventions``); for
    calls without an entry the ``add esp`` look-ahead below decides. Offsets are relative to the base: esp right after the allocation, so
    the locals occupy [0, N) and offset 0 is the slot nearest esp. Callee-saved
    pushes after the allocation (VC6 interleaves them with the first body
    instructions) and argument pushes raise the tracked depth; ``pop``,
    ``add esp`` lower it. After a ``call`` that no ``add esp`` follows, the
    pushes since the previous call are assumed popped by the callee
    (stdcall/thiscall); pushes of ebx/esi/edi/ebp before the first call are
    callee-saved and exempt. The ``add esp`` may be scheduled a few
    instructions after the call (before the next push, call or branch). That
    reset and the return to the base depth after every ``jmp``/``ret`` (a
    new basic block) are the tracker's heuristics. ``[ebp-N]`` operands count
    only in ``mov ebp, esp`` frames.
    """
    info = FrameInfo()
    n = len(insns)
    i = 0
    pushed_before_frame = 0
    pushed_after_mov_ebp = 0
    ebp_based = False
    frame = 0
    while i < n:
        ins = insns[i]
        m, o = ins.mnemonic, ins.op_str
        if m == 'push' and re.fullmatch(r'e[a-d]x|e[sd]i|ebp|-?0x[0-9a-f]+|-?\d+', o):
            pushed_before_frame += 4
            if ebp_based:
                pushed_after_mov_ebp += 4
            i += 1
            continue
        if m == 'mov' and o == 'ebp, esp' and pushed_before_frame:
            ebp_based = True
            i += 1
            continue
        if m == 'mov' and (o.startswith('eax, dword ptr fs:') or o.startswith('dword ptr fs:')):
            i += 1
            continue
        if m == 'sub' and o.startswith('esp, '):
            frame = _imm(o[5:])
            i += 1
            break
        if m == 'mov' and o.startswith('eax, 0x'):
            # mov eax, N ; [mov fs:[0], esp ;] call __chkstk
            k = i + 1
            while k < n and insns[k].mnemonic == 'mov' and insns[k].op_str.startswith('dword ptr fs:'):
                k += 1
            if k < n and insns[k].mnemonic == 'call':
                frame = _imm(o[5:])
                i = k + 1
                break
        break
    info.frame_size = frame
    info.ebp_based = ebp_based
    info.saved_regs = pushed_before_frame
    info.ebp_delta = pushed_after_mov_ebp + frame
    info.prologue_end = i
    depth = 0            # bytes pushed below the base
    pending = 0          # argument bytes pushed since the last call / add esp
    seen_call = False
    base_depth = None    # depth outside argument sequences (callee-saved pushes)
    for j in range(i, n):
        ins = insns[j]
        m, o = ins.mnemonic, ins.op_str
        fo = frame_operand(o, m)
        if fo is not None:
            reg, disp, width = fo
            if reg == 'esp':
                info.accesses.append(FrameAccess(j, disp - depth, width, m == 'lea', f'{m} {o}'))
            elif ebp_based:
                info.accesses.append(FrameAccess(j, info.ebp_delta + disp, width, m == 'lea', f'{m} {o}'))
        if m in ('jmp', 'ret', 'retn'):
            # a new basic block starts at the base level (VC6 cleans arguments
            # before branching except in rare shared tails)
            if base_depth is not None:
                depth = base_depth
            pending = 0
        if m == 'push':
            depth += 4
            if seen_call or o not in ('ebx', 'esi', 'edi', 'ebp'):
                pending += 4
        elif m == 'pop':
            depth = max(0, depth - 4)
            pending = max(0, pending - 4)
        elif m == 'sub' and o.startswith('esp, '):
            depth += _imm(o[5:])
        elif m == 'add' and o.startswith('esp, '):
            depth = max(0, depth - _imm(o[5:]))
            pending = 0
        elif m == 'call':
            if not seen_call:
                base_depth = depth - pending
            seen_call = True
            known = conventions.get(j) if conventions else None
            if known is None:
                # cdecl callers clean up with ``add esp, N``, which VC6 may schedule a
                # few instructions later; look ahead to the next push/call/branch.
                cdecl = False
                for k in range(j + 1, min(n, j + 12)):
                    nm, no = insns[k].mnemonic, insns[k].op_str
                    if nm == 'add' and no.startswith('esp, '):
                        cdecl = True
                        break
                    if nm in ('push', 'pop', 'call', 'ret', 'retn') or nm.startswith('j'):
                        break
                known = not cdecl
                if known and pending:
                    info.depth_resets += 1
            if known and pending:
                depth = max(0, depth - pending)
            pending = 0     # a cdecl callee's arguments wait for a later ``add esp``
        info.depths.append((j, depth))
    return info


# --------------------------------------------------------------------------
# Alignment of candidate and retail instruction streams
# --------------------------------------------------------------------------

def normalize(ins: Insn) -> str:
    """Instruction text with addresses, frame offsets and relocated values blanked."""
    op = ins.op_str
    if ins.mnemonic.startswith('j') or ins.mnemonic == 'call':
        op = 'X'
    op = re.sub(r'\[0x[0-9a-f]{5,}\]', '[G]', op)
    op = re.sub(r'\b0x[0-9a-f]{6,}\b', 'G', op)
    op = re.sub(r'\[0\]', '[G]', op)
    op = re.sub(r'(esp|ebp) [+-] 0x[0-9a-f]+', r'\1 + S', op)
    op = re.sub(r'(esp|ebp) [+-] \d+', r'\1 + S', op)
    return f'{ins.mnemonic} {op}'


def align(a: list[Insn], b: list[Insn], lookahead: int = 4, max_skip: int = 48) -> list[tuple[int, int]]:
    """Pairs (i, j) of equal normalized instructions, walked in sync.

    Both streams advance together while their normalized instructions agree;
    at a mismatch the smallest skip (candidate and/or retail instructions)
    after which ``lookahead`` instructions agree again resynchronises them.
    Staying in sync is what matters for repeated code such as a run of
    ``strcpy(buffer, "")`` expansions, where an LCS would pair the blocks
    one slot off. Falls back to difflib's LCS when the walk pairs less.
    """
    na = [normalize(x) for x in a]
    nb = [normalize(x) for x in b]
    pairs = []
    i = j = 0
    while i < len(na) and j < len(nb):
        if na[i] == nb[j]:
            pairs.append((i, j))
            i += 1
            j += 1
            continue
        found = None
        for total in range(1, 2 * max_skip + 1):
            for di in range(0, min(total, max_skip) + 1):
                dj = total - di
                if dj > max_skip:
                    continue
                ii, jj = i + di, j + dj
                if ii + lookahead > len(na) or jj + lookahead > len(nb):
                    if ii < len(na) and jj < len(nb) and na[ii:ii + lookahead] == nb[jj:jj + lookahead]:
                        found = (ii, jj)
                        break
                    continue
                if na[ii:ii + lookahead] == nb[jj:jj + lookahead]:
                    found = (ii, jj)
                    break
            if found:
                break
        if not found:
            i += 1
            j += 1
            continue
        i, j = found
    sm = difflib.SequenceMatcher(None, na, nb, autojunk=False)
    lcs = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal':
            lcs.extend((i1 + k, j1 + k) for k in range(i2 - i1))
    return pairs if len(pairs) >= len(lcs) - len(lcs) // 10 else lcs


# --------------------------------------------------------------------------
# The layout rule
# --------------------------------------------------------------------------

@dataclass
class FrameSymbol:
    name: str
    count: int
    size: int
    last_use: int        # order key: position of the symbol's last reference
    region: int = 0      # 0: own slot; >0: sibling-scope members sharing one region
    members: list = field(default_factory=list)   # names merged into this entry (regions)


def vc6_quicksort(items: list, better) -> list:
    """The quicksort VC6's frame packer runs over its symbol list.

    ``better(x, y)`` is True when x must come before y. Equal keys are treated
    as "y before x" by both scans, which gives the characteristic permutation
    of equal-key runs (docs/VC6_FRAME_LAYOUT.md). Pivot: a[(lo+hi)//2] swapped
    to lo, then the CRT-style two-pointer partition.
    """
    a = list(items)

    def comp(x, y):
        return -1 if better(x, y) else 1

    def sort(lo, hi):
        while hi - lo + 1 >= 2:
            mid = (lo + hi) // 2
            a[mid], a[lo] = a[lo], a[mid]
            loguy, higuy = lo, hi + 1
            while True:
                loguy += 1
                while loguy <= hi and comp(a[loguy], a[lo]) <= 0:
                    loguy += 1
                higuy -= 1
                while higuy > lo and comp(a[higuy], a[lo]) >= 0:
                    higuy -= 1
                if higuy < loguy:
                    break
                a[loguy], a[higuy] = a[higuy], a[loguy]
            a[lo], a[higuy] = a[higuy], a[lo]
            sort(lo, higuy - 1)
            lo = loguy

    sort(0, len(a) - 1)
    return a


SMALL_FRAME = 0x80


def predict_layout(symbols: list[FrameSymbol]) -> list[FrameSymbol]:
    """Order the frame symbols nearest-esp first by the VC6 rule.

    Frames over SMALL_FRAME bytes (the sum of the locals' sizes) rank by use
    density, uses per byte:
    1. stable sort by (density desc, size asc, last_use asc);
    2. sibling-scope members (same ``region``) merge into one entry at the first
       member's position with the summed count and the largest size;
    3. the never-equal quicksort, whose comparator knows only density (two
       symbols of equal density compare "not better" both ways, whatever
       their sizes).
    Frames of at most SMALL_FRAME bytes rank by size ascending, then use count
    descending (the count only separates equal sizes; probes in
    docs/VC6_FRAME_LAYOUT.md).
    """
    pre = sorted(symbols, key=lambda s: (-s.count / max(s.size, 1), s.size, s.last_use))
    merged: list[FrameSymbol] = []
    seen: dict[int, FrameSymbol] = {}
    for s in pre:
        if s.region and s.region in seen:
            r = seen[s.region]
            r.count += s.count
            r.size = max(r.size, s.size)
            r.members.append(s.name)
            continue
        entry = FrameSymbol(s.name, s.count, s.size, s.last_use, s.region, [s.name])
        if s.region:
            seen[s.region] = entry
        merged.append(entry)
    total = sum(e.size for e in merged)
    if total > SMALL_FRAME:
        def better(x: FrameSymbol, y: FrameSymbol) -> bool:
            return x.count * max(y.size, 1) > y.count * max(x.size, 1)
    else:
        merged.sort(key=lambda s: (s.size, -s.count, s.last_use))

        def better(x: FrameSymbol, y: FrameSymbol) -> bool:
            return x.size < y.size or (x.size == y.size and x.count > y.count)
    return vc6_quicksort(merged, better)


def flatten(order: list[FrameSymbol]) -> list:
    """Names of a predicted order, region members listed at their region's rank."""
    out = []
    for s in order:
        out.extend(s.members or [s.name])
    return out


def equal_run_permutation(n: int) -> list[int]:
    """0-based order (nearest esp first) of n symbols with equal keys, by last use."""
    return flatten(predict_layout([FrameSymbol(i, 1, 1, i) for i in range(n)]))


def assign_offsets(order: list[FrameSymbol], align_to: int = 4) -> dict[str, int]:
    """Frame offsets (from the base) for a nearest-esp-first order."""
    off = 0
    out = {}
    for s in order:
        a = 4 if s.size >= 4 else (2 if s.size == 2 else 1)
        off = (off + a - 1) // a * a
        out[s.name] = off
        off += s.size
    return out


def suggest_adjustments(symbols: list[FrameSymbol], target: list[str], max_rounds: int = 200) -> dict[str, int] | None:
    """Find count changes that make ``predict_layout`` reproduce ``target`` (names, nearest first).

    A greedy local search: in each round the first symbol out of place moves
    one count toward where it has to be. Returns {name: delta} or None.
    """
    syms = {s.name: FrameSymbol(s.name, s.count, s.size, s.last_use, s.region) for s in symbols}
    delta: dict[str, int] = defaultdict(int)
    want = [n for n in target if n in syms]
    for _ in range(max_rounds):
        order = [n for n in flatten(predict_layout(list(syms.values()))) if n in syms]
        if order == want:
            return dict(delta)
        # first disagreement: the symbol retail has at this rank
        k = next((i for i, (x, y) in enumerate(zip(order, want)) if x != y), None)
        if k is None:
            return None
        wanted = want[k]
        got = order[k]
        if order.index(wanted) > k:     # wanted sits too far from esp: it needs more uses
            syms[wanted].count += 1
            delta[wanted] += 1
        else:
            syms[got].count += 1        # the usurper has to stay, so the wanted one lacks uses
            delta[got] += 1
        if any(abs(v) > 40 for v in delta.values()):
            return None
    return None


# --------------------------------------------------------------------------
# Report
# --------------------------------------------------------------------------

def local_ranges(proc: CvProc, info: FrameInfo) -> list[dict]:
    """Candidate locals as frame-base offsets with sizes (gap-filled when the type is unknown)."""
    locs = [l for l in proc.locals if l.register is None and l.bprel < 0]
    # CodeView bprel offsets are relative to the entry esp (return-address slot) for
    # esp frames and to ebp for ebp frames; convert to the base = esp after the prologue.
    shift = info.ebp_delta if info.ebp_based else info.frame_size + info.saved_regs
    rows = []
    for l in locs:
        rows.append({'name': l.name, 'offset': l.bprel + shift, 'bprel': l.bprel, 'size': l.size,
                     'type': l.type_index, 'block': l.block_id, 'depth': l.block})
    rows.sort(key=lambda r: r['offset'])
    seen: Counter = Counter()
    for r in rows:
        seen[r['name']] += 1
        if seen[r['name']] > 1:
            r['name'] = '%s#%d' % (r['name'], seen[r['name']])
    for i, r in enumerate(rows):
        if r['size'] is None or r['size'] == 0:
            nxt = next((x['offset'] for x in rows[i + 1:] if x['offset'] > r['offset']), info.frame_size)
            r['size'] = max(nxt - r['offset'], 1)
            r['size_guessed'] = True
    return rows


def locate(rows: list[dict], off: int) -> dict | None:
    for r in rows:
        if r['offset'] <= off < r['offset'] + r['size']:
            return r
    return None


def build_report(cand: list[Insn], retail: list[Insn], proc: CvProc,
                 conventions: dict[int, bool] | None = None) -> dict:
    ci = track_frame(cand, conventions)
    pairs = align(cand, retail)
    # retail calls aligned with candidate calls inherit the callee's convention
    rconv: dict[int, bool] = {}
    if conventions:
        by_target: dict[str, bool] = {}
        for i, j in pairs:
            if i in conventions and retail[j].mnemonic == 'call':
                rconv[j] = conventions[i]
                by_target.setdefault(retail[j].op_str, conventions[i])
        # unaligned retail calls: the same target elsewhere, else the nearest
        # candidate call by translated index
        pair_of0 = dict(pairs)
        cand_calls = sorted(conventions)
        for j, ins in enumerate(retail):
            if ins.mnemonic != 'call' or j in rconv:
                continue
            if ins.op_str in by_target:
                rconv[j] = by_target[ins.op_str]
                continue
            prev = max((jj for jj in pair_of0.values() if jj <= j), default=None)
            if prev is None:
                continue
            i_prev = next(i for i, jj in pairs if jj == prev)
            ti = i_prev + (j - prev)
            near = [i for i in cand_calls if abs(i - ti) <= 3]
            if len(near) == 1:
                rconv[j] = conventions[near[0]]
    ri = track_frame(retail, rconv)
    rows = local_ranges(proc, ci)
    unmapped_retail: Counter = Counter()
    # candidate instruction index -> retail index (aligned, else the nearest aligned one)
    pair_of = dict(pairs)
    translate = []
    last = 0
    for i in range(len(cand)):
        if i in pair_of:
            last = pair_of[i] - i
        translate.append(i + last)
    # per-local reference counts and last use in the candidate
    refs: dict[str, list[FrameAccess]] = defaultdict(list)
    for a in ci.accesses:
        r = locate(rows, a.offset)
        if r is not None:
            refs[r['name']].append(a)
    # retail accesses by offset, for signature matching
    by_roff: dict[int, list[int]] = defaultdict(list)
    for a in ri.accesses:
        by_roff[a.offset].append(a.index)
    WINDOW = 3
    racc_by_index = {a.index: a for a in ri.accesses}
    for r in rows:
        acc = refs[r['name']]
        r['refs'] = len(acc)
        r['last_use'] = max(a.index for a in acc) if acc else -1
        # candidate retail bases: the retail access aligned with each candidate
        # access (weight 3) and same-kind accesses within a few instructions
        # of the translated index (weight 1)
        scores: Counter = Counter()
        dist: Counter = Counter()
        for a in acc:
            rel = a.offset - r['offset']
            ti = translate[a.index]
            cands_b = []
            if a.index in pair_of and pair_of[a.index] in racc_by_index:
                cands_b.append((racc_by_index[pair_of[a.index]], 3))
            for k in range(ti - WINDOW, ti + WINDOW + 1):
                b = racc_by_index.get(k)
                if b is not None and b.lea == a.lea and b.width == a.width:
                    cands_b.append((b, 1))
            for b, w in cands_b:
                base = b.offset - rel
                if 0 <= base < ri.frame_size:
                    scores[base] += w
                    dist[base] += abs(b.index - ti)
        if scores:
            best = max(scores, key=lambda b: (scores[b], -dist[b]))
            r['retail_offset'] = best
            r['votes'] = sorted(scores.items(), key=lambda kv: (-kv[1], dist[kv[0]]))[:3]
        else:
            r['retail_offset'] = None
            r['votes'] = []
    # a retail slot can hold only one candidate local: keep the better-supported claim
    claimed: dict[int, dict] = {}
    for r in sorted(rows, key=lambda r: -(r['votes'][0][1] if r['votes'] else 0)):
        ro = r['retail_offset']
        if ro is None:
            continue
        if ro in claimed and claimed[ro]['size'] == r['size']:
            r['retail_offset'] = None
            r['conflict'] = ro
        else:
            claimed[ro] = r
    mapped = {r['retail_offset'] for r in rows if r['retail_offset'] is not None}
    for a in ri.accesses:
        hit = any(r['retail_offset'] is not None and r['retail_offset'] <= a.offset < r['retail_offset'] + r['size'] for r in rows)
        if not hit and 0 <= a.offset < ri.frame_size:
            unmapped_retail[a.offset] += 1
    cand_order = [r['name'] for r in sorted(rows, key=lambda r: r['offset'])]
    retail_rows = [r for r in rows if r['retail_offset'] is not None]
    retail_order = [r['name'] for r in sorted(retail_rows, key=lambda r: r['retail_offset'])]
    # gaps in retail between mapped locals
    gaps = []
    prev_end = 0
    for r in sorted(retail_rows, key=lambda r: r['retail_offset']):
        if r['retail_offset'] > prev_end + 3:
            gaps.append((prev_end, r['retail_offset'] - prev_end))
        prev_end = max(prev_end, r['retail_offset'] + r['size'])
    rtop = ri.frame_size
    if rtop > prev_end + 3:
        gaps.append((prev_end, rtop - prev_end))
    # prediction from the disassembly's counts
    region_of = {}
    for r in rows:
        region_of[r['name']] = r['block'] if r['depth'] else 0
    syms = [FrameSymbol(r['name'], max(r['refs'], 1), r['size'], r['last_use'], 0) for r in rows]
    # sibling blocks that share an offset form a region
    by_offset: dict[int, list[dict]] = defaultdict(list)
    for r in rows:
        by_offset[r['offset']].append(r)
    for off, grp in by_offset.items():
        if len(grp) > 1:
            for s in syms:
                if s.name in {g['name'] for g in grp}:
                    s.region = off + 1
    predicted = flatten(predict_layout(syms))
    adjust = suggest_adjustments(syms, retail_order) if retail_order else None
    return {
        'candidate': {'frame_size': ci.frame_size, 'saved_regs': ci.saved_regs, 'ebp_based': ci.ebp_based,
                      'insns': len(cand), 'accesses': len(ci.accesses), 'depth_resets': ci.depth_resets},
        'retail': {'frame_size': ri.frame_size, 'saved_regs': ri.saved_regs, 'ebp_based': ri.ebp_based,
                   'insns': len(retail), 'accesses': len(ri.accesses), 'depth_resets': ri.depth_resets},
        'aligned_pairs': len(pairs),
        'locals': rows,
        'candidate_order': cand_order,
        'retail_order': retail_order,
        'predicted_order': predicted,
        'retail_gaps': gaps,
        'unmapped_retail_accesses': sorted(unmapped_retail.items()),
        'adjustments': adjust,
    }


def print_report(rep: dict) -> None:
    c, r = rep['candidate'], rep['retail']
    print(f"candidate: frame {c['frame_size']:#x} + saved {c['saved_regs']:#x}, {c['insns']} insns, "
          f"{c['accesses']} frame accesses, {c['depth_resets']} depth resets"
          f"{' (ebp frame)' if c['ebp_based'] else ''}")
    print(f"retail:    frame {r['frame_size']:#x} + saved {r['saved_regs']:#x}, {r['insns']} insns, "
          f"{r['accesses']} frame accesses, {r['depth_resets']} depth resets"
          f"{' (ebp frame)' if r['ebp_based'] else ''}")
    print(f"aligned instruction pairs: {rep['aligned_pairs']}")
    print()
    print('locals (candidate order, nearest esp first):')
    print('  %-28s %6s %8s %8s %5s %5s %s' % ('name', 'size', 'cand', 'retail', 'refs', 'rank', 'retail rank / votes'))
    rrank = {n: i for i, n in enumerate(rep['retail_order'])}
    for i, row in enumerate(rep['locals']):
        ro = row['retail_offset']
        flag = ''
        if ro is None:
            flag = '?'
        elif rrank.get(row['name']) != i:
            flag = '*'
        print('  %-28s %6x %8x %8s %5d %5d %s%s %s' % (
            row['name'], row['size'], row['offset'], ('%x' % ro) if ro is not None else '-',
            row['refs'], i, rrank.get(row['name'], '-'), flag,
            ' '.join('%x:%d' % (o, n) for o, n in row['votes'])))
    print()
    if rep['retail_gaps']:
        print('retail slots no candidate local maps to:')
        for off, size in rep['retail_gaps']:
            print(f'  retail offset {off:#x}, {size:#x} bytes')
    if rep['unmapped_retail_accesses']:
        print('retail accesses outside mapped locals (offset:count):',
              ' '.join(f'{o:#x}:{n}' for o, n in rep['unmapped_retail_accesses'][:20]))
    print()
    print('retail order    :', ' '.join(rep['retail_order']))
    print('candidate order :', ' '.join(rep['candidate_order']))
    print('rule from refs  :', ' '.join(rep['predicted_order']))
    if rep['predicted_order'] != rep['candidate_order']:
        print('  (the reference count read from the disassembly does not reproduce the candidate;'
              ' treat refs as a proxy)')
    print()
    adj = rep['adjustments']
    if adj is None:
        print('no use-count adjustment found that reproduces the retail order')
    elif not adj:
        print('the rule already gives the retail order with the current counts')
    else:
        print('use-count changes that give the retail order:')
        for name, d in sorted(adj.items(), key=lambda kv: -abs(kv[1])):
            print(f'  {name}: {d:+d}')


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('obj')
    ap.add_argument('symbol', nargs='?', default='', help='substring of the COFF function symbol')
    ap.add_argument('--va', default=None, help='retail function VA')
    ap.add_argument('--dump', action='store_true',
                    help='only list every CodeView procedure of OBJ with its locals nearest esp first')
    ap.add_argument('--size', type=int, default=None, help='retail bytes to disassemble (default: candidate size)')
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE'))
    ap.add_argument('--cv-name', default=None, help='CodeView procedure name substring (default: derived from the symbol)')
    ap.add_argument('--json', action='store_true')
    a = ap.parse_args()
    from mcm2tool.coff import CoffObject
    from mcm2tool.pe import PEImage
    obj = CoffObject(a.obj)
    if a.dump:
        types: dict[int, CvType] = {}
        procs: list[CvProc] = []
        for sec in obj.sections:
            raw = obj.data[sec.raw_ptr:sec.raw_ptr + sec.raw_size]
            if sec.name == '.debug$T':
                types.update(parse_cv_types(raw))
            elif sec.name == '.debug$S':
                procs.extend(parse_cv_procs(raw))
        for proc in procs:
            if a.symbol and a.symbol not in proc.name:
                continue
            locs = sorted((l for l in proc.locals if l.register is None and l.bprel < 0), key=lambda l: l.bprel)
            print(proc.name + ':', ' '.join('%s[%s]@%x' % (l.name, type_size(l.type_index, types) or '?', -l.bprel) for l in locs))
        return
    if not a.va:
        raise SystemExit('--va is required unless --dump is given')
    cands = [s for s in obj.symbols if a.symbol in s.name and s.section_number > 0 and not s.name.startswith('$')]
    cands = [s for s in cands if s.name == a.symbol] or [s for s in cands if '$' not in s.name[1:] or s.name.startswith('?')]
    if not cands:
        near = [s.name for s in obj.symbols if s.section_number > 0 and a.symbol.strip('?@').split('@')[0] in s.name][:5]
        raise SystemExit(f'no symbol matches {a.symbol!r}' + (f'; near: {near}' if near else ''))
    sym = cands[0]
    raw, _, _ = obj.symbol_extent(sym)
    cv_name = a.cv_name
    if cv_name is None:
        m = re.match(r'\?(\w+)@(\w+)@', sym.name)
        cv_name = f'{m.group(2)}::{m.group(1)}' if m else re.sub(r'^\?|@@.*$', '', sym.name)
    proc, _types = load_codeview(obj, cv_name)
    if not a.exe:
        raise SystemExit('set MCM2_EXE or pass --exe')
    va = int(a.va, 16)
    size = a.size or len(raw)
    retail = PEImage(a.exe).bytes_at_va(va, size)
    cand_insns = disassemble(raw)
    rep = build_report(cand_insns, disassemble(retail, va), proc, call_conventions(obj, sym, cand_insns))
    if a.json:
        print(json.dumps(rep, indent=1, default=str))
    else:
        print(f'{sym.name} ({len(raw)} bytes) vs retail {va:#x} ({size} bytes); CodeView proc {proc.name}')
        print_report(rep)


if __name__ == '__main__':
    main()
