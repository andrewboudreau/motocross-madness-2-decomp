#!/usr/bin/env python3
"""Rebuild-readiness audit of retail .text (code side).

Attributes every byte of the retail ``.text`` section to one category and
reports what is still missing before the whole executable could be rebuilt
from compiled source plus the original libraries:

  exact_src / exact_samples   strict VC6 matches (calibration cases and every
                              ``*targets.json`` entry), split by source tree
  library_crt_atlas           VC6 LIBCMT bodies admitted by the CRT atlas
  library_import_thunk        ``jmp [IAT]`` thunks (import libraries)
  library_data                library objects matched by bytes (DINPUT.LIB)
  compiler_eh_text_x          ``$ehhandler`` stubs and unwind funclets at the end
                              of .text (emitted with the owning game functions)
  inline_asm_excluded         functions whose retail body needs ``__asm``
                              (binary scan + documented cases), not exact
  near_miss                   registered partials / near misses
  library_span_unlabeled      bytes inside the LIBCMT span the atlas leaves out
  library_unknown             bytes between the CRT span and the EH tail
  padding                     int3/nop alignment runs
  unattributed                everything else

Verdicts come from the VC6 result files written by ``--run-vc6`` (calibration
and physics runners). Without them the declared ``expect`` labels are used and
the report says so. Inputs: the hash-pinned retail EXE, ``analysis/*.json``,
the CRT atlas (``make vc6-crt-atlas``) and the repository tree. Output goes to
``work/rebuild-coverage/`` (ignored).

    python3 tools/with_private_env.py -- python3 tools/rebuild_coverage.py --run-vc6
"""
from __future__ import annotations

import argparse
import ast
import bisect
import json
import os
import re
import struct
import subprocess
import sys
import tempfile
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

DEFAULT_OUT = ROOT / 'work' / 'rebuild-coverage'

# Highest priority first: a byte keeps the first category that claims it.
CATEGORIES = [
    'exact_src', 'exact_samples', 'library_crt_atlas', 'library_import_thunk',
    'library_data', 'compiler_eh_text_x', 'inline_asm_excluded', 'near_miss',
    'library_span_unlabeled', 'library_unknown', 'padding', 'unattributed',
]
PRIORITY = {name: index for index, name in enumerate(CATEGORIES)}
PAD_BYTES = {0xCC, 0x90}
ASM_KEYWORDS = re.compile(r'inline.?asm|inline assembly|__asm', re.I)
VA_RE = re.compile(r'0x(?:00)?([45][0-9a-fA-F]{5})\b')


def hx(value: int) -> str:
    return f'0x{value:08x}'


def parse_va(value) -> int:
    return int(value, 0) if isinstance(value, str) else int(value)


# --------------------------------------------------------------------------
# Repository loaders (no executable needed)

def load_calibration_cases(root: Path = ROOT) -> list[dict]:
    """Read CASES from tools/run_calibration.py without importing it."""
    tree = ast.parse((root / 'tools' / 'run_calibration.py').read_text(encoding='utf-8'))
    for node in tree.body:
        if isinstance(node, ast.Assign) and any(
                isinstance(t, ast.Name) and t.id == 'CASES' for t in node.targets):
            return ast.literal_eval(node.value)
    raise ValueError('CASES not found in tools/run_calibration.py')


def source_tree(path: str) -> str:
    parts = Path(path).as_posix().split('/')
    return 'src' if parts and parts[0] == 'src' else 'samples'


def load_targets(root: Path = ROOT) -> list[dict]:
    """Every *targets.json entry under src/ and samples/ with its manifest."""
    rows = []
    for top in ('src', 'samples'):
        for manifest in sorted((root / top).rglob('*targets.json')):
            data = json.loads(manifest.read_text(encoding='utf-8'))
            if isinstance(data, dict):
                data = data.get('targets', [])
            rel_dir = manifest.parent.relative_to(root)
            for entry in data:
                if 'target_va' not in entry or 'target_size' not in entry:
                    continue
                expect = entry.get('expect')
                if expect is None:
                    expect = 'exact' if 'exact' in str(entry.get('status', 'exact')) else 'partial'
                source = entry.get('source')
                source_path = os.path.normpath(rel_dir / source).replace(os.sep, '/') if source else rel_dir.as_posix()
                rows.append({
                    'manifest': manifest.relative_to(root).as_posix(),
                    'source': source_path,
                    'symbol': entry.get('candidate_symbol_contains', ''),
                    'va': parse_va(entry['target_va']),
                    'size': int(entry['target_size']),
                    'expect': expect,
                })
    return rows


NEAR_MISS_ROW = re.compile(
    r'^\|\s*(?P<tu>[^|]+?)\s*\|\s*`(?P<va>0x[0-9a-fA-F]+)`\s*\|\s*(?P<score>[^|]*?)\s*\|'
    r'\s*(?P<cls>[abc])\s*\|')


def parse_near_miss_index(text: str) -> list[dict]:
    rows = []
    for line in text.splitlines():
        m = NEAR_MISS_ROW.match(line)
        if not m:
            continue
        score = re.match(r'\s*(\d+)\s*/\s*(\d+)', m['score'])
        rows.append({
            'tu': m['tu'], 'va': int(m['va'], 16), 'class': m['cls'],
            'score_matching': int(score[1]) if score else None,
            'score_compared': int(score[2]) if score else None,
        })
    return rows


