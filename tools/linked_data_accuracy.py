#!/usr/bin/env python3
"""Compare the source-built game's initialised data with retail, symbol by symbol.

``tools/linked_accuracy.py`` scores linked code; this tool checks the data the
game links. It reads the build's objects (COFF symbols, section bytes and
relocations) and ``mcm2.map``, places every data symbol of ``.rdata``,
``.data`` and ``.bss`` at its built VA, names the retail VA it stands for and
compares the bytes with relocations resolved.

Built VAs. The map lists publics only; static data (most bring-up tables) is
placed through its section: a section's base is a public's map VA minus its
offset, or follows from a relocation in a placed section of the same object
(the built image holds the relocated value; the object holds the addend).

Retail VAs, by evidence (strongest first; one name, one VA):

* ``bindings``: ``src``/``samples`` ``*.bindings.json`` and calibration cases;
* ``alias``: the other spelling of a ``link_aliases`` pair;
* ``rtti``: ``??_R0`` type descriptors and primary ``??_7`` vtables by class name;
* ``import``/``library``: ``__imp_`` IAT slots; LIBCMT symbols from the CRT atlas;
* ``name``: a data-range address spelled in the symbol (``DAT_0056d060``,
  ``g_x_0056d060``, ``s_y_0x0056d060``, ``s_gridCounter68a088``);
* ``comment``: the first data-range address of the comment on the defining
  line, or on the first line of the comment block just above it (provisional);
* ``pointer``: the target of a relocated word in a bound symbol takes the
  retail word as its VA (the pointer then compares equal by construction);
* ``content``: a string literal (or char array) whose bytes occur once in
  retail ``.data`` (equal by construction; it serves pointer resolution).

A name bound to several VAs (header statics, one copy per unit) takes the VA
of the unit's own ``*.bindings.json``.

Retail game data ranges that no bound symbol covers are listed too, split at
anchors, with the instructions that refer into them: tables our source lacks,
holds unbound, or materialises in code.

Comparison. Plain bytes compare directly; ``.bss`` symbols compare as zeros.
A relocated word compares by target: its retail value must be the retail VA
of the symbol it points to plus the addend. Targets resolve by name, then by
address (inside another placed data symbol, or a bound function). An
unresolved target is an unknown pointer: counted, not compared. Vtables, RTTI
and EH tables are skipped (``tools/audit_game_vtables.py`` covers vtables).

Extents. A symbol's built size runs to the next symbol of its section (so it
can include alignment padding). Retail extents are inferred: the distance to
the next retail anchor (a bound symbol, an RTTI/vtable address, an absolute
code reference into data, or a retail copy of an unbound header static found
by content). ``smaller`` means retail has nonzero bytes between
our end and that anchor (missing entries, or the target of an overread such
as the chained-trick frame table); ``overlaps`` means our object runs past the
next bound retail symbol.

    python tools/linked_data_accuracy.py [--build work/game-build/build-XXXX]

Writes ``work/linked-data/report.{json,md}`` (ignored).
"""
from __future__ import annotations

import argparse
import bisect
import json
import re
import struct
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

DATA_LO, DATA_HI = 0x00550000, 0x0068c4a0   # retail .rdata start .. .data/.bss end
SKIP_SECTIONS = ('.debug', '.drectve', '.CRT$', '.xdata', '.rdata$r', '.idata', '.edata')
IMAGE_SCN_CNT_CODE, IMAGE_SCN_INIT, IMAGE_SCN_UNINIT = 0x20, 0x40, 0x80
IMAGE_SCN_LNK_INFO, IMAGE_SCN_LNK_REMOVE = 0x200, 0x800
REL_DIR32, REL_DIR32NB, REL_REL32 = 0x0006, 0x0007, 0x0014
# Retail game data (docs/REBUILD_GAPS.md section layout): game .rdata between the
# IAT and the library .rdata, game .data between .CRT$X* and the LIBCMT .data.
GAME_RANGES = ((0x0055040c, 0x00558e90), (0x005665c0, 0x00575b38))
PADDING_SLACK = 7
EVIDENCE = ('bindings', 'alias', 'rtti', 'import', 'library', 'name', 'comment', 'pointer', 'content')

MAP_LINE = re.compile(r'^\s*([0-9a-fA-F]{4}):([0-9a-fA-F]{8})\s+(\S+)\s+([0-9a-fA-F]{8})\s+'
                      r'(f\s+)?(?:i\s+)?(\S+)\s*$')
# `g_UnknownGlobal_0056a740`, `s_gridCounter68a088`: a retail VA ending the name.
ADDR_IN_NAME = re.compile(r'(?:0x|_|^|(?<=[A-Za-z]))(?:00)?([56][0-9a-fA-F]{5})(?![0-9a-fA-F])')
ADDR_IN_TEXT = re.compile(r'0x(00[56][0-9a-fA-F]{5})(?![0-9a-fA-F])')
DEFINITION = re.compile(
    r'^\s*(?:static\s+|const\s+|volatile\s+|extern\s+"C"\s+)*'
    r'(?:unsigned\s+|signed\s+|struct\s+|class\s+)?[A-Za-z_][\w:<>,]*'
    r'(?:\s*\*+\s*|\s+)(?:const\s+)?(?:\w+::)*([A-Za-z_]\w*)\s*(?:\[[^\]]*\]\s*)*(?:=|;|\(|\{)')


def hx(v: int) -> str:
    return f'0x{v:08x}'


# --------------------------------------------------------------------------- map

@dataclass(frozen=True)
class MapSymbol:
    va: int
    name: str
    obj: str          # 'Foo.obj', 'LIBCMT:x.obj' or '<common>'
    is_func: bool
    section: int


def parse_map(text: str) -> list[MapSymbol]:
    """Every public and static symbol of a VC6 ``/MAP`` (section headers excluded)."""
    out = set()
    for line in text.splitlines():
        m = MAP_LINE.match(line)
        if m:
            out.add(MapSymbol(int(m.group(4), 16), m.group(3), m.group(6), bool(m.group(5)),
                              int(m.group(1), 16)))
    return sorted(out, key=lambda s: (s.va, s.name, s.obj))


# ------------------------------------------------------------------- built data

