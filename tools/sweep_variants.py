#!/usr/bin/env python3
"""Exhaustive source-form sweeps for VC6 near misses.

A template is a C++ file with marked choice points. Every combination of the
choices is rendered, compiled with authentic VC6 in parallel and compared with
retail; the variants are ranked by strict (relocation-resolved) matching bytes
and by how many normalised instructions align with retail.

Template markers (a line marker is the first token on its own line):

  //@perm NAME          all orders of the items (separated by //@|)
  stmt_a;
  //@|
  stmt_b;
  //@end

  //@alt NAME           exactly one of the items
  int h = f();
  //@|
  //@end                (an empty item makes the block optional)

  //@opt NAME           shorthand for an alt of the body and nothing
  int dead = 0;
  //@end

  inline: {@NAME:a@|(a)@}  or anonymous {@a@|(a)@}; inline choices nest
  ({@E:m - {@P:w * h@|pixels@} * 2@|m + {@P:w * h@|pixels@} * -2@}) but stay
  on one line

Choice points that share a NAME are linked: they take the same choice (or
the same permutation), so a local's declaration and its uses can switch
together. Markers nest. The first item of every choice is the base variant
(index 0), so a template whose first items are the current source reproduces
it first.

  python3 tools/sweep_variants.py TEMPLATE.cpp --target 0x4705d0:101 \
      --bindings samples/ui/GameUiNearMisses.bindings.json \
      --exe work/game/mcm2.exe --vc6-root "$VC6_ROOT" --jobs 12

--target is VA[:SIZE][=SYMBOL] (repeatable). Without a symbol the bindings
entry for the VA is used; without a size the base variant's size is used.
Variants are written to --out (default work/sweep/<template stem>), with the
template's directory on the include path so its quoted includes resolve.
--dry-run lists the choice points and the variant count; --render N writes
variant N without compiling. Results go to results.jsonl; the best variants
are re-rendered as best_<rank>.cpp.
"""
from __future__ import annotations

import argparse
import difflib
import hashlib
import itertools
import json
import math
import os
import random
import re
import subprocess
import sys
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT))
sys.path.insert(0, str(ROOT / 'tools'))

# --------------------------------------------------------------------------
# Template parsing and rendering (no VC6 dependency)
# --------------------------------------------------------------------------

_LINE_MARKER = re.compile(r'^\s*//@(perm|alt|opt|\||end)(?![\w|])\s*(\S*)')
_INLINE_NAME = re.compile(r'([A-Za-z_][\w.]*):')


class TemplateError(ValueError):
    pass


@dataclass
class Choice:
    name: str
    kind: str                      # 'perm' or 'alt'
    items: list = field(default_factory=list)   # list[list[node]]


def _inline_nodes(text: str, counter: list[int]) -> list:
    """Literal text and inline choices; choices nest (`{@a@|{@B:b@|c@}@}`)."""
    return _inline_parse(text, 0, counter, top=True)[0]


def _inline_parse(text: str, pos: int, counter: list[int], top: bool):
    """Nodes up to the end of `text` (top) or of the current item (`@|`/`@}`)."""
    nodes: list = []
    start = pos
    while pos < len(text):
        if text.startswith('{@', pos):
            if pos > start:
                nodes.append(text[start:pos])
            pos += 2
            m = _INLINE_NAME.match(text, pos)
            if m:
                name, pos = m.group(1), m.end()
            else:
                counter[0] += 1
                name = f'inline{counter[0]}'
            items = []
            while True:
                item, pos = _inline_parse(text, pos, counter, top=False)
                items.append(item)
                pos += 2
                if text.startswith('@}', pos - 2):
                    break
            nodes.append(Choice(name, 'alt', items))
            start = pos
            continue
        if not top and (text.startswith('@|', pos) or text.startswith('@}', pos)):
            break
        pos += 1
    else:
        if not top:
            raise TemplateError('unterminated inline choice {@...@}')
    if pos > start:
        nodes.append(text[start:pos])
    return nodes, pos


