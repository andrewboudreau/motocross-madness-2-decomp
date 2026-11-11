#!/usr/bin/env python3
"""Lay out one compiled translation unit the way LINK would and compare it with retail.

Per-function matching (tools/match.py, run_calibration.py) compares one COMDAT
at a time. This tool places an object's code and EH sections at retail
addresses, as VC6 LINK does for one object, and compares whole ranges:

  ``.text``     function COMDATs in section-table order, each at its section
                alignment (VC6 /O2 implies /Gy; every function is its own
                16-byte-aligned COMDAT that the compiler already pads with 0x90);
  ``.text$x``   EH unwind funclets and ``$ehhandler`` stubs (associative COMDATs
                of their function). LINK sorts grouped sections by name, so all
                objects' ``.text$x`` follow all ``.text``: laid out separately;
  ``.xdata$x``  the FuncInfo/unwind tables (associative), merged into ``.rdata``.

Gaps that alignment leaves between sections are filled with 0xCC in code
groups (LINK's code fill) and 0x00 in data groups.

A ``.text`` section whose function is bound (see below) outside the span this
object can occupy is skipped as kept elsewhere: a pick-any COMDAT (inline
function, deleting destructor) is kept only by the first object that defines
it, and identical-COMDAT folding (/OPT:ICF) keeps one copy of equal bodies.
Associative sections follow their parent. ``--drop`` skips sections by
function symbol by hand (for example a body folded into an unbound survivor).

Relocations resolve to the laid-out address when their symbol is defined in a
placed section, else through the bindings with the keys resolved_match uses
(``<fn>$ehhandler``, ``<fn>$scopetable``, ``__FILE__:<basename>``,
``__except_list``), else through names every repository bindings file agrees
on. Anything still unresolved is masked and counted: such a range is
"masked_identical", never "strict_identical".

Bindings default to ``<stem>.bindings.json`` next to the source plus every
bindings file that calibration cases and ``targets.json`` register for it;
``--bindings`` adds more. Bindings are reviewed addresses, not identity proof.
``auto`` starts anchor on one bound address each: ``.text`` on the first bound
function, ``.text$x`` on the first bound ``$ehhandler`` stub, ``.xdata$x`` on
the FuncInfo operand of that stub's retail ``mov eax, imm32``. The anchors are
reported; everything else is predicted.

    python tools/unit_layout.py --source src/reconstructed/ArcadeObject.cpp \
        --start auto --text-x-start auto --xdata-start auto

Without VC6, pass a prebuilt ``--obj`` (``--source`` then only selects bindings).
"""
from __future__ import annotations

import argparse
import json
import os
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from mcm2tool.coff import CoffObject  # noqa: E402
from mcm2tool.resolved_match import _source_path_literal  # noqa: E402

IMAGE_SCN_LNK_COMDAT = 0x1000
IMAGE_SCN_LNK_REMOVE = 0x800
COMDAT_PICK_ANY = 2
COMDAT_ASSOCIATIVE = 5
GROUPS = ('.text', '.text$x', '.xdata$x')   # LINK name order within .text, then .rdata
FILL = {'.text': 0xCC, '.text$x': 0xCC, '.xdata$x': 0x00}


def hx(value: int) -> str:
    return f'0x{value:08x}'