@dataclass
class Reloc:
    offset: int           # within the symbol
    type: int
    target: str           # COFF symbol name
    target_static: bool   # static, section or label symbol (no unique global name)
    addend: int


@dataclass
class DataSymbol:
    name: str
    obj: str
    section: str          # COFF section name (.data, .rdata, .bss)
    va: int
    size: int
    data: bytes           # object bytes (zeros for .bss)
    relocs: list[Reloc] = field(default_factory=list)
    static: bool = False
    bss: bool = False
    source: str | None = None   # repository-relative .cpp the object was compiled from


def is_data_section(sec) -> bool:
    ch = sec.characteristics
    return (bool(ch & (IMAGE_SCN_INIT | IMAGE_SCN_UNINIT)) and not ch & IMAGE_SCN_CNT_CODE
            and not ch & (IMAGE_SCN_LNK_INFO | IMAGE_SCN_LNK_REMOVE)
            and not sec.name.startswith(SKIP_SECTIONS))


def is_section_symbol(obj, s) -> bool:
    return s.storage_class == 3 and s.value == 0 and s.aux_count and s.section_number > 0 \
        and obj.section(s.section_number).name == s.name


def _u32(data: bytes, off: int) -> int:
    return struct.unpack_from('<I', data, off)[0] if 0 <= off <= len(data) - 4 else 0


def section_bases(obj, publics: dict[str, int], read_built) -> dict[int, int]:
    """Section number -> built VA for the sections of ``obj`` that can be placed.

    ``publics`` maps this object's public names to map VAs; ``read_built(va)``
    reads a built dword. A relocation in a placed section to a static symbol
    of an unplaced section places that section: DIR32 holds target + addend,
    REL32 holds target + addend - (site + 4).
    """
    bases: dict[int, int] = {}
    for s in obj.symbols:
        # The map's static section lists static functions (`_$E2`) by object too.
        if s.section_number > 0 and (s.storage_class == 2 or s.type == 0x20) and s.name in publics:
            bases.setdefault(s.section_number, publics[s.name] - s.value)
    changed = True
    while changed:
        changed = False
        for r in obj.relocations:
            if r.section_number not in bases or r.type not in (REL_DIR32, REL_REL32):
                continue
            t = obj.symbol_by_index.get(r.symbol_index)
            if t is None or t.section_number <= 0 or t.section_number in bases or t.storage_class != 3:
                continue
            sec = obj.section(r.section_number)
            addend = _u32(obj.data, sec.raw_ptr + r.virtual_address) if sec.raw_ptr else 0
            site = bases[r.section_number] + r.virtual_address
            word = read_built(site)
            if word is None:
                continue
            target = (word - addend) if r.type == REL_DIR32 else (word - addend + site + 4)
            bases[t.section_number] = (target - t.value) & 0xffffffff
            changed = True
    return bases


def object_data_symbols(obj, obj_name: str, bases: dict[int, int]) -> list[DataSymbol]:
    """Data symbols of placed sections, sized to the next symbol of their section."""
    by_sec: dict[int, list] = defaultdict(list)
    for s in obj.symbols:
        if s.section_number <= 0 or s.storage_class not in (2, 3) or is_section_symbol(obj, s):
            continue
        if s.type == 0x20:
            continue
        sec = obj.section(s.section_number)
        if is_data_section(sec):
            by_sec[s.section_number].append(s)
    out = []
    for secnum, syms in sorted(by_sec.items()):
        if secnum not in bases:
            continue
        sec = obj.section(secnum)
        bss = bool(sec.characteristics & IMAGE_SCN_UNINIT) or not sec.raw_ptr
        raw = bytes(sec.raw_size) if bss else obj.data[sec.raw_ptr:sec.raw_ptr + sec.raw_size]
        starts = sorted({s.value for s in syms} | {sec.raw_size})
        rels = [r for r in obj.relocations if r.section_number == secnum]
        for s in sorted(syms, key=lambda s: (s.value, s.name)):
            end = starts[bisect.bisect_right(starts, s.value)] if s.value < sec.raw_size else s.value
            size = end - s.value
            relocs = []
            for r in rels:
                if s.value <= r.virtual_address < end:
                    t = obj.symbol_by_index.get(r.symbol_index)
                    relocs.append(Reloc(r.virtual_address - s.value, r.type,
                                        t.name if t else '?', bool(t and t.storage_class != 2),
                                        _u32(raw, r.virtual_address)))
            out.append(DataSymbol(s.name, obj_name, sec.name, bases[secnum] + s.value, size,
                                  raw[s.value:end], relocs, s.storage_class == 3, bss))
    return out


def common_symbols(objects: dict[str, object], map_syms: list[MapSymbol]) -> list[DataSymbol]:
    """``<common>`` storage: size is the largest declaration among the objects."""
    sizes: dict[str, int] = {}
    for obj in objects.values():
        for s in obj.symbols:
            if s.section_number == 0 and s.storage_class == 2 and s.value > 0:
                sizes[s.name] = max(sizes.get(s.name, 0), s.value)
    return [DataSymbol(m.name, '<common>', '.bss', m.va, sizes[m.name], bytes(sizes[m.name]),
                       bss=True)
            for m in map_syms if m.obj == '<common>' and m.name in sizes]


# --------------------------------------------------------------------- names

def symbol_category(name: str) -> str:
    if name.startswith('??_C@') or re.match(r'_?\$SG\d+$', name):
        return 'string'
    if name.startswith('__real@'):
        return 'real'
    if name.startswith(('??_7', '??_8')):
        return 'vtable'
    if name.startswith('??_R'):
        return 'rtti'
    if name.startswith(('__imp_', '\x7f')):
        return 'import'
    if '$unwind' in name or name.startswith(('$T', '__ehfuncinfo', '__unwindtable', '__tryblocktable',
                                              '__catchsym', '__TI', '__CT', '__CTA')):
        return 'eh'
    if re.search(r'(?:^_|\b)(?:IID|CLSID|GUID|DPSPGUID|DPAID|DSPROPSETID)_', name) or 'U_GUID@@' in name:
        return 'guid'
    return 'game'


