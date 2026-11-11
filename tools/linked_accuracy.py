#!/usr/bin/env python3
"""Score every bound function body in the source-built game against retail.

``tools/build_game.py`` links whichever provider defines a symbol: canonical
source, a ``candidate_sources`` sample, or a bring-up stand-in. Calibration and
the physics runner score the samples and canonical files they compile, but not
the body the game actually links. This tool reads the build's linker map,
names each linked function, looks up the retail VA that ``*.bindings.json``
files and calibration cases claim for that symbol (``load_bindings`` from the
vtable audit), and compares the two bodies instruction by instruction.

Scores are a difflib ratio over normalised instructions with call/jump targets,
absolute addresses, stack displacements and registers masked (``--strict``
keeps registers and frame offsets). The score measures code shape, so it is
evidence about accuracy, not a byte match: a 1.000 body can still differ in
relocated operands, and a low score can come from inlining or helper boundaries
rather than different behaviour, and a tiny body is usually a forwarder whose
implementation is linked under another name. A symbol bound to several retail
VAs is skipped (folded tiny bodies prove nothing about identity); when several
linked symbols claim one retail VA the best-scoring body is reported and the
rest are listed as ``other_bodies``. A forwarder (argument moves around a single
call or jmp) is followed to the body it calls, recorded as ``via``, unless the
retail body is forwarder-shaped too.

    python tools/linked_accuracy.py [--build work/game-build/build-XXXX] [--min-size 64]

Writes ``work/linked-accuracy/report.{json,md}`` (ignored), worst first by
``retail_bytes * (1 - score)``.
"""
from __future__ import annotations

import argparse
import ast
import difflib
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from mcm2tool.pe import PEImage  # noqa: E402
from tools.audit_game_vtables import DEFAULT_RETAIL, latest_build, load_bindings  # noqa: E402
from tools.frame_layout import disassemble  # noqa: E402
from tools.rank_near_misses import norm  # noqa: E402

MAP_FUNC = re.compile(r'^\s*0001:[0-9a-fA-F]{8}\s+(\S+)\s+([0-9a-fA-F]{8})\s+f\s+(?:i\s+)?(\S+)\s*$')
PAD = ('int3', 'nop')


def parse_functions(map_text: str) -> list[tuple[int, str, str]]:
    """(VA, symbol, object) for every code symbol in a VC6 map, sorted by address."""
    out = []
    for line in map_text.splitlines():
        m = MAP_FUNC.match(line)
        if m:
            out.append((int(m.group(2), 16), m.group(1), m.group(3)))
    return sorted(set(out))


def read_va(pe: PEImage, va: int, size: int) -> bytes:
    off = pe.va_to_offset(va)
    return pe.data[off:off + size]


def retail_sizes(root: Path = ROOT) -> dict[int, int]:
    """Retail VA -> size declared by calibration cases and targets.json entries.

    Reviewed extents come first: a disassembly walk runs past a tail ``jmp``.
    """
    out: dict[int, int] = {}
    calibration = root / 'tools/run_calibration.py'
    if calibration.is_file():
        for node in ast.walk(ast.parse(calibration.read_text(encoding='utf-8'))):
            if isinstance(node, ast.Dict):
                keys = [k.value for k in node.keys if isinstance(k, ast.Constant)]
                if 'target_va' in keys and 'target_size' in keys:
                    try:
                        case = ast.literal_eval(node)
                        out.setdefault(int(case['target_va'], 16), int(case['target_size']))
                    except (ValueError, TypeError):
                        pass
    for path in sorted((root / 'samples').rglob('targets.json')):
        try:
            data = json.loads(path.read_text(encoding='utf-8'))
        except ValueError:
            continue
        for t in data if isinstance(data, list) else []:
            if isinstance(t, dict) and t.get('target_va') and t.get('target_size'):
                out.setdefault(int(t['target_va'], 16), int(t['target_size']))
    return out


