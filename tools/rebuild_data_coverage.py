#!/usr/bin/env python3
"""Rebuild-readiness audit of the data and link side of the retail image.

Reads the retail PE (never executes it) and reports:

* PE header fields that VC6 LINK.EXE determines, with the link options they
  imply;
* the import directory and IAT layout, and which imports the reconstructed
  source already references (``__imp_`` bindings and API names in ``src/``);
* a byte attribution of ``.rdata``, the file-backed part of ``.data`` and the
  zero-filled tail of ``.data`` (``.bss``);
* the ``.rsrc`` tree (types, ids, languages and sizes only, no contents);
* extern globals declared in ``src/`` that no ``src/`` file defines.

Every attributed range carries an evidence tier (``confirmed``, ``strong`` or
``heuristic``, following AGENTS.md). The outputs go to an ignored directory
(default ``analysis/rebuild_data_coverage``); only a summary is printed.

Optional inputs, each skipped with a note when missing:
``work/vc6-crt-atlas/atlas.json`` (``make vc6-crt-atlas``) and
``$VC6_ROOT/VC98/LIB/LIBCMT.LIB`` for byte-matched CRT data contributions,
and ``capstone`` for code references into data.
"""
from __future__ import annotations

import argparse
import bisect
from collections import Counter, defaultdict
import json
import os
from pathlib import Path
import re
import struct
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from mcm2tool.pe import PEImage  # noqa: E402
from mcm2tool.rich import parse_rich  # noqa: E402
from mcm2tool.rtti import parse_rtti, find_type_descriptors  # noqa: E402

TIERS = ('confirmed', 'strong', 'heuristic')

# Claim order: an earlier category keeps bytes that a later one also claims.
CATEGORIES = (
    'import-tables',       # import descriptors, ILT, IAT, hint/name table, DLL names
    'crt-init-tables',     # .CRT$XC*/XI*/XP*/XT* pointer tables, incl. game $E entries
    'rtti',                # type descriptors, COL, class hierarchy, base class array/descriptors
    'vtables',             # COL slot + virtual function pointers, vbtables
    'eh-tables',           # FuncInfo, unwind/try/handler maps, throw info, SEH scope tables
    'crt-data',            # LIBCMT member .data/.rdata contributions (masked byte match)
    'float-constants',     # __real@4/__real@8 symbols bound by matched objects
    'string-literals',     # ??_C@ and __FILE__ literals bound by matched objects
    'guids',               # IID_/CLSID_/GUID_ ... constants bound by matched objects
    'library-data',        # other static-library data (dinput c_dfDI*, strmiids GUIDs)
    'game-globals',        # bound game globals and statics (extent partly inferred)
    'string-heuristic',    # NUL-terminated text found by scanning unattributed bytes
    'library-tail',        # unattributed bytes inside a run of library contributions
    'short-zero-runs',     # zero runs under 16 bytes: padding or unbound zero values
    'zero-unattributed',   # longer zero runs with no owner
    'unattributed',
)
CODE = {name: i + 1 for i, name in enumerate(CATEGORIES)}

GUID_PREFIXES = ('IID_', 'CLSID_', 'GUID_', 'DPSPGUID_', 'DPAID_', 'MSPID_',
                 'DSPROPSETID_', 'DPLPROPERTY_', 'DPSPGUID', 'FMTID_', 'LIBID_')
LIB_DATA_NAMES = ('c_dfDI',)
SCALAR_SIZES = {'C': 1, 'D': 1, 'E': 1, 'F': 2, 'G': 2, 'H': 4, 'I': 4, 'J': 4,
                'K': 4, 'M': 4, 'N': 8, 'O': 8, '_N': 1, '_J': 8, '_K': 8}

API_FAMILY_LIBS = {
    'kernel32.dll': 'kernel32.lib', 'user32.dll': 'user32.lib', 'gdi32.dll': 'gdi32.lib',
    'advapi32.dll': 'advapi32.lib', 'shell32.dll': 'shell32.lib', 'ole32.dll': 'ole32.lib',
    'winmm.dll': 'winmm.lib', 'msvfw32.dll': 'vfw32.lib', 'imm32.dll': 'imm32.lib',
    'dsound.dll': 'dsound.lib', 'wsock32.dll': 'wsock32.lib', 'dinput.dll': 'dinput.lib',
    'dplayx.dll': 'dplayx.lib', 'd3drm.dll': 'd3drm.lib', 'blade.dll': None,
}


def hx(n: int) -> str:
    return f'0x{n:08x}'


# --------------------------------------------------------------------------
# Pure helpers (unit tested without the retail image)
# --------------------------------------------------------------------------

def string_literal_size(symbol: str) -> int | None:
    """Byte size of a VC6 ``??_C@_`` string literal symbol, terminator included.

    The length follows the char-width digit: one decimal digit encodes n-1,
    otherwise hex letters A..P end with '@'.
    """
    m = re.match(r'\?\?_C@_([01])([0-9]|[A-P]+@)', symbol)
    if not m:
        return None
    code = m.group(2)
    if code.isdigit():
        return int(code) + 1
    value = 0
    for ch in code[:-1]:
        value = value * 16 + (ord(ch) - ord('A'))
    return value


def real_size(symbol: str) -> int | None:
    m = re.match(r'__real@([48])@', symbol)
    return int(m.group(1)) if m else None


def mangled_scalar_size(symbol: str) -> int | None:
    """Size of a global whose VC6 mangling names a scalar or pointer type.

    ``?x@@3HA`` (int) gives 4. Pointers give 4, but VC6 also mangles arrays
    as pointers, so callers must treat a pointer size as a lower bound.
    """
    m = re.match(r'\?[^@]+@(?:[^@]+@)*@[23](_[NJK]|[CDEFGHIJKMNO]|P[AB])', symbol)
    if not m:
        return None
    code = m.group(1)
    if code.startswith('P'):
        return 4
    return SCALAR_SIZES.get(code)


def symbol_base_name(symbol: str) -> str | None:
    """Source-level name of a bound data symbol (``Class::name`` or ``name``)."""
    if symbol.startswith('?') and not symbol.startswith('??'):
        m = re.match(r'\?([^@?]+)@((?:[^@?]+@)*)@[23]', symbol)
        if not m:
            return None
        scopes = [s for s in m.group(2).split('@') if s]
        return '::'.join(list(reversed(scopes)) + [m.group(1)])
    if re.match(r'_[A-Za-z][A-Za-z0-9_]*$', symbol):
        return symbol[1:]
    return None


def symbol_kind(symbol: str) -> str:
    if symbol.startswith('__imp_'):
        return 'import'
    if symbol.startswith('__real@'):
        return 'real'
    if symbol.startswith('??_C@'):
        return 'string'
    if symbol.startswith('__FILE__:'):
        return 'file-string'
    if symbol.startswith('??_7'):
        return 'vtable'
    if symbol.startswith('??_8'):
        return 'vbtable'
    if symbol.startswith('??_R'):
        return 'rtti'
    if symbol.endswith('$scopetable') or symbol.endswith('$ehhandler'):
        return 'eh'
    if '$E' in symbol or symbol.startswith('??__E'):
        return 'initializer'
    base = symbol[1:] if symbol.startswith('_') else symbol
    if base.startswith(GUID_PREFIXES) or 'U_GUID@@' in symbol or 'UUnknownGuid@@' in symbol:
        return 'guid'
    if base.startswith(LIB_DATA_NAMES):
        return 'library-data'
    if symbol.startswith('___') or symbol in ('__iob', '__osver', '__winver', '__winmajor',
                                               '__winminor', '__pgmptr', '__environ'):
        return 'crt-global'
    if '?$S' in symbol or '@?' in symbol or symbol.startswith('_$S'):
        return 'local-static'
    return 'global'


def scan_c_strings(data: bytes, base: int, min_len: int = 4) -> list[tuple[int, int]]:
    """4-aligned NUL-terminated printable strings: [(va, size incl. NUL)]."""
    out = []
    printable = set(range(0x20, 0x7f)) | {9, 10, 13} | set(range(0xa0, 0x100))
    i = 0
    n = len(data)
    while i < n:
        if i & 3 or data[i] not in printable:
            i += 1
            continue
        j = i
        while j < n and data[j] in printable:
            j += 1
        if j < n and data[j] == 0 and j - i >= min_len:
            out.append((base + i, j - i + 1))
            i = j + 1
        else:
            i = j + 1 if j > i else i + 1
    return out