DOC_EXTENT = re.compile(r'extent[^`\n]{0,4}`(0x[0-9a-fA-F]+)\.\.(0x[0-9a-fA-F]+)`', re.I)


def parse_doc_extents(root: Path = ROOT) -> list[dict]:
    """`Extent: `a..b`` statements in docs/*.md and src/krusty2 READMEs."""
    rows = []
    files = sorted((root / 'docs').glob('*.md')) + sorted((root / 'src').rglob('README.md'))
    for path in files:
        if path.name == 'REBUILD_GAPS.md':
            continue
        for lineno, line in enumerate(path.read_text(encoding='utf-8').splitlines(), 1):
            for m in DOC_EXTENT.finditer(line):
                start, end = int(m[1], 16), int(m[2], 16)
                if end <= start:
                    continue
                if end & 1:  # inclusive end such as ..0x004238bb
                    end += 1
                rows.append({'doc': f'{path.relative_to(root).as_posix()}:{lineno}',
                             'start': start, 'end': end})
    return rows


CLASS_DEF = re.compile(r'^\s*(?:template\s*<[^>]*>\s*)?(?:class|struct)\s+(?:__declspec\([^)]*\)\s*)?'
                       r'(\w+)\s*(?:final\s*)?(?::[^;{]*)?\{', re.M)


def class_definitions(text: str) -> set[str]:
    return set(CLASS_DEF.findall(text))


def scan_class_definitions(root: Path = ROOT) -> dict[str, dict[str, list[str]]]:
    found: dict[str, dict[str, list[str]]] = {'src': defaultdict(list), 'samples': defaultdict(list)}
    for top in ('src', 'samples'):
        for path in sorted((root / top).rglob('*')):
            if path.suffix.lower() not in ('.h', '.hpp', '.cpp', '.inl'):
                continue
            for name in class_definitions(path.read_text(encoding='utf-8', errors='replace')):
                found[top][name].append(path.relative_to(root).as_posix())
    return found


def doc_asm_mentions(root: Path = ROOT, reach: int = 60) -> list[dict]:
    """For each inline-asm keyword, the nearest VA within ``reach`` characters.

    The previous line is prepended so that a statement wrapped across lines
    still pairs the VA with its keyword. Instruction-level evidence (fistp,
    rdtsc, fs: reads) comes from the binary scan, not from this text search.
    """
    rows, seen = [], set()
    paths = [p for top in ('docs', 'src', 'samples') for p in sorted((root / top).rglob('*'))
             if p.suffix.lower() in ('.md', '.cpp', '.h', '.json') and p.name != 'REBUILD_GAPS.md']
    for path in paths:
        lines = path.read_text(encoding='utf-8', errors='replace').splitlines()
        for index, line in enumerate(lines):
            if not ASM_KEYWORDS.search(line):
                continue
            before = ' '.join(lines[max(0, index - 1):index])
            text = f'{before} {line}'
            offset = len(before) + 1
            vas = [(m.start(), int(m[1], 16)) for m in VA_RE.finditer(text)]
            for k in ASM_KEYWORDS.finditer(line):
                near = [(abs(pos - (offset + k.start())), va) for pos, va in vas]
                near = [row for row in near if row[0] <= reach]
                if not near:
                    continue
                va = min(near)[1]
                where = f'{path.relative_to(root).as_posix()}:{index + 1}'
                if (va, where) in seen:
                    continue
                seen.add((va, where))
                rows.append({'va': va, 'where': where, 'text': line.strip()[:160]})
    return rows


# --------------------------------------------------------------------------
# Interval painting (pure; unit tested)

class CoverageMap:
    """One category code per .text byte; higher priority wins."""

    def __init__(self, start: int, end: int):
        self.start, self.end = start, end
        self.codes = bytearray([PRIORITY['unattributed']]) * (end - start)

    def paint(self, start: int, end: int, category: str) -> None:
        code = PRIORITY[category]
        lo, hi = max(start, self.start) - self.start, min(end, self.end) - self.start
        codes = self.codes
        for i in range(lo, hi):
            if code < codes[i]:
                codes[i] = code

    def runs(self):
        """(start, end, category) maximal runs."""
        out = []
        codes, n = self.codes, len(self.codes)
        i = 0
        while i < n:
            c = codes[i]
            j = i + 1
            while j < n and codes[j] == c:
                j += 1
            out.append((self.start + i, self.start + j, CATEGORIES[c]))
            i = j
        return out

    def totals(self) -> dict[str, int]:
        counts = Counter(self.codes)
        return {name: counts.get(PRIORITY[name], 0) for name in CATEGORIES}


TERMINATORS = {'ret', 'retf', 'jmp', 'int3', 'nop'}


def capstone_decoder():
    """decode(data, va) -> (size, mnemonic) or None, for one instruction."""
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

    def decode(chunk: bytes, va: int):
        for ins in md.disasm(chunk[:16], va, 1):
            return ins.size, ins.mnemonic
        return None
    return decode


