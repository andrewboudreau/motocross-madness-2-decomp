"""Hash-bound allocation-subsystem inspection; no target code is executed.

Unlike the broad provenance pass, this uses explicitly reviewed ranges. The
labels are semantic hypotheses, not library identities or recovered symbols.
"""
from __future__ import annotations
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
import hashlib
import re
import struct
import subprocess

from .pe import PEImage, PEFormatError
from .msvc import scalar_deleting_destructor
from .rtti import parse_rtti


@dataclass(frozen=True)
class Insn:
    va: int
    raw: bytes
    mnemonic: str
    operands: str

    @property
    def end(self):
        return self.va + len(self.raw)


def hx(value: int) -> str:
    return f'0x{value:08x}'


def read_va(pe: PEImage, va: int, size: int) -> bytes:
    if size < 0:
        raise PEFormatError('negative read size')
    offset = pe.va_to_offset(va)
    sec = pe.section_for_rva(va - pe.image_base)
    end = sec.file_end if sec else pe.size_of_headers
    if offset + size > min(end, len(pe.data)):
        raise PEFormatError('read exceeds file-backed region')
    return pe.data[offset:offset + size]


def ascii_at(pe: PEImage, va: int, limit: int = 512) -> str | None:
    try:
        off = pe.va_to_offset(va)
    except (ValueError, struct.error):
        return None
    sec = pe.section_for_rva(va - pe.image_base)
    end = min(len(pe.data), sec.file_end if sec else pe.size_of_headers, off + limit)
    stop = pe.data.find(b'\0', off, end)
    if stop <= off:
        return None
    raw = pe.data[off:stop]
    return raw.decode('ascii') if all(32 <= b < 127 for b in raw) else None


def iat_symbols(pe: PEImage) -> dict[int, dict]:
    # Keep this focused pass independent of generated provenance snapshots.
    rva, size = pe.directories[1] if len(pe.directories) > 1 else (0, 0)
    if not rva:
        return {}
    result = {}
    for offset in range(0, min(size, 4096 * 20) - 19, 20):
        oft, stamp, chain, name, iat = struct.unpack('<5I', read_va(pe, pe.image_base + rva + offset, 20))
        if not any((oft, stamp, chain, name, iat)):
            return result
        module = ascii_at(pe, pe.image_base + name)
        if not module or not iat:
            raise PEFormatError('invalid import descriptor')
        for index in range(65536):
            value = struct.unpack('<I', read_va(pe, pe.image_base + (oft or iat) + 4 * index, 4))[0]
            if not value:
                break
            slot = pe.image_base + iat + 4 * index
            read_va(pe, slot, 4)
            name_value, ordinal = None, None
            if not oft and stamp:
                resolution = 'bound_iat_without_lookup'
            elif value & 0x80000000:
                ordinal, resolution = value & 65535, 'ordinal'
            else:
                name_value = ascii_at(pe, pe.image_base + value + 2)
                if not name_value:
                    raise PEFormatError('unreadable import name')
                resolution = 'name'
            result[slot] = {'module': module, 'name': name_value, 'ordinal': ordinal,
                            'iat_va': hx(slot), 'resolution': resolution}
        else:
            raise PEFormatError('import thunk limit exceeded')
    raise PEFormatError('import descriptor terminator missing')


def parse_objdump(text: str, pe: PEImage | None = None) -> dict[int, Insn]:
    result = {}
    for line in text.splitlines():
        fields = line.split('\t')
        if len(fields) < 3 or not re.fullmatch(r'\s*[0-9a-fA-F]+:', fields[0]):
            continue
        va = int(fields[0].strip()[:-1], 16)
        raw = bytes.fromhex(fields[1].strip())
        asm = fields[2].strip().split(None, 1)
        if not raw or not asm:
            continue
        if pe:
            if not pe.is_code_va(va):
                continue
            if read_va(pe, va, len(raw)) != raw:
                raise PEFormatError('disassembly bytes do not match the input')
        if va in result:
            raise ValueError('duplicate instruction address')
        result[va] = Insn(va, raw, asm[0], asm[1] if len(asm) == 2 else '')
    return result


def decode_image(pe: PEImage, objdump: str = 'objdump'):
    version = subprocess.run([objdump, '--version'], check=True, text=True,
                             capture_output=True, timeout=10).stdout.splitlines()[0]
    run = subprocess.run([objdump, '-d', '-w', '-Mintel', str(pe.path)], check=True,
                         text=True, capture_output=True, timeout=90)
    return parse_objdump(run.stdout, pe), version