def is_ascii_sorted(names: list[str]) -> bool:
    return names == sorted(names)


def derive_link_options(h: dict) -> list[dict]:
    """Map PE header facts to VC6 LINK options.

    ``h`` holds plain header values. Each row says which option is implied and
    why; 'default' means the VC6 default produces it with no switch.
    """
    rows = []

    def add(option, tier, evidence):
        rows.append({'option': option, 'tier': tier, 'evidence': evidence})

    if h['subsystem'] == 2:
        add('/SUBSYSTEM:WINDOWS', 'confirmed',
            f"subsystem 2 (GUI), version {h['subsystem_version']}")
    elif h['subsystem'] == 3:
        add('/SUBSYSTEM:CONSOLE', 'confirmed', 'subsystem 3 (CUI)')
    if h['image_base'] == 0x400000:
        add('/BASE default (0x400000)', 'confirmed', 'image base 0x00400000 is the EXE default')
    else:
        add(f"/BASE:{hx(h['image_base'])}", 'confirmed', 'non-default image base')
    if h['characteristics'] & 1 and not h['reloc_dir_size']:
        add('/FIXED (VC6 EXE default)', 'confirmed',
            'IMAGE_FILE_RELOCS_STRIPPED set and no base relocation directory; /FIXED:NO or '
            '/INCREMENTAL would emit .reloc')
    if h['file_alignment'] == 0x1000:
        add('/OPT:WIN98 (default; not /OPT:NOWIN98, no /ALIGN)', 'confirmed',
            'file alignment 0x1000; /OPT:NOWIN98 gives 0x200')
    elif h['file_alignment'] == 0x200:
        add('/OPT:NOWIN98', 'confirmed', 'file alignment 0x200')
    if not h['debug_dir_size']:
        add('no /DEBUG', 'confirmed', 'no debug directory, no PDB path')
    if not h['has_idata_section'] and h['iat_section'] == '.rdata':
        add('/INCREMENTAL:NO (default without /DEBUG)', 'strong',
            'IAT and import descriptors merged into .rdata; an incremental link keeps a '
            'separate .idata section, .reloc and a jump-thunk table')
    if h['stack'] == (0x100000, 0x1000) and h['heap'] == (0x100000, 0x1000):
        add('no /STACK, no /HEAP (defaults 1 MB/4 KB)', 'confirmed',
            'reserve/commit 0x100000/0x1000 for both')
    if h['checksum'] == 0:
        add('no /RELEASE (checksum 0)', 'heuristic',
            'checksum field is zero; under wibo a /RELEASE test link also wrote 0, so this '
            'is not discriminating here')
    if h['dll_characteristics'] == 0:
        add('no DLL characteristics', 'confirmed', 'DllCharacteristics 0 (VC6 has no /DYNAMICBASE)')
    if h['linker_version'] == '6.00':
        add('LINK.EXE 6.00', 'confirmed', 'optional header linker version 6.00')
    if h.get('entry_symbol'):
        add(f"entry {h['entry_symbol']} (default for /SUBSYSTEM:WINDOWS with WinMain)",
            'confirmed' if h.get('entry_symbol_source') == 'crt-atlas' else 'strong',
            f"entry point {hx(h['entry_va'])} is {h['entry_symbol']} ({h.get('entry_symbol_source')})")
    return rows


class Coverage:
    """Per-byte category map over a set of VA ranges; first claim wins."""

    def __init__(self, ranges: dict[str, tuple[int, int]]):
        self.ranges = dict(ranges)
        self.maps = {name: bytearray(end - start) for name, (start, end) in ranges.items()}
        self.tiers = {name: bytearray(end - start) for name, (start, end) in ranges.items()}
        self.items: list[dict] = []
        self.conflicts = Counter()

    def region_of(self, va: int) -> str | None:
        for name, (start, end) in self.ranges.items():
            if start <= va < end:
                return name
        return None

    def claim(self, va: int, size: int, category: str, tier: str, label: str = '',
              origin: str = 'game') -> int:
        if size <= 0:
            return 0
        code = CODE[category]
        tcode = TIERS.index(tier) + 1
        claimed = 0
        first = None
        for name, (start, end) in self.ranges.items():
            lo, hi = max(va, start), min(va + size, end)
            if lo >= hi:
                continue
            cmap, tmap = self.maps[name], self.tiers[name]
            for i in range(lo - start, hi - start):
                if cmap[i]:
                    if cmap[i] != code:
                        self.conflicts[(CATEGORIES[cmap[i] - 1], category)] += 1
                    continue
                cmap[i] = code
                tmap[i] = tcode
                claimed += 1
            first = first or name
        if claimed:
            self.items.append({'va': hx(va), 'size': size, 'category': category, 'tier': tier,
                               'label': label, 'origin': origin, 'section': first})
        return claimed

    def runs(self, name: str, code: int = 0):
        start, _ = self.ranges[name]
        cmap = self.maps[name]
        i = 0
        n = len(cmap)
        while i < n:
            if cmap[i] != code:
                i += 1
                continue
            j = i
            while j < n and cmap[j] == code:
                j += 1
            yield start + i, j - i
            i = j

    def summary(self) -> dict:
        out = {}
        for name, cmap in self.maps.items():
            counts = Counter(cmap)
            tiers = Counter()
            for c, t in zip(cmap, self.tiers[name]):
                if c:
                    tiers[(CATEGORIES[c - 1], TIERS[t - 1] if t else 'none')] += 1
            rows = {}
            for cat in CATEGORIES:
                b = counts.get(CODE[cat], 0)
                if cat == 'unattributed':
                    b += counts.get(0, 0)
                if b:
                    rows[cat] = {'bytes': b,
                                 'by_tier': {t: tiers[(cat, t)] for t in TIERS if tiers[(cat, t)]}}
            out[name] = {'range': [hx(self.ranges[name][0]), hx(self.ranges[name][1])],
                         'bytes': len(cmap), 'categories': rows}
        return out


def parse_resource_tree(blob: bytes, rsrc_rva: int) -> list[dict]:
    """Leaves of a PE resource tree: type/name/lang path, data RVA and size."""
    def name_at(off):
        n = struct.unpack_from('<H', blob, off)[0]
        return blob[off + 2:off + 2 + 2 * n].decode('utf-16le', 'replace')

    leaves = []

    def walk(off, path, depth):
        if depth > 3:
            return
        _, _, _, _, named, ids = struct.unpack_from('<IIHHHH', blob, off)
        for i in range(named + ids):
            nm, ptr = struct.unpack_from('<II', blob, off + 16 + 8 * i)
            key = name_at(nm & 0x7fffffff) if nm & 0x80000000 else nm
            if ptr & 0x80000000:
                walk(ptr & 0x7fffffff, path + [key], depth + 1)
            else:
                data_rva, size, codepage, _ = struct.unpack_from('<IIII', blob, ptr)
                leaves.append({'path': path + [key], 'data_rva': data_rva, 'size': size,
                               'codepage': codepage,
                               'data_offset': data_rva - rsrc_rva})
    walk(0, [], 0)
    return leaves


RT_NAMES = {1: 'RT_CURSOR', 2: 'RT_BITMAP', 3: 'RT_ICON', 4: 'RT_MENU', 5: 'RT_DIALOG',
            6: 'RT_STRING', 7: 'RT_FONTDIR', 8: 'RT_FONT', 9: 'RT_ACCELERATOR',
            10: 'RT_RCDATA', 11: 'RT_MESSAGETABLE', 12: 'RT_GROUP_CURSOR',
            14: 'RT_GROUP_ICON', 16: 'RT_VERSION', 17: 'RT_DLGINCLUDE', 19: 'RT_PLUGPLAY',
            20: 'RT_VXD', 21: 'RT_ANICURSOR', 22: 'RT_ANIICON', 23: 'RT_HTML', 24: 'RT_MANIFEST'}

EXTERN_RE = re.compile(r'^\s*extern\s+(?!"C"\s*\{|"C\+\+"\s*\{)(?:"C"\s+|"C\+\+"\s+)?([^;(){}]*?)'
                       r'([A-Za-z_][A-Za-z0-9_]*)\s*(\[[^\]]*\])*\s*;', re.M)
COMMENT_RE = re.compile(r'//[^\n]*|/\*.*?\*/', re.S)


def extern_variables(text: str) -> set[str]:
    """Names of variables declared ``extern`` (functions excluded)."""
    text = COMMENT_RE.sub('', text)
    out = set()
    for m in EXTERN_RE.finditer(text):
        decl = m.group(1)
        if not decl.strip() or 'typedef' in decl:
            continue
        out.add(m.group(2))
    return out


