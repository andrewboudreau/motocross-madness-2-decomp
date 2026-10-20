"""Rank the near misses of docs/NEAR_MISS_INDEX.md by how much code really differs.

usage:
  PYTHONPATH=. python tools/rank_near_misses.py --vc6-root DIR [--exe mcm2.exe]
         [--jobs 8] [--json OUT] [--md OUT] [--lane LANE] [--va 0xVA ...]

The index's score is positional (strict bytes at equal offsets), so one inserted
instruction early in a function costs every later byte, and its "what differs"
column names only the first difference. This tool recompiles each row's
candidate(s) with the calibrated profile (vc6_o2_mt) and compares the
instruction streams with an aligned diff instead:

- Rows are the index's `| TU | `0xVA` | score | class |` lines. A row's
  candidates are every `.cpp` whose `*.bindings.json` binds a symbol to the VA,
  plus every `targets.json` entry (under src/ and samples/) with that
  `target_va` and a `source`. A `.cpp` with only a `// 0xVA` marker line is
  compared through the object's closest function of similar size (reported
  with `by_marker`). When several candidates exist, the one with the fewest
  differing instructions is reported.
- The retail extent comes from the targets.json entry, else from
  analysis/function_manifest.json when present, else it runs to the next
  known function start (any bound or targeted VA), cut at a jump table the
  function indexes and stripped of trailing nop/int3 padding.
- Relocated fields are masked on both sides (the candidate's relocation bytes
  are replaced by a sentinel address; any 6+ hex-digit value is an address),
  direct call and jump targets are masked, indirect calls keep their slot.
  A candidate's own jump table (located through the relocation inside its
  `jmp [reg*4 + table]`) is cut off before disassembly.

Per row the report gives the differing instructions and hunks of the aligned
diff (frame displacements and registers kept), the same count with frame
displacements masked, with registers masked, and with both, the mnemonic-only
count, candidate and retail sizes, instruction counts and frame sizes, and a
kind: identical (equal once relocations are masked), frame-only, reg-only,
frame+reg, or structural. "register only" means equal once general register
names are masked; this is a proxy for a renaming, not a proof of one.

The lane is read from the TU's directory per the session 14 brief
(work/assignments/S14.md); --lane filters on it.
"""
from __future__ import annotations

import argparse
import difflib
import json
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INDEX_ROW = re.compile(r'^\| ([^|]+?) \| `0x([0-9a-fA-F]+)` \| ([^|]+?) \| ([abc]) \|')
REG = re.compile(r'\b(e?[abcd]x|[abcd][lh]|e?si|e?di|e?bp|sil|dil)\b')
FRAME = re.compile(r'\b(esp|ebp) ([+-]) (0x[0-9a-f]+|\d+)')
ADDR = re.compile(r'\b0x[0-9a-f]{6,}\b')
SENTINEL = 0x00600000
ABSOLUTE_EXTERNALS = {'__except_list'}   # undefined in the object, linked as absolute 0
JUMP_TABLE = re.compile(r'\[e[a-z]{2}\*4 \+ (0x[0-9a-f]+)\]')
MARKER = re.compile(r'^// 0x([0-9a-fA-F]{8})\s*$', re.M)   # a `// 0xVA` line above a definition

RIDER_PHYSICS = {'bikeai', 'krustybike', 'tire', 'motion', 'contact', 'constraint', 'effects', 'suspension'}
RENDER = {'render', 'camera', 'ecosystem'}
SHELL = {'race', 'ui', 'track', 'game', 'net', 'io', 'inputdevice', 'audio', 'control', 'gameui'}


def parse_index(text: str) -> list[dict]:
    rows = []
    for line in text.splitlines():
        m = INDEX_ROW.match(line)
        if m:
            rows.append(dict(tu=m.group(1).strip(), va=int(m.group(2), 16),
                             score=m.group(3).strip(), index_class=m.group(4)))
    return rows


