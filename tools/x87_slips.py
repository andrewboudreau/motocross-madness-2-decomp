#!/usr/bin/env python3
"""List the x87 operand-form slips of a near miss and the rule that flips each.

Two forms combine a reused stack value with a memory word: the dup form
``fld st(0) ; fop m`` and the load form ``fld m ; fop st(1)``. Two memory
words are combined as ``fld a ; fop b`` or the reverse. The rules that pick
these forms are in docs/VC6_PATTERN_CORPUS.md section 6. This tool aligns
candidate and retail like tools/rank_near_misses.py and finds every hunk
whose difference is one of these forms. For each one it reports:
- the source line, from the candidate's /Z7 COFF line numbers;
- whether each memory word is a frame word, an indirect word or a global;
- the named local behind each frame word, from CodeView;
- the candidate's direct stores to that word before the site (rule 6);
- the rule that predicts retail's form.

The stores are listed in code order, which can differ from source order
(rule 6 counts any earlier store in the source, also one on a path that
does not reach the use). The hint names a rule; it does not prove that
retail's source had that construct.

    python tools/x87_slips.py 0x52c510
    python tools/x87_slips.py --lane render
    python tools/x87_slips.py 0x4a1a50 --rank work/s14/rank.json --json
"""
from __future__ import annotations

import argparse
import difflib
import json
import os
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

ARITH = re.compile(r'^f(add|sub|subr|mul|div|divr)$')
COMMUTATIVE = ('fadd', 'fmul')
MEM = re.compile(r'(?:dword|qword) ptr (?:\w+:)?\[([^\]]*)\]')


def operand_kind(op: str) -> str | None:
    """'frame', 'indirect' or 'global' for a memory operand, None otherwise."""
    m = MEM.search(op)
    if not m:
        return None
    inner = m.group(1).replace(' ', '')
    regs = re.findall(r'e[a-ds][xip]|e[sd]i', inner)
    if not regs:
        return 'global'
    if regs[0] in ('esp', 'ebp') and len(regs) == 1:
        return 'frame'
    return 'indirect'


def forms(lines: list[str]) -> list[tuple[str, str, str, int]]:
    """(form, op mnemonic, memory operand, index) of each two-instruction x87 pair."""
    out = []
    for k in range(len(lines) - 1):
        m, _, o = lines[k].partition(' ')
        n, _, p = lines[k + 1].partition(' ')
        if m != 'fld' or not ARITH.match(n):
            continue
        if o == 'st(0)' and MEM.search(p):
            out.append(('dup', n, p, k))
        elif MEM.search(o) and p in ('st(1)', 'st(0), st(1)'):
            out.append(('load', n, o, k))
        elif MEM.search(o) and MEM.search(p):
            out.append(('memmem', n, o + ' | ' + p, k))
    return out


def classify(a: list[str], b: list[str]) -> list[dict]:
    """The x87 operand-form slips in one hunk (a: candidate lines, b: retail lines)."""
    fa, fb = forms(a), forms(b)
    out = []
    used = set()
    for x in fa:
        for j, y in enumerate(fb):
            if j in used or x[1] != y[1]:
                continue
            if x[0] in ('dup', 'load') and y[0] in ('dup', 'load') and x[0] != y[0] and x[2] == y[2]:
                out.append(dict(kind=f'ours {x[0]}, retail {y[0]}', op=x[1], mem=[x[2]], at=x[3]))
                used.add(j)
                break
            if x[0] == y[0] == 'memmem':
                l1, r1 = x[2].split(' | ')
                l2, r2 = y[2].split(' | ')
                if (l1, r1) == (r2, l2):
                    out.append(dict(kind='memory operands swapped', op=x[1], mem=[l1, r1],
                                    retail_first=l2, at=x[3]))
                    used.add(j)
                    break
    return out