def target_symbols(funcs, bindings, root: Path = ROOT) -> dict[str, int]:
    """Linked symbol -> VA for targets.json entries named only by ``candidate_symbol_contains``.

    Used only when exactly one unbound linked symbol contains the substring.
    """
    out: dict[str, int] = {}
    for path in sorted((root / 'samples').rglob('targets.json')):
        try:
            data = json.loads(path.read_text(encoding='utf-8'))
        except ValueError:
            continue
        for t in data if isinstance(data, list) else []:
            sub = t.get('candidate_symbol_contains') if isinstance(t, dict) else None
            if not sub or not t.get('target_va'):
                continue
            hits = {name for _, name, _ in funcs if sub in name and name not in bindings}
            if len(hits) == 1:
                out[hits.pop()] = int(t['target_va'], 16)
    return out


TABLE_OPERAND = re.compile(r'\[[^\]]*\+ (0x[0-9a-f]+)\]')


def drop_tables(insns):
    """Cut a body at its first switch table (``jmp [reg*4 + T]``, ``mov cl, [eax + T]``).

    VC6 places jump and byte-index tables after the code, inside the function's
    extent. Decoded as instructions they are noise, and their absolute entries
    differ between retail and a relinked body, so they would lower the score of
    an exact switch."""
    if not insns:
        return insns
    start, end = insns[0].address, insns[-1].address + insns[-1].size
    tables = [int(m.group(1), 16) for x in insns if x.mnemonic in ('jmp', 'mov', 'movzx')
              for m in [TABLE_OPERAND.search(x.op_str)] if m]
    tables = [t for t in tables if start < t < end]
    if not tables:
        return insns
    cut = min(tables)
    return [x for x in insns if x.address < cut]


def body(pe: PEImage, va: int, size: int):
    insns = drop_tables(disassemble(read_va(pe, va, size), va))
    while insns and insns[-1].mnemonic in PAD:
        insns.pop()
    return insns


def similarity(a, b, strict: bool) -> float:
    if strict:
        na, nb = norm(a), norm(b)
    else:   # an ebp frame (retail inline-asm functions) and an esp frame compare equal
        na = [x.replace('esp', 'R') for x in norm(a, True, True)]
        nb = [x.replace('esp', 'R') for x in norm(b, True, True)]
    if not na and not nb:
        return 1.0
    sm = difflib.SequenceMatcher(None, na, nb, autojunk=False)
    return 2 * sum(t.size for t in sm.get_matching_blocks()) / (len(na) + len(nb))


REGS32 = ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp')
FORWARD_OPS = ('mov', 'push', 'lea', 'add', 'sub', 'ret', 'pop', 'xor')


def forward_target(insns) -> int | None:
    """Callee of a forwarder: argument moves/pushes around exactly one direct call or jmp."""
    calls = [x for x in insns if x.mnemonic in ('call', 'jmp')]
    # A store through anything but the outgoing argument area (a vptr write in a
    # constructor, say) is work, not forwarding. By-value arguments are copied
    # through a register loaded from esp (`mov ecx, esp; mov [ecx], eax`).
    stack_regs = {'esp'}
    for x in insns:
        dest = x.op_str.split(',')[0]
        if x.mnemonic == 'mov' and dest.strip() in REGS32:
            src = x.op_str.split(',')[1].strip()
            stack_regs.discard(dest.strip())
            if src == 'esp':
                stack_regs.add(dest.strip())
        m = re.match(r'\w+ ptr \[(\w+)', dest)
        if x.mnemonic == 'mov' and m and m.group(1) not in stack_regs:
            return None
    if (len(insns) > 200 or len(calls) != 1 or not calls[0].op_str.startswith('0x')
            or any(x.mnemonic not in FORWARD_OPS + ('call', 'jmp') for x in insns)):
        return None
    return int(calls[0].op_str, 16)