def lane_for(tu: str) -> str:
    parts = tu.replace('\\', '/').split('/')
    top = parts[0]
    if top in RENDER:
        return 'render'
    if top in SHELL:
        return 'shell'
    if top == 'krusty2':
        return 'rider' if parts[-1] == 'Bike' else 'body'
    if top == 'physics':
        return 'rider' if len(parts) > 1 and parts[1] in RIDER_PHYSICS else 'body'
    return '?'


def norm(insns, mask_frame: bool = False, mask_reg: bool = False, mnemonic_only: bool = False) -> list[str]:
    out = []
    for x in insns:
        if mnemonic_only:
            out.append(x.mnemonic)
            continue
        op = x.op_str
        if (x.mnemonic.startswith('j') or x.mnemonic.startswith('loop') or x.mnemonic == 'call') \
                and re.match(r'^0x[0-9a-f]+$', op):
            op = 'X'
        op = ADDR.sub('G', op)
        if mask_frame:
            op = FRAME.sub(lambda m: m.group(1) + ' + S', op)
        if mask_reg:
            op = REG.sub('R', op)
        out.append(x.mnemonic + ' ' + op)
    return out


def diff_counts(a: list[str], b: list[str]) -> tuple[int, int]:
    """(differing instructions, hunks) of an aligned diff; a replace of m by n counts max(m, n)."""
    if a == b:
        return 0, 0
    sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
    diffs = hunks = 0
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag != 'equal':
            hunks += 1
            diffs += max(i2 - i1, j2 - j1)
    return diffs, hunks


def frame_size(insns) -> int:
    """The prologue's `sub esp, N` (`push ecx` alone counts 4); 0 when there is none."""
    for k, x in enumerate(insns[:24]):
        if x.mnemonic == 'sub' and x.op_str.startswith('esp, '):
            return int(x.op_str[5:], 0)
        if x.mnemonic in ('ret', 'call') or x.mnemonic.startswith('j'):
            break
    if insns and insns[0].mnemonic == 'push' and insns[0].op_str == 'ecx':
        return 4
    return 0


def compare(cand, retail) -> dict:
    full = diff_counts(norm(cand), norm(retail))
    frame = diff_counts(norm(cand, mask_frame=True), norm(retail, mask_frame=True))
    reg = diff_counts(norm(cand, mask_reg=True), norm(retail, mask_reg=True))
    both = diff_counts(norm(cand, True, True), norm(retail, True, True))
    mnem = diff_counts(norm(cand, mnemonic_only=True), norm(retail, mnemonic_only=True))
    if full[0] == 0:
        kind = 'identical'
    elif frame[0] == 0:
        kind = 'frame-only'
    elif reg[0] == 0:
        kind = 'reg-only'
    elif both[0] == 0:
        kind = 'frame+reg'
    else:
        kind = 'structural'
    return dict(diffs=full[0], hunks=full[1], diffs_frame_masked=frame[0], diffs_reg_masked=reg[0],
                diffs_frame_reg_masked=both[0], mnemonic_diffs=mnem[0], kind=kind,
                cand_insns=len(cand), retail_insns=len(retail),
                cand_frame=frame_size(cand), retail_frame=frame_size(retail))