def parse_template(text: str) -> list:
    """Nodes: str (literal text) or Choice."""
    counter = [0]
    root: list = []
    stack: list[tuple[Choice, list]] = []   # (choice, current item)
    current = root
    for line in text.splitlines(keepends=True):
        m = _LINE_MARKER.match(line)
        if not m:
            current.extend(_inline_nodes(line, counter))
            continue
        kind, name = m.group(1), m.group(2)
        if kind in ('perm', 'alt', 'opt'):
            if not name:
                counter[0] += 1
                name = f'{kind}{counter[0]}'
            choice = Choice(name, 'perm' if kind == 'perm' else 'alt')
            choice.opt = kind == 'opt'
            current.append(choice)
            item: list = []
            choice.items.append(item)
            stack.append((choice, current))
            current = item
        elif kind == '|':
            if not stack:
                raise TemplateError('//@| outside a choice')
            choice, _ = stack[-1]
            if getattr(choice, 'opt', False):
                raise TemplateError(f'//@| inside //@opt {choice.name}')
            item = []
            choice.items.append(item)
            current = item
        else:   # end
            if not stack:
                raise TemplateError('//@end without a choice')
            choice, parent = stack.pop()
            if getattr(choice, 'opt', False):
                choice.items.append([])
            current = parent
    if stack:
        raise TemplateError(f'unterminated choice {stack[-1][0].name}')
    return root


def dimensions(nodes: list) -> dict[str, tuple[str, int]]:
    """name -> (kind, item count), in first-appearance order."""
    dims: dict[str, tuple[str, int]] = {}

    def walk(ns):
        for n in ns:
            if isinstance(n, Choice):
                key = (n.kind, len(n.items))
                if n.name in dims and dims[n.name] != key:
                    raise TemplateError(f'linked choice {n.name} differs: {dims[n.name]} vs {key}')
                dims.setdefault(n.name, key)
                for item in n.items:
                    walk(item)
    walk(nodes)
    return dims


def dim_size(kind: str, n: int) -> int:
    return math.factorial(n) if kind == 'perm' else n


def _permutation(n: int, index: int) -> tuple[int, ...]:
    """index-th permutation of range(n) in lexicographic order (0 = identity)."""
    pool, out = list(range(n)), []
    for k in range(n, 0, -1):
        f = math.factorial(k - 1)
        q, index = divmod(index, f)
        out.append(pool.pop(q))
    return tuple(out)


def render(nodes: list, assignment: dict[str, int]) -> str:
    out: list[str] = []

    def walk(ns):
        for n in ns:
            if isinstance(n, str):
                out.append(n)
                continue
            pick = assignment.get(n.name, 0)
            if n.kind == 'perm':
                for i in _permutation(len(n.items), pick):
                    walk(n.items[i])
            else:
                walk(n.items[pick])
    walk(nodes)
    return ''.join(out)


def describe(dims: dict[str, tuple[str, int]], assignment: dict[str, int]) -> str:
    parts = []
    for name, (kind, n) in dims.items():
        v = assignment.get(name, 0)
        if kind == 'perm':
            parts.append(f'{name}={"".join(str(i) for i in _permutation(n, v)) if n <= 10 else _permutation(n, v)}')
        else:
            parts.append(f'{name}={v}')
    return ' '.join(parts)


def assignments(dims: dict[str, tuple[str, int]], limit: int | None, seed: int):
    """All assignments, or the base plus a seeded sample of `limit` - 1 others."""
    names = list(dims)
    sizes = [dim_size(*dims[n]) for n in names]
    total = math.prod(sizes) if sizes else 1
    if limit is None or total <= limit:
        for combo in itertools.product(*[range(s) for s in sizes]):
            yield dict(zip(names, combo))
        return
    rng = random.Random(seed)
    picked = {0}
    picked.update(rng.sample(range(1, total), limit - 1))
    for flat in sorted(picked):
        combo = []
        for s in reversed(sizes):
            flat, r = divmod(flat, s)
            combo.append(r)
        yield dict(zip(names, reversed(combo)))


# --------------------------------------------------------------------------
# Scoring
# --------------------------------------------------------------------------

@dataclass
class Target:
    va: int
    size: int | None
    symbol: str | None


def parse_target(spec: str) -> Target:
    m = re.fullmatch(r'(0x[0-9a-fA-F]+|\d+)(?::(0x[0-9a-fA-F]+|\d+))?(?:=(.+))?', spec)
    if not m:
        raise argparse.ArgumentTypeError(f'bad target {spec!r}; want VA[:SIZE][=SYMBOL]')
    return Target(int(m.group(1), 0), int(m.group(2), 0) if m.group(2) else None, m.group(3))