CTOR_ARGS_RE = re.compile(r"""\(\s*[-+\w.'"]+(?:\s*,\s*[-+\w.'"]+)*\s*\)\s*;""")


def strip_comments(text: str) -> str:
    return COMMENT_RE.sub('', text)


def defines_variable(text: str, name: str, stripped: bool = False) -> bool:
    """True when a file-scope (column 0) non-extern declaration defines ``name``.

    ``Type name;``, ``Type name[N] = {...};``, ``Type Class::name = v;`` and
    ``Type name(1, 2);`` count; prototypes such as ``Type name(int a);`` do not.
    """
    if not stripped:
        text = strip_comments(text)
    short = name.split('::')[-1]
    if short not in text:
        return False
    scope = re.escape(name) if '::' in name else rf'(?:[A-Za-z_]\w*::)*{re.escape(short)}'
    pat = re.compile(rf'^(?!extern\b|typedef\b|return\b|using\b|delete\b|goto\b)'
                     rf'[A-Za-z_][\w:<>,]*(?:[ \t\*&]+(?:const\b|volatile\b|[A-Za-z_][\w:<>,]*)?)*?'
                     rf'[ \t\*&]+{scope}[ \t]*(?:\[[^\]\n]*\][ \t]*)*(=|;|\{{|\()', re.M)
    for m in pat.finditer(text):
        if m.group(1) == '(' and not CTOR_ARGS_RE.match(text, m.end() - 1):
            continue
        return True
    return False


# --------------------------------------------------------------------------
# Retail image analysis
# --------------------------------------------------------------------------

def load_bindings(roots: list[Path]) -> dict[int, list[dict]]:
    by_va: dict[int, list[dict]] = defaultdict(list)
    for root in roots:
        for path in sorted(root.rglob('*.bindings.json')):
            try:
                data = json.loads(path.read_text(encoding='utf-8'))
            except (OSError, ValueError):
                continue
            for name, value in data.items():
                if not isinstance(value, str):
                    continue
                try:
                    va = int(value, 16)
                except ValueError:
                    continue
                rel = path.relative_to(ROOT).as_posix() if path.is_relative_to(ROOT) else str(path)
                by_va[va].append({'symbol': name, 'file': rel,
                                  'origin': 'src' if rel.startswith('src/') else 'samples'})
    return by_va


def pe_layout(pe: PEImage, entry_symbol: str | None, entry_source: str | None) -> dict:
    oh = pe.pe_offset + 24
    d = pe.data
    osv = struct.unpack_from('<HHHHHH', d, oh + 40)
    stack = struct.unpack_from('<II', d, oh + 72)
    heap = struct.unpack_from('<II', d, oh + 80)
    names = ['export', 'import', 'resource', 'exception', 'security', 'basereloc', 'debug',
             'architecture', 'globalptr', 'tls', 'load_config', 'bound_import', 'iat',
             'delay_import', 'clr', 'reserved']
    iat_rva = pe.directories[12][0] if len(pe.directories) > 12 else 0
    iat_sec = pe.section_for_rva(iat_rva) if iat_rva else None
    import datetime
    h = {
        'machine': hx(pe.machine), 'characteristics': pe.characteristics,
        'characteristics_flags': [n for bit, n in ((1, 'RELOCS_STRIPPED'), (2, 'EXECUTABLE_IMAGE'),
                                                  (4, 'LINE_NUMS_STRIPPED'), (8, 'LOCAL_SYMS_STRIPPED'),
                                                  (0x20, 'LARGE_ADDRESS_AWARE'), (0x100, '32BIT_MACHINE'),
                                                  (0x200, 'DEBUG_STRIPPED'), (0x2000, 'DLL'))
                                  if pe.characteristics & bit],
        'timestamp': pe.timestamp,
        'timestamp_utc': datetime.datetime.fromtimestamp(pe.timestamp, datetime.timezone.utc).isoformat(),
        'linker_version': f'{pe.linker_major}.{pe.linker_minor:02d}',
        'size_of_code': hx(pe.u32(oh + 4)), 'size_of_initialized_data': hx(pe.u32(oh + 8)),
        'size_of_uninitialized_data': hx(pe.u32(oh + 12)),
        'entry_va': pe.image_base + pe.entry_rva, 'base_of_code': hx(pe.u32(oh + 20)),
        'base_of_data': hx(pe.u32(oh + 24)), 'image_base': pe.image_base,
        'section_alignment': pe.section_alignment, 'file_alignment': pe.file_alignment,
        'os_version': f'{osv[0]}.{osv[1]}', 'image_version': f'{osv[2]}.{osv[3]}',
        'subsystem_version': f'{osv[4]}.{osv[5]}', 'win32_version_value': pe.u32(oh + 52),
        'size_of_image': hx(pe.size_of_image), 'size_of_headers': hx(pe.size_of_headers),
        'checksum': pe.u32(oh + 64), 'subsystem': pe.subsystem,
        'dll_characteristics': pe.u16(oh + 70), 'stack': stack, 'heap': heap,
        'loader_flags': pe.u32(oh + 88),
        'directories': {names[i]: [hx(r + pe.image_base) if r else '0', s]
                        for i, (r, s) in enumerate(pe.directories) if r or s},
        'reloc_dir_size': pe.directories[5][1] if len(pe.directories) > 5 else 0,
        'debug_dir_size': pe.directories[6][1] if len(pe.directories) > 6 else 0,
        'has_idata_section': any(s.name == '.idata' for s in pe.sections),
        'iat_section': iat_sec.name if iat_sec else None,
        'entry_symbol': entry_symbol, 'entry_symbol_source': entry_source,
        'sections': [{'name': s.name, 'va': hx(pe.image_base + s.virtual_address),
                      'virtual_size': hx(s.virtual_size), 'raw_size': hx(s.raw_size),
                      'raw_offset': hx(s.raw_offset), 'characteristics': hx(s.characteristics)}
                     for s in pe.sections],
    }
    rich = parse_rich(pe)
    h['rich'] = None if rich is None else [
        {'product': e.product, 'build': e.build, 'count': e.count} for e in rich[1]]
    h['link_options'] = derive_link_options(h)
    h['stack'] = [hx(x) for x in stack]
    h['heap'] = [hx(x) for x in heap]
    h['image_base'] = hx(pe.image_base)
    h['entry_va'] = hx(pe.image_base + pe.entry_rva)
    h['section_alignment'] = hx(pe.section_alignment)
    h['file_alignment'] = hx(pe.file_alignment)
    h['checksum'] = hx(h['checksum'])
    h['characteristics'] = hx(pe.characteristics)
    h['dll_characteristics'] = hx(h['dll_characteristics'])
    return h


def import_layout(pe: PEImage, cov: Coverage | None) -> dict:
    imp_rva, imp_size = pe.directories[1]
    off = pe.rva_to_offset(imp_rva)
    dlls = []
    index = 0
    while True:
        oft, stamp, chain, name_rva, iat = struct.unpack_from('<5I', pe.data, off + 20 * index)
        if not any((oft, stamp, chain, name_rva, iat)):
            break
        dll = pe.read_c_string_rva(name_rva)
        funcs = []
        k = 0
        while True:
            v = pe.u32(pe.rva_to_offset(oft) + 4 * k)
            if not v:
                break
            if v & 0x80000000:
                funcs.append({'ordinal': v & 0xffff, 'iat_va': hx(pe.image_base + iat + 4 * k)})
            else:
                hint = pe.u16(pe.rva_to_offset(v))
                nm = pe.read_c_string_rva(v + 2)
                funcs.append({'name': nm, 'hint': hint, 'iat_va': hx(pe.image_base + iat + 4 * k)})
                if cov:
                    size = 2 + len(nm) + 1
                    cov.claim(pe.image_base + v, size + (size & 1), 'import-tables', 'confirmed',
                              f'hint/name {nm}', 'import')
            k += 1
        if cov:
            cov.claim(pe.image_base + oft, 4 * (k + 1), 'import-tables', 'confirmed', f'ILT {dll}', 'import')
            cov.claim(pe.image_base + iat, 4 * (k + 1), 'import-tables', 'confirmed', f'IAT {dll}', 'import')
            size = len(dll) + 1
            cov.claim(pe.image_base + name_rva, size + (size & 1), 'import-tables', 'confirmed',
                      f'name {dll}', 'import')
        dlls.append({'dll': dll, 'descriptor_index': index, 'time_date_stamp': stamp,
                     'ilt_va': hx(pe.image_base + oft), 'iat_va': hx(pe.image_base + iat),
                     'iat_end_va': hx(pe.image_base + iat + 4 * (k + 1)),
                     'count': k, 'by_ordinal': sum(1 for f in funcs if 'ordinal' in f),
                     'functions': funcs})
        index += 1
    if cov:
        cov.claim(pe.image_base + imp_rva, 20 * (index + 1), 'import-tables', 'confirmed',
                  'import descriptors', 'import')
    iat_order = [d['dll'] for d in sorted(dlls, key=lambda d: int(d['iat_va'], 16))]
    return {'descriptor_order': [d['dll'] for d in dlls], 'iat_order': iat_order,
            'iat_order_is_ascii_sorted': is_ascii_sorted(iat_order),
            'total_functions': sum(d['count'] for d in dlls), 'dlls': dlls}