def rel32_target(ins: Insn) -> int | None:
    if len(ins.raw) == 5 and ins.raw[0] in (0xe8, 0xe9):
        return (ins.end + struct.unpack_from('<i', ins.raw, 1)[0]) & 0xffffffff
    return None


def direct_call_target(ins: Insn) -> int | None:
    return rel32_target(ins) if ins.raw[:1] == b'\xe8' else None


def imported_target(ins: Insn, imports: dict[int, dict]) -> dict | None:
    if len(ins.raw) == 6 and ins.raw[:2] in (b'\xff\x15', b'\xff\x25'):
        return imports.get(struct.unpack_from('<I', ins.raw, 2)[0])
    return None


def reviewed_body(instructions: dict[int, Insn], start: int, end: int) -> list[Insn]:
    if end <= start or end - start > 65536:
        raise ValueError('invalid reviewed function range')
    body = []
    pos = start
    while pos < end:
        ins = instructions.get(pos)
        if ins is None or ins.end > end or ins.mnemonic in ('(bad)', '.byte'):
            raise ValueError(f'reviewed range has a decoding gap at {pos:#x}')
        body.append(ins)
        pos = ins.end
    if not body[-1].mnemonic.startswith('ret') and body[-1].mnemonic != 'jmp':
        raise ValueError('reviewed range must end with return or tail jump')
    return body


def compare_resolved_bodies(left: list[Insn], right: list[Insn]) -> dict:
    """Normalize ONLY external rel32 transfers, verifying their actual targets.

Internal transfers compare relative positions; all other bytes remain exact.
This is same-body evidence, not original-symbol or library attribution.
"""
    lraw, rraw = b''.join(x.raw for x in left), b''.join(x.raw for x in right)
    pairs, errors = [], []
    if len(left) != len(right) or len(lraw) != len(rraw):
        errors.append('instruction_count_or_extent_differs')
    if left and right:
        for a, b in zip(left, right):
            ta, tb = rel32_target(a), rel32_target(b)
            if (a.va - left[0].va, len(a.raw)) != (b.va - right[0].va, len(b.raw)):
                errors.append('instruction_layout_differs')
                continue
            if ta is not None and tb is not None and a.raw[0] == b.raw[0]:
                internal_a = left[0].va <= ta < left[-1].end
                internal_b = right[0].va <= tb < right[-1].end
                same = (internal_a and internal_b and ta - left[0].va == tb - right[0].va) or (
                    not internal_a and not internal_b and ta == tb)
                pairs.append({'offset': a.va - left[0].va, 'left_target': hx(ta),
                              'right_target': hx(tb), 'internal': internal_a and internal_b,
                              'same_resolved_target': same})
                if not same:
                    errors.append('transfer_target_differs')
            elif a.raw != b.raw:
                errors.append('nonrelocation_bytes_differ')
    return {'left_va': hx(left[0].va) if left else None,
            'right_va': hx(right[0].va) if right else None,
            'left_size': len(lraw), 'right_size': len(rraw),
            'raw_bytes_equal': bool(left and right and lraw == rraw),
            'equal_with_resolved_transfers': bool(left and right and not errors),
            'transfers_checked': pairs, 'failures': sorted(set(errors)),
            'library_identity_established': False}


def category_literals(instructions: dict[int, Insn], select_va: int, pe: PEImage) -> list[dict]:
    """Only accept push-imm, optional register moves, and call in one local block.

Branches/calls/pushes kill the backward search. This intentionally misses some
arguments rather than guessing through arbitrary register/stack manipulation.
"""
    by_end = {ins.end: ins for ins in instructions.values()}
    rows = []
    for ins in instructions.values():
        if direct_call_target(ins) != select_va:
            continue
        pos = ins.va
        for _ in range(4):
            previous = by_end.get(pos)
            if not previous:
                break
            raw = previous.raw
            if len(raw) == 5 and raw[0] == 0x68:
                va = struct.unpack_from('<I', raw, 1)[0]
                name = ascii_at(pe, va, 128)
                if name:
                    rows.append({'call_va': hx(ins.va), 'push_va': hx(previous.va),
                                 'string_va': hx(va), 'label': name,
                                 'evidence': 'local_push_immediate_argument'})
                break
            # MOV r32,r32 neither changes the stack nor consumes an argument.
            if not (len(raw) == 2 and raw[0] in (0x8b, 0x89) and raw[1] >= 0xc0):
                break
            pos = previous.va
    return sorted(rows, key=lambda r: r['call_va'])


