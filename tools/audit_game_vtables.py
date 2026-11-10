#!/usr/bin/env python3
"""Compare virtual dispatch in the source-built game against retail.

Both executables are parsed with the same RTTI walker (``mcm2tool.rtti``), so
every vtable is keyed by its RTTI class and complete-object-locator
``object_offset`` (secondary and virtual-base vtables included). Built slot
pointers are named through the linker map that ``tools/build_game.py`` writes.
Each slot is classified:

  missing_override  retail has a class-specific function (absent from the same
                    slot of every base-class vtable) but the build dispatches to
                    a function owned by another class or a free function.
  purecall          the build dispatches to ``__purecall`` where retail does not.
  stub              the built target is tiny (a ``ret``-sized body) while the
                    retail function is substantially larger.
  bound_mismatch    the built symbol is bound to a different retail VA in a
                    ``*.bindings.json`` file or calibration case.
  extra_override    the build overrides a slot that retail inherits (often fine:
                    retail folds identical tiny bodies, so this is informational).
  slot_count        retail and built vtables have different lengths.
  ok                none of the above.

Tiny retail bodies are folded across unrelated classes, so a class-specific
retail address of at most ``--tiny`` bytes is marked ``low_priority``: a shared
address alone does not prove method identity. Class ownership of a built
symbol comes from its mangled scope; it is evidence about the build only.

    python tools/audit_game_vtables.py --build work/game-build/build-XXXX

Output goes to ``work/vtable-audit/`` (ignored).
"""
from __future__ import annotations

import argparse
import ast
import json
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

MAP_LINE = re.compile(r'^\s*[0-9a-fA-F]{4}:[0-9a-fA-F]{8}\s+(\S+)\s+([0-9a-fA-F]{8})\b')
PURECALL = ('__purecall', '_purecall')
DEFAULT_RETAIL = ROOT / 'work/game/mcm2.exe'


# ---------------------------------------------------------------- symbols

def parse_map(text: str) -> dict[int, list[str]]:
    """Address -> symbols from a VC6 ``/MAP`` file (publics and statics)."""
    out: dict[int, list[str]] = {}
    for line in text.splitlines():
        m = MAP_LINE.match(line)
        if m:
            out.setdefault(int(m.group(2), 16), []).append(m.group(1))
    return out


def _read_scope(name: str, pos: int) -> str | None:
    """Read ``A@B@@`` (innermost first) starting at ``pos``; None for templates."""
    end = name.find('@@', pos)
    if end < 0 or end == pos:
        return None
    scope = name[pos:end]
    if '?' in scope:
        return None
    return scope


def symbol_owner(symbol: str) -> tuple[str | None, str]:
    """Return (class scope, member name) for a decorated VC6 symbol.

    The scope is spelled like the RTTI body (``Inner@Outer``) so it compares
    directly with ``.?AVInner@Outer@@``. Free functions return (None, name).
    """
    if not symbol.startswith('?'):
        return None, symbol
    if symbol.startswith('??'):
        # Special member: ??0 ctor, ??1 dtor, ??_G/??_E deleting dtors, ??4 ...
        if symbol.startswith('??__'):
            pos = 5
        elif symbol.startswith('??_'):
            pos = 4
        else:
            pos = 3
        return _read_scope(symbol, pos), symbol[:pos]
    at = symbol.find('@', 1)
    if at < 0:
        return None, symbol
    member = symbol[1:at]
    rest = symbol[at + 1:]
    if rest.startswith('@'):
        return None, member  # ?Function@@YA... is a free function
    return _read_scope(symbol, at + 1), member


def rtti_scope(decorated: str) -> str:
    """``.?AVInner@Outer@@`` -> ``Inner@Outer``."""
    body = decorated[4:] if decorated.startswith('.?A') else decorated
    return body[:-2] if body.endswith('@@') else body


def load_bindings(root: Path = ROOT) -> dict[str, set[int]]:
    """Symbol -> retail VAs claimed by bindings files and calibration cases."""
    out: dict[str, set[int]] = {}
    for tree in ('src', 'samples'):
        base = root / tree
        if not base.is_dir():
            continue
        for path in sorted(base.rglob('*.bindings.json')):
            try:
                data = json.loads(path.read_text(encoding='utf-8'))
            except (OSError, ValueError):
                continue
            if not isinstance(data, dict):
                continue
            for symbol, va in data.items():
                if isinstance(va, str) and va.startswith('0x'):
                    out.setdefault(symbol, set()).add(int(va, 16))
    calibration = root / 'tools/run_calibration.py'
    if calibration.is_file():
        tree = ast.parse(calibration.read_text(encoding='utf-8'))
        for node in tree.body:
            if isinstance(node, ast.Assign) and any(
                    isinstance(t, ast.Name) and t.id == 'CASES' for t in node.targets):
                for case in ast.literal_eval(node.value):
                    if case.get('symbol') and case.get('target_va'):
                        out.setdefault(case['symbol'], set()).add(int(case['target_va'], 16))
    return out