def padding_runs(data: bytes, base: int, start: int, end: int, decode) -> list[tuple[int, int]]:
    """int3/nop alignment runs inside [start,end).

    Decodes linearly from ``start`` (a function start or the end of a
    registered extent). A run of 0xcc/0x90 counts as padding only when it
    starts on an instruction boundary right after an unconditional
    terminator (ret/jmp/int3/nop, or ``start`` itself) and ends on a 16-byte
    boundary or at ``end``. Displacement bytes that happen to be 0x90/0xcc are
    therefore never taken for padding.
    """
    out = []
    pc, after_terminator = start, True
    while pc < end:
        if after_terminator and data[pc - base] in PAD_BYTES:
            j = pc
            while j < end and data[j - base] in PAD_BYTES:
                j += 1
            if j % 16 == 0 or j == end:
                out.append((pc, j))
                pc = j
                continue
        decoded = decode(data[pc - base:end - base], pc)
        if decoded is None:
            pc, after_terminator = pc + 1, False
            continue
        size, mnemonic = decoded
        pc += size
        after_terminator = mnemonic in TERMINATORS
    return sorted(set(out) | set(jump_table_padding(data, base, start, end)))


def jump_table_padding(data: bytes, base: int, start: int, end: int) -> list[tuple[int, int]]:
    """Padding after a switch table, where linear decoding is out of step.

    VC6 places a function's dword jump tables after its code; a short 0xcc/0x90
    run ending on a 16-byte boundary whose preceding dword is an address in the
    64 KiB before it is the alignment after such a table.
    """
    out = []
    boundary = (start + 15) & ~15
    while boundary <= end:
        i = boundary
        while i > start and boundary - i < 15 and data[i - 1 - base] in PAD_BYTES:
            i -= 1
        if i < boundary and i - 4 >= start:
            target = struct.unpack_from('<I', data, i - 4 - base)[0]
            if i - 0x10000 <= target < i:
                out.append((i, boundary))
        boundary += 16
    return out


def split_chunks(start: int, end: int, pads: list[tuple[int, int]]) -> list[tuple[int, int]]:
    chunks, cursor = [], start
    for a, b in sorted(pads):
        if a > cursor:
            chunks.append((cursor, a))
        cursor = max(cursor, b)
    if cursor < end:
        chunks.append((cursor, end))
    return chunks


def nearest_sources(va: int, xrefs: list[tuple[int, str]], count: int = 3) -> list[dict]:
    """Same ordering as tools/nearest_source.py: absolute distance to a __FILE__ xref."""
    rows = sorted((abs(x - va), x - va, x, p) for x, p in xrefs)[:count]
    return [{'distance': d, 'delta': delta, 'xref_va': hx(x), 'path': p} for d, delta, x, p in rows]


def basename(path: str) -> str:
    return re.split(r'[\\/]', path)[-1]


def order_inversions(names: list[str]) -> int:
    keys = [n.lower() for n in names]
    return sum(1 for a, b in zip(keys, keys[1:]) if a > b)


# --------------------------------------------------------------------------
# Executable-dependent detection

def text_section(pe):
    for s in pe.sections:
        if s.name == '.text':
            start = pe.image_base + s.virtual_address
            data = pe.data[s.raw_offset:s.raw_offset + s.virtual_size]
            return start, start + s.virtual_size, data
    raise SystemExit('.text not found')


def find_import_thunks(pe, data: bytes, base: int) -> list[dict]:
    from mcm2tool.provenance import import_records
    iat = {int(r['iat_va'], 16): r for r in import_records(pe)}
    out = []
    pos = data.find(b'\xff\x25')
    while pos >= 0:
        if pos + 6 <= len(data):
            slot = struct.unpack_from('<I', data, pos + 2)[0]
            if slot in iat:
                r = iat[slot]
                out.append({'va': base + pos, 'size': 6, 'module': r['module'],
                            'name': r['name'] or f"ordinal:{r['ordinal']}"})
        pos = data.find(b'\xff\x25', pos + 1)
    return out


def find_eh_stubs(data: bytes, base: int, handler_va: int | None) -> tuple[list[int], int | None]:
    """`mov eax, FuncInfo ; jmp ___CxxFrameHandler` stubs and the handler VA."""
    pos = data.find(b'\xb8')
    candidates = Counter()
    found = []
    while pos >= 0:
        if pos + 10 <= len(data) and data[pos + 5] == 0xE9:
            target = base + pos + 10 + struct.unpack_from('<i', data, pos + 6)[0]
            found.append((base + pos, target))
            candidates[target] += 1
        pos = data.find(b'\xb8', pos + 1)
    if handler_va is None and candidates:
        handler_va = candidates.most_common(1)[0][0]
    hits = [va for va, target in found if target == handler_va]
    return hits, handler_va


def scan_library_data(lib: Path, data: bytes, base: int) -> list[dict]:
    """Single-hit byte matches of any defined symbol of ``lib`` inside .text."""
    from mcm2tool.coff import CoffObject, relocation_mask
    from mcm2tool.coff_archive import read_archive
    from mcm2tool.crt_atlas import longest_unmasked_run, masked_equal
    out = []
    with tempfile.TemporaryDirectory(prefix='mcm2-rebuild-') as td:
        tmp = Path(td) / 'member.obj'
        for member in read_archive(lib):
            tmp.write_bytes(member.data)
            try:
                obj = CoffObject(tmp)
            except Exception:
                continue
            if obj.machine != 0x14C:
                continue
            for sym in obj.symbols:
                if sym.section_number <= 0 or sym.storage_class not in (2, 3) or sym.name.startswith('.'):
                    continue
                try:
                    raw, _, rel = obj.symbol_extent(sym)
                    mask = relocation_mask(obj, sym, len(raw), rel)
                except Exception:
                    continue
                if len(raw) - sum(mask) < 64:
                    continue
                a0, alen = longest_unmasked_run(mask)
                anchor = raw[a0:a0 + alen]
                hits, p = [], data.find(anchor)
                while p >= 0:
                    s = p - a0
                    if s >= 0 and masked_equal(raw, data[s:s + len(raw)], mask):
                        hits.append(s)
                    p = data.find(anchor, p + 1)
                if len(hits) == 1:
                    out.append({'library': lib.name, 'member': member.name, 'symbol': sym.name,
                                'va': base + hits[0], 'size': len(raw)})
    return out