def crt_atlas_symbols(path: Path) -> dict[str, list]:
    if not path.is_file():
        return {}
    data = json.loads(path.read_text())
    out: dict[str, list] = defaultdict(list)
    for m in data.get('libcmt', {}).get('matches', []):
        out[m['symbol']].append(m)
    return out


def crt_intervals(atlas: dict[str, list]) -> list[tuple[int, int]]:
    iv = sorted({(m['target_va'], m['target_va'] + m['size']) for ms in atlas.values() for m in ms})
    return iv


def x87_memory_operand(insn: bytes) -> tuple[int, int] | None:
    """(absolute address, operand bytes) of an x87 ``[disp32]`` memory operand."""
    i = 0
    while i < len(insn) and insn[i] in (0x26, 0x2e, 0x36, 0x3e, 0x64, 0x65, 0x66, 0x67, 0x9b):
        i += 1
    if i + 6 > len(insn) or not 0xd8 <= insn[i] <= 0xdf:
        return None
    op, modrm = insn[i], insn[i + 1]
    if modrm & 0xc7 != 0x05:
        return None
    reg = (modrm >> 3) & 7
    size = {0xd8: 4, 0xda: 4, 0xdc: 8, 0xde: 2}.get(op)
    if op == 0xd9:
        size = 4 if reg in (0, 2, 3) else None
    elif op == 0xdb:
        size = 4 if reg <= 3 else (10 if reg in (5, 7) else None)
    elif op == 0xdd:
        size = 8 if reg <= 3 else None
    elif op == 0xdf:
        size = 2 if reg <= 3 else (10 if reg in (4, 6) else (8 if reg in (5, 7) else None))
    if not size:
        return None
    return struct.unpack_from('<I', insn, i + 2)[0], size


def code_references(pe: PEImage, lo: int, hi: int, fpu: list | None = None) -> list[tuple[int, int]] | None:
    """(instruction VA, absolute target) for every imm/disp in [lo, hi). Linear sweep.

    When ``fpu`` is a list, (target, size) of x87 absolute memory operands is appended.
    """
    try:
        import capstone
    except ImportError:
        return None
    sec = pe.sections[0]
    base = pe.image_base + sec.virtual_address
    code = pe.data[sec.raw_offset:sec.raw_offset + min(sec.virtual_size, sec.raw_size)]
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    refs = []
    off = 0
    while off < len(code):
        progressed = False
        for ins in md.disasm(code[off:], base + off):
            progressed = True
            b = ins.bytes
            for i in range(len(b) - 3):
                v = struct.unpack_from('<I', b, i)[0]
                if lo <= v < hi:
                    refs.append((ins.address, v))
            if fpu is not None and b[0] in (0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf):
                operand = x87_memory_operand(bytes(b))
                if operand and lo <= operand[0] < hi:
                    fpu.append(operand)
            off = ins.address + ins.size - base
        if not progressed or off < len(code):
            off += 1
    return refs


def u32va(pe: PEImage, va: int) -> int | None:
    try:
        return pe.u32(pe.va_to_offset(va))
    except Exception:
        return None


def claim_crt_tables(pe: PEImage, cov: Coverage, atlas: dict, report: dict) -> None:
    """Find _initterm(a, z) calls and claim each [a, z] pointer table."""
    init = atlas.get('__initterm')
    if not init:
        report['crt_init_tables'] = {'note': 'crt atlas missing: __initterm address unknown'}
        return
    target = init[0]['target_va']
    sec = pe.sections[0]
    base = pe.image_base + sec.virtual_address
    code = pe.data[sec.raw_offset:sec.raw_offset + sec.virtual_size]
    tables = []
    for m in re.finditer(rb'\x68(....)\x68(....)\xe8(....)', code, re.S):
        call_va = base + m.start() + 10
        rel = struct.unpack('<i', m.group(3))[0]
        if call_va + 5 + rel != target:
            continue
        z, a = struct.unpack('<I', m.group(1))[0], struct.unpack('<I', m.group(2))[0]
        if a < z and cov.region_of(a):
            tables.append((a, z))
    rows = []
    game_entries = crt_entries = 0
    span = report.get('crt_code_span')
    for a, z in sorted(set(tables)):
        ptrs = [u32va(pe, va) for va in range(a + 4, z, 4)]
        live = [p for p in ptrs if p]
        g = sum(1 for p in live if span and p < span[0])
        game_entries += g
        crt_entries += len(live) - g
        cov.claim(a, z + 4 - a, 'crt-init-tables', 'confirmed', f'_initterm table {hx(a)}..{hx(z)}', 'crt')
        rows.append({'start': hx(a), 'end_sentinel': hx(z), 'entries': len(ptrs),
                     'nonnull': len(live), 'game_entries': g})
    report['crt_init_tables'] = {'tables': rows, 'game_initializer_entries': game_entries,
                                 'crt_initializer_entries': crt_entries}


def claim_rtti_and_vtables(pe: PEImage, cov: Coverage, bindings: dict, report: dict) -> set[int]:
    tds = find_type_descriptors(pe)
    classes = parse_rtti(pe)
    lib_classes = {'type_info', 'exception', 'bad_cast', 'bad_typeid', '__non_rtti_object'}
    for va, td in tds.items():
        size = 8 + len(td.decorated) + 1
        cov.claim(va, (size + 3) & ~3, 'rtti', 'confirmed', f'TD {td.name}',
                  'crt' if td.name in lib_classes else 'game')
    vt_starts = set()
    seen_bcd = set()
    n_col = 0
    for c in classes:
        origin = 'crt' if c.name in lib_classes else 'game'
        chd = c.class_hierarchy_va
        cov.claim(chd, 16, 'rtti', 'confirmed', f'CHD {c.name}', origin)
        pba = u32va(pe, chd + 12)
        nb = u32va(pe, chd + 8) or 0
        if pba:
            cov.claim(pba, 4 * nb, 'rtti', 'confirmed', f'BCA {c.name}', origin)
            for i in range(nb):
                bcd = u32va(pe, pba + 4 * i)
                if bcd and bcd not in seen_bcd:
                    seen_bcd.add(bcd)
                    cov.claim(bcd, 24, 'rtti', 'confirmed', f'BCD in {c.name}', origin)
        for rec in c.vtable_records:
            n_col += 1
            cov.claim(rec.complete_object_locator_va, 20, 'rtti', 'confirmed', f'COL {c.name}', origin)
            for vt in rec.vtables:
                vt_starts.add(vt)
                n = vtable_length(pe, vt)
                cov.claim(vt - 4, 4 * (n + 1), 'vtables', 'confirmed',
                          f'vtable {c.name}+{rec.object_offset:#x} ({n} slots)', origin)
    # vtables and vbtables bound by matched objects but without an RTTI locator
    extra_vt = 0
    vbt = 0
    for va, rows in bindings.items():
        kinds = {symbol_kind(r['symbol']) for r in rows}
        if 'vtable' in kinds and va not in vt_starts:
            n = vtable_length(pe, va)
            if n:
                extra_vt += 1
                vt_starts.add(va)
                cov.claim(va, 4 * n, 'vtables', 'confirmed', f"vtable {rows[0]['symbol']} ({n} slots, no COL)")
        if 'vbtable' in kinds:
            n = vbtable_length(pe, va)
            vbt += 1
            cov.claim(va, 4 * n, 'vtables', 'strong', f"vbtable {rows[0]['symbol']} ({n} entries)")
    report['rtti'] = {'type_descriptors': len(tds), 'classes': len(classes),
                      'complete_object_locators': n_col, 'base_class_descriptors': len(seen_bcd),
                      'vtables_with_col': len(vt_starts) - extra_vt, 'vtables_bound_without_col': extra_vt,
                      'vbtables_bound': vbt}
    return vt_starts


