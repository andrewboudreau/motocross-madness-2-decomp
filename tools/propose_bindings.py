#!/usr/bin/env python3
"""Propose relocation bindings for physics targets, each backed by independent evidence.

Usage:
    python tools/propose_bindings.py --report work/physics.json --objs work/physics-objs \
        --source samples/physics/vehicle/Vehicle.cpp [--source ...] [--write]

The report and objects come from `tools/run_physics_samples.py --json-out ... --out ...`.
Only targets whose relocation-masked comparison is exact are used, because only there
do the candidate's relocation fields line up with retail.

For each relocation the retail bytes imply an address. That alone would be fitting, so
a binding is accepted only when a check independent of those bytes confirms what lives
at the address:

    reviewed     the same symbol is bound to the same VA in a reviewed *.bindings.json
                 (or another non-local symbol is reviewed at that VA: a stand-in alias)
    matched      the symbol is a physics target at that VA whose body matches (masked)
    target       the symbol is a declared physics target at that VA (partial body)
                 (a masked-exact target at that VA under another stand-in name also counts,
                 unless this symbol is itself a target elsewhere: that is a conflict)
    vptr-write   ??0X / ??1X: the retail function installs X's RTTI primary table;
                 ??_E / ??_G X: one of X's RTTI tables holds the address
    vtable       ??_7 table: RTTI lists that table for the class at the named base offset
    rtti-slot    UnknownVirtualSlotN / <Base>VirtualSlotN: the RTTI table holds it at slot N
    real         __real@ constant: the retail bytes there encode the named value
    file         __FILE__ literal: the retail string is that krusty2 source path
    string       ??_C@ literal: the retail bytes equal the object's literal bytes
    data-bytes   initialised data defined in the object (e.g. a ??_8 vbtable): retail bytes equal
    initializer  data written by a matched `$E` initializer body of the same object at that VA
    ehhandler    EH stub `mov eax, FuncInfo; jmp`, and FuncInfo starts with 0x19930520
    import       __imp_ slot: the import table names that function there
    named-address  a stand-in whose name is its own address (Fn_00438e70, g_Bike_0056e26c),
                 or whose declaration line in samples/physics or src/krusty2 names that address:
                 code must be a retail call target or table entry, data must be outside .text.
                 This proves only where the reference goes, not what the callee is.

TU-local symbols (static storage: `_$E` thunks, per-TU constants) never take evidence from
another object's bindings, because every TU has its own copy.

Every site of a symbol must imply the same address, or the symbol is a conflict. A
reconstructed function is identified only by its own target: if a symbol is a target at one
address and its call sites imply another, the source calls the wrong function (or lays out
cases in a different order) and the masked match hid it. A recursive function's self-call is
resolved like an internal label.
Anything else is listed as unproven with its implied address and site count, for review;
it is never written. Accepted bindings are evidence for the comparison, not identity proof.
"""
from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from mcm2tool.coff import CoffObject  # noqa: E402
from mcm2tool.pe import PEImage  # noqa: E402
from mcm2tool.resolved_match import RelocationError, match_object  # noqa: E402

ACCEPTED = ('reviewed', 'matched', 'target', 'vtable', 'rtti-slot', 'vptr-write', 'real', 'file', 'string',
            'data-bytes', 'initializer', 'ehhandler', 'import', 'named-address')


class _Recorder(dict):
    """Bindings stand-in: every symbol resolves to 0 so the audit lists every relocation."""

    def __init__(self, own: str = ''):
        super().__init__()
        self.own = own  # a recursive function's self-call resolves like an internal label

    def __contains__(self, key):
        return not key.startswith('$L') and key != self.own  # internal labels resolve themselves

    def __getitem__(self, key):
        return 0


def obj_path(objs: Path, source: str) -> Path:
    rel = Path(source.replace('\\', '/'))
    return objs / (str(rel.with_suffix('')).replace('\\', '_').replace('/', '_') + '.obj')