def asm_scan(data: bytes, base: int, start: int, end: int) -> list[dict]:
    """Instructions VC6 does not emit from plain C++ at /O2 without /QIfist."""
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    hits, pc = [], start
    while pc < end:
        progressed = False
        for ins in md.disasm(data[pc - base:end - base], pc):
            progressed = True
            pc = ins.address + ins.size
            m = ins.mnemonic
            if m in ('rdtsc', 'fistp', 'fist', 'fldcw', 'fnstcw', 'fstcw') or (
                    m == 'mov' and re.search(r'fs:\[(?!0\])(?:0x)?[0-9a-f]+\]', ins.op_str)):
                hits.append({'va': hx(ins.address), 'insn': f'{m} {ins.op_str}'.strip()})
        if not progressed:
            pc += 1
    return hits


def run_vc6(out: Path, exe: str, vc6_root: str) -> None:
    out.mkdir(parents=True, exist_ok=True)
    env = {**os.environ, 'PYTHONPATH': str(ROOT)}
    with (out / 'calibration.json').open('w') as fh:
        subprocess.run([sys.executable, 'tools/run_calibration.py', '--compiler', 'vc6',
                        '--profile', 'vc6_o2_mt', '--vc6-root', vc6_root, '--exe', exe,
                        '--jobs', str(os.cpu_count() or 4)], cwd=ROOT, env=env, stdout=fh, check=True)
    subprocess.run([sys.executable, 'tools/run_physics_samples.py', '--exe', exe, '--vc6-root', vc6_root,
                    '--profile', 'vc6_o2_mt', '--out', str(out / 'physics-objs'), '--strict',
                    '--json-out', str(out / 'physics.json')], cwd=ROOT, env=env,
                   stdout=subprocess.DEVNULL, check=False)


# --------------------------------------------------------------------------