# ---------------------------------------------------------------- vtables

def collect_entries(pe, vtable_va: int, limit: int = 256) -> list[int]:
    out = []
    off = pe.va_to_offset(vtable_va)
    for i in range(limit):
        if off + i * 4 + 4 > len(pe.data):
            break
        p = pe.u32(off + i * 4)
        if not pe.is_code_va(p):
            break
        out.append(p)
    return out


def image_vtables(pe) -> dict:
    """{'classes': {scope: {name, bases}}, 'vtables': {(scope, offset): {...}}}."""
    from mcm2tool.rtti import parse_rtti
    classes, vtables = {}, {}
    for cls in parse_rtti(pe):
        scope = rtti_scope(cls.decorated)
        classes[scope] = {'name': cls.name,
                          'bases': [rtti_scope(b.decorated) for b in cls.bases[1:]]}
        for record in cls.vtable_records:
            if not record.vtables:
                continue
            va = record.vtables[0]
            vtables[(scope, record.object_offset)] = {
                'class': cls.name, 'scope': scope, 'object_offset': record.object_offset,
                'vtable_va': va, 'entries': collect_entries(pe, va)}
    return {'classes': classes, 'vtables': vtables}


def function_size(pe, va: int) -> int | None:
    from tools.disasm_fn import disassemble
    try:
        _, end, _, _ = disassemble(pe, va, 0x4000)
    except Exception:
        return None
    return end - va


def find_purecall(pe, built_pe, built_purecall: int | None) -> set[int]:
    """Retail VAs whose bytes equal the built ``__purecall`` (rel32 masked)."""
    if built_purecall is None:
        return set()
    size = function_size(built_pe, built_purecall) or 0
    if not 0 < size <= 32:
        return set()
    body = built_pe.bytes_at_va(built_purecall, size)
    # Mask call rel32 operands: they differ between images.
    i, parts = 0, []
    while i < len(body):
        if body[i] == 0xe8 and i + 5 <= len(body):
            parts.append(re.escape(body[i:i + 1]) + b'.{4}')
            i += 5
        else:
            parts.append(re.escape(body[i:i + 1]))
            i += 1
    pattern = b''.join(parts)
    out = set()
    for sec in pe.sections:
        if not sec.characteristics & 0x20000000:
            continue
        blob = pe.data[sec.raw_offset:sec.raw_offset + sec.raw_size]
        for m in re.finditer(pattern, blob, re.S):
            out.add(pe.offset_to_va(sec.raw_offset + m.start()))
    return out


# ---------------------------------------------------------------- audit

def thunk_target(blob: bytes, va: int) -> tuple[int, bytes] | None:
    """Follow a VC6 ``this`` adjustor/vtordisp thunk or a bare tail ``jmp``.

    A body that is only ``jmp rel32`` (a forwarding override such as
    ``return Base::F();``) dispatches to the same code as its target, so it is
    followed with an empty adjustment, reported as None. Accepted prefixes: ``sub ecx,[ecx-N]`` (2b 49 xx), ``sub ecx,imm8``
    (83 e9 xx), ``sub ecx,imm32`` (81 e9 ...), ``add ecx,imm8`` (83 c1 xx),
    ``add ecx,imm32`` (81 c1 ...). Returns (target, adjustment bytes).
    """
    pos = 0
    while pos < 16 and pos < len(blob):
        if blob[pos:pos + 2] in (b'\x2b\x49', b'\x83\xe9', b'\x83\xc1'):
            pos += 3
        elif blob[pos:pos + 2] in (b'\x81\xe9', b'\x81\xc1'):
            pos += 6
        elif blob[pos] == 0xe9 and pos + 5 <= len(blob):
            rel = int.from_bytes(blob[pos + 1:pos + 5], 'little', signed=True)
            return (va + pos + 5 + rel) & 0xffffffff, bytes(blob[:pos]) or None
        else:
            return None
    return None


class Side:
    """Per-image callbacks; the defaults make ``classify`` usable on plain data."""

    def __init__(self, size=None, body=None, resolve=None):
        self.size = size or (lambda va: None)
        self.body = body or (lambda va: None)
        self.resolve = resolve or (lambda va: (va, None))


