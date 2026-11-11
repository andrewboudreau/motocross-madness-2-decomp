#!/usr/bin/env python3
"""Audit ``*.bindings.json``: does each binding name the retail object its symbol stands for?

A strict match resolves every relocation through the unit's bindings, so a
binding to the wrong retail object compares as exact whenever the code only
takes its address. ``NetworkInterface::CreateAddress`` tagged the modem name
``DPAID_Phone`` while the bindings bound ``DPAID_Phone`` to retail's
``DPAID_Modem``; the masked score and the emulator agreed with retail. Only
the bytes at the bound address show such a swap. This tool checks them.

Data bindings (the bound VA lies outside ``.text``) compare our definition's
bytes with the retail bytes at the bound VA:

* ``__real@``/``??_C@`` literals: the value the name spells
  (``tools/check_literal_bindings.py``); ``__FILE__`` keys are skipped (they
  name the build machine's path by design);
* ``__imp_`` keys: the retail import table slot of that name;
* ``??_7`` vtables and ``??_R0`` type descriptors: ``analysis/vtables.json``
  and ``analysis/rtti_type_descriptors.json`` by class; a vtable's subobject
  is our COL's object offset (VC6 can name a non-zero offset ``6B@``);
* GUIDs with no definition in our sources: ``DEFINE_GUID`` in the VC6/DirectX
  SDK headers under ``VC6_ROOT`` (read locally, never copied);
* everything else: the symbol's bytes in the linked build's objects
  (``tools/build_game.py``). The unit's own object comes first (header statics
  have one copy per unit); a name no object defines takes its
  ``config/game_build.json`` link alias provider's definition. Relocated words compare by target: the retail word
  must be the retail VA of the symbol it points to plus the addend; targets
  without a unique retail VA are left out. ``.bss`` definitions compare as
  zeros. The compared extent is our symbol's (to the next symbol of its
  section) cut at the next retail anchor (another bound VA or vtable), and a
  difference confined to trailing alignment padding counts as equal.

Data that C++ dynamic initializers write (header statics such as
``kVec3XAxis``: ``.bss`` in each unit, valued by the unit's ``$E``) compares
after both images ran their ``__xc_a..__xc_z`` tables
(``tools/static_initializers.py``). The copy compared is the binding unit's own
copy in the linked build, else the public definition, else every linked copy
(they must agree). Bytes an initializer that could not be emulated may write
are ``unknown``; bytes several initializers write (allocator counters) are
running state, reported ``uninitialised``. A definition that is nothing but
pointers to statics without a retail VA follows each pointer: the bytes our
pointer names, after initializers, must equal the bytes the retail pointer
names (``pointer target``).

A data symbol that names its VA (``s_gridCounter68a088``) must be bound to that
VA; a comment VA on the definition that disagrees is reported as a note
(comments are provisional evidence).

Function bindings (the bound VA lies in ``.text``) are checked for a function
start (a call or jump target, an address stored in the image, a vtable entry,
or a 16-aligned address after ``ret``/padding) and against the LIBCMT CRT atlas
both ways (a CRT name must be bound where the atlas puts it, and an address
the atlas names may only carry its names: ``_strcmp`` at ``__stricmp`` is a
mismatch). Across files:

* one symbol bound to several VAs: per-unit statics (``_$E`` initialisers, a
  name static in the build) and duplicate literals (retail keeps some
  constants twice) are legitimate; otherwise the retail bodies are compared,
  and identical bodies are reported as ``identical_bodies``; the rest are
  ``conflict`` findings;
* one VA bound to several symbols (``collision`` is the finding):
  - data: ``collision`` when two of the names have initialised definitions
    with different bytes, otherwise ``alias`` (spellings of one object);
  - code: ``collision`` when the atlas names the VA and one of the names is
    not the atlas's; ``deleting-dtor`` for a class's ``??_E``/``??_G``;
    ``fold`` when all linked bodies are identical with relocations masked;
    ``alias`` when at most one name has a body (callers' declarations of one
    callee); ``tiny_fold`` for retail bodies of at most 8 bytes; otherwise
    ``distinct-bodies``, listed for review (a stand-in body next to the
    reconstruction, or two reconstructions of one function);
* ``config/game_build.json`` link aliases whose two names are bound to
  disjoint VAs are findings.

Statuses: ``match``, ``mismatch``, ``uninitialised`` (our ``.bss``, retail
zero), ``unknown`` (no definition or no evidence), ``skipped`` and
``placeholder`` (bound to 0). The exit status is 1 when any finding exists.

    python tools/audit_bindings.py [--retail EXE] [--build work/game-build/build-X]
                                   [--vc6-root DIR] [--json out.json] [FILE.bindings.json ...]

Writes ``work/binding-audit/report.{json,md}`` (ignored) unless ``--json`` names
another path.
"""
from __future__ import annotations

import argparse
import bisect
import json
import os
import re
import struct
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))
if str(ROOT / 'tools') not in sys.path:
    sys.path.insert(0, str(ROOT / 'tools'))

REL_DIR32, REL_DIR32NB, REL_REL32 = 0x0006, 0x0007, 0x0014
IMAGE_SCN_CNT_CODE, IMAGE_SCN_INIT, IMAGE_SCN_UNINIT = 0x20, 0x40, 0x80
IMAGE_SCN_LNK_INFO, IMAGE_SCN_LNK_REMOVE = 0x200, 0x800
SKIP_SECTIONS = ('.debug', '.drectve', '.CRT$', '.xdata', '.idata', '.edata')
PADDING_SLACK = 3
TINY_BODY = 8
GUID_NAME = re.compile(r'^_?((?:IID|CLSID|GUID|DPSPGUID|DPAID|DSPROPSETID|LIBID|FMTID|CATID)_\w+)$')
CXX_GUID_NAME = re.compile(r'^\?((?:IID|CLSID|GUID|DPSPGUID|DPAID|DSPROPSETID)_\w+)@@3U_GUID@@B$')
DEFINE_GUID = re.compile(
    r'(?:DEFINE_GUID|OUR_GUID_ENTRY)\s*\(\s*(\w+)\s*,(?:\s*/\*[^*]*\*/)?\s*'
    + r'\s*,\s*'.join([r'(0x[0-9a-fA-F]+|\d+)L?'] * 11) + r'\s*\)')
# MIDL headers: `EXTERN_C const IID IID_X;` then `interface DECLSPEC_UUID("...")`.
MIDL_GUID = re.compile(r'EXTERN_C\s+const\s+(?:IID|CLSID)\s+(\w+)\s*;(?:(?!EXTERN_C)[\s\S]){0,300}?'
                       r'DECLSPEC_UUID\(\s*"([0-9a-fA-F-]{36})"\s*\)')
