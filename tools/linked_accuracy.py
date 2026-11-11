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
call or jmp) is followed to the body it calls, recorded as ``via``.

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


def body(pe: PEImage, va: int, size: int):
    insns = disassemble(read_va(pe, va, size), va)
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


def score_build(build: Path, retail: Path, min_size: int, strict: bool,
                libraries: bool = False) -> list[dict]:
    from tools.audit_game_vtables import function_size
    funcs = parse_functions((build / 'mcm2.map').read_text(encoding='latin-1'))
    built = PEImage(build / 'mcm2.exe')
    ret = PEImage(retail)
    bindings = load_bindings()
    sizes = retail_sizes()
    index = {va: k for k, (va, _, _) in enumerate(funcs)}

    def linked_body(k: int):
        va = funcs[k][0]
        end = next((v for v, _, _ in funcs[k + 1:] if v > va), va + 0x4000)
        return body(built, va, end - va)

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
        b, via = linked_body(i), []
        for _ in range(3):      # follow forwarders to the body they call
            target = forward_target(b)
            if target not in index or forward_target(b) is None:
                break
            k = index[target]
            b = linked_body(k)
            via.append(funcs[k][1])
        r = body(ret, rva, rsize)
        s = similarity(b, r, strict)
        row = dict(symbol=symbol, object=obj, retail_va=f'0x{rva:08x}', retail_bytes=rsize,
                   built_insns=len(b), retail_insns=len(r), score=round(s, 3),
                   weight=round(rsize * (1 - s), 1), via=via, other_bodies=[])
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
    print(f"Report: {args.out / 'report.md'}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