def decode_real(name: str) -> bytes | None:
    """`__real@4@<x87 80-bit hex>` -> the 4- or 8-byte IEEE encoding of that value."""
    m = re.fullmatch(r'__real@([48])@([0-9a-f]{20})', name)
    if not m:
        return None
    width, h = int(m.group(1)), m.group(2)
    se, mant = int(h[:4], 16), int(h[4:], 16)
    sign, exp = se >> 15, se & 0x7fff
    value = 0.0 if exp == 0 and mant == 0 else mant / (1 << 63) * 2.0 ** (exp - 16383)
    value = -value if sign else value
    return struct.pack('<f' if width == 4 else '<d', value)


def iat_names(pe: PEImage) -> dict[int, str]:
    out = {}
    imp_rva = pe.directories[1][0] if len(pe.directories) > 1 else 0
    if not imp_rva:
        return out
    off = pe.rva_to_offset(imp_rva)
    while True:
        oft, _, _, name_rva, ft = struct.unpack_from('<IIIII', pe.data, off)
        if not any((oft, name_rva, ft)):
            break
        to, slot = pe.rva_to_offset(oft or ft), ft
        while True:
            val = pe.u32(to)
            if val == 0:
                break
            if not val & 0x80000000:
                out[pe.image_base + slot] = pe.read_c_string_at_offset(pe.rva_to_offset(val) + 2)
            to += 4
            slot += 4
        off += 20
    return out