def hint(slip: dict, kinds: list[str], stores: list[list]) -> str:
    """The rule (VC6_PATTERN_CORPUS section 6) that predicts retail's form."""
    k = slip['kind']
    if k == 'ours dup, retail load':
        if kinds[0] != 'frame':
            return ('outside the rules: retail loads a non-frame word with the stack value as '
                    'operand (4 of 116 exact sites); check the operand kind')
        if stores[0]:
            return ('rule 6: retail has no direct store to this word before the use; ours has '
                    'one (listed). Store it through a pointer or after the use')
        return ('rules 3-5: retail\'s stack value is a named local used again; ours is an '
                'unnamed or assignment-expression value (rules 4, 5). Name it in a local')
    if k == 'ours load, retail dup':
        if kinds[0] != 'frame':
            return 'rule 2 expects dup for a non-frame word; check the operand kind'
        return ('rule 6, 4 or 5: retail stored this word directly before the use (even on '
                'another path), or its stack value is an unnamed repeated expression, or '
                'this is the first use of `x *= (s = e)`')
    if k == 'memory operands swapped':
        if kinds == ['frame', 'frame']:
            if stores[0] and not stores[1]:
                return ('two frame words, ours stores the first earlier: the unstored word '
                        'loads first; retail had the store on the other word or on neither')
            if stores[1] and not stores[0]:
                return ('two frame words, ours stores the second earlier: retail had the store '
                        'on the other word or on neither')
            if not stores[0] and not stores[1]:
                return ('two never-stored frame words: the written right operand loads first; '
                        'swap the written order (or retail stored one of them earlier)')
            return 'two stored frame words: canonical order, written order inert'
        if 'frame' in kinds:
            return ('frame against non-frame: the frame word loads first in either written '
                    'order; retail\'s word is not a frame word here, or the other is')
        base = [re.findall(r'\[(e\w\w)', m) for m in slip['mem']]
        disp = [int((re.findall(r'([+-] 0x[0-9a-f]+|[+-] \d+)\]', m) or ['+ 0'])[0].replace(' ', ''), 0)
                for m in slip['mem']]
        if base[0] and base[0] == base[1]:
            if disp[0] < disp[1]:
                return ('two words on one base register, ours loads the smaller offset first: '
                        'outside the probed rule (larger offset first)')
            return ('two words on one base register: VC6 loads the larger offset first, so '
                    'in retail the two words were reached through different pointers (or one is '
                    'a by-value copy)')
        return ('two non-frame words on different bases: canonical order, written order '
                'inert (no source rule known)')
    return ''


def line_table(obj_path: Path) -> dict:
    """Section -> [(offset, line)] from /Z7 COFF line numbers."""
    d = obj_path.read_bytes()
    _, ns, _, sp, nsym, _, _ = struct.unpack_from('<HHIIIHH', d, 0)
    syms = {}
    i = 0
    while i < nsym:
        o = sp + 18 * i
        nm = d[o:o + 8]
        if nm[:4] == b'\0\0\0\0':
            name = d[sp + 18 * nsym + struct.unpack_from('<I', nm, 4)[0]:].split(b'\0')[0]
        else:
            name = nm.rstrip(b'\0')
        _, sec, _, sc, naux = struct.unpack_from('<IhHBB', d, o + 8)
        syms[i] = (name.decode('latin-1'), sec, sc, d[o + 18:o + 18 + 18 * naux])
        i += 1 + naux
    base = {}
    for k, (n, sec, sc, aux) in syms.items():
        if sec > 0 and len(aux) >= 4 and sc in (2, 3) and not n.startswith('.'):
            t = struct.unpack_from('<I', aux, 0)[0]
            if t in syms and syms[t][0] == '.bf':
                base[k] = struct.unpack_from('<H', syms[t][3], 4)[0]
    secs = {}
    for s in range(ns):
        o = 20 + 40 * s
        lp = struct.unpack_from('<I', d, o + 28)[0]
        nl = struct.unpack_from('<H', d, o + 34)[0]
        ent, fn = [], None
        for j in range(nl):
            a, ln = struct.unpack_from('<IH', d, lp + 6 * j)
            if ln == 0:
                fn = a
                continue
            ent.append((a, base.get(fn, 1) + ln - 1))
        secs[s + 1] = ent
    return secs


def source_line(entries: list, off: int) -> int | None:
    before = [(a, ln) for a, ln in entries if a <= off]
    return max(before)[1] if before else None


