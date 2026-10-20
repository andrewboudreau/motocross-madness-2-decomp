"""Compare the resolved references of each near miss's candidate with retail.

usage:
  PYTHONPATH=. python tools/reference_diff.py --vc6-root DIR [--exe mcm2.exe]
         [--jobs 8] [--json OUT] [--md OUT] [--lane LANE] [--va 0xVA ...] [--reuse]

Scheduling and register allocation reorder a function's references, but they do
not add or remove them. A candidate that lacks a call, a string, a global or a
constant that retail uses is missing source (a statement, a switch case, an
argument). This tool counts each row's references on both sides as multisets
and reports what one side has and the other lacks.

Rows and candidates are those of tools/rank_near_misses.py (the index rows, the
bindings/targets/marker candidates, the same compile profile and retail extent);
of several candidates the one with the fewest differing instructions is used.

References per instruction:
- call: a direct call, or a direct jmp leaving the function (a tail call).
  Retail gives the target VA; the candidate's REL32 relocation symbol is resolved
  through its own bindings, then through every other bindings file.
- addr: any other absolute address (imm32 or disp32). Retail: a value mapped in
  the image outside the function and its tables. Candidate: a DIR32 relocation,
  resolved as above plus the field's addend.
  Unresolved candidate symbols are matched by content where possible: string
  literals by their text, `__real@` constants by value, `__imp__` by import name;
  the same keys are computed for retail's leftover addresses. What still has no
  partner is `unbound` (candidate) or missing (retail).
- imm: non-address immediates, except 0 and stack adjustments of esp.
- switch: per `jmp [reg*4 + table]` the table's entry count and distinct
  targets, and the number of byte index tables.

Skipped on both sides: jump/index tables, the C++ EH handler pushed after
`push -1`, absolute symbols such as __except_list, and candidate symbols in the
function's own section.

Rows are ranked by what retail has that the candidate lacks (calls and
addresses net of unbound candidate references of the same kind, plus switch
entries), then by missing immediates.
"""
from __future__ import annotations

import argparse
import bisect
import json
import os
import struct
import sys
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.rank_near_misses import (candidates, compare, compile_one, function_symbols, lane_for,  # noqa: E402
                                    parse_index, retail_extent, symbol_insns)

REL32 = 0x0014
DIR32 = (0x0006, 0x0007)
FRAME_OPS = {'add', 'sub', 'and'}


def disasm(code: bytes, base: int):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    return list(md.disasm(code, base))


def is_branch(i) -> bool:
    return i.mnemonic == 'call' or i.mnemonic.startswith('j')


def imm_of(i):
    """The non-branch immediate operand values of an instruction."""
    from capstone import x86
    return [op.imm & 0xffffffff for op in i.operands if op.type == x86.X86_OP_IMM]


def disp_of(i):
    from capstone import x86
    return [op.mem.disp & 0xffffffff for op in i.operands if op.type == x86.X86_OP_MEM]


def frame_adjust(i) -> bool:
    return i.mnemonic in FRAME_OPS and i.op_str.startswith('esp, ')


def keep_imm(i, value: int) -> bool:
    return value != 0 and not frame_adjust(i)


def float_width(i) -> int:
    if not i.mnemonic.startswith('f'):
        return 0
    for w, n in (('dword ptr', 4), ('qword ptr', 8), ('tbyte ptr', 10)):
        if w in i.op_str:
            return n
    return 0


def float_key(data: bytes, width: int):
    if width == 4 and len(data) >= 4:
        return 'flt:%r' % struct.unpack('<f', data[:4])[0]
    if width == 8 and len(data) >= 8:
        return 'flt:%r' % struct.unpack('<d', data[:8])[0]
    return None


def string_key(data: bytes):
    end = data.find(b'\0')
    if end < 0:
        return None
    s = data[:end]
    if s and all(32 <= b < 127 or b in (9, 10, 13) for b in s):
        text = s.decode('latin-1')
        if ':' + chr(92) in text and text.lower().endswith(('.cpp', '.c', '.h')):
            return 'str:__FILE__'      # the build path differs; any source path stands for __FILE__
        return 'str:' + text
    return None