def candidates(root: Path) -> tuple[dict, dict]:
    """VA -> [(cpp, symbol, exact_name)] and VA -> retail size from targets.json."""
    by_va: dict[int, list] = {}
    sizes: dict[int, int] = {}
    for b in sorted(list((root / 'src').rglob('*.bindings.json')) + list((root / 'samples').rglob('*.bindings.json'))):
        cpp = b.with_name(b.name[:-len('.bindings.json')] + '.cpp')
        if not cpp.exists():
            continue
        try:
            d = json.loads(b.read_text(encoding='utf-8'))
        except ValueError:
            continue
        if not isinstance(d, dict):
            continue
        for sym, va in d.items():
            if isinstance(va, str) and sym.startswith('?') and '$' not in sym:
                by_va.setdefault(int(va, 16), []).append((cpp, sym, True))
    for t in sorted(list((root / 'src').rglob('targets.json')) + list((root / 'samples').rglob('targets.json'))):
        try:
            d = json.loads(t.read_text(encoding='utf-8'))
        except ValueError:
            continue
        for e in d if isinstance(d, list) else []:
            if not isinstance(e, dict) or 'target_va' not in e:
                continue
            va = int(e['target_va'], 16)
            if e.get('target_size'):
                sizes.setdefault(va, int(e['target_size']))
            if e.get('source') and e.get('candidate_symbol_contains'):
                cpp = t.parent / e['source']
                if cpp.exists():
                    by_va.setdefault(va, []).append((cpp, e['candidate_symbol_contains'], False))
    for cpp in sorted(list((root / 'src').rglob('*.cpp')) + list((root / 'samples').rglob('*.cpp'))):
        try:
            text = cpp.read_text(encoding='latin-1')
        except OSError:
            continue
        for m in MARKER.finditer(text):
            va = int(m.group(1), 16)
            if not any(c[0].resolve() == cpp.resolve() and c[1] for c in by_va.get(va, [])):
                by_va.setdefault(va, []).append((cpp, None, False))
    for va, lst in by_va.items():
        seen, uniq = set(), []
        for c in lst:
            key = (c[0].resolve(), c[1])
            if key not in seen:
                seen.add(key)
                uniq.append(c)
        by_va[va] = uniq
    return by_va, sizes


def retail_extent(pe, va: int, starts: list[int], limit: int = 0x4000) -> int:
    """Bytes from va to the next known start, less jump tables and nop/int3 padding."""
    import bisect
    from tools.frame_layout import disassemble
    k = bisect.bisect_right(starts, va)
    end = starts[k] if k < len(starts) else va + limit
    end = min(end, va + limit)
    insns = disassemble(pe.bytes_at_va(va, end - va), va)
    for x in insns:
        m = JUMP_TABLE.search(x.op_str)
        if x.mnemonic == 'jmp' and m:
            t = int(m.group(1), 16)
            if va < t < end:
                end = t
    insns = [x for x in insns if x.address + x.size <= end]
    data = pe.bytes_at_va(va, end - va)
    for x in insns:                     # a ret padded with nop/int3 to the next 16-byte boundary
        if x.mnemonic not in ('ret', 'jmp'):
            continue
        after = x.address + x.size
        pad = (-after) % 16
        if pad and after + pad <= end and all(b in (0x90, 0xcc) for b in data[after - va:after - va + pad]):
            end = after
            break
    insns = [x for x in insns if x.address + x.size <= end]
    while insns and (insns[-1].mnemonic in ('nop', 'int3') or insns[-1].op_str == 'esi, [esi]' and insns[-1].mnemonic == 'lea'):
        insns.pop()
    return (insns[-1].address + insns[-1].size - va) if insns else end - va


def compile_one(root: Path, vc6_root: str, cpp: Path, obj: Path) -> str | None:
    cmd = [sys.executable, str(root / 'tools/compile.py'), '--compiler', 'vc6', '--vc6-root', vc6_root,
           '--profile', 'vc6_o2_mt', str(cpp), '-o', str(obj)]
    rel = cpp.resolve().relative_to(root).as_posix()
    if 'krusty2' in rel or rel.startswith('samples/physics/'):
        cmd += ['--include', str(root / 'src/krusty2')]
    env = dict(os.environ, PYTHONPATH=str(root))
    p = subprocess.run(cmd, capture_output=True, text=True, encoding='latin-1', env=env, cwd=root)
    if p.returncode or not obj.exists():
        lines = [l for l in (p.stdout + p.stderr).splitlines() if 'error' in l.lower()]
        return (lines[0] if lines else 'compile failed')[:200]
    return None


def candidate_insns(obj_path: Path, symbol: str, exact: bool):
    from mcm2tool.coff import CoffObject
    from tools.frame_layout import disassemble
    o = CoffObject(obj_path)
    syms = [s for s in o.symbols if s.section_number > 0 and s.name == symbol]
    if not syms and not exact and symbol:
        syms = [s for s in o.symbols if s.section_number > 0 and symbol in s.name and not s.name.startswith('$')]
    if not syms:
        return None
    return symbol_insns(o, syms[0])