ADDR_IN_NAME = re.compile(r'(?:0x|_|^|(?<=[A-Za-z]))(?:00)?([56][0-9a-fA-F]{5})(?![0-9a-fA-F])')
STATIC_LOCAL = re.compile(r'^_?\$E\d+(?:\$|$)|^\$')


def hx(v: int) -> str:
    return f'0x{v:08x}'


# ------------------------------------------------------------------ bindings

def binding_files(root: Path) -> list[Path]:
    return sorted(p for tree in ('src', 'samples') if (root / tree).is_dir()
                  for p in (root / tree).rglob('*.bindings.json'))


def load_file(path: Path) -> dict[str, int]:
    data = json.loads(path.read_text(encoding='utf-8'))
    if not isinstance(data, dict):
        return {}
    return {k: int(v, 0) if isinstance(v, str) else int(v) for k, v in data.items()
            if isinstance(v, (str, int))}


def source_for(bindings: Path) -> Path:
    return bindings.with_name(bindings.name[:-len('.bindings.json')] + '.cpp')


# ------------------------------------------------------------------ SDK GUIDs

def guid_bytes(fields: list[int]) -> bytes:
    return struct.pack('<IHH8B', *fields)


def parse_define_guids(text: str) -> dict[str, bytes]:
    """``DEFINE_GUID``/``OUR_GUID_ENTRY`` and MIDL ``DECLSPEC_UUID`` GUIDs of a header."""
    out = {}
    for m in DEFINE_GUID.finditer(text):
        out[m.group(1)] = guid_bytes([int(x, 0) for x in m.groups()[1:]])
    for m in MIDL_GUID.finditer(text):
        h = m.group(2).replace('-', '')
        fields = [int(h[:8], 16), int(h[8:12], 16), int(h[12:16], 16)] + list(bytes.fromhex(h[16:]))
        out.setdefault(m.group(1), guid_bytes(fields))
    return out


def sdk_guids(vc6_root: str | None) -> dict[str, set[bytes]]:
    """GUID name -> values ``DEFINE_GUID`` gives it in the VC6/DirectX SDK headers."""
    out: dict[str, set[bytes]] = defaultdict(set)
    if not vc6_root:
        return out
    base = Path(vc6_root)
    dirs = [d for d in base.rglob('*') if d.is_dir() and d.name.lower() == 'include']
    for d in dirs:
        for h in d.iterdir():
            if h.suffix.lower() not in ('.h', '.idl') or not h.is_file():
                continue
            text = h.read_text(encoding='latin-1', errors='replace')
            if 'DEFINE_GUID' not in text and 'GUID_ENTRY' not in text and 'DECLSPEC_UUID' not in text:
                continue
            for name, value in parse_define_guids(text).items():
                out[name].add(value)
    return out


def guid_identifier(name: str) -> str | None:
    m = GUID_NAME.match(name) or CXX_GUID_NAME.match(name)
    return m.group(1) if m else None


def format_guid(b: bytes) -> str:
    d1, d2, d3 = struct.unpack_from('<IHH', b)
    return f'{d1:08x}-{d2:04x}-{d3:04x}-{b[8:10].hex()}-{b[10:16].hex()}'


# ------------------------------------------------------------------ names

def base_name(name: str) -> str | None:
    """Source identifier of a data symbol: ``?x@Cls@@3HA`` -> ``Cls::x``, ``_x`` -> ``x``."""
    if name.startswith('?') and not name.startswith('??'):
        m = re.match(r'\?([^@?]+)@((?:[^@?]+@)*)@3', name)
        if not m:
            return None
        scope = [s for s in m.group(2).split('@') if s]
        return '::'.join(list(reversed(scope)) + [m.group(1)])
    m = re.match(r'_([A-Za-z_]\w*)$', name)
    return m.group(1) if m else None


def address_in_name(name: str, lo: int, hi: int) -> int | None:
    ident = base_name(name) or ''
    if not ident or ident.startswith(('IID_', 'CLSID_')):
        return None
    hits = {int(m.group(1), 16) for m in ADDR_IN_NAME.finditer(ident.split('::')[-1])}
    hits = {v for v in hits if lo <= v < hi}
    return hits.pop() if len(hits) == 1 else None


def vtable_class(name: str) -> tuple[str, str | None] | None:
    """``??_7Cls@Outer@@6BBase@@@`` -> (``Outer::Cls``, ``Base``); primary -> (cls, None)."""
    m = re.match(r'\?\?_7((?:[^@?]+@)+)@6B(.*)$', name)
    if not m:
        return None
    cls = '::'.join(reversed([s for s in m.group(1).split('@') if s]))
    rest = m.group(2)
    if rest == '@':
        return cls, None
    b = re.match(r'((?:[^@?]+@)+)@@$', rest)
    base = '::'.join(reversed([s for s in b.group(1).split('@') if s])) if b else rest
    return cls, base


# ------------------------------------------------------------------ the build

@dataclass
class Defined:
    name: str
    source: str           # repository-relative source of the object
    section: str
    data: bytes
    relocs: list = field(default_factory=list)    # (offset, type, target, target_static, addend)
    static: bool = False
    bss: bool = False
    exact_size: bool = False   # COMMON: the declared size


def is_data_section(sec) -> bool:
    ch = sec.characteristics
    return (bool(ch & (IMAGE_SCN_INIT | IMAGE_SCN_UNINIT)) and not ch & IMAGE_SCN_CNT_CODE
            and not ch & (IMAGE_SCN_LNK_INFO | IMAGE_SCN_LNK_REMOVE)
            and not sec.name.startswith(SKIP_SECTIONS))