def iat_names(pe) -> dict[int, str]:
    """IAT slot VA -> imported name."""
    out = {}
    if len(pe.directories) < 2 or not pe.directories[1][0]:
        return out
    off = pe.rva_to_offset(pe.directories[1][0])
    while True:
        oft, _, _, name_rva, ft = struct.unpack_from('<IIIII', pe.data, off)
        if not any((oft, name_rva, ft)):
            break
        to = pe.rva_to_offset(oft or ft)
        k = 0
        while True:
            val = pe.u32(to + 4 * k)
            if val == 0:
                break
            name = 'ordinal:%d' % (val & 0xffff) if val & 0x80000000 else \
                pe.read_c_string_at_offset(pe.rva_to_offset(val) + 2)
            out[pe.image_base + ft + 4 * k] = name
            k += 1
        off += 20
    return out


class Retail:
    """Read-only view of the retail image for reference keys."""

    def __init__(self, pe):
        self.pe = pe
        self.lo = pe.image_base + 0x1000
        self.hi = pe.image_base + pe.size_of_image
        self.iat = iat_names(pe)

    def is_address(self, v: int) -> bool:
        """Inside a section's virtual extent (uninitialised data included)."""
        if not self.lo <= v < self.hi:
            return False
        rva = v - self.pe.image_base
        return any(s.virtual_address <= rva < s.virtual_address + max(s.virtual_size, s.raw_size)
                   for s in self.pe.sections)

    def read(self, va: int, n: int) -> bytes:
        try:
            return self.pe.bytes_at_va(va, n)
        except Exception:
            return b''

    def alt_key(self, va: int, width: int):
        if va in self.iat:
            return 'imp:' + self.iat[va]
        if width:
            return float_key(self.read(va, width), width)
        if self.pe.is_code_va(va):
            return None
        return string_key(self.read(va, 512))


def table_entries(read_u32, table: int, lo: int, hi: int, limit: int) -> list[int]:
    """Consecutive dwords at table whose values fall in [lo, hi)."""
    out = []
    while len(out) < limit:
        v = read_u32(table + 4 * len(out))
        if v is None or not lo <= v < hi:
            break
        out.append(v)
    return out


def retail_refs(rt: Retail, va: int, size: int, internal_end: int) -> dict:
    code = rt.read(va, size)
    refs, tables, byte_tables = [], [], 0
    insns = disasm(code, va)
    internal = lambda v: va <= v < internal_end  # noqa: E731

    def u32(a):
        b = rt.read(a, 4)
        return struct.unpack('<I', b)[0] if len(b) == 4 else None
    eh_seen = False
    for k, i in enumerate(insns):
        if is_branch(i):
            targets = imm_of(i)
            if targets:
                if not internal(targets[0]) and (i.mnemonic == 'call' or i.mnemonic == 'jmp'):
                    refs.append(dict(kind='call', va=targets[0], alt=None))
                continue
        # the C++ EH handler: the first push of an address after the prologue's `push -1`
        eh = k <= 4 and not eh_seen and i.mnemonic == 'push' and any(
            x.mnemonic == 'push' and imm_of(x) == [0xffffffff] for x in insns[:k])
        for v in imm_of(i):
            if rt.is_address(v):
                if eh:
                    eh_seen = True
                    continue
                if not internal(v):
                    refs.append(dict(kind='addr', va=v, alt=rt.alt_key(v, 0)))
            elif keep_imm(i, v):
                refs.append(dict(kind='imm', value=v))
        for v in disp_of(i):
            if not rt.is_address(v):
                continue
            if internal(v):
                if i.mnemonic == 'jmp' and '*4' in i.op_str:
                    ent = table_entries(u32, v, va, v, 1024)
                    tables.append((len(ent), len(set(ent))))
                elif 'byte ptr' in i.op_str:
                    byte_tables += 1
                continue
            refs.append(dict(kind='addr', va=v, alt=rt.alt_key(v, float_width(i))))
    return dict(refs=refs, tables=sorted(tables), byte_tables=byte_tables)


