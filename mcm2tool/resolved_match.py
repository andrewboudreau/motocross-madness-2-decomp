"""Strict i386 COFF relocation application for a function at its retail VA.

User-reviewed address bindings are not proof of symbol or library identity.
Unlike masking, every supported relocation is applied and every byte compared.

Relocations whose final target is demonstrably inside the selected function
extent may be resolved from the function's own section mapping. This covers
compiler-emitted jump tables without inventing addresses for other functions.
"""
from __future__ import annotations
import struct
from .coff import CoffObject


class RelocationError(ValueError):
    pass


def apply_relocations(code: bytes, relocations: list[dict],
                      bindings: dict[str, int] | None, target_va: int):
    bindings = bindings or {}
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

        if row.get('bound_va') is not None:
            address = row['bound_va']
            source = row.get('binding_source', 'internal')
        else:
            if symbol not in bindings:
                raise RelocationError(f'unresolved symbol: {symbol}')
            address = bindings[symbol]
            source = 'explicit'

        if not isinstance(address, int) or not 0 <= address <= 0xffffffff:
            raise RelocationError(f'invalid symbol address: {symbol}')
        addend = struct.unpack_from('<I', code, offset)[0]
        value = address + addend
        if kind == 0x0014:  # S + A - (P + 4)
            value -= target_va + offset + 4
        value &= 0xffffffff
        struct.pack_into('<I', patched, offset, value)
        audit.append({
            'offset': offset,
            'type': kind,
            'symbol': symbol,
            'bound_va': f'0x{address:08x}',
            'binding_source': source,
            'addend_u32': addend,
            'written_u32': value,
        })
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


def _internal_bound_va(obj: CoffObject, function, record, raw: bytes,
                       relocation, target_va: int) -> int | None:
    """Map a same-section relocation only when S+A stays in this function.

    A VC6 switch table commonly relocates both the table base and each table
    entry against the containing .text section. Mapping the selected function's
    section offset to target_va is sufficient for those references. References
    outside the independently measured function extent remain unresolved and
    therefore still require reviewed external bindings.
    """
    if record.section_number != function.section_number:
        return None
    offset = relocation.virtual_address - function.value
    if offset < 0 or offset + 4 > len(raw):
        return None
    addend = struct.unpack_from('<I', raw, offset)[0]
    section_base = target_va - function.value
    bound = (section_base + record.value) & 0xffffffff
    absolute = (bound + addend) & 0xffffffff
    if not (target_va <= absolute < target_va + len(raw)):
        return None
    return bound


def match_object(obj: CoffObject, symbol: str, target_va: int, retail: bytes,
                 bindings: dict[str, int] | None = None) -> dict:
    if obj.machine != 0x14c:
        raise RelocationError('only i386 COFF supported')
    bindings = bindings or {}
    sym = obj.find_symbol(symbol)
    raw, _, rels = obj.symbol_extent(sym)
    section = obj.section(sym.section_number)
    if sym.value < 0 or sym.value + len(raw) > section.raw_size or section.raw_ptr + section.raw_size > len(obj.data):
        raise RelocationError('truncated COFF function storage')

    converted = []
    for rel in rels:
        record = obj.symbol_by_index.get(rel.symbol_index)
        if record is None:
            raise RelocationError('relocation references missing/auxiliary symbol')
        row = {
            'offset': rel.virtual_address - sym.value,
            'type': rel.type,
            'symbol': record.name,
        }
        # Explicit evidence always wins. Otherwise resolve only an address whose
        # S+A is demonstrably inside this measured function extent.
        if record.name not in bindings:
            internal = _internal_bound_va(obj, sym, record, raw, rel, target_va)
            if internal is not None:
                row['bound_va'] = internal
                row['binding_source'] = 'internal_same_function'
        converted.append(row)

    patched, audit = apply_relocations(raw, converted, bindings, target_va)
    return {**compare_bytes(retail, patched), 'symbol': sym.name,
            'extent_source': sym.extent_source,
            'target_va': f'0x{target_va:08x}', 'relocations_applied': audit,
            'alignment_padding_bytes': 0,
            'bindings_are_identity_proof': False}