def compared_body(i: int, retail_insns, index: dict, linked_body, funcs):
    """The linked body to score for function ``i`` and the forwarders followed to it.

    Forwarders are followed (up to three) to the body they call, unless the retail
    body is forwarder-shaped too: CullQuad 0x0052f4d0 gathers four points on the
    stack and calls CullPolygon, and its own linked body is the one to compare.
    """
    b, via = linked_body(i), []
    if forward_target(retail_insns) is not None:
        return b, via
    for _ in range(3):
        target = forward_target(b)
        if target is None or target not in index:
            break
        k = index[target]
        b = linked_body(k)
        via.append(funcs[k][1])
    return b, via


def call_targets(insns) -> list[int]:
    """Direct call targets of a body, in order."""
    return [int(x.op_str, 16) for x in insns if x.mnemonic == 'call' and x.op_str.startswith('0x')]


def callee_mismatch(built_calls, retail_calls, to_retail) -> tuple[list[int], list[int]]:
    """Retail VAs the built body calls that retail's body never calls, and the reverse.

    ``to_retail`` maps a built call target to the set of retail VAs it may stand
    for (every symbol the linker put at that address, since aliases and folded
    functions share one; empty when none is bound). A built call counts as
    expected when any of its candidates is a retail callee. A missing callee is
    often an inlining difference; an unexpected one usually means a wrong call
    target, which the masked score cannot see because call displacements are masked.
    """
    want = set(retail_calls)
    mapped = [frozenset(to_retail(t)) for t in built_calls]
    unexpected = sorted({min(c) for c in mapped if c and not (c & want)})
    if any(not c for c in mapped):
        return unexpected, []
    covered = set().union(*mapped) if mapped else set()
    missing = sorted(want - covered)
    return unexpected, missing


def score_build(build: Path, retail: Path, min_size: int, strict: bool,
                libraries: bool = False) -> list[dict]:
    from tools.audit_game_vtables import function_size
    funcs = parse_functions((build / 'mcm2.map').read_text(encoding='latin-1'))
    built = PEImage(build / 'mcm2.exe')
    ret = PEImage(retail)
    bindings = load_bindings()
    for symbol, va in target_symbols(funcs, bindings).items():
        bindings.setdefault(symbol, set()).add(va)
    sizes = retail_sizes()
    index = {va: k for k, (va, _, _) in enumerate(funcs)}
    at_va: dict[int, list[int]] = {}
    for k, (va, _, _) in enumerate(funcs):
        at_va.setdefault(va, []).append(k)

    def linked_body(k: int):
        va = funcs[k][0]
        end = next((v for v, _, _ in funcs[k + 1:] if v > va), va + 0x4000)
        return body(built, va, end - va)

    def to_retail(target: int, hops: int = 3) -> set[int]:
        """Retail VAs a built call target may stand for: the bindings of every symbol at
        that address, else a forwarder's callee's."""
        ks = at_va.get(target, [])
        vas = set()
        for k in ks:
            vas |= bindings.get(funcs[k][1], set())
        if vas or not ks:
            return vas
        nxt = forward_target(linked_body(ks[0])) if hops else None
        return to_retail(nxt, hops - 1) if nxt is not None else set()

    best: dict[int, dict] = {}
    seen = set()
    for i, (va, symbol, obj) in enumerate(funcs):
        vas = bindings.get(symbol, set())
        if len(vas) != 1 or va in seen or (':' in obj and not libraries):
            continue
        seen.add(va)
        rva = next(iter(vas))
        rsize = sizes.get(rva) or function_size(ret, rva)
        if not rsize or rsize < min_size:
            continue
        r = body(ret, rva, rsize)
        b, via = compared_body(i, r, index, linked_body, funcs)
        s = similarity(b, r, strict)
        unexpected, missing = callee_mismatch(call_targets(b), call_targets(r), to_retail)
        row = dict(symbol=symbol, object=obj, retail_va=f'0x{rva:08x}', retail_bytes=rsize,
                   built_insns=len(b), retail_insns=len(r), score=round(s, 3),
                   weight=round(rsize * (1 - s), 1), via=via, other_bodies=[],
                   unexpected_calls=[f'0x{v:08x}' for v in unexpected],
                   missing_calls=[f'0x{v:08x}' for v in missing])
        # Several linked symbols can claim one retail VA (forwarders, renamed
        # stand-ins); the best-scoring body is taken as the implementation.
        prev = best.get(rva)
        if prev is None or row['score'] > prev['score']:
            if prev is not None:
                row['other_bodies'] = prev['other_bodies'] + [f"{prev['object']}:{prev['score']}"]
            best[rva] = row
        else:
            prev['other_bodies'].append(f"{obj}:{row['score']:.3f}")
    rows = list(best.values())
    rows.sort(key=lambda r: (-r['weight'], r['retail_va']))
    return rows