def object_definitions(obj, source: str) -> tuple[list[Defined], list[tuple[str, bool, object]]]:
    """Data definitions of one object, and (name, static, symbol) for its functions."""
    data: list[Defined] = []
    funcs = []
    by_sec: dict[int, list] = defaultdict(list)
    for s in obj.symbols:
        if s.section_number == 0 and s.storage_class == 2 and s.value > 0 and s.type != 0x20:
            data.append(Defined(s.name, source, '<common>', bytes(s.value), bss=True, exact_size=True))
            continue
        if s.section_number <= 0 or s.storage_class not in (2, 3):
            continue
        sec = obj.section(s.section_number)
        if s.storage_class == 3 and s.value == 0 and s.aux_count and sec.name == s.name:
            continue    # section symbol
        if s.type == 0x20:
            if sec.characteristics & IMAGE_SCN_CNT_CODE:
                funcs.append((s.name, s.storage_class == 3, s))
            continue
        if is_data_section(sec):
            by_sec[s.section_number].append(s)
    for secnum, syms in by_sec.items():
        sec = obj.section(secnum)
        bss = bool(sec.characteristics & IMAGE_SCN_UNINIT) or not sec.raw_ptr
        raw = bytes(sec.raw_size) if bss else obj.data[sec.raw_ptr:sec.raw_ptr + sec.raw_size]
        starts = sorted({s.value for s in syms} | {sec.raw_size})
        rels = [r for r in obj.relocations if r.section_number == secnum]
        for s in syms:
            end = starts[bisect.bisect_right(starts, s.value)] if s.value < sec.raw_size else s.value
            relocs = []
            for r in rels:
                if s.value <= r.virtual_address < end:
                    t = obj.symbol_by_index.get(r.symbol_index)
                    addend = struct.unpack_from('<I', raw, r.virtual_address)[0] \
                        if r.virtual_address + 4 <= len(raw) else 0
                    relocs.append((r.virtual_address - s.value, r.type, t.name if t else '?',
                                   bool(t is None or t.storage_class != 2), addend))
            data.append(Defined(s.name, source, sec.name, raw[s.value:end], relocs,
                                s.storage_class == 3, bss))
    return data, funcs


class BuildIndex:
    """Data definitions and function bodies of a linked build's objects, by symbol."""

    def __init__(self, build: Path | None, root: Path = ROOT):
        self.data: dict[str, list[Defined]] = defaultdict(list)
        self.funcs: dict[str, list[tuple[str, bool, object, object]]] = defaultdict(list)
        self.sources: set[str] = set()
        self.build = build
        if build is None:
            return
        from mcm2tool.coff import CoffObject
        report = build / 'report.json'
        rows = json.loads(report.read_text()).get('sources', []) if report.is_file() else []
        for row in rows:
            path = Path(row.get('object', ''))
            if not path.is_file():
                path = (build / 'objects' / row['source']).with_suffix('.obj')
            if not path.is_file():
                continue
            self.add_object(CoffObject(path), row['source'])

    def add_object(self, obj, source: str) -> None:
        self.sources.add(source)
        data, funcs = object_definitions(obj, source)
        for d in data:
            self.data[d.name].append(d)
        for name, static, sym in funcs:
            self.funcs[name].append((source, static, obj, sym))

    def definition(self, name: str, source: str | None) -> tuple[Defined | None, str]:
        """Our definition for a binding in ``source``'s file, and how it was chosen."""
        defs = self.data.get(name, [])
        if not defs:
            return None, 'undefined'
        own = [d for d in defs if d.source == source]
        if own:
            return own[0], 'unit'
        public = [d for d in defs if not d.static]
        if public:
            best = max(public, key=lambda d: (not d.bss, len(d.data)))
            return best, 'public'
        values = {(d.data, tuple(r[:3] for r in d.relocs)) for d in defs}
        if len(values) == 1:
            return defs[0], 'static-copy'
        if all(d.bss for d in defs):
            # Zeros everywhere; the copies differ only in the padding before the
            # next symbol. Their values, if any, come from dynamic initializers.
            return min(defs, key=lambda d: len(d.data)), 'static-copy'
        return None, 'ambiguous-static'

    def is_static(self, name: str) -> bool:
        return any(d.static for d in self.data.get(name, ())) or any(
            st for _, st, _, _ in self.funcs.get(name, ()))

    def body(self, name: str) -> tuple[bytes, bytes] | None:
        """(bytes, relocation mask) of a linked function body, if one object defines it."""
        from mcm2tool.coff import relocation_mask
        rows = self.funcs.get(name)
        if not rows:
            return None
        _, _, obj, sym = rows[0]
        try:
            raw, size, rels = obj.symbol_extent(sym)
        except Exception:
            return None
        return raw, bytes(relocation_mask(obj, sym, size, rels))


# ------------------------------------------------------------------ retail

class Retail:
    def __init__(self, pe, root: Path = ROOT):
        self.pe = pe
        self.base = pe.image_base
        text = next(s for s in pe.sections if s.characteristics & 0x20)
        self.text_lo = self.base + text.virtual_address
        self.text_hi = self.text_lo + text.virtual_size
        data_secs = [s for s in pe.sections if not s.characteristics & 0x20 and s.name != '.rsrc']
        self.data_lo = min(self.base + s.virtual_address for s in data_secs)
        self.data_hi = max(self.base + s.virtual_address + max(s.virtual_size, s.raw_size)
                           for s in data_secs)
        from linked_data_accuracy import iat_slots
        self.iat = iat_slots(pe)
        self.iat_by_va = {v: k for k, v in self.iat.items()}
        self.vtables: dict[int, dict] = {}
        self.rtti: dict[str, int] = {}
        self.atlas: dict[str, set[int]] = defaultdict(set)
        analysis = root / 'analysis'
        if (analysis / 'vtables.json').is_file():
            for vt in json.loads((analysis / 'vtables.json').read_text()):
                self.vtables[int(vt['vtable_va'], 16)] = vt
        if (analysis / 'rtti_type_descriptors.json').is_file():
            for td in json.loads((analysis / 'rtti_type_descriptors.json').read_text()):
                if td.get('decorated', '').startswith('.?A'):
                    self.rtti['??_R0' + td['decorated'][1:] + '@8'] = int(td['va'], 16)
        atlas = root / 'work/vc6-crt-atlas/atlas.json'
        if atlas.is_file():
            for m in json.loads(atlas.read_text()).get('libcmt', {}).get('matches', []):
                self.atlas[m['symbol']].add(int(m['target_va']))
                for a in m.get('aliases', []):
                    self.atlas[a].add(int(m['target_va']))
        self.atlas_names: dict[int, set[str]] = defaultdict(set)
        for sym, vas in self.atlas.items():
            for v in vas:
                self.atlas_names[v].add(sym)
        self._starts: set[int] | None = None

    def read(self, va: int, size: int) -> bytes:
        out = bytearray(size)
        for s in self.pe.sections:
            lo = self.base + s.virtual_address
            hi = lo + max(s.virtual_size, s.raw_size)
            a, b = max(va, lo), min(va + size, hi)
            if a >= b:
                continue
            fb = min(b, lo + s.raw_size)
            if a < fb:
                off = s.raw_offset + (a - lo)
                out[a - va:fb - va] = self.pe.data[off:off + (fb - a)]
        return bytes(out)

    def is_code(self, va: int) -> bool:
        return self.text_lo <= va < self.text_hi

    def is_mapped(self, va: int) -> bool:
        return any(self.base + s.virtual_address <= va
                   < self.base + s.virtual_address + max(s.virtual_size, s.raw_size)
                   for s in self.pe.sections)

    def function_starts(self) -> set[int]:
        """Call/jump targets, addresses stored anywhere in the image, vtable entries."""
        if self._starts is not None:
            return self._starts
        from array import array
        lo, hi = self.text_lo, self.text_hi
        text = self.read(lo, hi - lo)
        starts: set[int] = set()
        for op in (b'\xe8', b'\xe9'):
            i = text.find(op)
            while i >= 0:
                if i + 5 <= len(text):
                    t = (lo + i + 5 + struct.unpack_from('<i', text, i + 1)[0]) & 0xffffffff
                    if lo <= t < hi:
                        starts.add(t)
                i = text.find(op, i + 1)
        image = self.pe.data
        for shift in range(4):
            n = (len(image) - shift) // 4
            words = array('I', image[shift:shift + 4 * n])
            if sys.byteorder != 'little':
                words.byteswap()
            starts.update(w for w in words if lo <= w < hi)
        for vt in self.vtables.values():
            starts.update(int(e, 16) for e in vt.get('entries', []))
        self._starts = starts
        return starts

    def is_function_start(self, va: int) -> str | None:
        if va in self.function_starts():
            return 'referenced'
        if va % 16 == 0 and self.read(va - 1, 1) in (b'\x90', b'\xc3', b'\xcc'):
            return 'aligned'
        return None

    def body(self, va: int, limit: int = 0x400) -> bytes:
        """Retail bytes from ``va`` to the next 16-aligned start after padding (bounded)."""
        raw = self.read(va, limit)
        for k in range(16, len(raw), 16):
            if raw[k - 1] in (0x90, 0xcc, 0xc3) and (va + k) in self.function_starts():
                return raw[:k]
        return raw