def build(args) -> dict:
    from mcm2tool.pe import PEImage
    pe = PEImage(args.exe)
    t_start, t_end, data = text_section(pe)
    cov = CoverageMap(t_start, t_end)
    decode = capstone_decoder()
    warnings = []

    # --- verdicts
    cal_rows = None
    if args.calibration_json.is_file():
        cal_rows = json.loads(args.calibration_json.read_text())
    else:
        warnings.append('no calibration results: calibration cases use their declared label (exact)')
    phys_rows = None
    if args.physics_json.is_file():
        phys_rows = json.loads(args.physics_json.read_text())
    else:
        warnings.append('no physics results: targets.json entries use their declared expect')

    functions = []  # every registered extent with category
    cal_verdict = {}
    if cal_rows is not None:
        for r in cal_rows:
            ok = bool(r.get('result', {}).get('strict_exact'))
            key = (int(r['target_va'], 16), int(r['target_size']), r['source'])
            cal_verdict[key] = cal_verdict.get(key, False) or ok
    stale = 0
    for c in load_calibration_cases():
        va, size = int(c['target_va'], 16), int(c['target_size'])
        key = (va, size, c['source'])
        if cal_rows is not None and key not in cal_verdict:
            stale += 1
        ok = cal_verdict.get(key, True)  # no verdict: declared label (exact)
        tree = source_tree(c['source'])
        functions.append({'va': va, 'size': size, 'source': c['source'], 'origin': 'calibration',
                          'symbol': c['symbol'], 'category': f'exact_{tree}' if ok else 'near_miss'})
    if stale:
        warnings.append(f'{stale} calibration cases have no verdict in {args.calibration_json}; '
                        'rerun with --run-vc6')
    phys_verdict = {}
    if phys_rows is not None:
        for r in phys_rows:
            phys_verdict[(int(r['target_va'], 16), int(r['target_size']), r['source'])] = bool(r.get('strict_exact'))
    for t in load_targets():
        declared = t['expect'] in ('exact', 'masked')
        ok = phys_verdict.get((t['va'], t['size'], t['source']), declared)
        tree = source_tree(t['source'])
        functions.append({'va': t['va'], 'size': t['size'], 'source': t['source'], 'origin': t['manifest'],
                          'symbol': t['symbol'], 'category': f'exact_{tree}' if ok else 'near_miss'})
    registered_starts = sorted({f['va'] for f in functions})
    sizes_by_va = {}
    for f in functions:
        sizes_by_va.setdefault(f['va'], f['size'])

    nm_rows = parse_near_miss_index((ROOT / 'docs' / 'NEAR_MISS_INDEX.md').read_text(encoding='utf-8'))
    for row in nm_rows:
        va = row['va']
        if va in sizes_by_va:
            size, src = sizes_by_va[va], 'registered'
        else:
            # Extent: from the VA to the first alignment padding, bounded by the
            # next registered start; the score denominator is kept for review.
            k = bisect.bisect_right(registered_starts, va)
            limit = min(registered_starts[k] if k < len(registered_starts) else t_end, va + 0x10000)
            pads = [a for a, _ in padding_runs(data, t_start, va, limit, decode) if a > va]
            size, src = (pads[0] if pads else limit) - va, 'padding_delimited'
        functions.append({'va': va, 'size': size, 'source': row['tu'], 'origin': 'docs/NEAR_MISS_INDEX.md',
                          'symbol': '', 'category': 'near_miss', 'size_source': src,
                          'score_compared': row['score_compared']})

    # --- library / compiler-generated regions
    atlas = []
    if args.crt_atlas.is_file():
        atlas = json.loads(args.crt_atlas.read_text())['libcmt']['matches']
    else:
        warnings.append(f'no CRT atlas at {args.crt_atlas}; run make vc6-crt-atlas')
    for r in atlas:
        cov.paint(r['target_va'], r['target_va'] + r['size'], 'library_crt_atlas')
    crt_span = (min(r['target_va'] for r in atlas), max(r['target_va'] + r['size'] for r in atlas)) if atlas else None
    handler = next((r['target_va'] for r in atlas if r['symbol'] == '___CxxFrameHandler'), None)

    thunks = find_import_thunks(pe, data, t_start)
    for th in thunks:
        cov.paint(th['va'], th['va'] + 6, 'library_import_thunk')
    stubs, handler = find_eh_stubs(data, t_start, handler)

    game_max = max(f['va'] + f['size'] for f in functions if f['va'] < (crt_span[0] if crt_span else t_end))
    after_game = [th['va'] for th in thunks if th['va'] >= game_max]
    game_end = min(after_game) if after_game else game_max
    tail_stubs = [s for s in stubs if s >= (crt_span[1] if crt_span else game_end)]
    eh_start = min(tail_stubs) if tail_stubs else t_end
    cov.paint(eh_start, t_end, 'compiler_eh_text_x')

    lib_data = []
    if args.vc6_root:
        from mcm2tool.toolchain import find_child_ci, infer_vc98_root
        lib_dir = find_child_ci(infer_vc98_root(Path(args.vc6_root)), 'Lib')
        for name in args.data_libs:
            lib = find_child_ci(lib_dir, name) if lib_dir else None
            if lib:
                lib_data += [r for r in scan_library_data(lib, data, t_start) if game_end <= r['va'] < eh_start]
    else:
        warnings.append('VC6_ROOT not set: library data objects (DINPUT.LIB) not scanned')
    for r in lib_data:
        cov.paint(r['va'], r['va'] + r['size'], 'library_data')
    if crt_span:
        cov.paint(crt_span[0], crt_span[1], 'library_span_unlabeled')
        cov.paint(crt_span[1], eh_start, 'library_unknown')

    # --- registered functions
    for f in functions:
        cov.paint(f['va'], f['va'] + f['size'], f['category'])

    # --- source xrefs
    xrefs = []
    for r in json.loads((ROOT / 'analysis' / 'source_xrefs.json').read_text()):
        for q in r.get('text_xrefs', []):
            xrefs.append((parse_va(q), r['path']))

    # --- inline asm: scan registered extents and unregistered chunks of game code
    scan_ranges = {}
    for f in functions:
        if f['va'] < game_end:
            scan_ranges.setdefault(f['va'], (f['va'] + f['size'], f['category'], 'registered'))
    for s, e, cat in cov.runs():
        if cat == 'unattributed' and s < game_end:
            for a, b in split_chunks(s, min(e, game_end), padding_runs(data, t_start, s, min(e, game_end), decode)):
                scan_ranges.setdefault(a, (b, cat, 'padding_delimited_chunk'))
    asm_rows = {}
    for va, (end, cat, extent_source) in sorted(scan_ranges.items()):
        hits = asm_scan(data, t_start, va, end)
        if hits:
            asm_rows[va] = {'va': hx(va), 'size': end - va, 'category_before': cat,
                            'extent_source': extent_source, 'scan_hits': hits, 'doc_mentions': []}
    starts_sorted = sorted(scan_ranges)
    for m in doc_asm_mentions():
        k = bisect.bisect_right(starts_sorted, m['va']) - 1
        if k < 0:
            continue
        va = starts_sorted[k]
        end, cat, extent_source = scan_ranges[va]
        if not va <= m['va'] < end:
            continue
        row = asm_rows.setdefault(va, {'va': hx(va), 'size': end - va, 'category_before': cat,
                                       'extent_source': extent_source, 'scan_hits': [], 'doc_mentions': []})
        if m['where'] not in [d['where'] for d in row['doc_mentions']]:
            row['doc_mentions'].append({'where': m['where'], 'mentioned_va': hx(m['va']), 'text': m['text']})
    inline_asm = []
    for va, row in sorted(asm_rows.items()):
        if row['category_before'].startswith('exact_') and not row['scan_hits']:
            continue
        kinds = sorted({h['insn'].split()[0] for h in row['scan_hits']})
        row['scan_kinds'] = kinds
        strong = bool(set(kinds) & {'rdtsc', 'fistp', 'fist', 'fldcw', 'fnstcw', 'fstcw'}) or any(
            'fs:[' in h['insn'] for h in row['scan_hits'])
        row['evidence'] = ('binary+docs' if strong and row['doc_mentions'] else
                           'binary' if strong else 'docs')
        row['nearest_source'] = nearest_sources(va, xrefs, 1)
        if row['category_before'].startswith('exact_'):
            row['status'] = 'strict exact despite flagged instruction'
        else:
            row['status'] = 'excluded'
            cov.paint(va, va + row['size'], 'inline_asm_excluded')
        inline_asm.append(row)

    # --- padding and unattributed chunks
    unattributed = []
    for s, e, cat in cov.runs():
        if cat != 'unattributed':
            continue
        pads = padding_runs(data, t_start, s, e, decode)
        for a, b in pads:
            cov.paint(a, b, 'padding')
    for s, e, cat in cov.runs():
        if cat == 'unattributed':
            unattributed.append({'start': hx(s), 'end': hx(e), 'size': e - s,
                                 'region': 'game' if s < game_end else 'library/tail',
                                 'nearest_source': nearest_sources(s, xrefs, 2)})

    totals = cov.totals()
    counts = Counter()  # unique registered starts, by the category their first byte ended in
    for va in {f['va'] for f in functions}:
        counts[CATEGORIES[cov.codes[va - t_start]]] += 1
    counts['padding'] = 0
    counts['library_crt_atlas'] = len(atlas)
    counts['library_import_thunk'] = len(thunks)
    counts['library_data'] = len(lib_data)
    counts['compiler_eh_text_x'] = len(tail_stubs)
    counts['inline_asm_excluded'] = sum(1 for r in inline_asm if r['status'] == 'excluded')
    counts['unattributed'] = len(unattributed)

    # game-side totals
    game_totals = Counter()
    for s, e, cat in cov.runs():
        lo, hi = s, min(e, game_end)
        if hi > lo:
            game_totals[cat] += hi - lo

    exact_starts = {f['va'] for f in functions if f['category'].startswith('exact_')}
    category_at = lambda va: CATEGORIES[cov.codes[va - t_start]] if t_start <= va < t_end else 'outside_text'

    report = {
        'schema_version': 1,
        'text': {'start': hx(t_start), 'end': hx(t_end), 'bytes': t_end - t_start},
        'game_code': {'start': hx(t_start), 'end': hx(game_end), 'bytes': game_end - t_start,
                      'rule': 'end = first jmp [IAT] thunk at or after the highest registered game extent'},
        'library_layout': library_layout(thunks, lib_data, atlas, crt_span, eh_start, t_end, tail_stubs,
                                         stubs, handler, game_end),
        'verdict_sources': {'calibration': str(args.calibration_json) if cal_rows is not None else 'declared',
                            'targets': str(args.physics_json) if phys_rows is not None else 'declared'},
        'warnings': warnings,
        'bytes': totals,
        'game_bytes': {k: game_totals.get(k, 0) for k in CATEGORIES},
        'counts': {k: counts.get(k, 0) for k in CATEGORIES},
        'unattributed_ranges': unattributed,
        'inline_asm': inline_asm,
        'translation_units': translation_units(functions, xrefs, game_end),
        'classes': classes_report(exact_starts, category_at),
    }
    return report