def vtable_length(pe: PEImage, va: int, limit: int = 512) -> int:
    n = 0
    while n < limit:
        p = u32va(pe, va + 4 * n)
        if p is None or not pe.is_code_va(p):
            break
        n += 1
    return n


def vbtable_length(pe: PEImage, va: int) -> int:
    # vbtable: first entry is the vbptr's offset (<= 0), then positive vbase offsets.
    n = 1
    while n < 16:
        v = u32va(pe, va + 4 * n)
        if v is None or not (0 < v < 0x10000):
            break
        n += 1
    return n


def claim_eh(pe: PEImage, cov: Coverage, tds: set[int], report: dict) -> None:
    sec = pe.sections[0]
    base = pe.image_base + sec.virtual_address
    code = pe.data[sec.raw_offset:sec.raw_offset + sec.virtual_size]
    rdata = next(s for s in pe.sections if s.name == '.rdata')
    rlo, rhi = pe.image_base + rdata.virtual_address, pe.image_base + rdata.rva_end

    def in_r(v):
        return rlo <= v < rhi

    funcinfos = set()
    for m in re.finditer(rb'\xb8(....)\xe9', code, re.S):
        v = struct.unpack('<I', m.group(1))[0]
        if in_r(v) and u32va(pe, v) == 0x19930520:
            funcinfos.add(v)
    n_unwind = n_try = n_handlers = 0
    for fi in sorted(funcinfos):
        magic, max_state, p_unwind, n_tries, p_try, n_ip, p_ip = struct.unpack_from(
            '<7I', pe.data, pe.va_to_offset(fi))
        cov.claim(fi, 28, 'eh-tables', 'confirmed', f'FuncInfo {hx(fi)}')
        if p_unwind and 0 < max_state < 4096:
            cov.claim(p_unwind, 8 * max_state, 'eh-tables', 'confirmed', f'UnwindMap of {hx(fi)}')
            n_unwind += max_state
        if p_try and 0 < n_tries < 256:
            cov.claim(p_try, 20 * n_tries, 'eh-tables', 'confirmed', f'TryBlockMap of {hx(fi)}')
            n_try += n_tries
            for t in range(n_tries):
                _, _, _, n_catch, p_handlers = struct.unpack_from('<5I', pe.data, pe.va_to_offset(p_try + 20 * t))
                if p_handlers and 0 < n_catch < 64:
                    cov.claim(p_handlers, 16 * n_catch, 'eh-tables', 'confirmed', f'HandlerType of {hx(fi)}')
                    n_handlers += n_catch
    # Throw information: push offset ThrowInfo validated structurally.
    throwinfos = set()
    for m in re.finditer(rb'\x68(....)', code, re.S):
        v = struct.unpack('<I', m.group(1))[0]
        if not in_r(v) or v in throwinfos:
            continue
        ti = [u32va(pe, v + 4 * i) for i in range(4)]
        if None in ti or ti[0] not in (0, 1) or ti[2] != 0 or not in_r(ti[3]):
            continue
        if ti[1] and not pe.is_code_va(ti[1]):
            continue
        n = u32va(pe, ti[3])
        if not n or n > 32:
            continue
        cts = [u32va(pe, ti[3] + 4 + 4 * i) for i in range(n)]
        if not all(c and in_r(c) and u32va(pe, c + 4) in tds for c in cts):
            continue
        throwinfos.add(v)
        cov.claim(v, 16, 'eh-tables', 'confirmed', f'ThrowInfo {hx(v)}')
        cov.claim(ti[3], 4 + 4 * n, 'eh-tables', 'confirmed', f'CatchableTypeArray {hx(ti[3])}')
        for c in cts:
            cov.claim(c, 28, 'eh-tables', 'confirmed', f'CatchableType {hx(c)}')
    # SEH: push -1; push scopetable; push handler
    scopes = Counter()
    for m in re.finditer(rb'\x6a\xff\x68(....)\x68(....)', code, re.S):
        st, handler = struct.unpack('<I', m.group(1))[0], struct.unpack('<I', m.group(2))[0]
        if in_r(st) and pe.is_code_va(handler):
            scopes[st] += 1
    n_scope_entries = 0
    for st in sorted(scopes):
        k = 0
        while k < 64:
            row = [u32va(pe, st + 12 * k + 4 * i) for i in range(3)]
            if None in row:
                break
            enclosing = struct.unpack('<i', struct.pack('<I', row[0]))[0]
            if not (enclosing == -1 or 0 <= enclosing < k):
                break
            if not (pe.is_code_va(row[2]) and (row[1] == 0 or pe.is_code_va(row[1]))):
                break
            k += 1
        if k:
            n_scope_entries += k
            cov.claim(st, 12 * k, 'eh-tables', 'strong', f'SEH scopetable {hx(st)} ({k})')
    report['eh'] = {'funcinfo': len(funcinfos), 'unwind_entries': n_unwind, 'try_blocks': n_try,
                    'catch_handlers': n_handlers, 'throwinfo': len(throwinfos),
                    'seh_scopetables': len(scopes), 'seh_scope_entries': n_scope_entries}


def claim_libcmt_data(pe: PEImage, cov: Coverage, atlas: dict, libcmt: Path | None, report: dict) -> None:
    if not atlas or not libcmt or not libcmt.is_file():
        report['crt_data'] = {'note': 'LIBCMT.LIB or crt atlas missing; CRT data not byte-matched'}
        return
    from mcm2tool.coff import CoffObject, RELOC_WIDTH_I386, CoffError
    from mcm2tool.coff_archive import read_archive
    members = {m['archive_member'] for ms in atlas.values() for m in ms}
    regions = []
    for s in pe.sections:
        if s.name in ('.rdata', '.data'):
            regions.append((pe.image_base + s.virtual_address,
                            pe.data[s.raw_offset:s.raw_offset + min(s.raw_size, s.virtual_size)]))
    stats = Counter()
    with tempfile.TemporaryDirectory(prefix='mcm2-crtdata-') as tmp:
        obj_path = Path(tmp) / 'member.obj'
        for member in read_archive(libcmt):
            if member.name not in members:
                continue
            obj_path.write_bytes(member.data)
            try:
                obj = CoffObject(obj_path)
            except (CoffError, ValueError, IndexError):
                continue
            for sec in obj.sections:
                ch = sec.characteristics
                if (sec.name.startswith('.debug') or sec.name == '.drectve' or ch & 0x20
                        or not ch & 0x40 or not sec.raw_ptr or not sec.raw_size):
                    continue
                if sec.name.startswith(('.CRT', '.rdata$r', '.xdata')):
                    continue  # pointer tables, RTTI and EH data are claimed structurally
                raw = obj.data[sec.raw_ptr:sec.raw_ptr + sec.raw_size]
                mask = bytearray(len(raw))
                for r in obj.relocations:
                    if r.section_number == sec.index:
                        for i in range(r.virtual_address, min(len(raw), r.virtual_address + RELOC_WIDTH_I386.get(r.type, 4))):
                            mask[i] = 1
                nonzero = sum(1 for i, b in enumerate(raw) if b and not mask[i])
                best_start = best_len = run = 0
                for i in range(len(raw) + 1):
                    if i == len(raw) or mask[i]:
                        if i - run > best_len:
                            best_start, best_len = run, i - run
                        run = i + 1
                hits = []
                if best_len >= 4 and nonzero >= 8:
                    anchor = raw[best_start:best_start + best_len]
                    for base, data in regions:
                        p = data.find(anchor)
                        while p >= 0:
                            c = p - best_start
                            if c >= 0 and c + len(raw) <= len(data) and all(
                                    mask[i] or data[c + i] == raw[i] for i in range(len(raw))):
                                hits.append(base + c)
                            p = data.find(anchor, p + 1)
                key = 'unique' if len(hits) == 1 else ('ambiguous' if hits else 'unmatched')
                stats[f'{sec.name}:{key}:sections'] += 1
                stats[f'{sec.name}:{key}:bytes'] += len(raw)
                if len(hits) == 1:
                    cov.claim(hits[0], len(raw), 'crt-data', 'confirmed',
                              f"{member.name.split(chr(92))[-1]} {sec.name}", 'crt')
    report['crt_data'] = dict(sorted(stats.items()))