def load_bindings(root: Path) -> dict[str, int]:
    out: dict[str, int] = {}
    for b in sorted(list((root / 'src').rglob('*.bindings.json')) + list((root / 'samples').rglob('*.bindings.json'))):
        try:
            d = json.loads(b.read_text(encoding='utf-8'))
        except ValueError:
            continue
        if isinstance(d, dict):
            for k, v in d.items():
                if isinstance(v, str):
                    try:
                        out.setdefault(k, int(v, 16))
                    except ValueError:
                        pass
    return out


def own_bindings(cpp: Path) -> dict[str, int]:
    b = cpp.with_name(cpp.stem + '.bindings.json')
    if not b.exists():
        return {}
    try:
        d = json.loads(b.read_text(encoding='utf-8'))
    except ValueError:
        return {}
    out = {}
    for k, v in (d.items() if isinstance(d, dict) else []):
        if isinstance(v, str):
            try:
                out[k] = int(v, 16)
            except ValueError:
                pass
    return out


def symbol_data(o, s, n: int) -> bytes:
    if s.section_number <= 0:
        return b''
    sec = o.section(s.section_number)
    if not sec.raw_ptr:
        return b''
    start = sec.raw_ptr + s.value
    return o.data[start:min(start + n, sec.raw_ptr + sec.raw_size)]


def candidate_refs(o, sym, resolve, rt: Retail | None) -> dict:
    """References of a COFF function; resolve(name) gives a retail VA or None."""
    raw, _, rel = o.symbol_extent(sym)
    base = sym.value
    insns = disasm(raw, base)
    by_off = {r.virtual_address: r for r in rel}
    section_rel = {r.virtual_address: r for r in o.relocations if r.section_number == sym.section_number}
    refs, tables, byte_tables = [], [], 0
    cut = len(raw)
    for i in insns:                     # the function's own jump tables end its code
        if i.address - base >= cut:
            break
        here = [by_off[a] for a in range(i.address, i.address + i.size) if a in by_off]
        covered = set()
        for r in here:
            covered.add(r.virtual_address)
            t = o.symbol_by_index.get(r.symbol_index)
            if t is None:
                continue
            addend = int.from_bytes(raw[r.virtual_address - base:r.virtual_address - base + 4], 'little', signed=True)
            if t.section_number == sym.section_number:
                if r.type in DIR32 and i.mnemonic == 'jmp' and '*4' in i.op_str:
                    table = t.value + addend
                    ent = []
                    while table + 4 * len(ent) in section_rel and len(ent) < 1024:
                        e = section_rel[table + 4 * len(ent)]
                        et = o.symbol_by_index.get(e.symbol_index)
                        off = table + 4 * len(ent) - base
                        add = int.from_bytes(raw[off:off + 4], 'little') if 0 <= off < len(raw) - 3 else 0
                        ent.append((et.value if et else 0) + add)
                    tables.append((len(ent), len(set(ent))))
                    if base <= table < base + cut:
                        cut = table - base
                elif r.type in DIR32 and 'byte ptr' in i.op_str:
                    byte_tables += 1
                    table = t.value + addend
                    if base < table < base + cut:
                        cut = table - base
                continue
            if t.section_number == -1 or t.name == '__except_list' or t.name.startswith(('__ehhandler$', '$L')):
                continue
            if r.type == REL32:
                kind, add = 'call', 0
            elif r.type in DIR32:
                kind, add = 'addr', addend
            else:
                continue
            va = resolve(t.name)
            alt = None
            width = float_width(i)
            if t.name.startswith('__imp__'):
                alt = 'imp:' + t.name[len('__imp__'):].split('@')[0]
            elif t.name.startswith('__real@'):
                alt = float_key(symbol_data(o, t, 8), width or (4 if t.name.startswith('__real@4') else 8))
            elif t.name.startswith('??_C@'):
                alt = string_key(symbol_data(o, t, 4096)[add:])
            if va is not None:
                va += add
                if rt is not None and alt is None:
                    alt = rt.alt_key(va, width) if kind == 'addr' else None
            refs.append(dict(kind=kind, va=va, alt=alt, name=t.name if va is None else None))
        if is_branch(i):
            continue
        for v in imm_of(i):
            if i.imm_offset and i.address + i.imm_offset in covered:
                continue
            if keep_imm(i, v):
                refs.append(dict(kind='imm', value=v))
    return dict(refs=refs, tables=sorted(tables), byte_tables=byte_tables)