def base_name(name: str) -> str | None:
    """Source identifier of a data symbol: ``?x@Cls@@3HA`` -> ``x``, ``_x`` -> ``x``."""
    if name.startswith('?') and not name.startswith('??'):
        m = re.match(r'\?([^@?]+)@', name)
        return m.group(1) if m else None
    m = re.match(r'_([A-Za-z_]\w*)$', name)
    return m.group(1) if m else None


def mangled_scalar(name: str) -> bool:
    """A C++ global whose mangled type is a scalar (not a pointer or array)."""
    return bool(re.match(r'\?[^@?]+@(?:[^@]+@)*@[23](?:_[NJK]|[CDEFGHIJKMNO])[AB]$', name))


def address_in_name(name: str) -> int | None:
    ident = base_name(name) or name
    hits = {int(m.group(1), 16) for m in ADDR_IN_NAME.finditer(ident)}
    hits = {v for v in hits if DATA_LO <= v < DATA_HI}
    return hits.pop() if len(hits) == 1 else None


def comment_addresses(text: str) -> dict[str, int]:
    """Identifier -> retail VA from a comment on its defining line (or the block above).

    Only file-scope-looking definitions count; the address must lie in retail
    data. Provisional evidence: a comment can name a related address.
    """
    out: dict[str, int] = {}
    lines = text.splitlines()
    for i, line in enumerate(lines):
        if line.lstrip().startswith(('//', '#', '/*', '*')) or line.startswith((' ', '\t')):
            continue
        code, _, comment = line.partition('//')
        m = DEFINITION.match(code)
        if not m or m.group(1) in out:
            continue
        hit = ADDR_IN_TEXT.search(comment)
        following = lines[i + 1] if i + 1 < len(lines) else ''
        if not hit and not DEFINITION.match(following.partition('//')[0]):
            # A comment block above a single definition (not above a run of them).
            j = i - 1
            while j >= 0 and lines[j].lstrip().startswith('//'):
                j -= 1
            if j + 1 < i:
                hit = re.match(r'\s*//\s*0x(00[56][0-9a-fA-F]{5})\b', lines[j + 1])
        if hit:
            va = int(hit.group(1), 16)
            if DATA_LO <= va < DATA_HI:
                out[m.group(1)] = va
    return out


def string_content_index(pe) -> dict[bytes, list[tuple[int, bool]]]:
    """Retail ``.data`` NUL-terminated runs at 4-aligned starts -> [(VA, follows a zero byte)].

    VC6 aligns ``??_C`` COMDATs to 4, so a literal starts 4-aligned after a
    zero byte (the previous literal's NUL or padding), unless nonzero data
    precedes it. A start inside a longer string is the weaker kind.
    """
    sec = next(s for s in pe.sections if s.name == '.data')
    raw = pe.data[sec.raw_offset:sec.raw_offset + min(sec.raw_size, sec.virtual_size)]
    base = pe.image_base + sec.virtual_address
    out: dict[bytes, list[tuple[int, bool]]] = defaultdict(list)
    for off in range(0, len(raw), 4):
        end = raw.find(b'\0', off)
        if end < 0 or end - off > 4096:
            continue
        out[raw[off:end + 1]].append((base + off, off == 0 or raw[off - 1] == 0))
    return out


def unique_string(hits: list[tuple[int, bool]]) -> int | None:
    strong = [va for va, clean in hits if clean]
    if len(strong) == 1:
        return strong[0]
    if not strong and len(hits) == 1:
        return hits[0][0]
    return None


# ------------------------------------------------------------------ comparison

def interpret(word: bytes) -> str:
    if len(word) < 4:
        return word.hex()
    u = struct.unpack('<I', word)[0]
    i = struct.unpack('<i', word)[0]
    f = struct.unpack('<f', word)[0]
    text = f'0x{u:08x} ({i}'
    if f == f and 1e-6 < abs(f) < 1e9:
        text += f', {f:.7g}f'
    return text + ')'


def compare(sym_bytes: bytes, relocs: list[tuple[int, int, int | None]], retail: bytes) -> dict:
    """Compare a built symbol with retail bytes.

    ``relocs``: (offset, width, expected retail word or None) for each
    relocated field; None marks an unknown target (masked). Returns byte
    counts and the differing words.
    """
    n = len(sym_bytes)
    kind = bytearray(n)              # 0 plain, 1 resolved pointer, 2 unknown pointer
    expect = {}
    for off, width, value in relocs:
        for k in range(off, min(n, off + width)):
            kind[k] = 2 if value is None else 1
        if value is not None:
            expect[off] = struct.pack('<I', value & 0xffffffff)[:width]
    diffs = []
    equal = different = unknown = 0
    pointer_diffs = []
    for off, width, value in relocs:
        if value is None:
            unknown += min(width, n - off)
            continue
        got = retail[off:off + width]
        if got == expect[off]:
            equal += len(got)
        else:
            different += width
            pointer_diffs.append({'offset': off, 'built': expect[off].hex(), 'retail': got.hex()})
    for k in range(n):
        if kind[k]:
            continue
        if sym_bytes[k] == retail[k]:
            equal += 1
        else:
            different += 1
            diffs.append(k)
    words = sorted({k & ~3 for k in diffs})
    return {'equal': equal, 'different': different, 'unknown_pointer': unknown,
            'diff_bytes': diffs, 'diff_words': words, 'pointer_diffs': pointer_diffs}


def padding_only(diffs: list[int], sym_bytes: bytes) -> bool:
    """Differences confined to the zero bytes after the last nonzero byte, within 3 bytes of the end."""
    if not diffs:
        return False
    last = len(sym_bytes.rstrip(b'\0'))
    return min(diffs) >= max(last, len(sym_bytes) - 3)


# ------------------------------------------------------------------ the image

class RetailImage:
    """Retail bytes by VA; zero past a section's file data (``.bss``)."""

    def __init__(self, pe):
        self.pe = pe
        self.base = pe.image_base

    def read(self, va: int, size: int) -> bytes:
        out = bytearray(size)
        for s in self.pe.sections:
            lo = self.base + s.virtual_address
            hi = lo + max(s.virtual_size, s.raw_size)
            a, b = max(va, lo), min(va + size, hi)
            if a >= b:
                continue
            file_hi = lo + min(s.raw_size, max(s.virtual_size, s.raw_size))
            fb = min(b, file_hi)
            if a < fb:
                off = s.raw_offset + (a - lo)
                out[a - va:fb - va] = self.pe.data[off:off + (fb - a)]
        return bytes(out)