def symbol_for_va(bindings: dict[str, int], va: int) -> str:
    names = [k for k, v in bindings.items() if v == va and not k.startswith('__FILE__')]
    funcs = [k for k in names if '@' in k and not k.endswith(('$ehhandler', '$scopetable'))]
    if len(funcs) == 1:
        return funcs[0]
    raise SystemExit(f'cannot pick a symbol for 0x{va:08x} from the bindings: {names}; use VA:SIZE=SYMBOL')


def normalised_alignment(retail: bytes, cand: bytes) -> tuple[int, int, int]:
    """(aligned, retail instructions, candidate instructions) over normalised text."""
    from frame_layout import disassemble, normalize
    a = [normalize(i) for i in disassemble(retail)]
    b = [normalize(i) for i in disassemble(cand)]
    sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
    return sum(blk.size for blk in sm.get_matching_blocks()), len(a), len(b)


def instruction_diff(retail: bytes, cand: bytes, context: int = 2) -> list[str]:
    """Side-by-side retail | candidate listing of the differing instruction runs.

    Instructions are paired on their normalised text (as in the alignment
    score); equal runs longer than 2 * context are elided to their ends.
    """
    from frame_layout import disassemble, normalize
    a, b = disassemble(retail), disassemble(cand)
    sm = difflib.SequenceMatcher(None, [normalize(i) for i in a], [normalize(i) for i in b],
                                 autojunk=False)

    def text(ins):
        return f'{ins.address:4x} {ins.mnemonic} {ins.op_str}'.rstrip() if ins else ''
    lines = []
    for tag, i1, i2, j1, j2 in sm.get_opcodes():
        if tag == 'equal':
            n = i2 - i1
            keep = range(n) if n <= 2 * context else [*range(context), None, *range(n - context, n)]
            for k in keep:
                if k is None:
                    lines.append(f'{"...":<44}   ...')
                else:
                    lines.append(f'{text(a[i1 + k]):<44}   {text(b[j1 + k])}')
            continue
        for k in range(max(i2 - i1, j2 - j1)):
            left = a[i1 + k] if i1 + k < i2 else None
            right = b[j1 + k] if j1 + k < j2 else None
            lines.append(f'{text(left):<44} | {text(right)}')
    return lines


def score_target(pe, obj, target: Target, bindings: dict[str, int] | None,
                 want_diff: bool = False) -> dict:
    from mcm2tool.resolved_match import RelocationError, match_object
    from mcm2tool.coff import relocation_mask
    sym = obj.find_symbol(target.symbol)
    cand, _, rels = obj.symbol_extent(sym)
    size = target.size or len(cand)
    retail = pe.bytes_at_va(target.va, size)
    row = {'va': f'0x{target.va:08x}', 'size': size, 'candidate_size': len(cand)}
    strict = None
    if bindings is not None:
        try:
            strict = match_object(obj, sym.name, target.va, retail, bindings)
        except RelocationError as exc:
            row['strict_error'] = str(exc)
    if strict is not None:
        # align the relocated bytes, so bound addresses compare like retail's
        patched = bytearray(cand)
        for rel in strict['relocations_applied']:
            patched[rel['offset']:rel['offset'] + 4] = rel['written_u32'].to_bytes(4, 'little')
        cand = bytes(patched)
        row['exact'] = strict['strict_exact']
        row['matching'] = strict['matching_positions']
        row['compared'] = strict['compared_positions']
        row['mode'] = 'strict'
    else:
        mask = relocation_mask(obj, sym, len(cand), rels)
        n = max(len(retail), len(cand))
        row['matching'] = sum(1 for i in range(min(len(retail), len(cand)))
                              if not mask[i] and retail[i] == cand[i])
        row['compared'] = n - sum(mask)
        row['exact'] = len(retail) == len(cand) and row['matching'] == row['compared']
        row['mode'] = 'masked'
    aligned, nr, nc = normalised_alignment(retail, cand)
    row.update({'aligned': aligned, 'retail_insns': nr, 'candidate_insns': nc,
                'insn_diffs': max(nr, nc) - aligned})
    if want_diff:
        row['diff'] = instruction_diff(retail, cand)
    return row


# --------------------------------------------------------------------------
# Driver
# --------------------------------------------------------------------------