# ------------------------------------------------------------------ initializers

@dataclass
class Placed:
    """A data symbol of the linked build at its built VA (``linked_data_accuracy.place_build``)."""
    name: str
    source: str | None
    static: bool
    va: int
    size: int


class InitialisedImage:
    """An image's load bytes overlaid with what its C++ dynamic initializers write.

    ``read(va, n)`` -> (bytes, written by an initializer, touched by one that was not emulated);
    ``lo``/``hi`` bound the image, so a word in that range reads as a pointer.
    """

    def __init__(self, read, init=None, lo: int = 0, hi: int = 0):
        self._read = read
        self.init = init
        self.lo, self.hi = lo, hi

    def read(self, va: int, size: int) -> tuple[bytes, bool, bool]:
        raw = bytearray(self._read(va, size))
        written = uncertain = False
        if self.init is not None:
            for k in range(size):
                a = va + k
                if a in self.init.values:
                    raw[k] = self.init.values[a]
                    written = True
                if a in self.init.uncertain:
                    uncertain = True
        return bytes(raw), written, uncertain

    def shared(self, va: int, size: int) -> bool:
        """Whether several initializers write into [va, va+size) (running state)."""
        shared = getattr(self.init, 'shared', ())
        return any(a in shared for a in range(va, va + size))

    def word(self, va: int) -> int:
        return struct.unpack('<I', self.read(va, 4)[0])[0]

    def masked(self, data: bytes) -> bytes:
        """``data`` with aligned words that point into the image (or the initializers'
        scratch heap) replaced by ``PTR!``."""
        from static_initializers import HEAP, HEAP_END
        out = bytearray(data)
        for k in range(0, len(data) - 3, 4):
            w = struct.unpack_from('<I', data, k)[0]
            if self.lo <= w < self.hi or HEAP <= w < HEAP_END:
                out[k:k + 4] = b'PTR!'
        return bytes(out)


def float_reading(data: bytes) -> str:
    words = [struct.unpack_from('<f', data, k)[0] for k in range(0, len(data) - 3, 4)]
    return '(' + ', '.join(f'{w:g}' for w in words[:6]) + ')'


# ------------------------------------------------------------------ comparison

def compare_data(ours: Defined, retail: bytes, expected: dict[int, int | None]) -> dict:
    """Compare our definition with retail bytes of the same length.

    ``expected``: relocation offset -> the retail word it must hold (None: unknown target).
    """
    n = len(retail)
    data = ours.data[:n]
    masked = bytearray(n)
    pointer_diffs = []
    unknown = 0
    for off, typ, target, _, _ in ours.relocs:
        if off >= n:
            continue
        width = 4 if typ in (REL_DIR32, REL_DIR32NB, REL_REL32) else 2
        for k in range(off, min(n, off + width)):
            masked[k] = 1
        want = expected.get(off)
        if want is None:
            unknown += 1
            continue
        got = retail[off:off + width]
        if len(got) == 4 and struct.unpack('<I', got)[0] != want & 0xffffffff:
            pointer_diffs.append({'offset': off, 'target': target, 'expected': hx(want),
                                  'retail': hx(struct.unpack('<I', got)[0])})
    diffs = [k for k in range(min(n, len(data))) if not masked[k] and data[k] != retail[k]]
    if diffs and not pointer_diffs:
        last = len(data.rstrip(b'\0'))
        if min(diffs) >= max(last, len(data) - PADDING_SLACK) and not ours.exact_size:
            diffs = []      # alignment padding after the object
    return {'diffs': diffs, 'pointer_diffs': pointer_diffs, 'unknown_pointers': unknown,
            'compared': n}


def bodies_equal(a: tuple[bytes, bytes], b: tuple[bytes, bytes]) -> bool:
    (ra, ma), (rb, mb) = a, b
    if len(ra) != len(rb):
        return False
    return all(ma[i] or mb[i] or ra[i] == rb[i] for i in range(len(ra)))


# ------------------------------------------------------------------ the audit