def image_side(pe) -> Side:
    sizes: dict[int, int | None] = {}

    def size(va):
        if va not in sizes:
            sizes[va] = function_size(pe, va)
        return sizes[va]

    def body(va):
        n = size(va)
        return pe.bytes_at_va(va, n) if n else None

    def resolve(va):
        target, adjust = va, None
        for _ in range(4):
            try:
                hit = thunk_target(pe.bytes_at_va(target, 24), target)
            except Exception:
                hit = None
            if not hit or not pe.is_code_va(hit[0]) or (adjust is not None and hit[1] is not None):
                break
            target, adjust = hit[0], adjust if hit[1] is None else hit[1]
        return target, adjust
    return Side(size, body, resolve)


def classify(retail: dict, built: dict, symbols: dict[int, list[str]],
             bindings: dict[str, set[int]], retail_side: Side | None = None,
             built_side: Side | None = None, retail_purecall=frozenset(),
             tiny: int = 8) -> dict:
    """Classify every retail slot against the built vtable with the same key.

    ``retail``/``built`` are ``image_vtables`` results; thunks are followed on
    both sides before class ownership is judged.
    """
    rs, bs = retail_side or Side(), built_side or Side()
    findings, slot_counts, missing_vtables = [], [], []
    categories: Counter = Counter()
    rclasses, rvt = retail['classes'], retail['vtables']
    users: dict[int, set[str]] = {}
    for (scope, _), vt in rvt.items():
        for va in vt['entries']:
            users.setdefault(rs.resolve(va)[0], set()).add(scope)

    def providers(scope: str, slot: int, target: int) -> set[str]:
        """Retail base classes whose vtable holds ``target`` at ``slot``."""
        out = set()
        for base in rclasses.get(scope, {}).get('bases', []):
            for (other, _), vt in rvt.items():
                if (other == base and slot < len(vt['entries'])
                        and rs.resolve(vt['entries'][slot])[0] == target):
                    out.add(base)
        return out

    for key in sorted(rvt, key=lambda k: (rclasses[k[0]]['name'].lower(), k[1])):
        rv = rvt[key]
        bv = built['vtables'].get(key)
        scope, offset = key
        if bv is None:
            missing_vtables.append({'class': rv['class'], 'object_offset': offset,
                                    'retail_vtable': f"0x{rv['vtable_va']:08x}",
                                    'class_built': scope in built['classes']})
            categories['vtable_absent' if scope in built['classes'] else 'class_absent'] += 1
            continue
        if len(rv['entries']) != len(bv['entries']):
            slot_counts.append({'class': rv['class'], 'object_offset': offset,
                                'retail_slots': len(rv['entries']),
                                'built_slots': len(bv['entries'])})
            categories['slot_count'] += 1
        bases = set(rclasses.get(scope, {}).get('bases', []))
        for slot, rva in enumerate(rv['entries'][:len(bv['entries'])]):
            bva = bv['entries'][slot]
            rt, rthunk = rs.resolve(rva)
            bt, bthunk = bs.resolve(bva)
            names = symbols.get(bt, [])
            owners = {symbol_owner(n)[0] for n in names}
            bound = set().union(*(bindings.get(n, set()) for n in names)) if names else set()
            inherited_from = providers(scope, slot, rt)
            specific = not inherited_from
            built_pure = any(n in PURECALL for n in names)
            retail_pure = rt in retail_purecall
            rsize, bsize = rs.size(rt), bs.size(bt)
            same_body = (rsize is not None and rsize <= tiny and rsize == bsize
                         and rs.body(rt) == bs.body(bt))
            if built_pure or retail_pure:
                kind = 'ok' if built_pure and retail_pure else (
                    'purecall' if built_pure else 'retail_purecall')
            elif rt in bound or same_body:
                kind = 'ok'
            elif specific and scope not in owners:
                kind = 'missing_override'
            elif not specific and scope in owners:
                kind = 'extra_override'
            elif not specific and not owners & inherited_from:
                kind = 'inherited_mismatch'
            elif bound:
                kind = 'bound_mismatch'
            elif (bsize is not None and rsize is not None and bsize <= 3
                    and rsize >= 16 and rsize >= 4 * bsize):
                kind = 'stub'
            else:
                kind = 'ok'
            if kind == 'ok' and (rthunk != bthunk):
                kind = 'thunk_mismatch'
            categories[kind] += 1
            if kind == 'ok':
                continue
            row = {'kind': kind, 'class': rv['class'], 'object_offset': offset, 'slot': slot,
                   'retail_va': f'0x{rva:08x}', 'retail_target': f'0x{rt:08x}',
                   'retail_size': rsize, 'retail_specific': specific,
                   'retail_shared_by': len(users.get(rt, ())),
                   'built_va': f'0x{bva:08x}', 'built_target': f'0x{bt:08x}',
                   'built_size': bsize, 'built_symbol': names[0] if names else None,
                   'built_symbols': names,
                   'owner_is_base': bool(owners & bases),
                   'retail_inherited_from': sorted(rclasses[b]['name'] for b in inherited_from
                                                   if b in rclasses),
                   'low_priority': kind == 'missing_override' and rsize is not None and rsize <= tiny}
            if rthunk is not None or bthunk is not None:
                row['retail_thunk'] = rthunk.hex() if rthunk is not None else None
                row['built_thunk'] = bthunk.hex() if bthunk is not None else None
            if rsize is not None and rsize <= 16:
                row['retail_bytes'] = rs.body(rt).hex() if rs.body(rt) else None
            if bsize is not None and bsize <= 16:
                row['built_bytes'] = bs.body(bt).hex() if bs.body(bt) else None
            if bound:
                row['built_bound_to'] = [f'0x{v:08x}' for v in sorted(bound)]
            findings.append(row)
    extra = sorted({k[0] for k in built['vtables']} - {k[0] for k in rvt})
    return {'summary': dict(sorted(categories.items())), 'findings': findings,
            'slot_counts': slot_counts, 'missing_vtables': missing_vtables,
            'built_only_classes': [built['classes'][s]['name'] for s in extra]}