def render(rows: list[dict], top: int) -> str:
    lines = ['# Linked game accuracy (generated, ignored)', '',
             f'{len(rows)} bound retail functions scored; worst {min(top, len(rows))} by retail bytes x (1 - score).', '',
             '| retail VA | bytes | score | insns built/retail | object | symbol |',
             '|---|---:|---:|---|---|---|']
    for r in rows[:top]:
        via = f" via `{r['via'][-1]}`" if r['via'] else ''
        lines.append(f"| {r['retail_va']} | {r['retail_bytes']} | {r['score']:.3f} | "
                     f"{r['built_insns']}/{r['retail_insns']} | {r['object']} | `{r['symbol']}`{via} |")
    odd = [r for r in rows if r.get('unexpected_calls')]
    if odd:
        lines += ['', '## Calls retail does not make', '',
                  f"{len(odd)} linked bodies call a bound function that retail's body never calls "
                  '(a wrong call target, or a helper retail inlines).', '',
                  '| retail VA | score | object | symbol | unexpected callees | retail callees not called |',
                  '|---|---:|---|---|---|---|']
        for r in sorted(odd, key=lambda r: r['retail_va']):
            lines.append(f"| {r['retail_va']} | {r['score']:.3f} | {r['object']} | `{r['symbol']}` | "
                         f"{', '.join(r['unexpected_calls'])} | {', '.join(r['missing_calls'])} |")
    return '\n'.join(lines) + '\n'


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--build', type=Path, default=None, help='build directory holding mcm2.exe and mcm2.map')
    ap.add_argument('--retail', type=Path, default=DEFAULT_RETAIL)
    ap.add_argument('--out', type=Path, default=ROOT / 'work/linked-accuracy')
    ap.add_argument('--min-size', type=int, default=32)
    ap.add_argument('--top', type=int, default=60)
    ap.add_argument('--strict', action='store_true', help='keep registers and stack displacements')
    ap.add_argument('--libraries', action='store_true', help='include library members (LIBCMT:x.obj)')
    args = ap.parse_args(argv)
    build = args.build or latest_build()
    if build is None or not (build / 'mcm2.map').is_file():
        print('no linked build; run tools/build_game.py first', file=sys.stderr)
        return 2
    rows = score_build(build, args.retail, args.min_size, args.strict, args.libraries)
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / 'report.json').write_text(json.dumps(rows, indent=1) + '\n', encoding='utf-8')
    (args.out / 'report.md').write_text(render(rows, args.top), encoding='utf-8')
    total = sum(r['retail_bytes'] for r in rows)
    exactish = sum(r['retail_bytes'] for r in rows if r['score'] == 1.0)
    print(f'{len(rows)} bound retail functions, {total} retail bytes; {exactish} bytes at masked score 1.000')
    print(f"{sum(1 for r in rows if r['unexpected_calls'])} bodies call a bound function retail's body does not call")
    print(f"Report: {args.out / 'report.md'}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