def claim_bindings(pe: PEImage, cov: Coverage, bindings: dict, report: dict,
                   anchors: set[int] = frozenset()) -> list[int]:
    starts = []
    counts = Counter()
    pending_globals = []
    for va in sorted(bindings):
        if not cov.region_of(va):
            continue
        rows = bindings[va]
        sym = rows[0]['symbol']
        kinds = Counter(symbol_kind(r['symbol']) for r in rows)
        kind = kinds.most_common(1)[0][0]
        origin = 'game'
        starts.append(va)
        if kind == 'real':
            counts['real'] += 1
            cov.claim(va, real_size(sym), 'float-constants', 'confirmed', sym)
        elif kind == 'string':
            size = string_literal_size(sym)
            counts['string'] += 1
            if size:
                cov.claim(va, size, 'string-literals', 'confirmed', sym)
        elif kind == 'file-string':
            counts['file-string'] += 1
            s = pe.read_c_string_at_offset(pe.va_to_offset(va))
            cov.claim(va, len(s) + 1, 'string-literals', 'confirmed', sym)
        elif kind == 'guid':
            counts['guid'] += 1
            cov.claim(va, 16, 'guids', 'confirmed', sym)
        elif kind == 'library-data':
            counts['library-data'] += 1
            # DIDATAFORMAT: dwSize, dwObjSize, dwFlags, dwDataSize, dwNumObjs, rgodf
            n = u32va(pe, va + 16) or 0
            arr = u32va(pe, va + 20) or 0
            cov.claim(va, 24, 'library-data', 'confirmed', sym, 'lib')
            if arr and 0 < n < 512:
                cov.claim(arr, 16 * n, 'library-data', 'confirmed', f'{sym} objects', 'lib')
                for i in range(n):
                    g = u32va(pe, arr + 16 * i)
                    if g:
                        cov.claim(g, 16, 'library-data', 'strong', f'{sym} object GUID', 'lib')
        elif kind in ('global', 'local-static', 'crt-global'):
            counts[kind] += 1
            pending_globals.append((va, sym, kind))
        else:
            counts[kind] += 1
    # Globals last: scalars by mangled type; pointers (VC6 mangles arrays as
    # pointers) and C names extend to the next known start or code reference.
    known = sorted(set(starts) | {int(i['va'], 16) for i in cov.items})
    with_refs = sorted(set(known) | set(anchors))
    for va, sym, kind in pending_globals:
        size = mangled_scalar_size(sym)
        tier = 'confirmed'
        pointer = bool(re.search(r'@@[23]P[AB]', sym))
        if size is None or pointer:
            # Pointers (maybe arrays) and untyped C names stop at the next code
            # reference too; class/struct globals stop at the next bound start
            # only, because code also refers to their fields. Without code
            # references a pointer is taken as 4 bytes.
            if pointer and not anchors:
                cov.claim(va, 4, 'game-globals', 'heuristic', sym)
                continue
            struct_typed = bool(re.search(r'@@[23][UV]', sym))
            pool = known if struct_typed else with_refs
            i = bisect.bisect_right(pool, va)
            nxt = pool[i] if i < len(pool) else va + 4
            region = cov.ranges[cov.region_of(va)]
            size = min(max(size or 0, min(nxt, region[1]) - va), 0x40000)
            tier = 'heuristic'
        origin = 'crt' if kind == 'crt-global' else 'game'
        cov.claim(va, size, 'crt-data' if kind == 'crt-global' else 'game-globals', tier, sym, origin)
    report['bindings'] = {'bound_data_addresses': len(starts), 'by_kind': dict(counts)}
    return known


def claim_guid_blocks(cov: Coverage, report: dict) -> None:
    """16-byte slots between two bound GUIDs of one block are GUIDs too (heuristic)."""
    guids = sorted(int(i['va'], 16) for i in cov.items if i['category'] == 'guids')
    got = 0
    for a, b in zip(guids, guids[1:]):
        gap = b - (a + 16)
        if 0 < gap <= 0x100 and gap % 16 == 0 and not a & 15:
            for va in range(a + 16, b, 16):
                got += cov.claim(va, 16, 'guids', 'heuristic', 'unbound slot in a GUID block')
    report['guid_block_fill_bytes'] = got


def claim_library_tail(cov: Coverage, report: dict, pe: PEImage | None = None) -> None:
    """Unattributed bytes between two library-origin items with no game item between."""
    items = sorted(((int(i['va'], 16), i['size'], i['origin'], i['section']) for i in cov.items),
                   key=lambda t: t[0])
    total = 0
    by_section = defaultdict(int)
    prev = None
    for va, size, origin, section in items:
        if origin in ('crt', 'lib') and prev and prev[2] in ('crt', 'lib') and prev[3] == section:
            gap_start = prev[0] + prev[1]
            if gap_start < va:
                got = cov.claim(gap_start, va - gap_start, 'library-tail', 'strong',
                                'between library contributions', 'lib')
                total += got
                by_section[section] += got
        if origin != 'import':
            prev = (va, size, origin, section)
    # A section whose last contribution is library data: the rest, up to the last
    # nonzero byte, follows it in link order.
    if pe is not None:
        last = {}
        for va, size, origin, section in items:
            if origin != 'import' and section:
                last[section] = (va, size, origin)
        for section, (va, size, origin) in last.items():
            if origin not in ('crt', 'lib') or section == '.bss':
                continue
            lo, hi = cov.ranges[section]
            raw = pe.bytes_at_va(va + size, hi - va - size)
            end = len(raw.rstrip(b'\0'))
            if end:
                got = cov.claim(va + size, end, 'library-tail', 'strong', 'after the last library contribution', 'lib')
                by_section[section] += got
    report['library_tail'] = dict(by_section)


def claim_heuristics(pe: PEImage, cov: Coverage) -> None:
    for name in list(cov.ranges):
        if name == '.bss':
            continue
        for va, size in list(cov.runs(name, 0)):
            data = pe.bytes_at_va(va, size)
            for sva, ssize in scan_c_strings(data, va):
                cov.claim(sva, ssize, 'string-heuristic', 'heuristic', 'scanned text')
    for name in list(cov.ranges):
        for va, size in list(cov.runs(name, 0)):
            data = pe.bytes_at_va(va, size) if name != '.bss' else b'\0' * size
            i = 0
            while i < size:
                if data[i]:
                    i += 1
                    continue
                j = i
                while j < size and not data[j]:
                    j += 1
                if j - i < 16:
                    cov.claim(va + i, j - i, 'short-zero-runs', 'heuristic', '')
                else:
                    cov.claim(va + i, j - i, 'zero-unattributed', 'heuristic', '')
                i = j


def resources(pe: PEImage) -> dict:
    rva, size = pe.directories[2]
    if not rva:
        return {'present': False}
    sec = pe.section_for_rva(rva)
    off = pe.rva_to_offset(rva)
    blob = pe.data[off:off + sec.virtual_size - (rva - sec.virtual_address)]
    leaves = parse_resource_tree(blob, rva)
    by_type = defaultdict(list)
    for leaf in leaves:
        t = leaf['path'][0]
        tname = RT_NAMES.get(t, str(t)) if isinstance(t, int) else t
        row = {'id': leaf['path'][1], 'lang': leaf['path'][2] if len(leaf['path']) > 2 else None,
               'size': leaf['size'], 'data_va': hx(pe.image_base + leaf['data_rva'])}
        doff = pe.rva_to_offset(leaf['data_rva'])
        if t == 3 and leaf['size'] >= 40 and pe.u32(doff) == 40:  # BITMAPINFOHEADER
            row['width'] = pe.u32(doff + 4)
            row['height'] = pe.u32(doff + 8) // 2
            row['bit_count'] = pe.u16(doff + 14)
        if t == 14:
            row['entries'] = pe.u16(doff + 4)
        if t == 16 and leaf['size'] >= 92:
            sig = pe.data[doff + 6:doff + 38].decode('utf-16le', 'replace').rstrip('\0')
            fixed = doff + 40
            if pe.u32(fixed) == 0xFEEF04BD:
                fv = (pe.u32(fixed + 8), pe.u32(fixed + 12))
                pv = (pe.u32(fixed + 16), pe.u32(fixed + 20))
                row['key'] = sig
                row['file_version'] = f'{fv[0] >> 16}.{fv[0] & 0xffff}.{fv[1] >> 16}.{fv[1] & 0xffff}'
                row['product_version'] = f'{pv[0] >> 16}.{pv[0] & 0xffff}.{pv[1] >> 16}.{pv[1] & 0xffff}'
                row['file_os'] = hx(pe.u32(fixed + 32))
                row['file_type'] = pe.u32(fixed + 36)
        by_type[tname].append(row)
    return {'present': True, 'directory_va': hx(pe.image_base + rva), 'directory_size': size,
            'section_virtual_size': hx(sec.virtual_size), 'leaves': len(leaves),
            'types': {k: {'count': len(v), 'entries': v} for k, v in by_type.items()}}