# ------------------------------------------------------------------ resolution

def load_retail_names(root: Path, retail_pe=None) -> dict[str, tuple[set[int], str]]:
    """Symbol -> (retail VAs, evidence) from bindings, aliases, RTTI, imports and the CRT atlas."""
    from tools.audit_game_vtables import load_bindings
    names: dict[str, tuple[set[int], str]] = {}
    for sym, vas in load_bindings(root).items():
        names[sym] = (set(vas), 'bindings')
    spec_path = root / 'config/game_build.json'
    if spec_path.is_file():
        aliases = json.loads(spec_path.read_text()).get('link_aliases', {})
        for a, b in sorted(aliases.items()):
            for x, y in ((a, b), (b, a)):
                if x in names and y not in names:
                    names[y] = (names[x][0], 'alias')
    analysis = root / 'analysis'
    tds = analysis / 'rtti_type_descriptors.json'
    if tds.is_file():
        for td in json.loads(tds.read_text()):
            decorated = td.get('decorated', '')
            if decorated.startswith('.?A'):
                # `.?AVArcadeObject@@` -> `??_R0?AVArcadeObject@@@8` (the symbol drops the '.').
                names.setdefault('??_R0' + decorated[1:] + '@8', ({int(td['va'], 16)}, 'rtti'))
    vts = analysis / 'vtables.json'
    if vts.is_file():
        per_class: dict[str, set[int]] = defaultdict(set)
        for vt in json.loads(vts.read_text()):
            if vt.get('object_offset') == 0 and '<' not in vt['class']:
                per_class[vt['class']].add(int(vt['vtable_va'], 16))
        for cls, vas in per_class.items():
            scoped = '@'.join(reversed(cls.split('::')))
            names.setdefault(f'??_7{scoped}@@6B@', (vas, 'rtti'))
    atlas = root / 'work/vc6-crt-atlas/atlas.json'
    if atlas.is_file():
        per: dict[str, set[int]] = defaultdict(set)
        for m in json.loads(atlas.read_text()).get('libcmt', {}).get('matches', []):
            per[m['symbol']].add(int(m['target_va']))
        for sym, vas in per.items():
            names.setdefault(sym, (vas, 'library'))
    if retail_pe is not None:
        for name, va in iat_slots(retail_pe).items():
            names.setdefault(name, ({va}, 'import'))
    return names


def unit_bindings(root: Path) -> dict[str, dict[str, int]]:
    """``dir/Unit`` (lower case, no suffix) -> symbol -> VA, per ``dir/Unit.bindings.json``.

    Header statics (``kVec3XAxis``, ``g_gridRow16``) have one copy per unit, so
    their name binds many VAs; the unit's own bindings file picks its copy.
    """
    out: dict[str, dict[str, int]] = {}
    for tree in ('src', 'samples'):
        for path in sorted((root / tree).rglob('*.bindings.json')):
            try:
                data = json.loads(path.read_text(encoding='utf-8'))
            except (OSError, ValueError):
                continue
            if not isinstance(data, dict):
                continue
            key = path.relative_to(root).as_posix()[:-len('.bindings.json')].lower()
            out[key] = {k: int(v, 16) for k, v in data.items()
                        if isinstance(v, str) and v.startswith('0x')}
    return out


def iat_slots(pe) -> dict[str, int]:
    """``__imp__Name@N``-style keys (``__imp__Name`` prefix match) -> retail IAT slot VA."""
    out = {}
    if len(pe.directories) < 2 or not pe.directories[1][0]:
        return out
    off = pe.rva_to_offset(pe.directories[1][0])
    while True:
        oft, _, _, name_rva, ft = struct.unpack_from('<IIIII', pe.data, off)
        if not (oft or name_rva or ft):
            break
        to = pe.rva_to_offset(oft or ft)
        slot = ft
        while True:
            val = pe.u32(to)
            if val == 0:
                break
            if not val & 0x80000000:
                fn = pe.read_c_string_at_offset(pe.rva_to_offset(val) + 2)
                out['__imp__' + fn] = pe.image_base + slot
            to += 4
            slot += 4
        off += 20
    return out


def import_key(name: str) -> str | None:
    m = re.match(r'(__imp__[A-Za-z_]\w*?)(?:@\d+)?$', name)
    return m.group(1) if m else None


@dataclass
class Binding:
    va: int
    evidence: str


def bind_symbols(syms: list[DataSymbol], names: dict, comments: dict[str, dict[str, int]],
                 strings: dict | None,
                 units: dict[str, dict[str, int]] | None = None) -> tuple[dict[int, Binding], dict[int, str]]:
    """Index of ``syms`` -> Binding; second dict notes why a symbol stayed unbound."""
    bound: dict[int, Binding] = {}
    why: dict[int, str] = {}
    for i, s in enumerate(syms):
        got = names.get(s.name)
        if got is None and s.name.startswith('__imp_'):
            got = names.get(import_key(s.name) or '')
        if got is not None:
            vas = {v for v in got[0] if DATA_LO <= v < DATA_HI}
            if len(vas) > 1 and units and s.source:
                own = units.get(s.source.rsplit('.', 1)[0].lower(), {}).get(s.name)
                if own in vas:
                    vas = {own}
            if len(vas) == 1:
                bound[i] = Binding(vas.pop(), got[1])
                continue
            if len(vas) > 1:
                why[i] = f'{got[1]}: {len(vas)} retail VAs'
                continue
        va = address_in_name(s.name)
        if va is not None:
            bound[i] = Binding(va, 'name')
            continue
        ident = base_name(s.name)
        va = comments.get(s.obj, {}).get(ident) if ident else None
        if va is not None:
            bound[i] = Binding(va, 'comment')
            continue
        cat = symbol_category(s.name)
        if strings is not None and b'\0' in s.data and (
                (cat == 'string' and s.data.endswith(b'\0'))
                or (cat == 'game' and s.data[:1] != b'\0' and is_text(s.data))):   # char arrays
            text = s.data[:s.data.index(b'\0') + 1]
            hits = strings.get(text, [])
            va = unique_string(hits)
            if va is not None:
                bound[i] = Binding(va, 'content')
                continue
            why[i] = f'string: {len(hits)} retail occurrences'
    return bound, why