def reachable_imports(start: int, edges: dict[int, list[int]], api_calls: dict[int, list[dict]], max_depth=6):
    # Paths are may-call summaries; a branch or lock helper may not run on every allocation.
    pending, seen, out = [(start, [start])], set(), []
    while pending:
        node, path = pending.pop(0)
        if node in seen:
            continue
        seen.add(node)
        for row in api_calls.get(node, []):
            out.append({'path': [hx(x) for x in path], 'api': row})
        if len(path) > max_depth:
            continue
        for target in sorted(set(edges.get(node, []))):
            if target not in path:
                pending.append((target, path + [target]))
    return out


def analyze(pe: PEImage, config: dict, instructions: dict[int, Insn]) -> dict:
    if pe.machine != 0x14c or hashlib.sha256(pe.data).hexdigest() != config['input_sha256']:
        raise ValueError('input does not match the reviewed i386 build')
    imports = iat_symbols(pe)
    callers = defaultdict(list)
    for ins in instructions.values():
        target = direct_call_target(ins)
        if target is not None:
            callers[target].append(hx(ins.va))
    rows, bodies, edges, api_calls = [], {}, {}, {}
    for spec in config['functions']:
        start, size = int(spec['va'], 16), spec['size']
        raw = read_va(pe, start, size)
        digest = hashlib.sha256(raw).hexdigest()
        if digest != spec['sha256']:
            raise ValueError(f'function hash mismatch at {start:#x}')
        body = reviewed_body(instructions, start, start + size)
        bodies[start] = body
        calls, api, unresolved = [], [], []
        for ins in body:
            target = rel32_target(ins)
            imported = imported_target(ins, imports)
            if target is not None and (ins.raw[0] == 0xe8 or not start <= target < start + size):
                calls.append({'site_va': hx(ins.va), 'target_va': hx(target),
                              'transfer': 'call' if ins.raw[0] == 0xe8 else 'tail_jump'})
            if imported:
                api.append({'site_va': hx(ins.va), **imported})
            elif ins.mnemonic in ('call', 'jmp') and target is None:
                unresolved.append(hx(ins.va))
        edges[start] = [int(c['target_va'], 16) for c in calls]
        api_calls[start] = api
        rows.append({**spec, 'end_va_exclusive': hx(start + size), 'hash_verified': True,
                     'library_identity': 'unverified', 'original_symbol': None, 'original_source_unit': None,
                     'calls': calls, 'imported_calls': api, 'unresolved_transfers': unresolved,
                     'observed_direct_call_sites': sorted(callers[start]),
                     'observed_direct_call_count': len(callers[start]),
                     'instructions': [{'va': hx(i.va), 'size': len(i.raw),
                                       'mnemonic': i.mnemonic, 'operands': i.operands} for i in body]})
    for row in rows:
        row['may_reach_imports'] = reachable_imports(int(row['va'], 16), edges, api_calls)
    groups = []
    for members in config['same_body_groups']:
        for member in members[1:]:
            groups.append(compare_resolved_bodies(bodies[int(members[0], 16)], bodies[int(member, 16)]))
    wrapper_targets, seen = defaultdict(set), set()
    for cls in parse_rtti(pe):
        for vt in cls.vtables:
            for slot in range(256):
                try:
                    fn = struct.unpack('<I', read_va(pe, vt + 4 * slot, 4))[0]
                    if not pe.is_code_va(fn):
                        break
                    if fn in seen:
                        continue
                    seen.add(fn)
                    artifact = scalar_deleting_destructor(fn, read_va(pe, fn, 30))
                    if artifact:
                        wrapper_targets[artifact['operator_delete_va']].add(fn)
                except (ValueError, struct.error):
                    break
    labels = category_literals(instructions, int(config['category_select_va'], 16), pe)
    return {'functions': rows, 'same_body_comparisons': groups, 'category_arguments': labels,
            'known_globals': config['globals'],
            'deleting_wrapper_targets': [{'target_va': hx(v), 'wrapper_count': len(s),
                                           'wrapper_vas': [hx(x) for x in sorted(s)]}
                                          for v, s in sorted(wrapper_targets.items())],
            'summary': {'reviewed_functions': len(rows),
                        'all_reviewed_function_hashes_verified': True,
                        'observed_category_select_calls': len(callers[int(config['category_select_va'], 16)]),
                        'category_calls_with_literal_arguments': len(labels),
                        'distinct_literal_categories': sorted({x['label'] for x in labels}),
                        'same_body_pairs_validated': sum(x['equal_with_resolved_transfers'] for x in groups),
                        'library_signatures_matched': 0,
                        'original_translation_units_proven': 0}}
