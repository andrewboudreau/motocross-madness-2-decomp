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
        if symbol not in bindings and 'internal_bound_va' not in row:
            raise RelocationError(f'unresolved symbol: {symbol}')
        address = row['internal_bound_va'] if 'internal_bound_va' in row else bindings[symbol]
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
                      'written_u32': value,
                      'internal_label': 'internal_bound_va' in row})
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


def _is_eh_handler_push(prefix: bytes) -> bool:
    """Whether `prefix` is a /GX prologue ending just before the handler operand.

    Besides the fixed shapes in _EH_PROLOGUES, VC6 can schedule loads of stack
    arguments (`mov r8/r32, [esp+disp8]`) between `push -1` and
    `push offset handler` when the fs:[0] load comes first (Scene 0x004ea7e0).
    It can also schedule loads of globals (`mov r32, [abs32]`, unrelocated in
    the object) between the fs:[0] load and `push -1` (bikerace.cpp slot 10
    0x0041d2b0).
    """
    if prefix in _EH_PROLOGUES.get(len(prefix), ()):
        return True
    fs_load = b'\x64\xa1\x00\x00\x00\x00'
    while (prefix.startswith(fs_load) and len(prefix) >= len(fs_load) + 6
           and prefix[6] == 0x8b and prefix[7] & 0xc7 == 0x05):
        prefix = fs_load + prefix[len(fs_load) + 6:]
    if prefix in _EH_PROLOGUES.get(len(prefix), ()):
        return True
    head = b'\x64\xa1\x00\x00\x00\x00\x6a\xff'
    if not prefix.startswith(head) or not prefix.endswith(b'\x68'):
        return False
    loads = prefix[len(head):-1]
    if not loads or len(loads) % 4:
        return False
    for i in range(0, len(loads), 4):
        opcode, modrm, sib = loads[i], loads[i + 1], loads[i + 2]
        if opcode not in (0x8a, 0x8b) or modrm & 0xc7 != 0x44 or sib != 0x24:
            return False
    return True


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


def _internal_bound_va(function, record, raw: bytes, relocation, target_va: int) -> int | None:
    """Resolve only same-section S+A destinations within the measured extent."""
    if record.section_number != function.section_number:
        return None
    offset = relocation.virtual_address - function.value
    if relocation.type not in (0x0006, 0x0014) or offset < 0 or offset + 4 > len(raw):
        return None
    addend = struct.unpack_from('<I', raw, offset)[0]
    bound = (target_va - function.value + record.value) & 0xffffffff
    destination = (bound + addend) & 0xffffffff
    if target_va <= destination < target_va + len(raw):
        return bound
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
    for rel in rels:
        record = obj.symbol_by_index.get(rel.symbol_index)
        if record is None:
            raise RelocationError('relocation references missing/auxiliary symbol')
        address = _internal_bound_va(sym, record, raw, rel, target_va)
        if address is not None:
            if record.name in bindings and bindings[record.name] != address:
                raise RelocationError(f'binding contradicts internal label: {record.name}')
        offset = rel.virtual_address - sym.value
        name = record.name
        # /GX frame prologue `push -1; push offset handler`: the handler stub is
        # a compiler label in .text$x whose number shifts with unrelated edits,
        # so it is bound under the stable key '<function symbol>$ehhandler'.
        # VC6 may also schedule the fs:[0] load first:
        # `mov eax, fs:[0]; push -1; push offset handler` (handler at +9).
        if (_is_eh_handler_push(raw[:offset]) and record.storage_class == 6
                and obj.section(record.section_number).name.startswith('.text$x')):
            name = f'{sym.name}$ehhandler'
        # __except_list is the CRT's absolute symbol for the fs:[0] SEH chain head.
        if name == '__except_list' and rel.type == 0x0006 and record.section_number in (0, -1):
            if name in bindings and bindings[name] != 0:
                raise RelocationError('binding contradicts __except_list')
            address = 0
        # A __FILE__ literal embeds the build path, so its pooled ??_C@ name is not
        # stable. It is bound as '__FILE__:<basename>' (a .cpp and a header can
        # both appear in one object), falling back to plain '__FILE__'.
        elif name.startswith('??_C@') and _source_path_literal(obj, record):
            name = f'__FILE__:{_source_path_literal(obj, record)}'
            if name not in bindings and '__FILE__' in bindings:
                name = '__FILE__'
        row = {'offset': offset, 'type': rel.type, 'symbol': name}
        # Keep inferred addresses per relocation: another reference to the same
        # section symbol may have an addend that escapes this function.
        if address is not None:
            row['internal_bound_va'] = address
        converted.append(row)
    patched, audit = apply_relocations(raw, converted, bindings, target_va)
    return {**compare_bytes(retail, patched), 'symbol': sym.name,
            'extent_source': sym.extent_source,
            'target_va': f'0x{target_va:08x}', 'relocations_applied': audit,
            'alignment_padding_bytes': padding,
            'bindings_are_identity_proof': False}