def render(report: dict) -> str:
    lines = ['# Game vtable audit', '', 'Category counts (slots unless noted):', '']
    for key, value in report['summary'].items():
        lines.append(f'- {key}: {value}')
    for kind in ('missing_override', 'inherited_mismatch', 'purecall', 'stub', 'thunk_mismatch',
                 'bound_mismatch',
                 'retail_purecall', 'extra_override'):
        rows = [r for r in report['findings'] if r['kind'] == kind]
        if not rows:
            continue
        lines += ['', f'## {kind} ({len(rows)})', '',
                  '| class | off | slot | retail VA | size | build uses | low |',
                  '|---|---:|---:|---|---:|---|---|']
        for r in rows:
            lines.append(f"| {r['class']} | {r['object_offset']} | {r['slot']} | {r['retail_va']} | "
                         f"{r['retail_size']} | `{r['built_symbol']}` | {'yes' if r['low_priority'] else ''} |")
    if report['slot_counts']:
        lines += ['', '## slot_count', '', '| class | off | retail | built |', '|---|---:|---:|---:|']
        for r in report['slot_counts']:
            lines.append(f"| {r['class']} | {r['object_offset']} | {r['retail_slots']} | {r['built_slots']} |")
    if report['missing_vtables']:
        lines += ['', '## retail vtables without a built counterpart', '']
        for r in report['missing_vtables']:
            what = 'offset absent' if r['class_built'] else 'class absent'
            lines.append(f"- {r['class']} +{r['object_offset']} {r['retail_vtable']} ({what})")
    return '\n'.join(lines) + '\n'


def latest_build() -> Path | None:
    builds = sorted((ROOT / 'work/game-build').glob('build-*/mcm2.map'),
                    key=lambda p: p.stat().st_mtime)
    return builds[-1].parent if builds else None


def main(argv=None) -> int:
    from mcm2tool.pe import PEImage
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--retail', type=Path, default=DEFAULT_RETAIL)
    parser.add_argument('--build', type=Path, default=None,
                        help='build directory with mcm2.exe and mcm2.map (default: newest)')
    parser.add_argument('--out', type=Path, default=ROOT / 'work/vtable-audit')
    parser.add_argument('--tiny', type=int, default=8)
    args = parser.parse_args(argv)
    build = args.build or latest_build()
    if build is None:
        parser.error('no build directory; run tools/build_game.py first')
    retail_pe = PEImage(args.retail)
    built_pe = PEImage(build / 'mcm2.exe')
    symbols = parse_map((build / 'mcm2.map').read_text(encoding='latin-1'))
    built_purecall = next((va for va, names in symbols.items()
                           if any(n in PURECALL for n in names)), None)
    report = classify(image_vtables(retail_pe), image_vtables(built_pe), symbols,
                      load_bindings(), image_side(retail_pe), image_side(built_pe),
                      find_purecall(retail_pe, built_pe, built_purecall), args.tiny)
    report['build'] = str(build)
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    (args.out / 'report.md').write_text(render(report))
    for key, value in report['summary'].items():
        print(f'{key}: {value}')
    print(f"Report: {args.out / 'report.md'}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