class Auditor:
    def __init__(self, retail: Retail, build: BuildIndex, guids: dict[str, set[bytes]],
                 files: dict[str, dict[str, int]], root: Path = ROOT,
                 ours_image: InitialisedImage | None = None,
                 retail_image: InitialisedImage | None = None,
                 placed: list[Placed] | None = None):
        self.r = retail
        self.b = build
        self.ours_image = ours_image
        self.retail_image = retail_image
        self.placed: dict[str, list[Placed]] = defaultdict(list)
        for p in placed or ():
            self.placed[p.name].append(p)
        self.placed_by_va = sorted(placed or (), key=lambda p: p.va)
        self.placed_vas = [p.va for p in self.placed_by_va]
        self.guids = guids
        self.files = files
        self.root = root
        per_symbol: dict[str, set[int]] = defaultdict(set)
        for d in files.values():
            for k, v in d.items():
                per_symbol[k].add(v)
        self.per_symbol = per_symbol
        anchors = {v for vs in per_symbol.values() for v in vs if not retail.is_code(v)}
        anchors |= set(retail.vtables)
        anchors |= {v - 4 for v in retail.vtables}        # the COL slot before each vtable
        anchors |= set(retail.rtti.values())
        self.anchors = sorted(a for a in anchors if a)
        self.comments: dict[str, dict[str, int]] = {}
        spec = root / 'config/game_build.json'
        self.link_aliases: dict[str, str] = (json.loads(spec.read_text()).get('link_aliases', {})
                                             if spec.is_file() else {})

    # -- helpers
    def resolve(self, name: str, unit: dict[str, int]) -> int | None:
        if name in unit:
            return unit[name]
        vas = self.per_symbol.get(name)
        if vas and len(vas) == 1:
            return next(iter(vas))
        if name in self.r.rtti:
            return self.r.rtti[name]
        if name in self.r.atlas and len(self.r.atlas[name]) == 1:
            return next(iter(self.r.atlas[name]))
        return None

    def next_anchor(self, va: int) -> int | None:
        k = bisect.bisect_right(self.anchors, va)
        return self.anchors[k] if k < len(self.anchors) else None

    def comment_va(self, path: str, name: str) -> int | None:
        if path not in self.comments:
            from linked_data_accuracy import comment_addresses
            src = source_for(self.root / path)
            text = src.read_text(encoding='latin-1') if src.is_file() else ''
            self.comments[path] = comment_addresses(text)
        ident = base_name(name)
        return self.comments[path].get(ident.split('::')[-1]) if ident else None

    # -- dynamic initializers and pointer targets
    def our_copies(self, name: str, source: str | None) -> tuple[list[Placed], str]:
        """The built copies of ``name`` that stand for the binding in ``source``'s file."""
        copies = self.placed.get(name, [])
        own = [c for c in copies if c.source == source]
        if own:
            return own[:1], 'unit'
        public = [c for c in copies if not c.static]
        if public:
            return public[:1], 'public'
        return copies, f'{len(copies)} linked copies' if len(copies) > 1 else 'linked copy'

    def compare_initialised(self, copies: list[Placed], how: str, va: int, size: int) -> dict | None:
        """Our copies after their initializers vs retail at ``va`` after its initializers.

        None when no initializer writes either side (a plain load-image compare stands).
        """
        if self.ours_image is None or self.retail_image is None or not copies:
            return None
        n = min([size] + [c.size for c in copies])
        if n <= 0:
            return None
        rb, rw, ru = self.retail_image.read(va, n)
        ours = [self.ours_image.read(c.va, n) for c in copies]
        if not rw and not any(w for _, w, _ in ours):
            return None
        if ru or any(u for _, _, u in ours):
            return {'status': 'unknown', 'evidence': 'initializer not emulated',
                    'detail': 'an initializer that faulted or called out may write it'}
        if self.retail_image.shared(va, n) or any(self.ours_image.shared(c.va, n) for c in copies):
            # A counter every allocating initializer bumps: its value after them depends
            # on the whole initializer set, not on this definition.
            return {'status': 'uninitialised', 'evidence': 'several initializers write it'}
        values = {self.ours_image.masked(b) for b, _, _ in ours}
        if len(values) > 1:
            return {'status': 'unknown', 'evidence': f'initializer ({how})',
                    'detail': 'linked copies disagree: ' + '; '.join(
                        f'{c.source} {float_reading(b)}' for c, (b, _, _) in zip(copies, ours))}
        ob = ours[0][0]
        mine, theirs = values.pop(), self.retail_image.masked(rb)
        row = {'evidence': f'initializer ({how})', 'initialised': ob.hex()}
        if mine == theirs:
            return dict(row, status='match')
        k = next(i for i in range(n) if mine[i] != theirs[i])
        return dict(row, status='mismatch',
                    detail=f'after initializers ours {ob[:16].hex()} {float_reading(ob)}, '
                           f'retail {rb[:16].hex()} {float_reading(rb)} (first difference +0x{k:x})')

    def follow_pointer(self, our_va: int, retail_va: int) -> dict:
        """Compare the static our pointer names with what the retail pointer names."""
        if self.ours_image is None or self.retail_image is None:
            return {'status': 'unknown'}
        target = self.ours_image.word(our_va)
        k = bisect.bisect_right(self.placed_vas, target) - 1
        if k < 0 or not self.placed_by_va[k].va <= target < self.placed_by_va[k].va + max(
                self.placed_by_va[k].size, 1):
            return {'status': 'unknown', 'detail': f'our pointer {hx(target)} names no placed symbol'}
        p = self.placed_by_va[k]
        delta = target - p.va
        rword = struct.unpack('<I', self.r.read(retail_va, 4))[0]
        rbase = rword - delta
        if not self.r.is_mapped(rbase) or self.r.is_code(rbase):
            return {'status': 'mismatch', 'target': p.name,
                    'detail': f'retail points to {hx(rword)}, not data'}
        n = p.size
        nxt = self.next_anchor(rbase)
        if nxt is not None and nxt - rbase < n:
            n = nxt - rbase
        if n <= 0:
            return {'status': 'unknown', 'target': p.name, 'detail': 'empty extent'}
        ob, _, ou = self.ours_image.read(p.va, n)
        rb, _, ru = self.retail_image.read(rbase, n)
        if ou or ru:
            return {'status': 'unknown', 'target': p.name, 'detail': 'target initializer not emulated'}
        mine, theirs = self.ours_image.masked(ob), self.retail_image.masked(rb)
        diffs = [i for i in range(n) if mine[i] != theirs[i]]
        if diffs and min(diffs) >= len(ob.rstrip(b'\0')) and min(diffs) >= n - PADDING_SLACK:
            diffs = []
        if diffs:
            return {'status': 'mismatch', 'target': p.name,
                    'detail': f'{p.name} at retail {hx(rbase)}: {len(diffs)} bytes differ from '
                              f'+0x{diffs[0]:x}: ours {ob[diffs[0]:diffs[0] + 8].hex()} '
                              f'retail {rb[diffs[0]:diffs[0] + 8].hex()}'}
        return {'status': 'match', 'target': p.name, 'retail_target': hx(rbase), 'size': n}

    # -- one data binding
    def audit_data(self, path: str, name: str, va: int, unit: dict[str, int]) -> dict:
        from check_literal_bindings import check_one, decode_string
        from propose_bindings import decode_real
        row = {'file': path, 'symbol': name, 'va': hx(va), 'kind': 'data'}
        if name.startswith('__FILE__'):
            return dict(row, kind='file', status='skipped', evidence='build path literal')
        if decode_real(name) is not None or decode_string(name) is not None:
            problem = check_one(name, va, self.r.pe)
            return dict(row, kind='literal', status='mismatch' if problem else 'match',
                        evidence='name spells the value', detail=problem)
        if name.startswith('__imp_'):
            got = self.r.iat_by_va.get(va)
            m = re.match(r'(__imp__[A-Za-z_]\w*?)(?:@\d+)?$', name)
            key = m.group(1) if m else name
            if got is None:
                return dict(row, kind='import', status='mismatch', evidence='import table',
                            detail='not an import slot')
            return dict(row, kind='import', status='match' if got == key else 'mismatch',
                        evidence='import table', detail=None if got == key else f'slot imports {got}')
        vt = vtable_class(name)
        if vt is not None:
            row['kind'] = 'vtable'
            cls, base = vt
            got = self.r.vtables.get(va)
            if got is None:
                same = [hx(v) for v, t in self.r.vtables.items() if t['class'] == cls]
                return dict(row, status='mismatch' if same else 'unknown', evidence='analysis/vtables.json',
                            detail=f'not a vtable; {cls} vtables at {same}' if same else 'class not in RTTI')
            if got['class'] != cls:
                return dict(row, status='mismatch', evidence='analysis/vtables.json',
                            detail=f'vtable of {got["class"]} (object offset {got["object_offset"]})')
            # VC6 names a vtable `6B@` whenever one base path needs no qualifier, which
            # need not be offset 0 (Tire's CollisionPoint table at +0xb8). Our COL
            # (`??_R4`, offset at +4) says which subobject the name stands for.
            col, _ = self.b.definition('??_R4' + name[4:], None)
            if col is not None and len(col.data) >= 8:
                offset = struct.unpack_from('<I', col.data, 4)[0]
                ok = offset == got['object_offset']
                return dict(row, status='match' if ok else 'mismatch', evidence='COL object offset',
                            detail=None if ok else f'our COL offset {offset}, retail table is at '
                                                   f'object offset {got["object_offset"]}')
            if base is None and got['object_offset'] != 0:
                return dict(row, status='unknown', evidence='analysis/vtables.json',
                            detail=f'class matches; object offset {got["object_offset"]}, no COL to check')
            return dict(row, status='match', evidence='analysis/vtables.json (class)')
        if name.startswith('??_R0'):
            want = self.r.rtti.get(name)
            if want is None:
                return dict(row, kind='rtti', status='unknown', evidence='rtti')
            return dict(row, kind='rtti', status='match' if want == va else 'mismatch', evidence='rtti',
                        detail=None if want == va else f'type descriptor is at {hx(want)}')

        src = source_for(self.root / path).relative_to(self.root).as_posix()
        ours, how = self.b.definition(name, src)
        if ours is None and how == 'undefined' and name in self.link_aliases:
            # A weak external: the linked build gives this name the provider's storage.
            ours, how = self.b.definition(self.link_aliases[name], None)
            how = 'link alias ' + how
        named = address_in_name(name, self.r.data_lo, self.r.data_hi)
        if named is not None and named != va:
            return dict(row, status='mismatch', evidence='address in name',
                        detail=f'the name spells {hx(named)}')
        gid = guid_identifier(name)
        if ours is None and gid:
            values = self.guids.get(gid)
            if not values:
                return dict(row, kind='guid', status='unknown', evidence='no SDK definition')
            got = self.r.read(va, 16)
            if got in values:
                return dict(row, kind='guid', status='match', evidence='SDK DEFINE_GUID',
                            detail=format_guid(got))
            who = [k for k, vs in self.guids.items() if got in vs]
            return dict(row, kind='guid', status='mismatch', evidence='SDK DEFINE_GUID',
                        detail=f'retail holds {format_guid(got)}' + (f' ({", ".join(sorted(who)[:4])})'
                                                                     if who else '')
                        + f', the SDK gives {gid} ' + ', '.join(format_guid(v) for v in values))
        if gid:
            row['kind'] = 'guid'
        if ours is None:
            note = None
            cva = self.comment_va(path, name)
            if cva is not None and cva != va:
                note = f'definition comment names {hx(cva)}'
            return dict(row, status='unknown', evidence=how, note=note)
        size = len(ours.data)
        nxt = self.next_anchor(va)
        if nxt is not None and nxt - va < size:
            size = nxt - va
        if size <= 0:
            return dict(row, status='unknown', evidence=how, detail='empty extent')
        retail = self.r.read(va, size)
        expected = {}
        for off, typ, target, static, addend in ours.relocs:
            if typ != REL_DIR32 or static:
                expected[off] = None
                continue
            t = self.resolve(target, unit)
            expected[off] = None if t is None else (t + addend) & 0xffffffff
        res = compare_data(ours, retail, expected)
        row.update(evidence=how, defined_in=ours.source, section=ours.section, size=size)
        cva = self.comment_va(path, name)
        if cva is not None and cva != va:
            row['note'] = f'definition comment names {hx(cva)}'
        if ours.bss:
            if any(retail):
                first = next(i for i, x in enumerate(retail) if x)
                return dict(row, status='mismatch',
                            detail=f'ours is zero-initialised; retail +0x{first:x} holds '
                                   f'{retail[first:first + 8].hex()}')
            copies, which = self.our_copies(name, src)
            if how.startswith('link alias') and not copies:
                copies, which = self.our_copies(self.link_aliases[name], None)
            init = self.compare_initialised(copies, which, va, size)
            if init is not None:
                return dict(row, **init)
            return dict(row, status='uninitialised')
        if res['diffs'] or res['pointer_diffs']:
            detail = []
            if res['diffs']:
                k = res['diffs'][0]
                detail.append(f'{len(res["diffs"])} bytes differ from +0x{k:x}: ours '
                              f'{ours.data[k:k + 8].hex()} retail {retail[k:k + 8].hex()}')
            for p in res['pointer_diffs'][:3]:
                detail.append(f'+0x{p["offset"]:x} points to {p["target"]} {p["expected"]}, '
                              f'retail {p["retail"]}')
            return dict(row, status='mismatch', detail='; '.join(detail))
        if res['unknown_pointers'] and res['unknown_pointers'] * 4 >= size:
            copies, which = self.our_copies(name, src)
            if how.startswith('link alias') and not copies:
                copies, which = self.our_copies(self.link_aliases[name], None)
            if len(copies) != 1:
                return dict(row, status='unknown', detail='only unresolved pointers')
            followed = [self.follow_pointer(copies[0].va + off, va + off)
                        for off, want in sorted(expected.items()) if want is None and off < size]
            bad = [f for f in followed if f['status'] == 'mismatch']
            if bad:
                return dict(row, status='mismatch', evidence='pointer target',
                            detail='; '.join(f['detail'] for f in bad[:3]))
            if followed and all(f['status'] == 'match' for f in followed):
                return dict(row, status='match', evidence='pointer target',
                            detail=', '.join(f'{f["target"]} at {f["retail_target"]}' for f in followed[:3]))
            return dict(row, status='unknown', detail='only unresolved pointers; '
                        + '; '.join(f.get('detail') or '' for f in followed if f['status'] != 'match'))
        return dict(row, status='match')

    def audit_function(self, path: str, name: str, va: int) -> dict:
        row = {'file': path, 'symbol': name, 'va': hx(va), 'kind': 'function'}
        atlas = self.r.atlas.get(name)
        if atlas and va not in atlas:
            return dict(row, status='mismatch', evidence='CRT atlas',
                        detail='atlas places it at ' + ', '.join(hx(v) for v in sorted(atlas)))
        there = self.r.atlas_names.get(va)
        if there and name not in there:
            return dict(row, status='mismatch', evidence='CRT atlas',
                        detail='atlas names this address ' + ', '.join(sorted(there)))
        how = self.r.is_function_start(va)
        if how is None:
            if '$' in name or name.startswith('__ehhandler'):
                return dict(row, status='skipped', evidence='label')
            return dict(row, status='mismatch', evidence='function start',
                        detail='no call/jump/address reference and not after padding')
        return dict(row, status='match', evidence='CRT atlas' if atlas else f'function start ({how})')

    def audit_binding(self, path: str, name: str, va: int, unit: dict[str, int]) -> dict:
        if va == 0 or not self.r.is_mapped(va):
            return {'file': path, 'symbol': name, 'va': hx(va), 'kind': 'unmapped',
                    'status': 'placeholder' if va == 0 else 'mismatch',
                    'detail': None if va == 0 else 'outside the retail image'}
        if self.r.is_code(va) and not name.startswith(('__imp_', '__real@', '??_C@', '??_7', '??_R')):
            return self.audit_function(path, name, va)
        return self.audit_data(path, name, va, unit)

    # -- cross-file consistency
    def symbol_conflicts(self) -> list[dict]:
        out = []
        for name, vas in sorted(self.per_symbol.items()):
            vas = {v for v in vas if v}
            if len(vas) < 2:
                continue
            files = {hx(v): sorted(p for p, d in self.files.items() if d.get(name) == v) for v in vas}
            row = {'symbol': name, 'vas': files}
            if name.startswith('__FILE__') or STATIC_LOCAL.match(name):
                row['class'] = 'static-local'
            elif name.startswith(('__real@', '??_C@')):
                # Retail keeps some constants twice (non-COMDAT copies); each binding
                # is checked against the value the name spells.
                row['class'] = 'duplicate-literal'
            elif self.b.is_static(name):
                row['class'] = 'per-unit-static'
            else:
                row['class'] = 'conflict'
            if row['class'] == 'conflict' and all(self.r.is_code(v) for v in vas):
                bodies = {self.r.body(v) for v in vas}
                if len(bodies) == 1:
                    row['class'] = 'identical_bodies'
            out.append(row)
        return out

    def va_collisions(self) -> list[dict]:
        by_va: dict[int, set[str]] = defaultdict(set)
        for d in self.files.values():
            for k, v in d.items():
                if v and not k.startswith('__FILE__') and not STATIC_LOCAL.match(k):
                    by_va[v].add(k)
        out = []
        for va, names in sorted(by_va.items()):
            if len(names) < 2:
                continue
            names_sorted = sorted(names)
            row = {'va': hx(va), 'symbols': names_sorted,
                   'files': {n: sorted(p for p, d in self.files.items() if d.get(n) == va)
                             for n in names_sorted}}
            row['class'] = self.collision_class(va, names_sorted, row)
            out.append(row)
        return out

    def collision_class(self, va: int, names: list[str], row: dict) -> str:
        """Why several symbols share one retail VA (``collision`` is a finding)."""
        if not self.r.is_code(va):
            defs = []
            for n in names:
                d = [x for x in self.b.data.get(n, ()) if not x.bss]
                if d:
                    defs.append((n, d[0].data.rstrip(b'\0')))
            values = {v for _, v in defs}
            if len(values) > 1:
                row['definitions'] = {n: v[:16].hex() for n, v in defs}
                return 'collision'
            # Spellings of one object (C/C++ names, stand-in types, a named literal).
            return 'alias'
        library = self.r.atlas_names.get(va)
        if library and any(n not in library for n in names):
            row['library'] = sorted(library)
            return 'collision'
        dtor = {re.sub(r'^\?\?_[EG]', '', n) for n in names}
        if len(dtor) == 1 and all(n.startswith(('??_E', '??_G')) for n in names):
            return 'deleting-dtor'      # scalar and vector deleting destructor of one class
        bodies = [(n, self.b.body(n)) for n in names]
        bodies = [(n, b) for n, b in bodies if b is not None]
        distinct = []
        for n, b in bodies:
            if not any(bodies_equal(b, d) for _, d in distinct):
                distinct.append((n, b))
        if len(bodies) >= 2 and len(distinct) == 1:
            return 'fold'               # identical linked bodies (COMDAT folding)
        if len(distinct) <= 1:
            # At most one body: the other names are callers' declarations of one
            # callee (stand-in classes and provisional signatures).
            return 'alias'
        if len(self.r.body(va)) <= TINY_BODY:
            return 'tiny_fold'
        row['linked_bodies'] = {n: len(b[0]) for n, b in distinct}
        return 'distinct-bodies'

    def link_alias_conflicts(self) -> list[dict]:
        """``config/game_build.json`` link aliases whose two names are bound to different VAs."""
        out = []
        for a, b in sorted(self.link_aliases.items()):
            va_a = {v for v in self.per_symbol.get(a, ()) if v}
            va_b = {v for v in self.per_symbol.get(b, ()) if v}
            if va_a and va_b and not va_a & va_b:
                out.append({'alias': a, 'provider': b, 'alias_vas': sorted(hx(v) for v in va_a),
                            'provider_vas': sorted(hx(v) for v in va_b), 'class': 'conflict'})
        return out

    def run(self) -> dict:
        rows = []
        for path, d in sorted(self.files.items()):
            for name, va in d.items():
                rows.append(self.audit_binding(path, name, va, d))
        conflicts = self.symbol_conflicts()
        collisions = self.va_collisions()
        aliases = self.link_alias_conflicts()
        summary = {
            'files': len(self.files), 'bindings': len(rows),
            'by_kind_status': {f'{k}/{s}': n for (k, s), n in sorted(
                Counter((r['kind'], r['status']) for r in rows).items())},
            'by_status': dict(sorted(Counter(r['status'] for r in rows).items())),
            'symbol_conflicts': dict(sorted(Counter(c['class'] for c in conflicts).items())),
            'va_collisions': dict(sorted(Counter(c['class'] for c in collisions).items())),
            'link_alias_conflicts': len(aliases),
        }
        findings = ([r for r in rows if r['status'] == 'mismatch']
                    + [c for c in conflicts if c['class'] == 'conflict']
                    + [c for c in collisions if c['class'] == 'collision'] + aliases)
        summary['findings'] = len(findings)
        return {'summary': summary, 'rows': rows, 'symbol_conflicts': conflicts,
                'va_collisions': collisions, 'link_alias_conflicts': aliases}