def library_layout(thunks, lib_data, atlas, crt_span, eh_start, t_end, tail_stubs, stubs, handler, game_end):
    blocks = []
    for th in sorted(thunks, key=lambda r: r['va']):
        if blocks and th['va'] == blocks[-1]['end']:
            blocks[-1]['end'] = th['va'] + 6
            blocks[-1]['modules'][th['module']] += 1
        else:
            blocks.append({'start': th['va'], 'end': th['va'] + 6, 'modules': Counter({th['module']: 1})})
    members = []
    for r in sorted(atlas, key=lambda r: r['target_va']):
        name = basename(r['archive_member'])
        if not members or members[-1]['member'] != name:
            members.append({'member': name, 'first_va': hx(r['target_va']), 'functions': 0})
        members[-1]['functions'] += 1
    return {
        'import_thunk_blocks': [{'start': hx(b['start']), 'end': hx(b['end']), 'thunks': sum(b['modules'].values()),
                                 'modules': dict(b['modules'])} for b in blocks],
        'library_data_objects': [{**r, 'va': hx(r['va'])} for r in sorted(lib_data, key=lambda r: r['va'])],
        'crt_span': [hx(crt_span[0]), hx(crt_span[1])] if crt_span else None,
        'crt_object_runs': len(members),
        'crt_objects_in_address_order': members,
        'eh_tail': {'start': hx(eh_start), 'end': hx(t_end), 'ehhandler_stubs': len(tail_stubs),
                    'ehhandler_stubs_elsewhere': len(stubs) - len(tail_stubs),
                    'cxx_frame_handler': hx(handler) if handler else None},
        'unknown_between_crt_and_eh': [hx(crt_span[1]), hx(eh_start)] if crt_span else None,
    }