def function_symbols(obj_path: Path) -> list:
    from mcm2tool.coff import CoffObject
    o = CoffObject(obj_path)
    out = []
    for s in o.symbols:
        if s.section_number <= 0 or s.storage_class not in (2, 3) or s.name.startswith('$') or '$' in s.name:
            continue
        if not o.section(s.section_number).characteristics & 0x20:    # code
            continue
        out.append((o, s))
    return out


def symbol_insns(o, sym):
    from tools.frame_layout import disassemble
    raw, _, rel = o.symbol_extent(sym)
    base = sym.value
    jumps = [x for x in disassemble(raw, base) if x.mnemonic == 'jmp' and '*4' in x.op_str]
    cut = len(raw)
    for r in rel:                       # cut the candidate's own jump tables, as for retail
        target = o.symbol_by_index.get(r.symbol_index)
        if target is None or target.section_number != sym.section_number:
            continue
        if any(x.address <= r.virtual_address < x.address + x.size for x in jumps):
            off = target.value + int.from_bytes(raw[r.virtual_address - base:r.virtual_address - base + 4], 'little')
            if base < off < base + cut:
                cut = off - base
    raw = raw[:cut]
    rel = [r for r in rel if r.virtual_address - base < cut]
    code = bytearray(raw)
    for r in rel:
        off = r.virtual_address - base
        target = o.symbol_by_index.get(r.symbol_index)
        if target is not None and (target.section_number == -1 or target.name in ABSOLUTE_EXTERNALS):
            continue                    # absolute symbols, e.g. fs:[0] through __except_list
        if r.type in (0x0006, 0x0007) and 0 <= off <= len(code) - 4:      # DIR32 / DIR32NB
            code[off:off + 4] = SENTINEL.to_bytes(4, 'little')
    return disassemble(bytes(code), base), len(raw)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE', 'work/game/mcm2.exe'))
    ap.add_argument('--index', default='docs/NEAR_MISS_INDEX.md')
    ap.add_argument('--manifest', default='analysis/function_manifest.json')
    ap.add_argument('--work', default='work/rank_near_misses')
    ap.add_argument('--jobs', type=int, default=8)
    ap.add_argument('--lane')
    ap.add_argument('--va', action='append', default=[])
    ap.add_argument('--json')
    ap.add_argument('--md')
    ap.add_argument('--top', type=int, default=15, help='rows per lane in the summary')
    a = ap.parse_args(argv)
    if not a.vc6_root:
        ap.error('--vc6-root (or VC6_ROOT) is required')
    sys.path.insert(0, str(ROOT))
    from mcm2tool.pe import PEImage
    from tools.frame_layout import disassemble

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
    with ThreadPoolExecutor(max_workers=a.jobs) as pool:
        errors = dict(zip(cpps, pool.map(lambda c: compile_one(ROOT, a.vc6_root, c, objs[c]), cpps)))
    pe = PEImage(str(ROOT / a.exe) if not os.path.isabs(a.exe) else a.exe)
    starts = sorted(set(by_va) | set(sizes) | set(manifest) | {r['va'] for r in rows})
    out = []
    for r in rows:
        r['lane'] = lane_for(r['tu'])
        size = sizes.get(r['va']) or manifest.get(r['va']) or retail_extent(pe, r['va'], starts)
        r['retail_size'] = size
        best, tried = None, []
        for cpp, sym, exact in by_va.get(r['va'], []):
            cpp = cpp.resolve()
            src = cpp.relative_to(ROOT).as_posix()
            if errors.get(cpp):
                tried.append(dict(source=src, symbol=sym, error=errors[cpp]))
                continue
            retail = disassemble(pe.bytes_at_va(r['va'], size), r['va'])
            if sym is None:
                # `// 0xVA` marker without a binding: the object's closest function
                if best is not None:
                    continue
                pool = []
                for o, s in function_symbols(objs[cpp]):
                    try:
                        pool.append((s.name, symbol_insns(o, s)))
                    except Exception:
                        continue
                pool = [p for p in pool if abs(p[1][1] - size) <= max(16, size // 4)]
                if not pool:
                    tried.append(dict(source=src, symbol=None, error='no marker candidate'))
                    continue
                scored = [(compare(c, retail), name, c_size) for name, (c, c_size) in pool]
                cm, name, cand_size = min(scored, key=lambda t: t[0]['diffs'])
                m = dict(source=src, symbol=name, cand_size=cand_size, by_marker=True, **cm)
            else:
                got = candidate_insns(objs[cpp], sym, exact)
                if got is None:
                    tried.append(dict(source=src, symbol=sym, error='symbol not in object'))
                    continue
                cand, cand_size = got
                m = dict(source=src, symbol=sym, cand_size=cand_size, **compare(cand, retail))
            tried.append(dict(source=src, symbol=m['symbol'], diffs=m['diffs']))
            if best is None or m['diffs'] < best['diffs']:
                best = m
        if best:
            r.update(best)
        else:
            r['kind'] = 'unmeasured'
        r['candidates'] = tried
        out.append(r)
    if a.json:
        Path(a.json).write_text(json.dumps(out, indent=1), encoding='utf-8')
    md = render_md(out, a.top)
    if a.md:
        Path(a.md).write_text(md, encoding='utf-8')
    else:
        print(md)
    return 0


REGISTER_KINDS = ('reg-only', 'frame+reg')


def sort_key(r: dict):
    return (r.get('diffs') is None, r.get('diffs') or 0, r.get('hunks') or 0, r['va'])


def render_md(rows: list[dict], top: int) -> str:
    lines = ['# Near misses ranked by differing instructions', '',
             'Aligned instruction diff against retail, relocations masked (tools/rank_near_misses.py).',
             'diffs/hunks keep frame displacements and registers; F, R and FR mask frame, registers, both;',
             'M compares mnemonics only. Frames are the prologue `sub esp`.', '']
    lanes = sorted({r['lane'] for r in rows})
    lines += ['| lane | rows | register-only | identical | unmeasured |', '|---|---|---|---|---|']
    for ln in lanes:
        lr = [r for r in rows if r['lane'] == ln]
        lines.append('| %s | %d | %d | %d | %d |' % (ln, len(lr), sum(r['kind'] in REGISTER_KINDS for r in lr),
                                                    sum(r['kind'] == 'identical' for r in lr),
                                                    sum(r['kind'] == 'unmeasured' for r in lr)))
    for ln in lanes:
        lr = sorted((r for r in rows if r['lane'] == ln), key=sort_key)
        lines += ['', '## %s' % ln, '',
                  '| rank | TU | VA | kind | diffs | hunks | F | R | FR | M | size c/r | insns c/r | frame c/r | index score |',
                  '|---|---|---|---|---|---|---|---|---|---|---|---|---|---|']
        k = 0
        for r in lr:
            if r['kind'] == 'unmeasured':
                lines.append('| - | %s | `0x%08x` | unmeasured | | | | | | | | | | %s |' % (r['tu'], r['va'], r['score']))
                continue
            k += 1
            rank = str(k) if r['kind'] not in REGISTER_KINDS else '(reg)'
            lines.append('| %s | %s | `0x%08x` | %s | %d | %d | %d | %d | %d | %d | %d/%d | %d/%d | 0x%x/0x%x | %s |' % (
                rank, r['tu'], r['va'], r['kind'], r['diffs'], r['hunks'], r['diffs_frame_masked'],
                r['diffs_reg_masked'], r['diffs_frame_reg_masked'], r['mnemonic_diffs'], r['cand_size'],
                r['retail_size'] or 0, r['cand_insns'], r['retail_insns'], r['cand_frame'], r['retail_frame'], r['score']))
    return '\n'.join(lines) + '\n'


if __name__ == '__main__':
    sys.exit(main())