def bind_by_pointer(syms: list[DataSymbol], bound: dict[int, Binding], read_built, retail) -> int:
    """Bind unbound data symbols that bound symbols point at, to retail's pointer value.

    A relocated word with addend 0 that points at the start of exactly one
    unbound symbol: retail's word at the same place is that symbol's retail VA
    (evidence ``pointer``; the pointer itself then compares equal by
    construction). Repeats until nothing new binds; returns the count.
    """
    starts: dict[int, list[int]] = defaultdict(list)
    for i, s in enumerate(syms):
        starts[s.va].append(i)
    added = 0
    changed = True
    while changed:
        changed = False
        for i, b in sorted(bound.items()):
            sym = syms[i]
            for r in sym.relocs:
                if r.type != REL_DIR32 or r.addend:
                    continue
                targets = [k for k in starts.get(read_built(sym.va + r.offset), ()) if k not in bound]
                if len(targets) != 1:
                    continue
                word = struct.unpack('<I', retail.read(b.va + r.offset, 4))[0]
                if DATA_LO <= word < DATA_HI:
                    bound[targets[0]] = Binding(word, 'pointer')
                    added += 1
                    changed = True
    return added


class Resolver:
    """Built pointer -> retail VA, by target name or by built address."""

    def __init__(self, syms, bound, names, funcs_by_va: dict[int, list[str]]):
        self.names = names
        self.syms = syms
        self.bound = bound
        self.funcs = funcs_by_va
        order = sorted(range(len(syms)), key=lambda i: (syms[i].va, -syms[i].size))
        self.starts = [syms[i].va for i in order]
        self.order = order
        self.by_name = {}
        for i, s in enumerate(syms):
            if i in bound and not s.static:
                self.by_name.setdefault(s.name, bound[i].va)

    def by_symbol(self, name: str) -> int | None:
        if name in self.by_name:
            return self.by_name[name]
        got = self.names.get(name)
        if got:
            vas = set(got[0])
            if len(vas) == 1:
                return next(iter(vas))
        return None

    def by_address(self, va: int) -> int | None:
        k = bisect.bisect_right(self.starts, va) - 1
        while k >= 0 and self.starts[k] > va - 0x40000:
            i = self.order[k]
            s = self.syms[i]
            if s.va <= va < s.va + max(s.size, 1) and i in self.bound:
                return self.bound[i].va + (va - s.va)
            if s.va <= va < s.va + s.size:
                return None
            k -= 1
        for name in self.funcs.get(va, ()):
            got = self.by_symbol(name)
            if got is not None:
                return got
        return None

    def expected(self, r: Reloc, built_word: int | None) -> int | None:
        if r.type != REL_DIR32:
            return None
        if not r.target_static:
            got = self.by_symbol(r.target)
            if got is not None:
                return got + r.addend
        return self.by_address(built_word) if built_word is not None else None


# ------------------------------------------------------------------ extents

def retail_anchors(names, bound_vas, code_refs) -> tuple[list[int], list[int]]:
    """Sorted retail VAs where a known object starts (bound) and every anchor (bound + code refs)."""
    known = {v for vas, _ in names.values() for v in vas if DATA_LO <= v < DATA_HI} | set(bound_vas)
    every = known | {v for v in code_refs if DATA_LO <= v < DATA_HI}
    return sorted(known), sorted(every)


def next_after(pool: list[int], va: int) -> int | None:
    i = bisect.bisect_right(pool, va)
    return pool[i] if i < len(pool) else None


def is_text(blob: bytes) -> bool:
    """NUL-separated printable text (with padding), at least one 4-character run."""
    parts = [p for p in blob.split(b'\0') if p]
    return bool(parts) and any(len(p) >= 4 for p in parts) and all(
        all(32 <= c < 127 or c in (9, 10, 13) for c in p) for p in parts)


def content_copies(retail, blobs: list[bytes], ranges) -> list[tuple[int, int]]:
    """Dword-aligned retail VAs (start, end) in ``ranges`` holding one of ``blobs``.

    Unbound header statics (one copy per unit, e.g. the grid row tables) are
    located by content, so their copies neither pad a neighbour's extent nor
    show up as uncovered data.
    """
    out = set()
    for lo, hi in ranges:
        hay = retail.read(lo, hi - lo)
        for blob in blobs:
            at = hay.find(blob)
            while at >= 0:
                if (lo + at) % 4 == 0:
                    out.add((lo + at, lo + at + len(blob)))
                at = hay.find(blob, at + 1)
    return sorted(out)


def uncovered_runs(retail, covered: list[tuple[int, int]], ranges, anchors: list[int],
                   sources: dict[int, list[int]]) -> list[dict]:
    """Retail game data with nonzero bytes that no bound built symbol covers.

    Each uncovered stretch of ``ranges`` is split at retail anchors; pieces
    with a nonzero byte are returned with the code instructions that refer
    into them (``sources``: target VA -> instruction VAs).
    """
    out = []
    iv = sorted(covered)
    for lo, hi in ranges:
        blob = retail.read(lo, hi - lo)
        mask = bytearray(hi - lo)
        for a, b in iv:
            if b <= lo or a >= hi:
                continue
            mask[max(a, lo) - lo:min(b, hi) - lo] = b'\1' * (min(b, hi) - max(a, lo))
        cuts = sorted({v - lo for v in anchors if lo <= v < hi})
        va = lo
        while va < hi:
            if mask[va - lo]:
                va += 1
                continue
            end = va + 1
            while end < hi and not mask[end - lo]:
                end += 1
            points = [va] + [lo + c for c in cuts if va < lo + c < end] + [end]
            for a, b in zip(points, points[1:]):
                piece = blob[a - lo:b - lo]
                nonzero = sum(1 for x in piece if x)
                if not nonzero:
                    continue
                refs = sorted({s for t in range(a, b) for s in sources.get(t, ())})
                out.append({'va': hx(a), 'size': b - a, 'nonzero': nonzero, 'text': is_text(piece),
                            'code_refs': [hx(s) for s in refs[:6]], 'code_ref_count': len(refs),
                            'head': piece[:32].hex()})
            va = end
    return out