class Evidence:
    def __init__(self, pe: PEImage, report: list[dict]):
        self.pe = pe
        self.reviewed: dict[str, dict[int, str]] = defaultdict(dict)
        for p in sorted(ROOT.glob('src/**/*.bindings.json')) + sorted(ROOT.glob('samples/**/*.bindings.json')):
            for sym, va in json.loads(p.read_text()).items():
                self.reviewed[sym].setdefault(int(va, 16), str(p.relative_to(ROOT)).replace('\\', '/'))
        # Only external, named code/data: TU-local `$E` thunks and per-TU constants of one
        # object never vouch for another object's symbol at the same address.
        self.reviewed_va = {va: (sym, sorted(where.values())[0]) for sym, where in self.reviewed.items()
                            if not sym.startswith(('_$E', '_k', '$', '__FILE__')) for va in where}
        # Declarations that state their retail address: identifier -> addresses on its line.
        self.declared = defaultdict(set)
        for p in list(ROOT.glob('samples/physics/**/*')) + list(ROOT.glob('src/krusty2/**/*')):
            if p.suffix not in ('.h', '.cpp'):
                continue
            for line in p.read_text(encoding='latin-1').splitlines():
                vas = {int(m, 16) for m in re.findall(r'0x0*([4-6][0-9a-fA-F]{5})\b', line)}
                if vas:
                    for ident in re.findall(r'[A-Za-z_]\w*', line.split('//')[0]):
                        self.declared[ident] |= vas
        self.targets: dict[str, list[tuple[int, bool, str]]] = defaultdict(list)
        self.by_va: dict[int, list[tuple[str, bool, str]]] = defaultdict(list)
        for r in report:
            if 'symbol' in r:
                va, masked = int(r['expected']['target_va'], 0), bool(r.get('exact_after_relocation_mask'))
                src = r['source'].replace('\\', '/')
                self.targets[r['symbol']].append((va, masked, src))
                self.by_va[va].append((r['symbol'], masked, src))
        self.vptr_writes = defaultdict(set)  # code VA -> classes whose primary table it installs
        for w in json.loads((ROOT / 'analysis/vtable_write_xrefs.json').read_text()):
            for o in w['vtable_owners']:
                if o['object_offset'] == 0:
                    self.vptr_writes[int(w['code_va'], 16)].add(o['class'])
        self.rtti = {c['name']: c for c in json.loads((ROOT / 'analysis/rtti_classes.json').read_text())}
        self.vtables = defaultdict(list)
        for v in json.loads((ROOT / 'analysis/vtables.json').read_text()):
            self.vtables[v['class']].append(v)
        self.paths = {line.strip().lower() for line in
                      (ROOT / 'analysis/source_paths.txt').read_text(encoding='latin-1').splitlines()}
        self.iat = iat_names(pe)
        text = next(sec for sec in pe.sections if sec.name.startswith('.text'))
        self.text = (pe.image_base + text.virtual_address, pe.image_base + text.virtual_address + text.raw_size)
        code = pe.data[text.raw_offset:text.raw_offset + text.raw_size]
        self.entries = {self.text[0] + i + 5 + struct.unpack_from('<i', code, i + 1)[0]
                        for i in range(len(code) - 5) if code[i] == 0xE8}
        for tables in self.vtables.values():
            for v in tables:
                self.entries.update(int(e, 16) for e in v['entries'])

    def _bytes(self, va: int, n: int) -> bytes:
        try:
            return self.pe.bytes_at_va(va, n)
        except Exception:  # unmapped (e.g. .bss)
            return b''

    def _cstr(self, va: int) -> bytes:
        b = self._bytes(va, 1024)
        return b.split(b'\0', 1)[0] if b'\0' in b else b''

    def _vtable(self, sym: str, va: int):
        m = re.fullmatch(r'\?\?_7(\w+)@@6B(?:(\w+)@@|0@)?@', sym)
        if not m:
            return None
        cls, base = m.group(1), m.group(2)  # `6B0@@`: the table for the class's own subobject
        tables = {int(v['vtable_va'], 16): v for v in self.vtables.get(cls, [])}
        if va not in tables:
            return None
        off = tables[va]['object_offset']
        if base is None:
            return f'RTTI primary table of {cls}' if off == 0 else None
        info = self.rtti.get(cls)
        bases = [b for b in (info or {}).get('bases', []) if b['name'] == base]
        if not bases:
            return None
        if any(b['pdisp'] == -1 and b['mdisp'] == off for b in bases):
            return f'RTTI table of {cls} at object offset {off:#x} ({base} subobject)'
        if any(b['pdisp'] >= 0 for b in bases) and off not in {b['mdisp'] for b in info['bases'] if b['pdisp'] == -1}:
            return f'RTTI table of {cls} at object offset {off:#x} (virtual base {base})'
        return None

    def _structor(self, sym: str, va: int):
        """??0X / ??1X installs X's primary table; ??_E / ??_G X is an entry of X's tables."""
        m = re.match(r'\?\?(0|1|_E|_G)(\w+)@@', sym)
        if not m:
            return None
        kind, cls = m.groups()
        if kind in ('_E', '_G'):
            for v in self.vtables.get(cls, []):
                if f'0x{va:08x}' in v['entries']:
                    return f'RTTI table of {cls} {v["vtable_va"]} holds it'
            return None
        end = min((e for e in self.entries if e > va), default=va + 0x200)
        hits = [a for a in self.vptr_writes if va <= a < end and cls in self.vptr_writes[a]]
        if hits:
            return f'installs the RTTI primary table of {cls} at {hits[0]:#x}'
        return None

    def _slot(self, sym: str, va: int):
        m = re.match(r'\?(\w*?)VirtualSlot(\d+)@(\w+)@@', sym)
        if not m:
            return None
        slot, cls = int(m.group(2)), m.group(3)
        for v in self.vtables.get(cls, []):
            if v['object_offset'] == 0 and slot < len(v['entries']) and int(v['entries'][slot], 16) == va:
                return f'RTTI primary table of {cls} {v["vtable_va"]} slot {slot}'
        return None

    def _defined(self, obj: CoffObject, sym: str):
        return next((x for x in obj.symbols if x.name == sym and x.section_number > 0), None)

    def _data_bytes(self, obj: CoffObject, rec) -> bytes | None:
        """Bytes of an initialised, relocation-free data symbol defined in the object."""
        sec = obj.section(rec.section_number)
        if not sec.raw_ptr or sec.name.startswith(('.text', '.bss')):
            return None
        later = [x.value for x in obj.symbols if x.section_number == rec.section_number and x.value > rec.value]
        end = min(later, default=sec.raw_size)
        if any(r.section_number == rec.section_number and rec.value <= r.virtual_address < end
               for r in obj.relocations):
            return None
        return obj.data[sec.raw_ptr + rec.value:sec.raw_ptr + end]

    def classify(self, sym: str, va: int, obj: CoffObject, funcs: set[str]) -> tuple[str, str] | None:
        rec = self._defined(obj, sym)
        local = rec is not None and rec.storage_class == 3
        rev = None if local else self.reviewed.get(sym)
        if rev:
            if va in rev:
                return 'reviewed', rev[va]
            if not sym.startswith('__real@'):  # retail keeps some duplicate constants
                return 'conflict', f'reviewed elsewhere at {", ".join(hex(a) for a in rev)}'
        own = self.targets.get(sym, [])
        for tva, masked, src in own:
            if tva == va:
                return ('matched' if masked else 'target'), src
        if own:  # a reconstructed function is identified by its own target, never by a fitted address
            return 'conflict', f'target of this symbol is at {", ".join(hex(t[0]) for t in own)}'
        if not local and va in self.reviewed_va and not sym.startswith(('__real@', '??_C@')):
            other, where = self.reviewed_va[va]
            return 'reviewed', f'{where} as {other}'
        for other, masked, src in self.by_va.get(va, []):
            if masked:  # the body at this address is reconstructed under another stand-in name
                return 'matched', f'{src} as {other}'
        if (why := self._structor(sym, va)):
            return 'vptr-write', why
        if (why := self._vtable(sym, va)) or (why := self._slot(sym, va)):
            return ('vtable' if sym.startswith('??_7') else 'rtti-slot'), why
        if (enc := decode_real(sym)) is not None:
            got = self._bytes(va, len(enc))
            return ('real', f'retail {got.hex()}') if got == enc else ('conflict', f'retail {got.hex()} != {enc.hex()}')
        if sym.startswith('__FILE__'):
            s = self._cstr(va).decode('latin-1').lower()
            base = sym.split(':', 1)[1] if ':' in sym else None
            if s in self.paths and (base is None or s.endswith('\\' + base)):
                return 'file', s
            return None
        if sym.startswith('??_C@'):
            if rec is not None:
                sec = obj.section(rec.section_number)
                lit = obj.data[sec.raw_ptr + rec.value:sec.raw_ptr + sec.raw_size].split(b'\0', 1)[0]
                if self._bytes(va, len(lit) + 1) == lit + b'\0':
                    return 'string', repr(lit.decode('latin-1'))[:60]
            return None
        if rec is not None and (data := self._data_bytes(obj, rec)):
            got = self._bytes(va, len(data))
            if got != data:
                return 'conflict', f'retail {got.hex()[:32]} != {data.hex()[:32]}'
            return 'data-bytes', data.hex()[:32]
        if any(f.startswith('_$E') for f in funcs):
            return 'initializer', 'written by matched ' + ', '.join(sorted(f for f in funcs if f.startswith('_$E')))
        if sym.endswith('$ehhandler'):
            stub = self._bytes(va, 10)
            if len(stub) == 10 and stub[0] == 0xB8 and stub[5] == 0xE9:
                info = struct.unpack_from('<I', stub, 1)[0]
                if self._bytes(info, 4) == struct.pack('<I', 0x19930520):
                    return 'ehhandler', f'FuncInfo {info:#x}'
            return None
        if sym.startswith('__imp__'):
            name = sym[len('__imp__'):].split('@', 1)[0]
            if self.iat.get(va) == name:
                return 'import', name
            return None
        ident = re.match(r'\??(\w+)', sym.lstrip('?'))
        if ident and va in self.declared.get(ident.group(1), ()) and (
                va in self.entries or not self.text[0] <= va < self.text[1]):
            return 'named-address', f'declaration of {ident.group(1)} states {va:#x}'
        m = re.search(r'_(?:0x)?0*([4-6][0-9a-fA-F]{5})(?:@|$)', sym)
        if m and int(m.group(1), 16) == va:
            if self.text[0] <= va < self.text[1]:
                if va in self.entries:
                    return 'named-address', 'retail call target / table entry'
            elif any(self.pe.image_base + x.virtual_address <= va
                     < self.pe.image_base + x.virtual_address + max(x.virtual_size, x.raw_size)
                     for x in self.pe.sections):
                return 'named-address', 'data outside .text'
        return None