def aligned_size(size: int, align: int) -> int:
    return -(-size // align) * align


@dataclass
class Placed:
    index: int
    name: str
    size: int
    align: int
    selection: int | None
    associated: int | None
    functions: list[tuple[str, int, int | None]]  # (name, offset, size)
    va: int | None = None
    status: str = 'pending'
    reason: str = ''


def section_alignment(characteristics: int) -> int:
    code = (characteristics >> 20) & 0xF
    return 1 << (code - 1) if code else 16  # COFF default when unspecified


def comdat_info(obj: CoffObject) -> dict[int, tuple[int, int]]:
    """Section number -> (COMDAT selection, associated section number), from the
    section symbol's auxiliary record (PE/COFF auxiliary format 5)."""
    out = {}
    for sym in obj.symbols:
        if (sym.storage_class == 3 and sym.aux_count and sym.value == 0
                and sym.section_number > 0 and sym.section_number not in out
                and sym.name == obj.section(sym.section_number).name
                and obj.section(sym.section_number).characteristics & IMAGE_SCN_LNK_COMDAT):
            number, selection = struct.unpack_from('<HB', obj.data, obj.sym_ptr + (sym.index + 1) * 18 + 12)
            out[sym.section_number] = (selection, number)
    return out


def layout_sections(obj: CoffObject) -> list[Placed]:
    comdats = comdat_info(obj)
    rows = []
    for sec in obj.sections:
        if sec.name not in GROUPS or sec.characteristics & IMAGE_SCN_LNK_REMOVE:
            continue
        fns = sorted(((s.name, s.value, s.function_size if s.function_size is not None else s.codeview_size)
                      for s in obj.symbols
                      if s.section_number == sec.index and s.type == 0x20 and s.storage_class in (2, 3)),
                     key=lambda f: f[1])
        selection, associated = comdats.get(sec.index, (None, None))
        rows.append(Placed(sec.index, sec.name, sec.raw_size, section_alignment(sec.characteristics),
                           selection, associated if selection == COMDAT_ASSOCIATIVE else None, fns))
    return rows


def load_bindings(paths: list[Path]) -> tuple[dict[str, int], list[str]]:
    bindings, conflicts = {}, []
    for path in paths:
        for name, value in json.loads(Path(path).read_text()).items():
            value = int(value, 0) if isinstance(value, str) else int(value)
            if name in bindings and bindings[name] != value:
                conflicts.append(f'{name}: {hx(bindings[name])} vs {hx(value)} ({path})')
                continue
            bindings[name] = value
    return bindings, conflicts


def repository_bindings(root: Path = ROOT) -> dict[str, int]:
    """Names every src/ bindings file that mentions them binds to the same VA."""
    seen: dict[str, set[int]] = {}
    for path in sorted((root / 'src').rglob('*.bindings.json')):
        for name, value in json.loads(path.read_text()).items():
            seen.setdefault(name, set()).add(int(value, 0) if isinstance(value, str) else int(value))
    return {name: values.pop() for name, values in seen.items() if len(values) == 1}


def default_binding_files(source: Path) -> list[Path]:
    """Bindings registered for this source by calibration cases and targets.json."""
    from tools.rebuild_coverage import load_calibration_cases
    try:
        rel = source.resolve().relative_to(ROOT).as_posix()
    except ValueError:
        rel = source.as_posix()
    files = [source.with_name(source.stem + '.bindings.json')]
    files += [ROOT / case['bindings'] for case in load_calibration_cases()
              if case.get('source') == rel and case.get('bindings')]
    manifest = source.parent / 'targets.json'
    if manifest.is_file():
        data = json.loads(manifest.read_text())
        files += [manifest.parent / entry['bindings']
                  for entry in (data.get('targets', []) if isinstance(data, dict) else data)
                  if entry.get('source') == source.name and entry.get('bindings')]
    unique = []
    for f in files:
        if f.is_file() and f.resolve() not in [u.resolve() for u in unique]:
            unique.append(f)
    return unique


def registered_addresses(source: Path, obj: CoffObject) -> dict[str, int]:
    """Function symbol -> retail VA from the calibration cases and targets.json
    entries registered for this source (targets match by unique substring)."""
    from tools.rebuild_coverage import load_calibration_cases, load_targets
    try:
        rel = source.resolve().relative_to(ROOT).as_posix()
    except ValueError:
        rel = source.as_posix()
    names = {s.name for s in obj.symbols if s.section_number > 0 and s.type == 0x20}
    out = {case['symbol']: int(case['target_va'], 16) for case in load_calibration_cases()
           if case.get('source') == rel and case['symbol'] in names}
    for target in load_targets():
        if target['source'] != rel or not target['symbol']:
            continue
        hits = [n for n in names if target['symbol'] in n]
        if len(hits) == 1:
            out.setdefault(hits[0], target['va'])
    return out


def compile_source(source: Path, out: Path, vc6_root: str) -> list[str]:
    """Compile with the game-build profile and includes; returns the command."""
    spec = json.loads((ROOT / 'config/game_build.json').read_text())
    command = [sys.executable, str(ROOT / 'tools/compile.py'), str(source), '-o', str(out),
               '--compiler', 'vc6', '--vc6-root', vc6_root, '--profile', spec['profile']]
    for include in spec['includes']:
        command += ['--include', str(ROOT / include)]
    env = dict(os.environ, PYTHONPATH=str(ROOT))
    result = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True, encoding='latin-1')
    if result.returncode or not out.is_file():
        raise SystemExit(f'VC6 compile failed:\n{result.stdout}{result.stderr}')
    return command