# ------------------------------------------------------------------ driver

def build_objects(build: Path, root: Path) -> dict[str, Path]:
    """Map object basename -> object path, from the build's report (or its objects tree)."""
    out: dict[str, list[Path]] = defaultdict(list)
    report = build / 'report.json'
    paths = []
    if report.is_file():
        for row in json.loads(report.read_text()).get('sources', []):
            p = Path(row.get('object', ''))
            if not p.is_file():
                p = (build / 'objects' / row['source']).with_suffix('.obj')
            paths.append((p, row.get('source')))
    else:
        paths = [(p, None) for p in sorted((build / 'objects').rglob('*.obj'))]
    for p, src in paths:
        if p.is_file():
            out[p.name.lower()].append((p, src))
    return out


def analyse(build: Path, retail_path: Path, root: Path = ROOT, code_refs: bool = True) -> dict:
    from mcm2tool.coff import CoffObject
    from mcm2tool.pe import PEImage
    built_pe = PEImage(build / 'mcm2.exe')
    retail_pe = PEImage(retail_path)
    built_img = RetailImage(built_pe)
    retail = RetailImage(retail_pe)
    map_syms = parse_map((build / 'mcm2.map').read_text(encoding='latin-1'))
    publics_by_obj: dict[str, dict[str, int]] = defaultdict(dict)
    funcs_by_va: dict[int, list[str]] = defaultdict(list)
    for m in map_syms:
        publics_by_obj[m.obj.lower()][m.name] = m.va
        if m.is_func:
            funcs_by_va[m.va].append(m.name)

    def read_built(va: int) -> int | None:
        word = built_img.read(va, 4)
        return struct.unpack('<I', word)[0]

    spec = json.loads((root / 'config/game_build.json').read_text())
    fragments = defaultdict(list)
    for frag in spec.get('included_sources', []):
        fragments[frag['included_by']].append(frag['path'])
    objects = {}
    comments: dict[str, dict[str, int]] = {}
    syms: list[DataSymbol] = []
    unplaced = Counter()
    for key, rows in sorted(build_objects(build, root).items()):
        publics = publics_by_obj.get(key, {})
        for path, source in rows:
            obj = CoffObject(path)
            names_here = {s.name for s in obj.symbols if s.section_number > 0
                          and (s.storage_class == 2 or s.type == 0x20)}
            mine = {n: v for n, v in publics.items() if n in names_here}
            objects[path.as_posix()] = obj
            bases = section_bases(obj, mine, read_built)
            label = path.name if len(rows) == 1 else (source or path.as_posix())
            got = object_data_symbols(obj, label, bases)
            # COMDAT copies the linker did not choose have no public with this object.
            got = [s for s in got if s.static or s.name in mine]
            for s in got:
                s.source = source
            syms.extend(got)
            for s in obj.symbols:   # externals left unplaced are COMDAT copies the linker folded
                if (s.section_number > 0 and s.storage_class == 3 and s.type != 0x20
                        and not is_section_symbol(obj, s) and s.section_number not in bases
                        and is_data_section(obj.section(s.section_number))):
                    unplaced[label] += 1
            if source:
                text = ''
                for rel in [source] + fragments.get(source, []):
                    p = root / rel
                    if p.is_file():
                        text += p.read_text(encoding='latin-1') + '\n'
                comments[label] = comment_addresses(text)
    syms.extend(common_symbols(objects, map_syms))
    # One entry per (name, VA): a public symbol defined once is listed once.
    seen = set()
    uniq = []
    for s in sorted(syms, key=lambda s: (s.va, s.name, s.obj)):
        if (s.name, s.va) in seen:
            continue
        seen.add((s.name, s.va))
        uniq.append(s)
    syms = uniq

    names = load_retail_names(root, retail_pe)
    strings = string_content_index(retail_pe)
    bound, why = bind_symbols(syms, names, comments, strings, unit_bindings(root))
    bind_by_pointer(syms, bound, read_built, retail)
    for i in bound:
        why.pop(i, None)
    resolver = Resolver(syms, bound, names, funcs_by_va)
    refs: list[int] = []
    ref_pairs: list[tuple[int, int]] = []
    if code_refs:
        from tools.rebuild_data_coverage import code_references
        ref_pairs = code_references(retail_pe, DATA_LO, DATA_HI) or []
        refs = sorted({t for _, t in ref_pairs})
    known, every = retail_anchors(names, [b.va for b in bound.values()], refs)
    # A vtable's object starts at its COL slot, 4 bytes before the first entry.
    rtti_starts = sorted(v - 4 * name.startswith('??_7') for name, (vas, ev) in names.items()
                         if ev == 'rtti' for v in vas)
    unbound_blobs = sorted({bytes(s.data) for i, s in enumerate(syms)
                            if i not in bound and not s.bss and symbol_category(s.name) == 'game'
                            and len(s.data) >= 16 and len(s.data.strip(b'\0')) >= 8})
    copies = content_copies(retail, unbound_blobs, GAME_RANGES)
    known = sorted(set(known) | set(rtti_starts) | {a for a, _ in copies})
    every = sorted(set(every) | set(rtti_starts) | {a for a, _ in copies})

    rows = []
    for i, s in enumerate(syms):
        cat = symbol_category(s.name)
        row = {'symbol': s.name, 'object': s.obj, 'section': s.section, 'built_va': hx(s.va),
               'size': s.size, 'category': cat, 'static': s.static, 'bss': s.bss}
        b = bound.get(i)
        if cat in ('vtable', 'rtti', 'eh', 'import'):
            row['status'] = 'skipped'
            rows.append(row)
            continue
        if b is None:
            row['status'] = 'unbound'
            row['why'] = why.get(i, '')
            row['nonzero'] = any(s.data)
            row['head'] = s.data[:16].hex()
            rows.append(row)
            continue
        row['retail_va'] = hx(b.va)
        row['evidence'] = b.evidence
        rbytes = retail.read(b.va, s.size)
        built_words = {r.offset: read_built(s.va + r.offset) for r in s.relocs}
        rel = []
        for r in s.relocs:
            width = 4 if r.type in (REL_DIR32, REL_DIR32NB, REL_REL32) else 2
            rel.append((r.offset, width, resolver.expected(r, built_words[r.offset])))
        res = compare(s.data, rel, rbytes)
        row.update(equal=res['equal'], different=res['different'], unknown_pointer=res['unknown_pointer'])
        if res['different']:
            row['status'] = 'different'
            row['padding_only'] = padding_only(res['diff_bytes'], s.data) and not res['pointer_diffs']
            words = []
            for w in res['diff_words'][:8]:
                words.append({'offset': w, 'built': interpret(s.data[w:w + 4]),
                              'retail': interpret(rbytes[w:w + 4])})
            row['diff_words'] = words
            row['diff_word_count'] = len(res['diff_words'])
            row['pointer_diffs'] = res['pointer_diffs'][:8]
            row['first_diff'] = min(res['diff_bytes'] + [p['offset'] for p in res['pointer_diffs']])
        else:
            row['status'] = 'equal'
        # Extent against retail anchors.
        nxt_known = next_after(known, b.va)
        nxt_any = next_after(every, b.va)
        row['retail_gap_known'] = (nxt_known - b.va) if nxt_known else None
        row['retail_gap_any'] = (nxt_any - b.va) if nxt_any else None
        # The built extent runs to the next symbol, so it can include up to 7
        # bytes of alignment padding; only a longer overrun is reported.
        if nxt_known is not None and s.size - (nxt_known - b.va) > PADDING_SLACK:
            row['overlaps'] = hx(nxt_known)
        if nxt_any is not None and s.size < nxt_any - b.va:
            tail = retail.read(b.va + s.size, nxt_any - b.va - s.size)
            if tail.strip(b'\0') and len(tail) >= 4 and cat == 'game' and not mangled_scalar(s.name):
                row['smaller'] = {'retail_extent': nxt_any - b.va, 'tail_nonzero': sum(1 for x in tail if x),
                                  'tail_head': tail[:32].hex()}
        rows.append(row)
    summary = summarise(rows)
    summary['unplaced_symbols'] = sum(unplaced.values())
    summary['code_reference_anchors'] = len(refs)
    summary['unbound_content_copies'] = len(copies)
    covered = [(b.va, b.va + max(syms[i].size, 1)) for i, b in bound.items()] + copies
    sources: dict[int, list[int]] = defaultdict(list)
    for src_va, target in ref_pairs:
        sources[target].append(src_va)
    gaps = uncovered_runs(retail, covered, GAME_RANGES, every, sources)
    summary['retail_uncovered'] = {
        'runs': len(gaps), 'nonzero_bytes': sum(g['nonzero'] for g in gaps),
        'referenced_runs': sum(1 for g in gaps if g['code_refs']),
        'referenced_nonzero_bytes': sum(g['nonzero'] for g in gaps if g['code_refs']),
        'text_runs': sum(1 for g in gaps if g['text'])}
    return {'build': str(build), 'summary': summary, 'rows': rows, 'retail_uncovered': gaps}