def analyse(row: dict, obj: Path, pe) -> dict:
    import tools.rank_near_misses as R
    from mcm2tool.coff import CoffObject
    from tools.frame_layout import disassemble, track_frame, load_codeview, local_ranges, locate, _is_write
    cand = R.candidate_insns(obj, row['symbol'], True) or R.candidate_insns(obj, row['symbol'], False)
    if cand is None:
        return dict(va=hex(row['va']), error='no symbol')
    cand = cand[0]
    retail = disassemble(pe.bytes_at_va(row['va'], row['retail_size']), row['va'])
    a, b = R.norm(cand), R.norm(retail)
    o = CoffObject(obj)
    sym = [s for s in o.symbols if s.section_number > 0 and s.name == row['symbol']] or \
          [s for s in o.symbols if s.section_number > 0 and row['symbol'] in s.name and not s.name.startswith('$')]
    lines = line_table(obj).get(sym[0].section_number, []) if sym else []
    info = track_frame(cand)
    acc = {x.index: x for x in info.accesses}
    names = []
    try:
        m = re.match(r'\?(\w+)@(\w+)@', row['symbol'])
        cv = f'{m.group(2)}::{m.group(1)}' if m else re.sub(r'^\?|@@.*$|@.*$', '', row['symbol'])
        proc, _ = load_codeview(o, cv)
        names = local_ranges(proc, info)
    except SystemExit:
        pass
    src = (ROOT / row['source']).read_text(encoding='latin-1').splitlines()

    def frame_word(idx: int):
        x = acc.get(idx)
        if x is None:
            return None, None
        loc = locate(names, x.offset) if names else None
        name = loc['name'] + ('+%d' % (x.offset - loc['offset']) if x.offset != loc['offset'] else '') if loc else None
        return x.offset, name

    def stores_before(idx: int, off: int, width: int):
        out = []
        for x in info.accesses:
            if x.index >= idx:
                break
            if x.lea or not _is_write(x.text) or not x.width:
                continue
            if x.offset < off + width and off < x.offset + x.width:
                ln = source_line(lines, cand[x.index].address)
                out.append([hex(cand[x.index].address - cand[0].address), x.text, ln])
        return out

    slips = []
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
        if tag == 'equal':
            continue
        lo, hi = max(0, i1 - 1), min(len(a), i2 + 1)
        for s in classify(a[lo:hi], b[max(0, j1 - 1):min(len(b), j2 + 1)]):
            k = lo + s['at']
            idxs = [k + 1] if s['kind'] != 'memory operands swapped' else [k, k + 1]
            if s['kind'] == 'ours load, retail dup':
                idxs = [k]
            kinds = [operand_kind(cand[i].op_str) for i in idxs]
            words, sts = [], []
            for i in idxs:
                off, name = frame_word(i)
                words.append(name or (('frame+0x%x' % off) if off is not None else None))
                sts.append(stores_before(i, off, 4) if off is not None else [])
            ln = source_line(lines, cand[k].address)
            slips.append(dict(kind=s['kind'], op=s['op'], at=hex(cand[k].address - cand[0].address),
                              retail=hex(retail[j1].address) if j1 < len(retail) else None,
                              operands=[cand[i].op_str.split(', ')[-1] for i in idxs], kinds=kinds,
                              words=words, stores=sts, line=ln,
                              text=src[ln - 1].strip() if ln and ln <= len(src) else None,
                              rule=hint(s, kinds, sts)))
    return dict(va=hex(row['va']), lane=row.get('lane'), symbol=row['symbol'], diffs=row.get('diffs'),
                source=row['source'], slips=slips)


def row_obj(row: dict, out: Path) -> Path:
    import tools.rank_near_misses as R
    obj = out / (Path(row['source']).with_suffix('').as_posix().replace('/', '__') + '.obj')
    if not obj.exists():
        err = R.compile_one(ROOT, os.environ['VC6_ROOT'], ROOT / row['source'], obj)
        if err:
            raise RuntimeError(err)
    return obj


def print_report(rep: dict) -> None:
    if not rep.get('slips'):
        return
    print(f"{rep['va']} {rep['lane']} diffs {rep['diffs']} {rep['symbol']} ({rep['source']})")
    for s in rep['slips']:
        print(f"  +{s['at']} (retail {s['retail']}) {s['kind']} {s['op']} "
              + ', '.join(f"{o} [{k}{' ' + w if w else ''}]" for o, k, w in zip(s['operands'], s['kinds'], s['words'])))
        if s['line']:
            print(f"    line {s['line']}: {s['text']}")
        for w, st in zip(s['words'], s['stores']):
            for at, text, ln in st:
                print(f"    earlier store to {w}: +{at} {text} (line {ln})")
        print(f"    {s['rule']}")


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('va', nargs='?')
    ap.add_argument('--rank', default=str(ROOT / 'work/s14/rank.json'),
                    help='rank_near_misses JSON (source, symbol and retail size of each row)')
    ap.add_argument('--lane', help='every row of this lane')
    ap.add_argument('--out', default=str(ROOT / 'work/x87_slips'), help='object directory')
    ap.add_argument('--fresh', action='store_true', help='recompile even when an object exists')
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE', str(ROOT / 'work/game/mcm2.exe')))
    ap.add_argument('--json', action='store_true')
    a = ap.parse_args(argv)
    from mcm2tool.pe import PEImage
    rows = json.load(open(a.rank))
    if a.va:
        rows = [r for r in rows if r['va'] == int(a.va, 16)]
    elif a.lane:
        rows = [r for r in rows if r.get('lane') == a.lane and r.get('diffs')]
    else:
        ap.error('give a VA or --lane')
    out = Path(a.out)
    out.mkdir(parents=True, exist_ok=True)
    pe = PEImage(a.exe)
    reps = []
    fresh = set()
    for r in rows:
        obj = out / (Path(r['source']).with_suffix('').as_posix().replace('/', '__') + '.obj')
        if a.fresh and obj not in fresh and obj.exists():
            obj.unlink()
        fresh.add(obj)
        try:
            rep = analyse(r, row_obj(r, out), pe)
        except RuntimeError as e:
            rep = dict(va=hex(r['va']), error=str(e))
        reps.append(rep)
        if not a.json:
            print_report(rep)
    if a.json:
        print(json.dumps(reps, indent=1))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