def render(report: dict) -> str:
    s = report['summary']
    lines = ['# Binding audit', '', f'{s["files"]} files, {s["bindings"]} bindings, '
             f'{s["findings"]} findings.', '', '| kind/status | bindings |', '|---|---:|']
    lines += [f'| {k} | {n} |' for k, n in s['by_kind_status'].items()]
    lines += ['', f'Symbol bound to several VAs: {s["symbol_conflicts"]}',
              f'VA bound to several symbols: {s["va_collisions"]}',
              f'Link aliases bound to different VAs: {s["link_alias_conflicts"]}', '', '## Mismatches', '']
    for r in report['rows']:
        if r['status'] == 'mismatch':
            lines.append(f'- `{r["symbol"]}` {r["va"]} ({r["kind"]}, {r["file"]}): {r.get("detail")}')
    lines += ['', '## Symbol conflicts', '']
    for c in report['symbol_conflicts']:
        if c['class'] in ('conflict', 'identical_bodies'):
            lines.append(f'- `{c["symbol"]}` [{c["class"]}]: {c["vas"]}')
    lines += ['', '## VA collisions', '']
    for c in report['va_collisions']:
        if c['class'] == 'collision':
            lines.append(f'- {c["va"]}: {c["files"]} {c.get("library") or c.get("definitions") or ""}')
    distinct = [c for c in report['va_collisions'] if c['class'] == 'distinct-bodies']
    if distinct:
        lines += ['', '## One VA, several distinct linked bodies (review)', '']
        lines += [f'- {c["va"]}: {c["linked_bodies"]}' for c in distinct]
    lines += ['', '## Link aliases bound to different VAs', '']
    for c in report.get('link_alias_conflicts', []):
        lines.append(f'- `{c["alias"]}` {c["alias_vas"]} -> `{c["provider"]}` {c["provider_vas"]}')
    notes = [r for r in report['rows'] if r.get('note')]
    if notes:
        lines += ['', '## Notes (comment VA disagrees with the binding)', '']
        lines += [f'- `{r["symbol"]}` {r["va"]} ({r["file"]}): {r["note"]}' for r in notes]
    return '\n'.join(lines) + '\n'