def summarise(rows: list[dict]) -> dict:
    out = {'symbols': len(rows), 'by_status': dict(sorted(Counter(r['status'] for r in rows).items()))}
    cats = defaultdict(Counter)
    for r in rows:
        c = cats[r['category']]
        c['symbols'] += 1
        c[r['status']] += 1
        c['bytes'] += r['size']
        for k in ('equal', 'different', 'unknown_pointer'):
            c[k + '_bytes'] += r.get(k, 0)
        if not r['bss']:
            c['initialised_equal_bytes'] += r.get('equal', 0)
        if r['status'] == 'unbound':
            c['unbound_bytes'] += r['size']
    out['by_category'] = {k: dict(sorted(v.items())) for k, v in sorted(cats.items())}
    out['by_evidence'] = dict(sorted(Counter(r['evidence'] for r in rows if 'evidence' in r).items()))
    out['smaller'] = sum(1 for r in rows if 'smaller' in r)
    out['overlaps'] = sum(1 for r in rows if 'overlaps' in r)
    return out


def render(report: dict, top: int = 200) -> str:
    s = report['summary']
    rows = report['rows']
    lines = ['# Linked data accuracy (generated, ignored)', '', f"Build: `{report['build']}`", '',
             '| category | symbols | bound bytes equal (of them initialised) | different | unknown pointer | unbound symbols (bytes) | skipped |',
             '|---|---:|---:|---:|---:|---:|---:|']
    for cat, c in s['by_category'].items():
        lines.append(f"| {cat} | {c['symbols']} | {c.get('equal_bytes', 0)} ({c.get('initialised_equal_bytes', 0)}) | {c.get('different_bytes', 0)} | "
                     f"{c.get('unknown_pointer_bytes', 0)} | {c.get('unbound', 0)} ({c.get('unbound_bytes', 0)}) | "
                     f"{c.get('skipped', 0)} |")
    lines += ['', f"Status: {s['by_status']}; binding evidence: {s['by_evidence']}; "
              f"{s['unplaced_symbols']} object data symbols could not be placed; "
              f"{s['code_reference_anchors']} retail code-reference anchors; "
              f"{s.get('unbound_content_copies', 0)} retail copies of unbound header statics found by content.", '']
    diff = [r for r in rows if r['status'] == 'different']
    diff.sort(key=lambda r: (r.get('padding_only', False), r['category'] != 'game', -r['different'], r['retail_va']))
    lines += ['## Different', '', f'{len(diff)} bound symbols differ (padding-only last).', '',
              '| retail VA | evidence | symbol | object | size | diff bytes | first diff | built | retail |',
              '|---|---|---|---|---:|---:|---:|---|---|']
    for r in diff[:top]:
        w = (r.get('diff_words') or [{}])[0]
        p = (r.get('pointer_diffs') or [{}])[0]
        built = w.get('built') or ('ptr ' + p.get('built', ''))
        ret = w.get('retail') or ('ptr ' + p.get('retail', ''))
        pad = ' (padding)' if r.get('padding_only') else ''
        lines.append(f"| {r['retail_va']} | {r['evidence']} | `{r['symbol']}` | {r['object']} | {r['size']} | "
                     f"{r['different']}{pad} | {r['first_diff']} | {built} | {ret} |")
    small = sorted((r for r in rows if 'smaller' in r), key=lambda r: r['retail_va'])
    lines += ['', '## Smaller than retail extent', '',
              'Retail has nonzero bytes between our end and the next retail anchor: missing entries, '
              'or bytes an unbounded index reads.', '',
              '| retail VA | evidence | symbol | object | our size | retail extent | nonzero tail bytes | tail |',
              '|---|---|---|---|---:|---:|---:|---|']
    for r in small:
        sm = r['smaller']
        lines.append(f"| {r['retail_va']} | {r['evidence']} | `{r['symbol']}` | {r['object']} | {r['size']} | "
                     f"{sm['retail_extent']} | {sm['tail_nonzero']} | {sm['tail_head']} |")
    over = sorted((r for r in rows if 'overlaps' in r), key=lambda r: r['retail_va'])
    lines += ['', '## Larger than retail extent', '',
              'Our object runs past the next bound retail symbol.', '',
              '| retail VA | evidence | symbol | object | our size | next bound retail symbol |',
              '|---|---|---|---|---:|---|']
    for r in over:
        lines.append(f"| {r['retail_va']} | {r['evidence']} | `{r['symbol']}` | {r['object']} | {r['size']} | {r['overlaps']} |")
    unb = [r for r in rows if r['status'] == 'unbound' and r['category'] == 'game']
    groups: dict[str, list[dict]] = defaultdict(list)
    for r in unb:
        groups[r['symbol']].append(r)
    order = sorted(groups.values(), key=lambda g: (not any(r['nonzero'] for r in g), -g[0]['size'], g[0]['symbol']))
    lines += ['', '## Unbound game data', '',
              f"{len(unb)} game data symbols ({len(groups)} names) have no retail VA "
              f"({sum(1 for r in unb if r['nonzero'])} with nonzero initial bytes). Header statics "
              'repeat once per unit.', '',
              '| symbol | objects | section | size | first bytes | note |', '|---|---|---|---:|---|---|']
    for g in order[:top]:
        r = g[0]
        objs = r['object'] if len(g) == 1 else f"{len(g)} units"
        lines.append(f"| `{r['symbol']}` | {objs} | {r['section']} | {r['size']} | {r['head']} | {r['why']} |")
    strs = [r for r in rows if r['status'] == 'unbound' and r['category'] == 'string']
    local = [r for r in strs if 'C?3?2Users' in r['symbol']]
    lines += ['', '## String literals not found once in retail', '',
              f'{len(strs)} literals; {len(local)} are local `__FILE__` paths (retail has `D:\\aardvark\\...`).', '']
    for r in sorted((r for r in strs if r not in local), key=lambda r: r['symbol'])[:top]:
        lines.append(f"- `{r['symbol']}` ({r['object']}): {r['why']}")
    gaps = report.get('retail_uncovered', [])
    shown = [g for g in gaps if g['code_refs'] and not g['text']]
    lines += ['', '## Retail game data no bound symbol covers', '',
              f"{len(gaps)} runs with nonzero bytes in the retail game data ranges are covered by no bound "
              f"built symbol; {sum(1 for g in gaps if g['text'])} look like text. The "
              f"{len(shown)} non-text runs that retail code refers to are listed (data our source holds "
              'unbound, materialises in code, or lacks).', '',
              '| retail VA | size | nonzero | code refs | first bytes |', '|---|---:|---:|---|---|']
    for g in shown[:top]:
        refs = ', '.join(g['code_refs']) + (' ...' if g['code_ref_count'] > len(g['code_refs']) else '')
        lines.append(f"| {g['va']} | {g['size']} | {g['nonzero']} | {refs} | {g['head']} |")
    return '\n'.join(lines) + '\n'