def translation_units(functions, xrefs, game_end):
    paths = [p.strip() for p in (ROOT / 'analysis' / 'source_paths.txt').read_text().splitlines() if p.strip()]
    spans, tail_xrefs = defaultdict(list), Counter()
    for va, p in xrefs:
        if va < game_end:
            spans[p].append(va)
        else:
            tail_xrefs[p] += 1  # unwind funclets in the EH tail
    repo_files = defaultdict(lambda: {'src': [], 'samples': []})
    for top in ('src', 'samples'):
        for f in sorted((ROOT / top).rglob('*.cpp')):
            repo_files[f.name.lower()][top].append(f.relative_to(ROOT).as_posix())
    fn_by_file = defaultdict(list)
    for f in functions:
        if f['source'].endswith('.cpp'):
            fn_by_file[f['source']].append((f['va'], f['va'] + f['size'], f['category']))
    doc_extents = parse_doc_extents()

    rows = []
    for p in paths:
        name = basename(p)
        v = sorted(spans.get(p, []))
        row = {'path': p, 'name': name, 'kind': 'header' if name.lower().endswith('.h') else 'cpp',
               'xref_first': hx(v[0]) if v else None, 'xref_last': hx(v[-1]) if v else None,
               'xref_count': len(v), 'eh_tail_xrefs': tail_xrefs.get(p, 0)}
        matches = repo_files.get(name.lower(), {'src': [], 'samples': []})
        row['name_match_src'] = matches['src']
        row['name_match_samples'] = matches['samples']
        rows.append(row)
    cpp = sorted([r for r in rows if r['kind'] == 'cpp' and r['xref_first']], key=lambda r: int(r['xref_first'], 16))
    # bracket = from the end of the previous unit's last xref to the next unit's first xref
    for i, r in enumerate(cpp):
        lo, hi = int(r['xref_first'], 16), int(r['xref_last'], 16) + 1
        for d in doc_extents:
            owners = {u['path'] for u in cpp if any(d['start'] <= x < d['end'] for x in spans[u['path']])}
            if owners == {r['path']}:  # the extent holds this unit's xrefs and no other unit's
                r.setdefault('doc_extents', []).append({'doc': d['doc'], 'start': hx(d['start']), 'end': hx(d['end'])})
        prev_hi = int(cpp[i - 1]['xref_last'], 16) + 1 if i else 0x401000
        next_lo = int(cpp[i + 1]['xref_first'], 16) if i + 1 < len(cpp) else game_end
        r['bracket'] = [hx(prev_hi), hx(next_lo)]
        overlap = prev_hi > lo
        r['overlaps_previous'] = overlap
        r['gap_to_next'] = next_lo - hi
        ext_lo = min([lo] + [int(d['start'], 16) for d in r.get('doc_extents', [])])
        ext_hi = max([hi] + [int(d['end'], 16) for d in r.get('doc_extents', [])])
        r['extent_estimate'] = [hx(ext_lo), hx(ext_hi)]
        inside = Counter()
        bytes_in = Counter()
        files = defaultdict(int)
        bracket_files = defaultdict(int)
        for src, fns in fn_by_file.items():
            for a, b, cat in fns:
                if ext_lo <= a < ext_hi:
                    files[src] += 1
                    inside[cat] += 1
                    bytes_in[cat] += b - a
                elif prev_hi <= a < next_lo:
                    bracket_files[src] += 1
        r['bracket_only_repo_files'] = dict(sorted(bracket_files.items(), key=lambda kv: -kv[1]))
        holders = Counter()
        for x in spans[r['path']]:
            for src, fns in fn_by_file.items():
                if any(a <= x < b for a, b, _ in fns):
                    holders[src] += 1
        r['repo_files_holding_xrefs'] = dict(holders)
        r['xrefs_in_registered_functions'] = sum(holders.values())
        r['registered_in_extent'] = dict(inside)
        r['registered_bytes_in_extent'] = dict(bytes_in)
        r['repo_files_in_extent'] = dict(sorted(files.items(), key=lambda kv: -kv[1]))
        owners = set(files) | set(holders)
        has_src = bool(r['name_match_src']) or any(source_tree(f) == 'src' for f in owners)
        has_samples = bool(r['name_match_samples']) or any(source_tree(f) == 'samples' for f in owners)
        r['status'] = 'src' if has_src else 'samples_only' if has_samples else 'none'
    for r in rows:
        if r['kind'] == 'header':
            r['status'] = 'header (inlined into many units)'
        elif not r['xref_first']:
            r['status'] = 'no xref'
    # repository .cpp files with no retail path
    retail_names = {basename(p).lower() for p in paths}
    unmatched = []
    for src, fns in sorted(fn_by_file.items()):
        if Path(src).name.lower() in retail_names:
            continue
        first = sorted(a for a, _, _ in fns)[len(fns) // 2]  # median: shared helpers stray
        inside = [u['name'] for u in cpp if int(u['xref_first'], 16) <= first <= int(u['xref_last'], 16)]
        before = [u['name'] for u in cpp if int(u['xref_last'], 16) < first]
        after = [u['name'] for u in cpp if int(u['xref_first'], 16) > first]
        lo_name = before[-1].lower() if before else ''
        hi_name = after[0].lower() if after else '~'
        fits = not inside and lo_name <= Path(src).name.lower() <= hi_name
        unmatched.append({'file': src, 'functions': len(fns), 'median_va': hx(first),
                          'name_fits_alphabetical_bracket': fits,
                          'first_va': hx(min(a for a, _, _ in fns)),
                          'last_end': hx(max(b for _, b, _ in fns)),
                          'position': (f'inside the xref span of {inside[0]}' if inside else
                                       f"between {before[-1] if before else 'start'} and "
                                       f"{after[0] if after else 'game end'}")})
    return {'units_in_address_order': cpp,
            'alphabetical_inversions': order_inversions([r['name'] for r in cpp]),
            'other_paths': [r for r in rows if r not in cpp],
            'repo_files_without_retail_path': unmatched}


def classes_report(exact_starts, category_at):
    rtti = json.loads((ROOT / 'analysis' / 'rtti_classes.json').read_text())
    vtables = json.loads((ROOT / 'analysis' / 'vtables.json').read_text())
    defs = scan_class_definitions()
    names = sorted({r['name'] for r in rtti})
    by_class = defaultdict(list)
    for v in vtables:
        by_class[v['class']].append(v)
    rtti_mentions = defaultdict(list)
    for path in sorted((ROOT / 'src').rglob('*.h')):
        for m in re.finditer(r'RTTI\s+`?(\w+)', path.read_text(encoding='utf-8', errors='replace')):
            rtti_mentions[m[1]].append(path.relative_to(ROOT).as_posix())
    library = ('library_crt_atlas', 'library_span_unlabeled')
    rows = []
    for name in names:
        bare = re.sub(r'<.*', '', name).split('::')[-1]
        definition = 'src' if bare in defs['src'] else 'samples' if bare in defs['samples'] else 'none'
        slot_vas = [parse_va(e) for v in by_class.get(name, []) for e in v['entries']]
        if definition == 'none' and slot_vas and all(category_at(va) in library for va in slot_vas):
            definition = 'library (CRT)'
        elif definition == 'none' and bare in rtti_mentions:
            definition = 'src (stand-in name)'
        tables = []
        for v in by_class.get(name, []):
            slots = [parse_va(e) for e in v['entries']]
            missing = [i for i, va in enumerate(slots) if va not in exact_starts
                       and category_at(va) not in ('library_crt_atlas', 'library_span_unlabeled')]
            tables.append({'vtable_va': v['vtable_va'], 'object_offset': v['object_offset'],
                           'slots': len(slots), 'missing_slots': len(missing),
                           'missing': [{'slot': i, 'va': hx(slots[i]), 'category': category_at(slots[i])}
                                       for i in missing]})
        rows.append({'class': name, 'definition': definition,
                     'definition_files': (defs['src'].get(bare) or defs['samples'].get(bare)
                                          or rtti_mentions.get(bare) or [])[:3],
                     'vtables': tables,
                     'missing_slots': sum(t['missing_slots'] for t in tables),
                     'slots': sum(t['slots'] for t in tables)})
    return rows


def summarize(report: dict) -> str:
    lines = [f"retail .text {report['text']['start']}..{report['text']['end']} ({report['text']['bytes']:,} bytes); "
             f"game code ends {report['game_code']['end']}"]
    for w in report['warnings']:
        lines.append(f'warning: {w}')
    lines.append(f"{'category':26} {'bytes':>10} {'%text':>7} {'game bytes':>11} {'count':>6}")
    total = report['text']['bytes']
    for k in CATEGORIES:
        b = report['bytes'][k]
        lines.append(f"{k:26} {b:>10,} {b * 100 / total:>6.2f}% {report['game_bytes'][k]:>11,} {report['counts'][k]:>6}")
    big = [r for r in report['unattributed_ranges'] if r['size'] > 16]
    lines.append(f"\nunattributed ranges > 16 bytes: {len(big)} ({sum(r['size'] for r in big):,} bytes)")
    for r in big:
        near = r['nearest_source'][0] if r['nearest_source'] else None
        hint = f"{basename(near['path'])} {near['delta']:+#x}" if near else ''
        lines.append(f"  {r['start']}..{r['end']} {r['size']:>6}  {hint}")
    asm = [r for r in report['inline_asm'] if r['status'] == 'excluded']
    lines.append(f"\ninline-asm excluded functions: {len(asm)} ({sum(r['size'] for r in asm):,} bytes)")
    tus = report['translation_units']
    st = Counter(r['status'] for r in tus['units_in_address_order'])
    lines.append(f"translation units with xrefs: {len(tus['units_in_address_order'])} {dict(st)}; "
                 f"alphabetical inversions {tus['alphabetical_inversions']}")
    cl = report['classes']
    lines.append(f"RTTI classes: {len(cl)} {dict(Counter(c['definition'] for c in cl))}; "
                 f"vtable slots {sum(c['slots'] for c in cl)}, not exact {sum(c['missing_slots'] for c in cl)}")
    return '\n'.join(lines)


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE', str(ROOT / 'work/game/mcm2.exe')))
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--out', type=Path, default=DEFAULT_OUT)
    ap.add_argument('--calibration-json', type=Path)
    ap.add_argument('--physics-json', type=Path)
    ap.add_argument('--crt-atlas', type=Path, default=ROOT / 'work/vc6-crt-atlas/atlas.json')
    ap.add_argument('--data-libs', nargs='*', default=['DINPUT.LIB'])
    ap.add_argument('--run-vc6', action='store_true', help='rerun calibration and physics targets first')
    args = ap.parse_args(argv)
    args.calibration_json = args.calibration_json or args.out / 'calibration.json'
    args.physics_json = args.physics_json or args.out / 'physics.json'
    if args.run_vc6:
        if not args.vc6_root:
            raise SystemExit('--run-vc6 needs VC6_ROOT')
        run_vc6(args.out, args.exe, args.vc6_root)
    report = build(args)
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / 'report.json').write_text(json.dumps(report, indent=1) + '\n')
    print(summarize(report))
    print(f"\nwrote {args.out / 'report.json'}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
