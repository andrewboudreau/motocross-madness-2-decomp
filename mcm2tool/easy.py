from __future__ import annotations
from collections import defaultdict
from dataclasses import dataclass, asdict
from typing import Any
from .pe import PEImage

@dataclass(frozen=True)
class EasyPattern:
    kind: str
    size: int
    details: dict[str, Any]
    compiler_stability: str


def _ret_pop(blob: bytes, pos: int) -> tuple[bool, int | None, int]:
    """Return (is_return, pop_bytes, instruction_size) at pos."""
    if pos < len(blob) and blob[pos] == 0xC3:
        return True, 0, 1
    if pos + 2 < len(blob) and blob[pos] == 0xC2:
        return True, int.from_bytes(blob[pos + 1:pos + 3], 'little'), 3
    return False, None, 0


def classify_easy_bytes(blob: bytes) -> EasyPattern | None:
    """Classify small x86 vtable bodies useful as decomp/bootstrap targets.

    This intentionally recognizes only byte sequences whose semantics are obvious
    without a general-purpose disassembler. `compiler_stability` describes how
    likely equivalent straightforward C++ is to preserve the same instruction
    shape across MSVC-family compilers; medium targets are particularly useful for
    compiler calibration.
    """
    if blob.startswith(b'\xc3'):
        return EasyPattern('return_void', 1, {}, 'high')
    if len(blob) >= 3 and blob[0] == 0xC2:
        return EasyPattern('return_void_pop', 3, {'pop_bytes': int.from_bytes(blob[1:3], 'little')}, 'high')

    # zero / -1 returns, with optional thiscall callee cleanup
    for op in (b'\x33\xc0', b'\x31\xc0'):
        if blob.startswith(op):
            ok, pop, rsz = _ret_pop(blob, 2)
            if ok:
                return EasyPattern('return_zero', 2 + rsz, {'pop_bytes': pop}, 'medium')
    if blob.startswith(b'\x83\xc8\xff'):
        ok, pop, rsz = _ret_pop(blob, 3)
        if ok:
            return EasyPattern('return_minus_one', 3 + rsz, {'pop_bytes': pop}, 'medium')

    # return immediate constant
    if len(blob) >= 6 and blob[0] == 0xB8:
        ok, pop, rsz = _ret_pop(blob, 5)
        if ok:
            value = int.from_bytes(blob[1:5], 'little', signed=True)
            return EasyPattern('return_constant', 5 + rsz, {'value': value, 'pop_bytes': pop}, 'high')

    if blob.startswith(b'\x8b\xc1\xc3'):
        return EasyPattern('return_this', 3, {}, 'high')

    # 32-bit member getter
    if len(blob) >= 4 and blob[:2] == b'\x8b\x41' and blob[3] == 0xC3:
        return EasyPattern('get_i32', 4, {'offset': blob[2]}, 'high')
    if len(blob) >= 7 and blob[:2] == b'\x8b\x81' and blob[6] == 0xC3:
        off = int.from_bytes(blob[2:6], 'little', signed=True)
        return EasyPattern('get_i32', 7, {'offset': off}, 'high')

    # 8-bit member getter
    if len(blob) >= 5 and blob[:3] == b'\x0f\xb6\x41' and blob[4] == 0xC3:
        return EasyPattern('get_u8', 5, {'offset': blob[3]}, 'high')
    if len(blob) >= 8 and blob[:3] == b'\x0f\xb6\x81' and blob[7] == 0xC3:
        off = int.from_bytes(blob[3:7], 'little', signed=True)
        return EasyPattern('get_u8', 8, {'offset': off}, 'high')

    # &member
    if len(blob) >= 4 and blob[:2] == b'\x8d\x41' and blob[3] == 0xC3:
        return EasyPattern('address_of_field', 4, {'offset': blob[2]}, 'high')
    if len(blob) >= 7 and blob[:2] == b'\x8d\x81' and blob[6] == 0xC3:
        off = int.from_bytes(blob[2:6], 'little', signed=True)
        return EasyPattern('address_of_field', 7, {'offset': off}, 'high')

    # float member getter
    if len(blob) >= 4 and blob[:2] == b'\xd9\x41' and blob[3] == 0xC3:
        return EasyPattern('get_float', 4, {'offset': blob[2]}, 'high')
    if len(blob) >= 7 and blob[:2] == b'\xd9\x81' and blob[6] == 0xC3:
        off = int.from_bytes(blob[2:6], 'little', signed=True)
        return EasyPattern('get_float', 7, {'offset': off}, 'high')

    # one-argument int/pointer setter: mov eax,[esp+4]; mov [ecx+off],eax; ret 4
    if len(blob) >= 10 and blob[:4] == b'\x8b\x44\x24\x04' and blob[4:6] == b'\x89\x41' and blob[7:10] == b'\xc2\x04\x00':
        return EasyPattern('set_i32_arg', 10, {'offset': blob[6], 'pop_bytes': 4}, 'high')
    if len(blob) >= 13 and blob[:4] == b'\x8b\x44\x24\x04' and blob[4:6] == b'\x89\x81' and blob[10:13] == b'\xc2\x04\x00':
        off = int.from_bytes(blob[6:10], 'little', signed=True)
        return EasyPattern('set_i32_arg', 13, {'offset': off, 'pop_bytes': 4}, 'high')

    # VC6 often chooses a second register for the RHS; modern clang may fold it
    # into a memory operand. Keep these as compiler-calibration candidates.
    if len(blob) >= 9 and blob[:2] == b'\x8b\x41' and blob[3:5] == b'\x8b\x51' and blob[6:8] == b'\x2b\xc2' and blob[8] == 0xC3:
        return EasyPattern('sub_i32_fields', 9, {'lhs_offset': blob[2], 'rhs_offset': blob[5]}, 'medium')

    # float global getter: fld dword ptr [absolute]; ret[/pop]
    if len(blob) >= 7 and blob[:2] == b'\xd9\x05':
        ok, pop, rsz = _ret_pop(blob, 6)
        if ok:
            addr = int.from_bytes(blob[2:6], 'little')
            return EasyPattern('return_float_global', 6 + rsz, {'address': addr, 'pop_bytes': pop}, 'high')

    # Pointer member -> float member: mov eax,[ecx+off]; fld [eax+suboff]; ret
    if len(blob) >= 7 and blob[:2] == b'\x8b\x41' and blob[3:5] == b'\xd9\x40':
        ok, pop, rsz = _ret_pop(blob, 6)
        if ok:
            return EasyPattern('get_nested_float', 6 + rsz, {'pointer_offset': blob[2], 'value_offset': blob[5], 'pop_bytes': pop}, 'high')
    if len(blob) >= 10 and blob[:2] == b'\x8b\x81' and blob[6:8] == b'\xd9\x40':
        ok, pop, rsz = _ret_pop(blob, 9)
        if ok:
            poff = int.from_bytes(blob[2:6], 'little', signed=True)
            return EasyPattern('get_nested_float', 9 + rsz, {'pointer_offset': poff, 'value_offset': blob[8], 'pop_bytes': pop}, 'high')

    # Copy one 32-bit member to another.
    if len(blob) >= 13 and blob[:2] == b'\x8b\x81' and blob[6:8] == b'\x89\x81' and blob[12] == 0xC3:
        src = int.from_bytes(blob[2:6], 'little', signed=True)
        dst = int.from_bytes(blob[8:12], 'little', signed=True)
        return EasyPattern('copy_i32_field', 13, {'source_offset': src, 'dest_offset': dst}, 'high')

    # Store a literal dword into one member, then store the sole argument
    # into another. This appears in UI state/configuration setters and is
    # stable under clang-cl's x86 MSVC ABI for the observed forms.
    if (len(blob) >= 20 and blob[:4] == b'\x8b\x44\x24\x04' and
        blob[4:6] == b'\xc7\x81' and blob[14:16] == b'\x89\x41' and
        blob[17:20] == b'\xc2\x04\x00'):
        const_off = int.from_bytes(blob[6:10], 'little', signed=True)
        value = int.from_bytes(blob[10:14], 'little', signed=False)
        arg_off = int.from_bytes(blob[16:17], 'little', signed=False)
        return EasyPattern('set_i32_const_and_arg', 20, {
            'const_offset': const_off, 'value_u32': value,
            'arg_offset': arg_off, 'pop_bytes': 4
        }, 'high')
    if (len(blob) >= 23 and blob[:4] == b'\x8b\x44\x24\x04' and
        blob[4:6] == b'\xc7\x81' and blob[14:16] == b'\x89\x81' and
        blob[20:23] == b'\xc2\x04\x00'):
        const_off = int.from_bytes(blob[6:10], 'little', signed=True)
        value = int.from_bytes(blob[10:14], 'little', signed=False)
        arg_off = int.from_bytes(blob[16:20], 'little', signed=True)
        return EasyPattern('set_i32_const_and_arg', 23, {
            'const_offset': const_off, 'value_u32': value,
            'arg_offset': arg_off, 'pop_bytes': 4
        }, 'high')

    # Indexed 32-byte element access. Retail VC6 and modern clang choose
    # semantically equivalent SIB operand orderings, so these are excellent
    # historical-compiler calibration targets rather than clang smoke tests.
    if (len(blob) >= 20 and blob[:2] == b'\x8b\x81' and blob[6:8] == b'\x8b\x89' and
        blob[12:15] == b'\xc1\xe0\x05' and blob[15:18] == b'\x8b\x44\x08' and blob[19] == 0xC3):
        index_off = int.from_bytes(blob[2:6], 'little', signed=True)
        base_off = int.from_bytes(blob[8:12], 'little', signed=True)
        value_off = int.from_bytes(blob[18:19], 'little', signed=False)
        return EasyPattern('get_indexed_i32_stride32', 20, {
            'index_offset': index_off, 'base_offset': base_off,
            'element_offset': value_off, 'stride': 32
        }, 'medium')
    if (len(blob) >= 20 and blob[:2] == b'\x8b\x81' and blob[6:8] == b'\x8b\x89' and
        blob[12:15] == b'\xc1\xe0\x05' and blob[15:18] == b'\x8d\x44\x08' and blob[19] == 0xC3):
        index_off = int.from_bytes(blob[2:6], 'little', signed=True)
        base_off = int.from_bytes(blob[8:12], 'little', signed=True)
        value_off = int.from_bytes(blob[18:19], 'little', signed=False)
        return EasyPattern('address_of_indexed_stride32', 20, {
            'index_offset': index_off, 'base_offset': base_off,
            'element_offset': value_off, 'stride': 32
        }, 'medium')

    # if (arg != 0) member = arg
    if (len(blob) >= 17 and blob[:6] == b'\x8b\x44\x24\x04\x85\xc0' and
        blob[6:8] == b'\x74\x06' and blob[8:10] == b'\x89\x81' and blob[14:17] == b'\xc2\x04\x00'):
        off = int.from_bytes(blob[10:14], 'little', signed=True)
        return EasyPattern('set_i32_arg_if_nonzero', 17, {'offset': off, 'pop_bytes': 4}, 'high')

    # *arg = immediate; return immediate (one pointer argument).
    if (len(blob) >= 18 and blob[:4] == b'\x8b\x44\x24\x04' and blob[4:6] == b'\xc7\x00' and
        blob[10] == 0xB8 and blob[15:18] == b'\xc2\x04\x00'):
        stored = int.from_bytes(blob[6:10], 'little', signed=True)
        returned = int.from_bytes(blob[11:15], 'little', signed=True)
        return EasyPattern('write_arg_i32_const_return_const', 18, {'stored_value': stored, 'return_value': returned, 'pop_bytes': 4}, 'high')

    # Store a literal dword into a member. For float-looking bit patterns the
    # consumer can reinterpret the literal; the bytes alone do not prove type.
    if len(blob) >= 11 and blob[:2] == b'\xc7\x81' and blob[10] == 0xC3:
        off = int.from_bytes(blob[2:6], 'little', signed=True)
        value = int.from_bytes(blob[6:10], 'little', signed=False)
        return EasyPattern('set_i32_const', 11, {'offset': off, 'value_u32': value}, 'high')

    # Getter that ignores stack arguments; useful for recovering signatures.
    if len(blob) >= 6 and blob[:2] == b'\x8b\x41':
        ok, pop, rsz = _ret_pop(blob, 3)
        if ok and pop:
            return EasyPattern('get_i32_ignore_args', 3 + rsz, {'offset': blob[2], 'pop_bytes': pop}, 'high')
    if len(blob) >= 9 and blob[:2] == b'\x8b\x81':
        ok, pop, rsz = _ret_pop(blob, 6)
        if ok and pop:
            off = int.from_bytes(blob[2:6], 'little', signed=True)
            return EasyPattern('get_i32_ignore_args', 6 + rsz, {'offset': off, 'pop_bytes': pop}, 'high')

    # A common VC6 bool materialization shape. Semantics are clear but modern
    # clang often chooses cmp/setne or a different register, so keep as medium.
    if len(blob) >= 11 and blob[:2] == b'\x8b\x51' and blob[3:5] == b'\x33\xc0' and blob[5:7] == b'\x85\xd2' and blob[7:10] == b'\x0f\x95\xc0' and blob[10] == 0xC3:
        return EasyPattern('test_i32_nonzero', 11, {'offset': blob[2]}, 'medium')
    if len(blob) >= 14 and blob[:2] == b'\x8b\x91' and blob[6:8] == b'\x33\xc0' and blob[8:10] == b'\x85\xd2' and blob[10:13] == b'\x0f\x95\xc0' and blob[13] == 0xC3:
        off = int.from_bytes(blob[2:6], 'little', signed=True)
        return EasyPattern('test_i32_nonzero', 14, {'offset': off}, 'medium')

    # 16-bit -1 return shape seen on the QuadTreeObject family. Upper EAX bits
    # are intentionally left unspecified by the ABI, which is consistent with a
    # short/unsigned-short return type rather than int.
    if blob.startswith(b'\x66\x0d\xff\xff\xc3'):
        return EasyPattern('return_u16_minus_one', 5, {}, 'medium')

    return None


def discover_easy_vtable_targets(pe: PEImage, vtables: list[dict]) -> list[dict]:
    refs: dict[int, list[dict]] = defaultdict(list)
    for vt in vtables:
        for slot, entry in enumerate(vt.get('entries', [])):
            va = int(entry, 16) if isinstance(entry, str) else int(entry)
            refs[va].append({
                'class': vt['class'],
                'slot': slot,
                'vtable_va': vt['vtable_va'],
                'object_offset': int(vt.get('object_offset', 0)),
                'complete_object_locator_va': vt.get('complete_object_locator_va'),
            })

    rows = []
    for va, uses in refs.items():
        try:
            blob = pe.bytes_at_va(va, 32)
        except Exception:
            continue
        pat = classify_easy_bytes(blob)
        if not pat:
            continue
        rows.append({
            'target_va': f'0x{va:08x}',
            'kind': pat.kind,
            'target_size': pat.size,
            'bytes': pe.bytes_at_va(va, pat.size).hex(' '),
            'details': pat.details,
            'compiler_stability': pat.compiler_stability,
            'uses': uses,
            'use_count': len(uses),
        })
    rows.sort(key=lambda r: int(r['target_va'], 16))
    return rows