IMPORT_LIBS = ('kernel32.lib', 'user32.lib', 'gdi32.lib', 'advapi32.lib', 'shell32.lib', 'ole32.lib',
               'winmm.lib', 'vfw32.lib', 'imm32.lib', 'dsound.lib', 'wsock32.lib', 'dinput.lib',
               'dplayx.lib', 'd3drm.lib', 'ddraw.lib')


def short_import_name(symbol: str, name_type: int) -> str:
    """Import name a short import member records (PE/COFF IMPORT_OBJECT_NAME_TYPE)."""
    if name_type == 0:
        return ''
    if name_type == 1:
        return symbol
    name = symbol[1:] if symbol[:1] in ('_', '@', '?') else symbol
    if name_type == 3:
        name = name.split('@', 1)[0]
    return name


def import_library_check(imports: dict, lib_dir: Path | None) -> dict:
    """Compare retail imports (name, hint, ordinal) with the VC6 import libraries."""
    if not lib_dir or not lib_dir.is_dir():
        return {'note': 'VC6 LIB directory missing'}
    from mcm2tool.coff_archive import read_archive
    files = {p.name.lower(): p for p in lib_dir.iterdir()}
    index: dict[tuple[str, str], tuple[str, int]] = {}
    by_ordinal: dict[tuple[str, int], tuple[str, str]] = {}
    for lib in IMPORT_LIBS:
        path = files.get(lib)
        if not path:
            continue
        for member in read_archive(path):
            d = member.data
            if len(d) < 24 or struct.unpack_from('<HH', d, 0) != (0, 0xffff):
                continue
            _, _, _, _, ord_hint, typ = struct.unpack_from('<HHIIHH', d, 4)
            sym = d[20:].split(b'\0', 1)[0].decode('ascii', 'replace')
            dll = d[20 + len(sym) + 1:].split(b'\0', 1)[0].decode('ascii', 'replace').lower()
            name_type = (typ >> 2) & 7
            if name_type == 0:
                by_ordinal[(dll, ord_hint)] = (lib, sym)
            else:
                index[(dll, short_import_name(sym, name_type))] = (lib, ord_hint)
    rows = []
    for d in imports['dlls']:
        dll = d['dll'].lower()
        same = differ = missing = 0
        missing_names = []
        libs = Counter()
        for f in d['functions']:
            if 'ordinal' in f:
                hit = by_ordinal.get((dll, f['ordinal']))
                if hit:
                    same += 1
                    libs[hit[0]] += 1
                    f['vc6_symbol'] = hit[1]
                else:
                    missing += 1
                    missing_names.append(f"ordinal {f['ordinal']}")
                continue
            hit = index.get((dll, f['name']))
            if not hit:
                missing += 1
                missing_names.append(f['name'])
            elif hit[1] == f['hint']:
                same += 1
                libs[hit[0]] += 1
            else:
                differ += 1
                libs[hit[0]] += 1
        rows.append({'dll': d['dll'], 'imports': d['count'], 'vc6_lib': sorted(libs),
                     'hint_or_ordinal_equal': same, 'hint_differs': differ,
                     'absent_from_vc6_libs': missing, 'absent': missing_names})
    return {'per_dll': rows,
            'absent_total': sum(r['absent_from_vc6_libs'] for r in rows),
            'hint_differs_total': sum(r['hint_differs'] for r in rows)}


def source_usage(imports: dict, bindings: dict, targets: dict | None = None) -> dict:
    """Which imported functions the reconstructed source references."""
    src_files = [p for p in (ROOT / 'src').rglob('*') if p.suffix.lower() in ('.cpp', '.h', '.c')]
    text = '\n'.join(p.read_text(encoding='utf-8', errors='replace') for p in src_files)
    bound = defaultdict(set)
    for va, rows in bindings.items():
        for r in rows:
            if r['symbol'].startswith('__imp_'):
                bound[va].add(r['origin'])
    per_dll = []
    for d in imports['dlls']:
        named = [f for f in d['functions'] if 'name' in f]
        in_text = [f['name'] for f in named if re.search(rf'\b{re.escape(f["name"])}\b', text)
                   or (f['name'].endswith('A') and re.search(rf'\b{re.escape(f["name"][:-1])}\s*\(', text))]
        src_bound = [f for f in d['functions'] if 'src' in bound.get(int(f['iat_va'], 16), ())]
        refs = {f.get('name', f"ordinal {f.get('ordinal')}"): sorted((targets or {}).get(int(f['iat_va'], 16), ()))
                for f in d['functions']}
        per_dll.append({'dll': d['dll'], 'imported': d['count'], 'by_ordinal': d['by_ordinal'],
                        'bound_by_src_objects': len(src_bound),
                        'named_in_src_text': len(set(in_text)),
                        'referenced_only_by_crt_code': sum(1 for v in refs.values() if v == ['crt']),
                        'not_bound_by_src': [f.get('name', f"ordinal {f.get('ordinal')}") for f in d['functions']
                                             if 'src' not in bound.get(int(f['iat_va'], 16), ())],
                        'not_bound_and_referenced_by_non_crt_code': [
                            n for f in d['functions'] for n in [f.get('name', f"ordinal {f.get('ordinal')}")]
                            if 'src' not in bound.get(int(f['iat_va'], 16), ()) and 'other' in refs[n]]})
    return {'per_dll': per_dll,
            'total_imported': imports['total_functions'],
            'total_referenced_only_by_crt_code': sum(r['referenced_only_by_crt_code'] for r in per_dll),
            'total_not_bound_but_used_by_non_crt_code': sum(len(r['not_bound_and_referenced_by_non_crt_code']) for r in per_dll),
            'total_bound_by_src': sum(r['bound_by_src_objects'] for r in per_dll)}