def containing_function(section: Placed, offset: int) -> str | None:
    best = None
    for name, start, _ in section.functions:
        if start <= offset:
            best = name
    return best


class Layout:
    def __init__(self, obj: CoffObject, bindings: dict[str, int], fallback: dict[str, int] | None = None):
        self.obj, self.bindings, self.fallback = obj, bindings, fallback or {}
        self.sections = layout_sections(obj)
        self.by_index = {s.index: s for s in self.sections}
        self.starts: dict[str, int] = {}
        self.anchors: dict[str, str] = {}
        self.fallback_used: set[str] = set()

    def group(self, name: str) -> list[Placed]:
        return [s for s in self.sections if s.name == name]

    def bound_va(self, section: Placed) -> int | None:
        for name, offset, _ in section.functions:
            if name in self.bindings:
                return self.bindings[name] - offset
        return None

    def place(self, name: str, start: int, drop: set[str] = frozenset()) -> None:
        """Assign addresses to one group. Call for '.text' first: the other
        groups follow the drop decisions of their parent sections."""
        self.starts[name] = start
        sections = self.group(name)
        span_end = start + sum(aligned_size(s.size, s.align) for s in sections)
        cursor = start
        for sec in sections:
            parent = self.by_index.get(sec.associated) if sec.associated else None
            if parent is not None and parent.status not in ('placed', 'pending'):
                sec.status, sec.reason = 'dropped', f'associated with skipped section {parent.index}'
                continue
            if any(fn in drop for fn, _, _ in sec.functions):
                sec.status, sec.reason = 'dropped', '--drop'
                continue
            bound = self.bound_va(sec)
            if name == '.text' and bound is not None and not start <= bound < span_end:
                sec.status = 'elsewhere'
                sec.reason = (f'bound to {hx(bound)}, outside this object\'s span: '
                              + ('pick-any COMDAT kept by another object' if sec.selection == COMDAT_PICK_ANY
                                 else 'folded (ICF) or misbound'))
                continue
            sec.va = -(-cursor // sec.align) * sec.align
            sec.status = 'placed'
            cursor = sec.va + sec.size

    def unplaced_groups(self) -> None:
        for sec in self.sections:
            if sec.status == 'pending':
                sec.status, sec.reason = 'not_laid_out', f'no start address for {sec.name}'

    def symbol_va(self, sym) -> int | None:
        sec = self.by_index.get(sym.section_number)
        if sec is not None and sec.status == 'placed':
            return sec.va + sym.value
        return None

    def resolve(self, section: Placed, rel) -> tuple[int | None, str]:
        record = self.obj.symbol_by_index[rel.symbol_index]
        va = self.symbol_va(record)
        if va is not None:
            return va, 'layout'
        name = record.name
        owner = containing_function(section, rel.virtual_address)
        if record.section_number > 0:
            target = self.obj.section(record.section_number).name
            if target == '.text$x' and owner and f'{owner}$ehhandler' in self.bindings:
                return self.bindings[f'{owner}$ehhandler'], 'binding'
            if name.startswith('$T') and target.startswith('.rdata') and owner and f'{owner}$scopetable' in self.bindings:
                return self.bindings[f'{owner}$scopetable'], 'binding'
            literal = _source_path_literal(self.obj, record) if name.startswith('??_C@') else None
            if literal:
                for key in (f'__FILE__:{literal}', '__FILE__'):
                    if key in self.bindings:
                        return self.bindings[key], 'binding'
        if name == '__except_list':
            return 0, 'binding'
        if name in self.bindings:
            return self.bindings[name], 'binding'
        if name in self.fallback:
            self.fallback_used.add(name)
            return self.fallback[name], 'repository_binding'
        return None, 'unresolved'

    def predicted(self, section: Placed) -> tuple[bytes, bytearray, list[dict]]:
        raw = self.obj.section(section.index)
        data = bytearray(self.obj.data[raw.raw_ptr:raw.raw_ptr + raw.raw_size])
        mask, unresolved = bytearray(len(data)), []
        for rel in self.obj.relocations:
            if rel.section_number != section.index or rel.type == 0:
                continue
            off = rel.virtual_address
            target, _ = self.resolve(section, rel)
            if rel.type not in (0x0006, 0x0014) or target is None:
                mask[off:off + 4] = b'\1' * len(mask[off:off + 4])
                unresolved.append({'offset': off, 'type': rel.type,
                                   'symbol': self.obj.symbol_by_index[rel.symbol_index].name})
                continue
            value = target + struct.unpack_from('<I', data, off)[0]
            if rel.type == 0x0014:
                value -= section.va + off + 4
            struct.pack_into('<I', data, off, value & 0xffffffff)
        return bytes(data), mask, unresolved

    # -- auto anchors
    def auto_text_start(self) -> int | None:
        secs = self.group('.text')
        for i, sec in enumerate(secs):
            bound = self.bound_va(sec)
            if bound is not None and sec.selection != COMDAT_PICK_ANY:
                self.anchors['.text'] = f'{sec.functions[0][0]} bound {hx(bound + sec.functions[0][1])}'
                return bound - sum(aligned_size(s.size, s.align) for s in secs[:i])
        return None

    def _stubs(self):
        """(.text$x section, stub offset, owner function) for every placed parent."""
        for sec in self.group('.text$x'):
            parent = self.by_index.get(sec.associated)
            if parent is None or parent.status != 'placed' or not parent.functions:
                continue
            labels = [s.value for s in self.obj.symbols if s.section_number == sec.index and s.storage_class == 6]
            if labels:  # the stub `mov eax, FuncInfo; jmp handler` is the last label
                yield sec, max(labels), parent.functions[0][0]

    def auto_text_x_start(self) -> int | None:
        kept = [s for s in self.group('.text$x') if s.status == 'pending']
        for sec, stub, owner in self._stubs():
            key = f'{owner}$ehhandler'
            if key in self.bindings and sec in kept:
                self.anchors['.text$x'] = f'{key} {hx(self.bindings[key])}'
                before = kept[:kept.index(sec)]
                return self.bindings[key] - stub - sum(aligned_size(s.size, s.align) for s in before)
        return None

    def auto_xdata_start(self, retail_bytes) -> int | None:
        kept = [s for s in self.group('.xdata$x') if s.status == 'pending']
        for sec, stub, owner in self._stubs():
            if sec.status != 'placed':
                continue
            rels = [r for r in self.obj.relocations if r.section_number == sec.index
                    and r.virtual_address == stub + 1 and r.type == 0x0006]
            if len(rels) != 1:
                continue
            table = self.obj.symbol_by_index[rels[0].symbol_index]
            target = self.by_index.get(table.section_number)
            code = retail_bytes(sec.va + stub, 5)
            if target not in kept or code[0] != 0xB8:
                continue
            retail_va = struct.unpack_from('<I', code, 1)[0]
            self.anchors['.xdata$x'] = f'{table.name} = retail operand {hx(retail_va)} of the stub at {hx(sec.va + stub)}'
            before = kept[:kept.index(target)]
            return retail_va - table.value - sum(aligned_size(s.size, s.align) for s in before)
        return None


def compare_layout(layout: Layout, retail_bytes) -> dict:
    """retail_bytes(va, size) -> bytes. Compares every placed section and the
    fill between consecutive placed sections of each group."""
    rows, groups = [], {}
    for sec in layout.sections:
        if sec.status != 'placed':
            rows.append({'section': sec.index, 'name': sec.name, 'status': sec.status, 'reason': sec.reason,
                         'functions': [f[0] for f in sec.functions]})
            continue
        data, mask, unresolved = layout.predicted(sec)
        retail = retail_bytes(sec.va, len(data))
        diffs = [i for i in range(len(data)) if not mask[i] and (i >= len(retail) or retail[i] != data[i])]
        functions = []
        for name, offset, size in sec.functions:
            bound = layout.bindings.get(name)
            end = offset + (size if size is not None else len(data) - offset)
            functions.append({'symbol': name, 'predicted_va': hx(sec.va + offset),
                              'bound_va': hx(bound) if bound is not None else None,
                              'offset_agrees': None if bound is None else bound == sec.va + offset,
                              'size': size, 'bytes_equal': not any(offset <= d < end for d in diffs),
                              'masked_bytes': sum(mask[offset:end])})
        rows.append({'section': sec.index, 'name': sec.name, 'status': 'placed', 'selection': sec.selection,
                     'va': hx(sec.va), 'size': sec.size, 'align': sec.align, 'functions': functions,
                     'masked_bytes': sum(mask), 'unresolved': unresolved, 'differing_bytes': len(diffs),
                     'first_differences': [hx(sec.va + d) for d in diffs[:8]], 'identical': not diffs})
        groups.setdefault(sec.name, []).append((sec, len(diffs), sum(mask)))
    group_rows = {}
    for name in GROUPS:
        placed = sorted(groups.get(name, []), key=lambda p: p[0].va)
        if not placed:
            continue
        fill_checked = fill_bad = 0
        for (a, _, _), (b, _, _) in zip(placed, placed[1:]):
            if b.va > a.va + a.size:
                got = retail_bytes(a.va + a.size, b.va - a.va - a.size)
                fill_checked += len(got)
                fill_bad += sum(x != FILL[name] for x in got)
        first, last = placed[0][0], placed[-1][0]
        differing = sum(p[1] for p in placed) + fill_bad
        masked = sum(p[2] for p in placed)
        group_rows[name] = {
            'start': hx(first.va), 'end': hx(last.va + last.size), 'bytes': last.va + last.size - first.va,
            'sections': len(placed), 'anchor': layout.anchors.get(name),
            'fill_bytes_checked': fill_checked, 'fill_bytes_differing': fill_bad,
            'differing_bytes': differing, 'masked_bytes': masked,
            'verdict': ('strict_identical' if not differing and not masked else
                        'masked_identical' if not differing else 'differs')}
    functions = [f for r in rows if r['status'] == 'placed' for f in r['functions']]
    return {'groups': group_rows, 'sections': rows,
            'functions_placed': len(functions),
            'functions_offset_agree': sum(1 for f in functions if f['offset_agrees']),
            'functions_without_binding': sum(1 for f in functions if f['offset_agrees'] is None),
            'functions_offset_disagree': [f['symbol'] for f in functions if f['offset_agrees'] is False],
            'repository_bindings_used': sorted(layout.fallback_used)}


def run(obj: CoffObject, bindings: dict[str, int], retail_bytes, start, text_x_start=None,
        xdata_start=None, drop=frozenset(), fallback=None) -> dict:
    """Place and compare. Starts are ints, 'auto' or None (group not laid out)."""
    layout = Layout(obj, bindings, fallback)
    if start == 'auto':
        start = layout.auto_text_start()
        if start is None:
            raise ValueError('cannot derive the .text start: no bound function')
    layout.place('.text', start, set(drop))
    for name, value, auto in (('.text$x', text_x_start, layout.auto_text_x_start),
                              ('.xdata$x', xdata_start, lambda: layout.auto_xdata_start(retail_bytes))):
        value = auto() if value == 'auto' else value
        if value is not None:
            layout.place(name, value)
    layout.unplaced_groups()
    return compare_layout(layout, retail_bytes)


def print_report(report: dict) -> None:
    for name, g in report['groups'].items():
        anchor = f" [anchor: {g['anchor']}]" if g['anchor'] else ''
        print(f"{name}: {g['start']}..{g['end']} ({g['bytes']} bytes, {g['sections']} sections) "
              f"{g['verdict']}: {g['differing_bytes']} differing, {g['masked_bytes']} masked, "
              f"fill {g['fill_bytes_checked'] - g['fill_bytes_differing']}/{g['fill_bytes_checked']}{anchor}")
    for row in report['sections']:
        if row['status'] != 'placed':
            print(f"  [{row['status']}] {row['name']} #{row['section']} {', '.join(row['functions'])}: {row['reason']}")
            continue
        names = ', '.join(f['symbol'] + ('' if f['offset_agrees'] in (True, None) else f" (bound {f['bound_va']})")
                          for f in row['functions']) or '(labels/data)'
        extra = f" masked={row['masked_bytes']}" if row['masked_bytes'] else ''
        if row['differing_bytes']:
            extra += f" diff={row['differing_bytes']} at {','.join(row['first_differences'][:3])}"
        print(f"  {'ok  ' if row['identical'] else 'DIFF'} {row['va']} {row['name']:<8} +{row['size']:#x} {names}{extra}")
    print(f"functions: {report['functions_placed']} placed, {report['functions_offset_agree']} at their bound VA, "
          f"{report['functions_without_binding']} unbound, disagree: {report['functions_offset_disagree'] or 'none'}")
    if report['repository_bindings_used']:
        print('repository-wide bindings used:', ', '.join(report['repository_bindings_used']))


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--source', type=Path, help='source file; compiled with the game-build profile unless --obj')
    ap.add_argument('--obj', type=Path, help='prebuilt COFF object')
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE', str(ROOT / 'work/game/mcm2.exe')))
    ap.add_argument('--start', required=True, help='retail VA of the first .text section, or "auto"')
    ap.add_argument('--text-x-start', help='retail VA of the first .text$x section, or "auto"')
    ap.add_argument('--xdata-start', help='retail VA of the first .xdata$x section, or "auto"')
    ap.add_argument('--bindings', action='append', type=Path, default=[])
    ap.add_argument('--no-default-bindings', action='store_true')
    ap.add_argument('--no-repository-bindings', action='store_true')
    ap.add_argument('--drop', action='append', default=[], help='function symbol whose section to skip')
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--json', type=Path, help='write the full report here')
    args = ap.parse_args(argv)
    if not args.obj and not args.source:
        ap.error('give --source and/or --obj')
    from mcm2tool.pe import PEImage
    pe = PEImage(args.exe)
    command = None
    if args.obj:
        obj_path = args.obj
    else:
        if not args.vc6_root:
            ap.error('compiling needs VC6_ROOT or --vc6-root (or pass --obj)')
        obj_path = Path(tempfile.mkdtemp(prefix='unit-layout-')) / (args.source.stem + '.obj')
        command = compile_source(args.source, obj_path, args.vc6_root)
    files = list(args.bindings)
    if args.source and not args.no_default_bindings:
        files = default_binding_files(args.source) + files
    bindings, conflicts = load_bindings(files)
    obj = CoffObject(obj_path)
    if args.source and not args.no_default_bindings:
        for name, va in registered_addresses(args.source, obj).items():
            if name in bindings and bindings[name] != va:
                conflicts.append(f'{name}: binding {hx(bindings[name])} vs registered case {hx(va)}')
            bindings.setdefault(name, va)
    parse = lambda v: v if v in (None, 'auto') else int(v, 0)
    try:
        report = run(obj, bindings, pe.bytes_at_va, parse(args.start),
                     parse(args.text_x_start), parse(args.xdata_start), set(args.drop),
                     None if args.no_repository_bindings else repository_bindings())
    except ValueError as exc:
        raise SystemExit(str(exc))
    report.update({'object': str(obj_path), 'compile_command': command,
                   'source': str(args.source) if args.source else None,
                   'bindings_files': [str(f) for f in files], 'binding_conflicts': conflicts})
    print_report(report)
    for c in conflicts:
        print('binding conflict:', c)
    if args.json:
        args.json.parent.mkdir(parents=True, exist_ok=True)
        args.json.write_text(json.dumps(report, indent=2) + '\n')
    ok = all(g['differing_bytes'] == 0 for g in report['groups'].values()) and not report['functions_offset_disagree']
    return 0 if ok else 1


if __name__ == '__main__':
    raise SystemExit(main())