def initialised_images(retail: Retail, build: Path | None, root: Path = ROOT):
    """(our image, retail image, our placed data symbols) after each image's C++ initializers."""
    from mcm2tool.pe import PEImage
    from linked_data_accuracy import iat_slots, place_build
    from static_initializers import atlas_names, initializers
    hi = retail.base + retail.pe.size_of_image
    names = atlas_names(root)
    retail_init = initializers(retail.pe, names.get('__initterm'), retail.iat, names)
    retail_image = InitialisedImage(retail.read, retail_init, retail.base, hi)
    if build is None or not (build / 'mcm2.exe').is_file() or not (build / 'mcm2.map').is_file():
        return None, retail_image, None
    built = place_build(build, root)
    pe = PEImage(build / 'mcm2.exe')
    names = {m.name: m.va for m in built.map_syms}
    ours_init = initializers(pe, names.get('__initterm'), iat_slots(pe), names)
    ours_image = InitialisedImage(built.image.read, ours_init, pe.image_base,
                                  pe.image_base + pe.size_of_image)
    placed = [Placed(s.name, s.source, s.static, s.va, s.size) for s in built.syms]
    return ours_image, retail_image, placed


def latest_build(root: Path = ROOT) -> Path | None:
    builds = sorted((root / 'work/game-build').glob('build-*/report.json'),
                    key=lambda p: p.stat().st_mtime)
    return builds[-1].parent if builds else None


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('files', nargs='*', type=Path)
    ap.add_argument('--retail', type=Path, default=Path(os.environ.get('MCM2_EXE', ROOT / 'work/game/mcm2.exe')))
    ap.add_argument('--build', type=Path, default=None, help='linked build directory (default: newest)')
    ap.add_argument('--vc6-root', default=os.environ.get('VC6_ROOT'))
    ap.add_argument('--json', type=Path, default=None)
    ap.add_argument('--quiet', action='store_true')
    a = ap.parse_args(argv)
    from mcm2tool.pe import PEImage
    build = a.build or latest_build()
    if build is None:
        print('note: no linked build (run tools/build_game.py); game data symbols stay unknown',
              file=sys.stderr)
    paths = [p.resolve() for p in a.files] or binding_files(ROOT)
    files = {p.relative_to(ROOT).as_posix(): load_file(p) for p in paths}
    retail = Retail(PEImage(a.retail))
    ours_image, retail_image, placed = initialised_images(retail, build)
    auditor = Auditor(retail, BuildIndex(build), sdk_guids(a.vc6_root), files,
                      ours_image=ours_image, retail_image=retail_image, placed=placed)
    report = auditor.run()
    report['build'] = str(build) if build else None
    out = a.json or ROOT / 'work/binding-audit/report.json'
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(report, indent=1))
    md = render(report)
    out.with_suffix('.md').write_text(md, encoding='utf-8')
    if not a.quiet:
        print(md)
    return 1 if report['summary']['findings'] else 0


if __name__ == '__main__':
    raise SystemExit(main())