def main(argv=None) -> int:
    from tools.audit_game_vtables import DEFAULT_RETAIL, latest_build
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--build', type=Path, default=None, help='build directory with mcm2.exe, mcm2.map and objects')
    ap.add_argument('--retail', type=Path, default=DEFAULT_RETAIL)
    ap.add_argument('--out', type=Path, default=ROOT / 'work/linked-data')
    ap.add_argument('--no-code-refs', action='store_true', help='skip the retail code-reference sweep (extents)')
    ap.add_argument('--top', type=int, default=400)
    args = ap.parse_args(argv)
    build = args.build or latest_build()
    if build is None or not (build / 'mcm2.map').is_file():
        print('no linked build; run tools/build_game.py first', file=sys.stderr)
        return 2
    if not args.retail.is_file():
        print(f'retail image missing: {args.retail}', file=sys.stderr)
        return 2
    report = analyse(build, args.retail, ROOT, not args.no_code_refs)
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / 'report.json').write_text(json.dumps(report, indent=1) + '\n', encoding='utf-8')
    (args.out / 'report.md').write_text(render(report, args.top), encoding='utf-8')
    s = report['summary']
    for cat, c in s['by_category'].items():
        print(f"{cat:8} {c['symbols']:5} symbols: {c.get('equal_bytes', 0)} B equal "
              f"({c.get('initialised_equal_bytes', 0)} initialised), {c.get('different_bytes', 0)} B "
              f"different ({c.get('different', 0)} symbols), {c.get('unknown_pointer_bytes', 0)} B unknown pointer, "
              f"{c.get('unbound', 0)} unbound ({c.get('unbound_bytes', 0)} B), {c.get('skipped', 0)} skipped")
    print(f"evidence {s['by_evidence']}; smaller {s['smaller']}, overlaps {s['overlaps']}; "
          f"{s['unplaced_symbols']} unplaced object data symbols")
    u = s['retail_uncovered']
    print(f"retail game data covered by no bound symbol: {u['runs']} nonzero runs ({u['nonzero_bytes']} nonzero B), "
          f"{u['referenced_runs']} referenced by code ({u['referenced_nonzero_bytes']} B), {u['text_runs']} text")
    print(f"Report: {args.out / 'report.md'}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
