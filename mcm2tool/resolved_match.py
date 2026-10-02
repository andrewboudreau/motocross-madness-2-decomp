"""Strict i386 COFF relocation application for a function at its retail VA.

User-reviewed address bindings are not proof of symbol or library identity.
Unlike masking, every supported relocation is applied and every byte compared.
"""
from __future__ import annotations
import struct
from .coff import CoffObject


class RelocationError(ValueError):
    pass


def apply_relocations(code: bytes, relocations: list[dict], bindings: dict[str, int], target_va: int):
    patched, audit, occupied = bytearray(code), [], set()
    if not 0 <= target_va <= 0xffffffff or target_va + len(code) > 0x100000000:
        raise RelocationError('function address outside i386 address space')
    for row in relocations:
        offset, kind, symbol = row['offset'], row['type'], row['symbol']
        if kind == 0:  # IMAGE_REL_I386_ABSOLUTE: no relocation, no bytes hidden.
            continue
        if kind not in (0x0006, 0x0014):
            raise RelocationError(f'unsupported i386 relocation {kind:#x}')
        if offset < 0 or offset + 4 > len(code):
            raise RelocationError('relocation crosses selected function extent')
        span = set(range(offset, offset + 4))
        if occupied & span:
            raise RelocationError('overlapping relocations')
        occupied |= span
        if symbol not in bindings:
            raise RelocationError(f'unresolved symbol: {symbol}')
        address = bindings[symbol]
        if not isinstance(address, int) or not 0 <= address <= 0xffffffff:
            raise RelocationError(f'invalid symbol address: {symbol}')
        addend = struct.unpack_from('<I', code, offset)[0]
        value = address + addend
        if kind == 0x0014:  # S + A - (P + 4)
            value -= target_va + offset + 4
        value &= 0xffffffff
        struct.pack_into('<I', patched, offset, value)
        audit.append({'offset': offset, 'type': kind, 'symbol': symbol,
                      'bound_va': f'0x{address:08x}', 'addend_u32': addend,
                      'written_u32': value})
    return bytes(patched), audit


def compare_bytes(retail: bytes, candidate: bytes) -> dict:
    total = max(len(retail), len(candidate))
    equal = sum(a == b for a, b in zip(retail, candidate))
    return {'retail_size': len(retail), 'candidate_size': len(candidate),
            'compared_positions': total, 'matching_positions': equal,
            'match_percent': round(100 * equal / total, 4) if total else 0.0,
            'strict_exact': bool(retail) and retail == candidate,
            'ignored_bytes': 0,
            'mismatches': [{'offset': i,
                            'retail': retail[i] if i < len(retail) else None,
                            'candidate': candidate[i] if i < len(candidate) else None}
                           for i in range(total)
                           if i >= len(retail) or i >= len(candidate) or retail[i] != candidate[i]][:64]}


_EH_PROLOGUES = {
    3: (b'\x6a\xff\x68',),
    9: (b'\x64\xa1\x00\x00\x00\x00\x6a\xff\x68',),
}


def _source_path_literal(obj: CoffObject, record) -> str | None:
    """Lower-cased basename of an absolute source-path literal, else None."""
    if record.section_number <= 0:
        return None
    section = obj.section(record.section_number)
    data = obj.data[section.raw_ptr + record.value:section.raw_ptr + section.raw_size]
    text = data.split(b'\0', 1)[0].decode('latin-1').lower()
    if (len(text) > 3 and text[1:3] in (':\\', ':/')
            and text.endswith(('.cpp', '.c', '.h'))):
        return text.replace('/', '\\').rsplit('\\', 1)[-1]
    return None


def match_object(obj: CoffObject, symbol: str, target_va: int, retail: bytes, bindings: dict[str, int]) -> dict:
    if obj.machine != 0x14c:
        raise RelocationError('only i386 COFF supported')
    sym = obj.find_symbol(symbol)
    raw, _, rels = obj.symbol_extent(sym)
    section = obj.section(sym.section_number)
    if sym.value < 0 or sym.value + len(raw) > section.raw_size or section.raw_ptr + section.raw_size > len(obj.data):
        raise RelocationError('truncated COFF function storage')
    # No target-sized trimming: symbol_extent owns the candidate boundary.
    padding = 0
    converted = []
    # Labels inside this function's own extent (e.g. switch jump-table targets)
    # have a fixed address: the function's retail VA plus the label offset. They
    # are resolved here instead of requiring hand-written bindings; anything
    # outside the extent must still be bound explicitly.
    internal: dict[str, int] = {}
    for rel in rels:
        record = obj.symbol_by_index.get(rel.symbol_index)
        if record is None:
            raise RelocationError('relocation references missing/auxiliary symbol')
        if (record.section_number == sym.section_number
                and sym.value <= record.value < sym.value + len(raw)):
            address = target_va + (record.value - sym.value)
            if internal.get(record.name, address) != address:
                raise RelocationError(f'ambiguous internal symbol: {record.name}')
            if record.name in bindings and bindings[record.name] != address:
                raise RelocationError(f'binding contradicts internal label: {record.name}')
            internal[record.name] = address
        offset = rel.virtual_address - sym.value
        name = record.name
        # /GX frame prologue `push -1; push offset handler`: the handler stub is
        # a compiler label in .text$x whose number shifts with unrelated edits,
        # so it is bound under the stable key '<function symbol>$ehhandler'.
        # VC6 may also schedule the fs:[0] load first:
        # `mov eax, fs:[0]; push -1; push offset handler` (handler at +9).
        if (raw[:offset] in _EH_PROLOGUES.get(offset, ()) and record.storage_class == 6
                and obj.section(record.section_number).name.startswith('.text$x')):
            name = f'{sym.name}$ehhandler'
        # __except_list is the CRT's absolute symbol for the fs:[0] SEH chain head.
        if name == '__except_list' and rel.type == 0x0006 and record.section_number in (0, -1):
            internal[name] = 0
        # A __FILE__ literal embeds the build path, so its pooled ??_C@ name is not
        # stable. It is bound as '__FILE__:<basename>' (a .cpp and a header can
        # both appear in one object), falling back to plain '__FILE__'.
        elif name.startswith('??_C@') and _source_path_literal(obj, record):
            name = f'__FILE__:{_source_path_literal(obj, record)}'
            if name not in bindings and '__FILE__' in bindings:
                name = '__FILE__'
        converted.append({'offset': offset, 'type': rel.type, 'symbol': name})
    patched, audit = apply_relocations(raw, converted, {**bindings, **internal}, target_va)
    for row in audit:
        row['internal_label'] = row['symbol'] in internal
    return {**compare_bytes(retail, patched), 'symbol': sym.name,
            'extent_source': sym.extent_source,
            'target_va': f'0x{target_va:08x}', 'relocations_applied': audit,
            'alignment_padding_bytes': padding,
            'bindings_are_identity_proof': False}