def match_refs(retail: dict, cand: dict) -> dict:
    """Multiset difference: primary VA keys, then content keys, then unbound names."""
    out = {}
    for kind in ('call', 'addr'):
        r = [x for x in retail['refs'] if x['kind'] == kind]
        c = [x for x in cand['refs'] if x['kind'] == kind]
        rc = Counter(x['va'] for x in r)
        cc = Counter(x['va'] for x in c if x['va'] is not None)
        common = rc & cc
        r_left, c_left = [], []
        used = Counter()
        for x in r:
            if used[x['va']] < common[x['va']]:
                used[x['va']] += 1
            else:
                r_left.append(x)
        used = Counter()
        for x in c:
            if x['va'] is not None and used[x['va']] < common[x['va']]:
                used[x['va']] += 1
            else:
                c_left.append(x)
        alts = Counter(x['alt'] for x in c_left if x['alt'])
        r_final = []
        for x in r_left:
            if x['alt'] and alts[x['alt']]:
                alts[x['alt']] -= 1
            else:
                r_final.append(x)
        matched_alt = Counter(x['alt'] for x in c_left if x['alt']) - alts
        c_final = []
        for x in c_left:
            if x['alt'] and matched_alt[x['alt']]:
                matched_alt[x['alt']] -= 1
            else:
                c_final.append(x)
        out[kind] = dict(missing=r_final, extra=[x for x in c_final if x['va'] is not None or x['alt']],
                         unbound=[x for x in c_final if x['va'] is None and not x['alt']])
    ri = Counter(x['value'] for x in retail['refs'] if x['kind'] == 'imm')
    ci = Counter(x['value'] for x in cand['refs'] if x['kind'] == 'imm')
    out['imm'] = dict(missing=sorted((ri - ci).elements()), extra=sorted((ci - ri).elements()))
    out['switch'] = dict(retail=retail['tables'], cand=cand['tables'],
                         retail_byte_tables=retail['byte_tables'], cand_byte_tables=cand['byte_tables'])
    return out


def score(m: dict) -> dict:
    calls = max(0, len(m['call']['missing']) - len(m['call']['unbound']))
    addrs = max(0, len(m['addr']['missing']) - len(m['addr']['unbound']))
    sw = m['switch']
    entries = sum(t[0] for t in sw['retail']) - sum(t[0] for t in sw['cand'])
    targets = sum(t[1] for t in sw['retail']) - sum(t[1] for t in sw['cand'])
    tables = len(sw['retail']) - len(sw['cand'])
    switch = max(0, entries) + max(0, targets) + max(0, tables) * 4
    return dict(missing_calls=calls, missing_addrs=addrs, missing_switch=switch,
                missing_imm=len(m['imm']['missing']), extra_calls=len(m['call']['extra']),
                extra_addrs=len(m['addr']['extra']), extra_imm=len(m['imm']['extra']),
                unbound=len(m['call']['unbound']) + len(m['addr']['unbound']),
                missing=calls + addrs + switch)


def describe(x: dict, names: dict[int, str]) -> str:
    if x.get('va') is not None:
        label = names.get(x['va'])
        s = '0x%08x' % x['va'] + (' ' + label if label else '')
        if x.get('alt'):
            s += ' ' + x['alt'][:48]
        return s
    if x.get('alt'):
        return x['alt'][:60]
    return x.get('name') or '?'


