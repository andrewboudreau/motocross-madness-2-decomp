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
    for rel in rels:
        record = obj.symbol_by_index.get(rel.symbol_index)
        if record is None:
            raise RelocationError('relocation references missing/auxiliary symbol')
        converted.append({'offset': rel.virtual_address - sym.value, 'type': rel.type, 'symbol': record.name})
    patched, audit = apply_relocations(raw, converted, bindings, target_va)
    return {**compare_bytes(retail, patched), 'symbol': sym.name,
            'target_va': f'0x{target_va:08x}', 'relocations_applied': audit,
            'alignment_padding_bytes': padding,
            'bindings_are_identity_proof': False}
