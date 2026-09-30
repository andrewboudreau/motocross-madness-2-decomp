#!/usr/bin/env python3
"""Disassemble one retail function with extent recovery and evidence annotations.

Extent is recovered by linear sweep: the function ends at the first ret/jmp
(unconditional) that lies at or beyond every forward branch target seen so far.
Jump tables (``jmp [reg*4+table]``) are decoded and their entries count as
branch targets; the table bytes themselves are reported but not included in
the instruction extent. The recovered size is a heuristic (evidence tier 2):
cross-check it against the next function start before trusting it.

Annotations are derived from generated analysis files:
- call/jmp targets that are known vtable entries -> ``Class slot N``;
- absolute memory operands in non-code sections -> float/double/string preview;
- ``__FILE__`` source path strings.
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
from pathlib import Path

import capstone
from capstone import x86

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from mcm2tool.pe import PEImage  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]


def load_slot_map(analysis: Path) -> dict[int, list[str]]:
    out: dict[int, list[str]] = {}
    try:
        vts = json.loads((analysis / 'vtables.json').read_text())
    except OSError:
        return out
    for vt in vts:
        for i, e in enumerate(vt.get('entries', [])):
            label = f"{vt['class']}+{vt.get('object_offset', 0)} slot {i}"
            out.setdefault(int(e, 16), []).append(label)
    return out


def load_source_strings(analysis: Path) -> dict[int, str]:
    try:
        rows = json.loads((analysis / 'source_xrefs.json').read_text())
    except OSError:
        return {}
    return {int(r['string_va'], 16): r['path'] for r in rows}


def preview_data(pe: PEImage, va: int, sources: dict[int, str]) -> str | None:
    if va in sources:
        return f'__FILE__ {sources[va]!r}'
    if not pe.is_va_mapped(va) or pe.is_code_va(va):
        return None
    try:
        off = pe.va_to_offset(va)
    except Exception:
        return None
    raw = pe.data[off:off + 8]
    parts = []
    if len(raw) >= 4:
        f = struct.unpack_from('<f', raw)[0]
        parts.append(f'f32={f:.9g}')
    if len(raw) == 8:
        d = struct.unpack_from('<d', raw)[0]
        if abs(d) < 1e12 and (d == 0 or abs(d) > 1e-12):
            parts.append(f'f64={d:.17g}')
    s = pe.data[off:off + 48]
    if s[:1].isascii() and s[:1] not in (b'\0',):
        txt = s.split(b'\0', 1)[0]
        if len(txt) >= 4 and all(32 <= c < 127 for c in txt):
            parts.append(f'str={txt.decode()!r}')
    return ' '.join(parts) or None


def disassemble(pe: PEImage, va: int, max_size: int = 0x4000):
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    off = pe.va_to_offset(va)
    code = pe.data[off:off + max_size]
    insns = []
    max_target = va
    tables: list[tuple[int, list[int]]] = []
    # byte index tables of sparse switches: (va, length); VC6 emits
    # `cmp eax,N; ja default; mov cl,[eax+idx]; jmp [ecx*4+tbl]` and places the
    # N+1 index bytes right after the dword jump table
    index_tables: list[tuple[int, int]] = []
    last_cmp_imm = None
    end = None
    pos = 0
    while pos < len(code):
        cur = va + pos
        if any(t <= cur < t + 4 * len(e) for t, e in tables):
            # skip over an inline jump table; entries already registered
            t, e = next((t, e) for t, e in tables if t <= cur < t + 4 * len(e))
            pos = t + 4 * len(e) - va
            continue
        if any(t <= cur < t + n for t, n in index_tables):
            t, n = next((t, n) for t, n in index_tables if t <= cur < t + n)
            pos = t + n - va
            continue
        batch = list(md.disasm(code[pos:pos + 16], cur, 1))
        if not batch:
            break
        ins = batch[0]
        insns.append(ins)
        pos += ins.size
        if ins.mnemonic == 'cmp' and len(ins.operands) == 2 and ins.operands[1].type == x86.X86_OP_IMM:
            last_cmp_imm = ins.operands[1].imm
        if (ins.mnemonic in ('mov', 'movzx') and len(ins.operands) == 2
                and ins.operands[1].type == x86.X86_OP_MEM and ins.operands[1].size == 1):
            m = ins.operands[1].mem
            if (m.base or m.index) and va < m.disp < va + max_size and last_cmp_imm is not None                     and 0 <= last_cmp_imm < 256:
                index_tables.append((m.disp, last_cmp_imm + 1))
        grp_jump = ins.group(capstone.CS_GRP_JUMP)
        if grp_jump and ins.operands and ins.operands[0].type == x86.X86_OP_IMM:
            tgt = ins.operands[0].imm
            if tgt > max_target and tgt < va + max_size:
                max_target = tgt
        if ins.mnemonic == 'jmp' and ins.operands and ins.operands[0].type == x86.X86_OP_MEM:
            m = ins.operands[0].mem
            if m.scale == 4 and m.base == 0 and m.disp and pe.is_va_mapped(m.disp):
                entries = []
                toff = pe.va_to_offset(m.disp)
                for k in range(256):
                    e = struct.unpack_from('<I', pe.data, toff + 4 * k)[0]
                    if not (va <= e < va + max_size):
                        break
                    entries.append(e)
                if entries:
                    tables.append((m.disp, entries))
                    max_target = max(max_target, *entries)
        is_ret = ins.mnemonic in ('ret', 'retn')
        is_jmp = ins.mnemonic == 'jmp'
        if (is_ret or is_jmp) and va + pos > max_target:
            end = va + pos
            break
    return insns, (end or va + pos), tables, index_tables


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('va', help='function start VA, e.g. 0x004cc0a0')
    ap.add_argument('--exe', default=str(ROOT / 'work/game/mcm2.exe'))
    ap.add_argument('--analysis', default=str(ROOT / 'analysis'))
    ap.add_argument('--size-only', action='store_true', help='print only the recovered size')
    ap.add_argument('--json', action='store_true')
    a = ap.parse_args()
    pe = PEImage(a.exe)
    va = int(a.va, 16)
    analysis = Path(a.analysis)
    slots = load_slot_map(analysis)
    sources = load_source_strings(analysis)
    insns, end, tables, index_tables = disassemble(pe, va)
    size = end - va
    # trailing tables that sit after the last instruction extend the COFF extent
    table_end = max([t + 4 * len(e) for t, e in tables if t >= end]
                    + [t + n for t, n in index_tables if t >= end], default=end)
    pad = pe.data[pe.va_to_offset(end):pe.va_to_offset(end) + 16]
    npad = 0
    for b in pad:
        if b in (0x90, 0xCC):
            npad += 1
        else:
            break
    if a.size_only:
        print(size)
        return 0
    if a.json:
        print(json.dumps({
            'va': f'0x{va:08x}', 'size': size, 'table_end': f'0x{table_end:08x}',
            # VC6 emits switch tables inline after the code; a COFF symbol extent covers them,
            # so compare jump-table functions with this size rather than 'size'.
            'extent_with_tables': table_end - va,
            'padding_after': npad, 'next_candidate_start': f'0x{end + npad:08x}',
            'vtable_slots': slots.get(va, []),
            'jump_tables': [{'va': f'0x{t:08x}', 'entries': [f'0x{x:08x}' for x in e]} for t, e in tables],
            'index_tables': [{'va': f'0x{t:08x}', 'length': n} for t, n in index_tables],
        }, indent=2))
        return 0
    print(f'; function 0x{va:08x}  size={size} (0x{size:x})  end=0x{end:08x}  padding_after={npad}  next_start~0x{end + npad:08x}')
    for s in slots.get(va, []):
        print(f';   vtable: {s}')
    for t, e in tables:
        print(f';   jump table @0x{t:08x}: {len(e)} entries')
    for t, n in index_tables:
        print(f';   byte index table @0x{t:08x}: {n} entries')
    for ins in insns:
        note = []
        for op in ins.operands:
            if op.type == x86.X86_OP_IMM:
                v = op.imm & 0xffffffff
                if ins.group(capstone.CS_GRP_CALL) or ins.group(capstone.CS_GRP_JUMP):
                    if v in slots:
                        note.append('-> ' + ', '.join(slots[v][:3]))
                else:
                    p = preview_data(pe, v, sources)
                    if p:
                        note.append(f'[imm 0x{v:08x}] {p}')
            elif op.type == x86.X86_OP_MEM and op.mem.base == 0 and op.mem.index == 0 and op.mem.disp:
                v = op.mem.disp & 0xffffffff
                p = preview_data(pe, v, sources)
                if p:
                    note.append(f'[0x{v:08x}] {p}')
        raw = ins.bytes.hex(' ')
        line = f'{ins.address:08x}: {raw:<30} {ins.mnemonic} {ins.op_str}'
        if note:
            line += '   ; ' + ' | '.join(note)
        print(line)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