def collect_sites(pe, report, objs, sources):
    """symbol -> list of (implied VA, function symbol, target VA)."""
    sites = defaultdict(list)
    objects = {}
    for r in report:
        src = r['source'].replace('\\', '/')
        if src not in sources or not r.get('exact_after_relocation_mask'):
            continue
        path = obj_path(objs, r['source'])
        obj = objects.setdefault(path, CoffObject(path))
        tva, size = int(r['expected']['target_va'], 0), r['expected']['target_size']
        retail = pe.bytes_at_va(tva, size)
        try:
            res = match_object(obj, r['expected']['candidate_symbol_contains'], tva, retail, _Recorder(r.get('symbol', '')))
        except RelocationError as exc:
            print(f'skip {r["expected"]["target_va"]}: {exc}', file=sys.stderr)
            continue
        for row in res['relocations_applied']:
            if row['internal_label']:
                continue
            off = row['offset']
            word = struct.unpack_from('<I', retail, off)[0]
            implied = (word - row['addend_u32']) & 0xffffffff
            if row['type'] == 0x14:
                implied = (implied + tva + off + 4) & 0xffffffff
            sites[(src, row['symbol'])].append((implied, res['symbol'], tva, src, obj))
    return sites


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--exe', default=str(ROOT / 'work/game/mcm2.exe'))
    ap.add_argument('--report', required=True, type=Path)
    ap.add_argument('--objs', required=True, type=Path)
    ap.add_argument('--source', action='append', required=True,
                    help='source path as printed in the report (repeatable)')
    ap.add_argument('--write', action='store_true',
                    help='write <source>.bindings.json and link every masked-exact target to it')
    ap.add_argument('--json-out', type=Path)
    a = ap.parse_args()
    pe = PEImage(a.exe)
    report = json.loads(a.report.read_text())
    sources = {s.replace('\\', '/') for s in a.source}
    ev = Evidence(pe, report)
    sites = collect_sites(pe, report, a.objs, sources)
    per_source = defaultdict(dict)
    rows = []
    for (src, sym), ss in sorted(sites.items()):
        vas = {s[0] for s in ss}
        funcs = {s[1] for s in ss}
        if len(vas) > 1:
            kind, why = 'conflict', 'sites disagree: ' + ', '.join(hex(v) for v in sorted(vas))
        else:
            va = next(iter(vas))
            kind, why = ev.classify(sym, va, ss[0][4], funcs) or ('unproven', '')
        row = {'source': src, 'symbol': sym, 'va': ', '.join(f'0x{v:08x}' for v in sorted(vas)), 'evidence': kind,
               'detail': why, 'sites': len(ss), 'functions': len(funcs)}
        rows.append(row)
        if kind in ACCEPTED:
            for s in ss:
                per_source[s[3]][sym] = f'0x{s[0]:08x}'
    counts = defaultdict(int)
    for row in rows:
        counts[row['evidence']] += 1
        if row['evidence'] not in ACCEPTED:
            print(f"{row['evidence']:9} {row['va']:12} sites {row['sites']:2} funcs {row['functions']:2}  "
                  f"{row['symbol']}  {row['detail']}  [{row['source'].rsplit('/', 1)[-1]}]")
    print('summary:', dict(sorted(counts.items())))
    if a.json_out:
        a.json_out.write_text(json.dumps(rows, indent=1) + '\n')
    if a.write:
        for src, binds in sorted(per_source.items()):
            # A snapshot run (work/snap/...) writes back to the live tree.
            path = ROOT / Path(src.removeprefix('work/snap/')).with_suffix('.bindings.json')
            old = json.loads(path.read_text()) if path.exists() else {}
            for k, v in binds.items():
                if k in old and old[k] != v:
                    raise SystemExit(f'{path}: {k} already bound to {old[k]}, proposal {v}')
            old.update(binds)
            path.write_text(json.dumps(dict(sorted(old.items())), indent=2) + '\n')
            print(f'wrote {path.relative_to(ROOT)} ({len(old)} bindings)')
            tjson = path.parent / 'targets.json'
            text = tjson.read_text()
            targets = json.loads(text)
            for t in targets:
                if Path(t.get('source', '')).with_suffix('.bindings.json').name == path.name:
                    t['bindings'] = path.name
            second = text.splitlines()[1] if text.count('\n') > 1 else '  '
            tjson.write_text(json.dumps(targets, indent=len(second) - len(second.lstrip()) or 2) + '\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