def vc6_command(root: Path, profile_flags: list[str], includes: list[Path], extra: list[str],
                src: Path, obj: Path) -> list[str]:
    from mcm2tool.toolchain import find_child_ci, find_vc6_bin, infer_vc98_root
    from mcm2tool.vc6_runtime import executable_command, runner_kind, windows_path
    runner = runner_kind()
    cl = find_child_ci(find_vc6_bin(root), 'cl.exe')
    vc98 = infer_vc98_root(root)
    incs = list(includes)
    inc = find_child_ci(vc98, 'Include')
    if inc and inc.is_dir():
        incs.append(inc)
    mfc = find_child_ci(vc98, 'MFC')
    if mfc:
        mi = find_child_ci(mfc, 'Include')
        if mi and mi.is_dir():
            incs.append(mi)
    return [*executable_command(cl, runner), *profile_flags,
            *[f'/I{windows_path(p, runner)}' for p in incs], *extra,
            windows_path(src, runner), f'/Fo{windows_path(obj, runner)}']


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split('\n\n')[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__)
    ap.add_argument('template')
    ap.add_argument('--target', action='append', type=parse_target, default=[],
                    help='VA[:SIZE][=SYMBOL] (repeatable)')
    ap.add_argument('--bindings', type=Path, help='symbol-to-VA JSON; strict scoring when given')
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE', 'work/game/mcm2.exe'))
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--profile', default='vc6_o2_mt')
    ap.add_argument('--profiles', default=str(ROOT / 'config/compile_profiles.json'))
    ap.add_argument('--include', action='append', default=[], type=Path)
    ap.add_argument('--extra', action='append', default=[])
    ap.add_argument('--jobs', type=int, default=os.cpu_count() or 4)
    ap.add_argument('--limit', type=int, help='sample this many variants when the space is larger')
    ap.add_argument('--seed', type=int, default=1)
    ap.add_argument('--out', type=Path)
    ap.add_argument('--top', type=int, default=15)
    ap.add_argument('--stop-on-exact', action='store_true')
    ap.add_argument('--dry-run', action='store_true')
    ap.add_argument('--render', type=int, metavar='N', help='write variant N (enumeration order) and exit')
    ap.add_argument('--diff', action='store_true',
                    help='print a retail | candidate instruction diff of the best variant')
    a = ap.parse_args(argv)

    template = Path(a.template).resolve()
    nodes = parse_template(template.read_text(encoding='latin-1'))
    dims = dimensions(nodes)
    total = math.prod(dim_size(*v) for v in dims.values()) if dims else 1
    out = (a.out or ROOT / 'work/sweep' / template.stem).resolve()
    print(f'{len(dims)} choice points, {total} variants'
          + (f' (sampling {a.limit})' if a.limit and total > a.limit else ''))
    for name, (kind, n) in dims.items():
        print(f'  {kind:4} {name}: {n} items -> {dim_size(kind, n)}')
    if a.dry_run:
        return 0
    if a.render is not None:
        for i, asg in enumerate(assignments(dims, a.limit, a.seed)):
            if i == a.render:
                out.mkdir(parents=True, exist_ok=True)
                path = out / f'render_{i}.cpp'
                path.write_text(render(nodes, asg), encoding='latin-1')
                print(path, describe(dims, asg))
                return 0
        raise SystemExit('variant index out of range')
    if not a.target:
        raise SystemExit('give at least one --target')
    if not a.vc6_root:
        raise SystemExit('set VC6_ROOT or pass --vc6-root')

    from mcm2tool.coff import CoffObject
    from mcm2tool.pe import PEImage
    pe = PEImage(a.exe)
    bindings = None
    if a.bindings:
        raw = json.loads(a.bindings.read_text())
        bindings = {k: int(v, 0) if isinstance(v, str) else v for k, v in raw.items()}
    for t in a.target:
        if not t.symbol:
            if bindings is None:
                raise SystemExit('a target without =SYMBOL needs --bindings')
            t.symbol = symbol_for_va(bindings, t.va)
    flags = json.loads(Path(a.profiles).read_text())['profiles'][a.profile]
    includes = [template.parent] + [p.resolve() for p in a.include]
    out.mkdir(parents=True, exist_ok=True)
    seen: dict[str, int] = {}
    results: list[dict] = []
    stop = [False]

    def job(index: int, asg: dict[str, int], text: str, want_diff: bool = False) -> dict:
        row = {'index': index, 'choices': describe(dims, asg), 'assignment': asg}
        if stop[0] and not want_diff:
            row['skipped'] = True
            return row
        src = out / f'v{index:06d}{template.suffix}'
        obj = out / f'v{index:06d}.obj'
        src.write_text(text, encoding='latin-1')
        obj.unlink(missing_ok=True)
        cmd = vc6_command(Path(a.vc6_root), flags, includes, a.extra, src, obj)
        r = subprocess.run(cmd, text=True, encoding='latin-1', stdout=subprocess.PIPE,
                           stderr=subprocess.STDOUT, cwd=out)
        try:
            if r.returncode or not obj.is_file():
                row['compile_error'] = r.stdout[-2000:]
                return row
            coff = CoffObject(obj)
            row['targets'] = [score_target(pe, coff, t, bindings, want_diff) for t in a.target]
            row['exact'] = all(t['exact'] for t in row['targets'])
            row['matching'] = sum(t['matching'] for t in row['targets'])
            row['aligned'] = sum(t['aligned'] for t in row['targets'])
            row['insn_diffs'] = sum(t['insn_diffs'] for t in row['targets'])
            if row['exact'] and a.stop_on_exact:
                stop[0] = True
        except Exception as exc:     # a broken variant is data, not a crash
            row['score_error'] = f'{type(exc).__name__}: {exc}'
        finally:
            src.unlink(missing_ok=True)
            obj.unlink(missing_ok=True)
        return row

    t0 = time.time()
    work = []
    for index, asg in enumerate(assignments(dims, a.limit, a.seed)):
        text = render(nodes, asg)
        digest = hashlib.sha1(text.encode('latin-1')).hexdigest()
        if digest in seen:
            continue        # an identical rendering (e.g. empty items swapped)
        seen[digest] = index
        work.append((index, asg, text))
    print(f'{len(work)} distinct renderings; compiling with {a.jobs} jobs into {out}')
    log = (out / 'results.jsonl').open('w')
    done = 0
    with ThreadPoolExecutor(max_workers=max(1, a.jobs)) as pool:
        futures = [pool.submit(job, *w) for w in work]
        for fut in as_completed(futures):
            row = fut.result()
            results.append(row)
            log.write(json.dumps(row) + '\n')
            done += 1
            if done % 50 == 0 or done == len(work):
                best = max((r for r in results if 'matching' in r),
                           key=lambda r: (r['exact'], r['matching'], r['aligned']), default=None)
                print(f'  {done}/{len(work)} {time.time() - t0:.0f}s best='
                      + (f"{best['matching']} aligned={best['aligned']} #{best['index']}" if best else '-'),
                      flush=True)
    log.close()

    scored = [r for r in results if 'matching' in r]
    errors = [r for r in results if 'compile_error' in r or 'score_error' in r]
    scored.sort(key=lambda r: (-r['exact'], -r['matching'], -r['aligned'], r['index']))
    base = next((r for r in results if r['index'] == 0), None)
    print(f'\n{len(scored)} scored, {len(errors)} failed, {sum(r["exact"] for r in scored)} exact, '
          f'{time.time() - t0:.0f}s')
    if base and 'matching' in base:
        print(f"base: matching={base['matching']} aligned={base['aligned']} insn_diffs={base['insn_diffs']}")
    if errors:
        first = errors[0]
        print(f"first failure (#{first['index']} {first['choices']}):\n"
              + (first.get('compile_error') or first.get('score_error', ''))[-800:])
    all_asg = {r['index']: r['assignment'] for r in results}
    for rank, r in enumerate(scored[:a.top], 1):
        per = ' '.join(f"{t['va']}:{t['matching']}/{max(t['size'], t['candidate_size'])}"
                       f"{'*' if t['exact'] else ''}({t['mode'][0]},d{t['insn_diffs']})"
                       for t in r['targets'])
        print(f"{rank:3} #{r['index']:<6} {per}  {r['choices']}")
        (out / f'best_{rank}{template.suffix}').write_text(render(nodes, all_asg[r['index']]),
                                                           encoding='latin-1')
    if a.diff and scored:
        best = scored[0]
        row = job(best['index'], all_asg[best['index']], render(nodes, all_asg[best['index']]), True)
        for t in row.get('targets', []):
            print(f"\n#{best['index']} {t['va']} retail | candidate ({t['matching']}/{t['compared']})")
            print('\n'.join(t['diff']))
    return 0 if any(r['exact'] for r in scored) else 1


if __name__ == '__main__':
    raise SystemExit(main())