def find_candidate(objs, by_va, errors, r, pe, rt, size):
    """(source, object, symbol) of the row's candidate with the fewest differing instructions."""
    from mcm2tool.coff import CoffObject
    from tools.frame_layout import disassemble
    retail = disassemble(pe.bytes_at_va(r['va'], size), r['va'])
    best = None
    for cpp, sym, exact in by_va.get(r['va'], []):
        cpp = cpp.resolve()
        if errors.get(cpp):
            continue
        try:
            o = CoffObject(objs[cpp])
        except Exception:
            continue
        if sym is None:
            if best is not None:
                continue
            pool = []
            for oo, s in function_symbols(objs[cpp]):
                try:
                    insns, n = symbol_insns(oo, s)
                except Exception:
                    continue
                if abs(n - size) <= max(16, size // 4):
                    pool.append((compare(insns, retail)['diffs'], s.name))
            if not pool:
                continue
            diffs, name = min(pool)
            s = next(x for x in o.symbols if x.name == name and x.section_number > 0)
        else:
            ss = [x for x in o.symbols if x.section_number > 0 and x.name == sym]
            if not ss and not exact and sym:
                ss = [x for x in o.symbols if x.section_number > 0 and sym in x.name and not x.name.startswith('$')]
            if not ss:
                continue
            s = ss[0]
            try:
                insns, _ = symbol_insns(o, s)
            except Exception:
                continue
            diffs = compare(insns, retail)['diffs']
        if best is None or diffs < best[0]:
            best = (diffs, cpp, o, s)
    return best


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE', 'work/game/mcm2.exe'))
    ap.add_argument('--index', default='docs/NEAR_MISS_INDEX.md')
    ap.add_argument('--manifest', default='analysis/function_manifest.json')
    ap.add_argument('--work', default='work/reference_diff')
    ap.add_argument('--jobs', type=int, default=8)
    ap.add_argument('--lane')
    ap.add_argument('--va', action='append', default=[])
    ap.add_argument('--json')
    ap.add_argument('--md')
    ap.add_argument('--reuse', action='store_true', help='reuse objects already in --work')
    a = ap.parse_args(argv)
    if not a.vc6_root and not a.reuse:
        ap.error('--vc6-root (or VC6_ROOT) is required')
    from mcm2tool.pe import PEImage

    rows = parse_index((ROOT / a.index).read_text(encoding='utf-8'))
    if a.lane:
        rows = [r for r in rows if lane_for(r['tu']) == a.lane]
    if a.va:
        want = {int(v, 16) for v in a.va}
        rows = [r for r in rows if r['va'] in want]
    by_va, sizes = candidates(ROOT)
    manifest = {}
    mpath = ROOT / a.manifest
    if mpath.exists():
        for f in json.loads(mpath.read_text(encoding='utf-8')):
            manifest[int(f['target_va'], 16)] = f['target_size']
    work = ROOT / a.work
    work.mkdir(parents=True, exist_ok=True)
    cpps = sorted({c[0].resolve() for r in rows for c in by_va.get(r['va'], [])})
    objs = {cpp: work / (cpp.relative_to(ROOT).with_suffix('').as_posix().replace('/', '__') + '.obj') for cpp in cpps}
    if a.reuse:
        errors = {c: (None if objs[c].exists() else 'no object') for c in cpps}
    else:
        with ThreadPoolExecutor(max_workers=a.jobs) as pool:
            errors = dict(zip(cpps, pool.map(lambda c: compile_one(ROOT, a.vc6_root, c, objs[c]), cpps)))
    pe = PEImage(str(ROOT / a.exe) if not os.path.isabs(a.exe) else a.exe)
    rt = Retail(pe)
    starts = sorted(set(by_va) | set(sizes) | set(manifest) | {r['va'] for r in rows})
    glob = load_bindings(ROOT)
    names: dict[int, str] = {}
    for k, v in sorted(glob.items(), key=lambda kv: len(kv[0])):
        names.setdefault(v, k)
    out = []
    for r in rows:
        r['lane'] = lane_for(r['tu'])
        size = sizes.get(r['va']) or manifest.get(r['va']) or retail_extent(pe, r['va'], starts)
        k = bisect.bisect_right(starts, r['va'])
        internal_end = min(starts[k] if k < len(starts) else r['va'] + 0x4000, r['va'] + 0x4000)
        internal_end = max(internal_end, r['va'] + size)
        best = find_candidate(objs, by_va, errors, r, pe, rt, size)
        if best is None:
            r['kind'] = 'unmeasured'
            out.append(r)
            continue
        diffs, cpp, o, s = best
        own = own_bindings(cpp)
        resolve = lambda n: own.get(n, glob.get(n))  # noqa: E731
        rr = retail_refs(rt, r['va'], size, internal_end)
        cr = candidate_refs(o, s, resolve, rt)
        m = match_refs(rr, cr)
        r.update(source=cpp.relative_to(ROOT).as_posix(), symbol=s.name, diffs=diffs, retail_size=size,
                 **score(m))
        r['detail'] = {
            kind: dict(missing=[describe(x, names) for x in m[kind]['missing']],
                       extra=[describe(x, names) for x in m[kind]['extra']],
                       unbound=[describe(x, names) for x in m[kind]['unbound']]) for kind in ('call', 'addr')}
        r['detail']['imm'] = dict(missing=['0x%x' % v for v in m['imm']['missing']],
                                  extra=['0x%x' % v for v in m['imm']['extra']])
        r['detail']['switch'] = m['switch']
        out.append(r)
    if a.json:
        Path(a.json).write_text(json.dumps(out, indent=1), encoding='utf-8')
    md = render_md(out)
    if a.md:
        Path(a.md).write_text(md, encoding='utf-8')
    else:
        print(md)
    return 0


def sort_key(r: dict):
    return (r.get('missing') is None, -(r.get('missing') or 0), -(r.get('missing_imm') or 0), r['va'])


def render_md(rows: list[dict]) -> str:
    lines = ['# Near misses ranked by references retail has and the candidate lacks', '',
             'tools/reference_diff.py. calls/addrs are net of unbound candidate references of the same kind;',
             'switch counts missing table entries, distinct targets and tables (x4).', '']
    for ln in sorted({r['lane'] for r in rows}):
        lr = sorted((r for r in rows if r['lane'] == ln), key=sort_key)
        lines += ['## %s' % ln, '',
                  '| TU | VA | missing | calls | addrs | switch | imm -/+ | extra calls/addrs | unbound | detail |',
                  '|---|---|---|---|---|---|---|---|---|---|']
        for r in lr:
            if r.get('kind') == 'unmeasured':
                lines.append('| %s | `0x%08x` | unmeasured | | | | | | | |' % (r['tu'], r['va']))
                continue
            d = r['detail']
            bits = []
            for kind in ('call', 'addr'):
                if d[kind]['missing']:
                    bits.append('-%s: %s' % (kind, ', '.join(d[kind]['missing'][:4])))
                if d[kind]['extra']:
                    bits.append('+%s: %s' % (kind, ', '.join(d[kind]['extra'][:4])))
            if d['imm']['missing']:
                bits.append('-imm: ' + ' '.join(d['imm']['missing'][:6]))
            if d['imm']['extra']:
                bits.append('+imm: ' + ' '.join(d['imm']['extra'][:6]))
            sw = d['switch']
            if sw['retail'] != sw['cand'] or sw['retail_byte_tables'] != sw['cand_byte_tables']:
                bits.append('switch r%s/%d c%s/%d' % (sw['retail'], sw['retail_byte_tables'], sw['cand'],
                                                      sw['cand_byte_tables']))
            lines.append('| %s | `0x%08x` | %d | %d | %d | %d | %d/%d | %d/%d | %d | %s |' % (
                r['tu'], r['va'], r['missing'], r['missing_calls'], r['missing_addrs'], r['missing_switch'],
                r['missing_imm'], r['extra_imm'], r['extra_calls'], r['extra_addrs'], r['unbound'],
                '; '.join(bits).replace('|', '\\|')))
        lines.append('')
    return '\n'.join(lines)


if __name__ == '__main__':
    sys.exit(main())