def globals_audit(bindings: dict, data_lo: int, data_hi: int) -> dict:
    files = [p for p in (ROOT / 'src').rglob('*') if p.suffix.lower() in ('.cpp', '.h', '.c')]
    texts = {p: strip_comments(p.read_text(encoding='utf-8', errors='replace')) for p in files}
    cpp_texts = {p: t for p, t in texts.items() if p.suffix.lower() in ('.cpp', '.c')}
    declared = set()
    for t in texts.values():
        declared |= extern_variables(t)
    undefined = sorted(n for n in declared if not any(defines_variable(t, n, True) for t in cpp_texts.values()))
    # Bound data symbols from src objects: the link must resolve each to one definition.
    addr_names = defaultdict(set)
    bound_symbols = {}
    for va, rows in bindings.items():
        if not data_lo <= va < data_hi:
            continue
        for r in rows:
            if r['origin'] != 'src':
                continue
            k = symbol_kind(r['symbol'])
            if k in ('global', 'crt-global', 'guid', 'library-data'):
                name = symbol_base_name(r['symbol'])
                if name:
                    addr_names[va].add(name)
                    bound_symbols[r['symbol']] = (va, name, k)
    game_bound = {s: v for s, v in bound_symbols.items() if v[2] == 'global'}
    undefined_bound = sorted({v[1] for v in game_bound.values()
                              if not any(defines_variable(t, v[1], True) for t in cpp_texts.values())})
    aliased = {hx(va): sorted(names) for va, names in addr_names.items() if len(names) > 1}
    return {'extern_variable_names_declared': len(declared),
            'extern_declared_without_src_definition': len(undefined),
            'extern_undefined_names': undefined,
            'bound_game_global_symbols': len(game_bound),
            'bound_game_global_addresses': len({v[0] for v in game_bound.values()}),
            'bound_game_global_names_without_src_definition': len(undefined_bound),
            'bound_names_without_definition': undefined_bound,
            'addresses_bound_under_several_names': len(aliased),
            'aliased_addresses': dict(sorted(aliased.items()))}


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('--exe', default=os.environ.get('MCM2_EXE', 'work/game/mcm2.exe'))
    ap.add_argument('--out', default='analysis/rebuild_data_coverage')
    ap.add_argument('--crt-atlas', default='work/vc6-crt-atlas/atlas.json')
    ap.add_argument('--libcmt', default=None, help='default: $VC6_ROOT/VC98/LIB/LIBCMT.LIB')
    ap.add_argument('--no-code-refs', action='store_true', help='skip the capstone sweep')
    args = ap.parse_args(argv)

    pe = PEImage(args.exe)
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    report: dict = {}
    atlas = crt_atlas_symbols(Path(args.crt_atlas))
    if atlas:
        iv = crt_intervals(atlas)
        report['crt_code_span'] = [iv[0][0], iv[-1][1]]
    entry_va = pe.image_base + pe.entry_rva
    entry = next(((s, 'crt-atlas') for s, ms in atlas.items() for m in ms if m['target_va'] == entry_va),
                 (None, None))
    layout = pe_layout(pe, *entry)

    secs = {s.name: s for s in pe.sections}
    rd, da = secs['.rdata'], secs['.data']
    b = pe.image_base
    # VC6 merges .bss into .data: the file-backed part ends with zero-filled
    # .bss contributions, so the split is placed after the last nonzero byte.
    data_lo = b + da.virtual_address
    raw = pe.data[da.raw_offset:da.raw_offset + min(da.raw_size, da.virtual_size)]
    init_end = data_lo + ((len(raw.rstrip(b'\0')) + 3) & ~3)
    ranges = {
        '.rdata': (b + rd.virtual_address, b + rd.virtual_address + rd.virtual_size),
        '.data': (data_lo, init_end),
        '.bss': (init_end, data_lo + da.virtual_size),
    }
    report['data_split'] = {'initialized_end': hx(init_end), 'file_backed_end': hx(data_lo + len(raw)),
                            'note': 'initialized end = last nonzero byte rounded to 4 (heuristic)'}
    cov = Coverage(ranges)
    bindings = load_bindings([ROOT / 'src', ROOT / 'samples'])

    fpu: list = []
    refs = None if args.no_code_refs else code_references(pe, ranges['.rdata'][0], ranges['.bss'][1], fpu)
    span = report.get('crt_code_span')
    targets: dict[int, set] = defaultdict(set)
    for src_va, tgt in refs or ():
        targets[tgt].add('crt' if span and span[0] <= src_va < span[1] else 'other')

    imports = import_layout(pe, cov)
    claim_crt_tables(pe, cov, atlas, report)
    claim_rtti_and_vtables(pe, cov, bindings, report)
    tds = set(find_type_descriptors(pe))
    claim_eh(pe, cov, tds, report)
    libcmt = Path(args.libcmt) if args.libcmt else (
        Path(os.environ['VC6_ROOT']) / 'VC98' / 'LIB' / 'LIBCMT.LIB' if os.environ.get('VC6_ROOT') else None)
    claim_libcmt_data(pe, cov, atlas, libcmt, report)
    claim_bindings(pe, cov, bindings, report, set(targets))
    if refs is not None:
        # CRT .bss: lowest address above which only CRT code (or CRT-named bindings) refers.
        lo, hi = ranges['.bss']
        bss_targets = sorted(t for t in targets if lo <= t < hi)
        crt_named = {va for va, rows in bindings.items()
                     if any(symbol_kind(r['symbol']) == 'crt-global' for r in rows)}
        boundary = hi
        for t in reversed(bss_targets):
            if targets[t] == {'crt'} or t in crt_named:
                boundary = t
            else:
                break
        if boundary < hi:
            got = cov.claim(boundary, hi - boundary, 'crt-data', 'strong',
                            'CRT .bss tail (only CRT code refers above this address)', 'crt')
            report['crt_bss_tail'] = {'start': hx(boundary), 'bytes': hi - boundary, 'newly_claimed': got}
    # x87 operands into .rdata: constants of the decoded operand size.
    n_fpu = Counter()
    for va, size in sorted(set(fpu)):
        if ranges['.rdata'][0] <= va < ranges['.rdata'][1]:
            got = cov.claim(va, size, 'float-constants', 'strong', f'x87 m{size * 8} operand')
            n_fpu['claimed_bytes'] += got
            n_fpu['operands'] += 1
    report['x87_rdata_operands'] = dict(n_fpu)
    claim_guid_blocks(cov, report)
    claim_library_tail(cov, report, pe)
    claim_heuristics(pe, cov)

    if refs is not None:
        unattr = {}
        for name in ranges:
            runs = sorted(list(cov.runs(name, CODE['zero-unattributed'])) + list(cov.runs(name, 0)))
            starts = sorted(t for t in targets if ranges[name][0] <= t < ranges[name][1])
            hit = hit_bytes = 0
            for va, size in runs:
                i = bisect.bisect_left(starts, va)
                if i < len(starts) and starts[i] < va + size:
                    hit += 1
                    hit_bytes += size
            unattr[name] = {'runs': len(runs), 'runs_with_code_reference': hit,
                            'bytes_in_runs_with_code_reference': hit_bytes}
        report['code_references'] = {'references': len(refs), 'distinct_targets': len(targets),
                                     'unattributed_and_zero_runs': unattr}
    else:
        report['code_references'] = {'note': 'capstone unavailable or --no-code-refs'}

    summary = cov.summary()
    usage = source_usage(imports, bindings, targets if refs is not None else None)
    vc6_lib = (Path(os.environ['VC6_ROOT']) / 'VC98' / 'LIB') if os.environ.get('VC6_ROOT') else None
    libcheck = import_library_check(imports, vc6_lib)
    globs = globals_audit(bindings, ranges['.rdata'][0], ranges['.bss'][1])
    res = resources(pe)

    (out / 'pe_layout.json').write_text(json.dumps(layout, indent=2) + '\n')
    (out / 'imports.json').write_text(json.dumps({'layout': imports, 'source_usage': usage, 'vc6_import_libraries': libcheck}, indent=2) + '\n')
    (out / 'coverage_summary.json').write_text(json.dumps({'sections': summary, 'details': report,
                                                           'conflicts': {f'{a}>{b}': n for (a, b), n in cov.conflicts.items()}},
                                                          indent=2, default=str) + '\n')
    (out / 'coverage_items.json').write_text(json.dumps(sorted(cov.items, key=lambda i: int(i['va'], 16)), indent=1) + '\n')
    gaps = []
    for name in ranges:
        for va, size in cov.runs(name, CODE['unattributed']):
            gaps.append({'section': name, 'va': hx(va), 'size': size})
        for va, size in cov.runs(name, 0):
            gaps.append({'section': name, 'va': hx(va), 'size': size})
    (out / 'unattributed.json').write_text(json.dumps(sorted(gaps, key=lambda g: -g['size']), indent=1) + '\n')
    (out / 'resources.json').write_text(json.dumps(res, indent=2) + '\n')
    (out / 'globals.json').write_text(json.dumps(globs, indent=2) + '\n')

    print(f'rebuild data coverage -> {out}/')
    print(f"link: {', '.join(r['option'] for r in layout['link_options'])}")
    print(f"imports: {len(imports['dlls'])} DLLs, {imports['total_functions']} functions, "
          f"{usage['total_bound_by_src']} bound by src objects; IAT order ASCII-sorted: "
          f"{imports['iat_order_is_ascii_sorted']}; {usage.get('total_referenced_only_by_crt_code')} "
          f"referenced only by CRT code")
    if 'per_dll' in libcheck:
        print(f"VC6 import libs: {libcheck['absent_total']} imports absent, {libcheck['hint_differs_total']} "
              f"with a different hint: " + ', '.join(f"{r['dll']} {r['absent']}" for r in libcheck['per_dll']
                                                       if r['absent_from_vc6_libs']))
    for name, s in summary.items():
        print(f"{name} {s['range'][0]}..{s['range'][1]} ({s['bytes']} bytes)")
        for cat, row in sorted(s['categories'].items(), key=lambda kv: -kv[1]['bytes']):
            tiers = ', '.join(f'{t} {n}' for t, n in row['by_tier'].items())
            print(f"  {cat:20} {row['bytes']:9} {100.0 * row['bytes'] / s['bytes']:6.2f}%  {tiers}")
    if res.get('present'):
        print('resources: ' + ', '.join(f"{t} x{v['count']}" for t, v in res['types'].items()))
    print(f"globals: {globs['extern_declared_without_src_definition']} of "
          f"{globs['extern_variable_names_declared']} extern variable names lack a src definition; "
          f"{globs['bound_game_global_addresses']} bound game-global addresses, "
          f"{globs['addresses_bound_under_several_names']} bound under several names")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
