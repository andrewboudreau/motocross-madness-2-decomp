"""Conservative PE32/i386 provenance inventory. Never executes an input binary.

Ownership, compiler-generated role, and source-unit evidence are independent.
GNU objdump boundaries and direct-call targets remain discovery heuristics.
"""
from __future__ import annotations

from collections import Counter, defaultdict
from dataclasses import dataclass
import hashlib
from pathlib import Path, PureWindowsPath
import re
import struct
import subprocess

from .pe import PEImage, PEFormatError
from .rtti import parse_rtti
from .msvc import scalar_deleting_destructor, this_adjustor_thunk

PROJECT_ROOT = 'D:\\aardvark\\VC\\krusty2\\'
SOURCE_RE = re.compile(rb'([A-Za-z]:\\[^\x00\r\n]{1,500}?\.(?:cpp|cxx|c|hpp|h|inl))\x00', re.I)
DIRECTX = {'ddraw.dll', 'd3drm.dll', 'd3dim.dll', 'dinput.dll', 'dsound.dll', 'dplayx.dll'}
WINDOWS = {'kernel32.dll', 'user32.dll', 'gdi32.dll', 'advapi32.dll', 'shell32.dll',
           'ole32.dll', 'oleaut32.dll', 'winmm.dll', 'msvfw32.dll', 'imm32.dll',
           'wsock32.dll', 'ws2_32.dll', 'comdlg32.dll', 'comctl32.dll', 'version.dll'}
CAVEATS = [
    'Candidate entry points are not an exhaustive or symbol-proven function inventory.',
    'Decoded ranges are reachable instructions, not original function extents or translation-unit ranges.',
    'A source string reference can come from a header or inlined code; it does not prove a complete original translation unit.',
    'Calling an imported API does not make the caller Microsoft code.',
    'Compiler-generated class glue can still be associated with the Rainbow project.',
    'An IAT slot is local pointer storage, not a resolved DLL function address.',
    'Indirect calls, COM methods, delay imports and LoadLibrary/GetProcAddress targets are not fully resolved.',
    'No static-library signature corpus has been applied; runtime candidates are not confirmed Microsoft CRT functions.',
    'DLL identity strings are self-reported metadata, not authenticated vendor signatures.',
    'No decomp completion percentage or total Rainbow function count is established.',
]


def hx(n: int) -> str:
    return f'0x{n:08x}'


def checked_read(pe: PEImage, rva: int, size: int) -> bytes:
    if rva < 0 or size < 0:
        raise PEFormatError('negative RVA or size')
    off = pe.rva_to_offset(rva)
    sec = pe.section_for_rva(rva)
    limit = sec.file_end if sec else pe.size_of_headers
    if off + size > min(limit, len(pe.data)):
        raise PEFormatError(f'truncated/unbacked range at RVA {rva:#x}')
    return pe.data[off:off + size]


def cstring(pe: PEImage, rva: int) -> str:
    off = pe.rva_to_offset(rva)
    sec = pe.section_for_rva(rva)
    limit = min(len(pe.data), sec.file_end if sec else pe.size_of_headers, off + 4096)
    end = pe.data.find(b'\0', off, limit)
    if end < 0:
        raise PEFormatError('unterminated PE string')
    return pe.data[off:end].decode('ascii', 'replace')


def directory(pe: PEImage, index: int) -> tuple[int, int]:
    return pe.directories[index] if index < len(pe.directories) else (0, 0)


def u32(pe: PEImage, rva: int) -> int:
    return struct.unpack('<I', checked_read(pe, rva, 4))[0]


def import_records(pe: PEImage) -> list[dict]:
    rva, size = directory(pe, 1)
    if not rva:
        return []
    rows = []
    for d in range(min(size // 20, 4096)):
        oft, stamp, chain, name_rva, iat = struct.unpack('<5I', checked_read(pe, rva + 20 * d, 20))
        if not any((oft, stamp, chain, name_rva, iat)):
            return rows
        dll = cstring(pe, name_rva)
        if not iat:
            raise PEFormatError('missing import address table')
        for i in range(65536):
            value = u32(pe, (oft or iat) + 4 * i)
            if not value:
                break
            checked_read(pe, iat + 4 * i, 4)
            name = ordinal = hint = None
            state = 'resolved_from_lookup'
            if not oft and stamp:
                state = 'bound_iat_without_lookup'
            elif value & 0x80000000:
                ordinal = value & 0xffff
            else:
                hint = struct.unpack('<H', checked_read(pe, value, 2))[0]
                name = cstring(pe, value + 2)
            rows.append({'module': dll, 'name': name, 'ordinal': ordinal, 'hint': hint,
                         'iat_rva': hx(iat + 4 * i), 'iat_va': hx(pe.image_base + iat + 4 * i),
                         'lookup_rva': hx((oft or iat) + 4 * i), 'resolution': state})
        else:
            raise PEFormatError('import thunk limit exceeded')
    raise PEFormatError('import descriptor terminator not found within directory')


def export_records(pe: PEImage) -> list[dict]:
    rva, size = directory(pe, 0)
    if not rva:
        return []
    h = struct.unpack('<IIHH7I', checked_read(pe, rva, 40))
    base, count, named, eat, names, ords = h[5:]
    if count > 65536 or named > 65536:
        raise PEFormatError('export table limit exceeded')
    labels = defaultdict(list)
    for i in range(named):
        index = struct.unpack('<H', checked_read(pe, ords + 2 * i, 2))[0]
        if index >= count:
            raise PEFormatError('export ordinal index outside EAT')
        labels[index].append(cstring(pe, u32(pe, names + 4 * i)))
    rows = []
    for i in range(count):
        address = u32(pe, eat + 4 * i)
        if not address:
            continue
        forwarder = cstring(pe, address) if rva <= address < rva + size else None
        rows.append({'ordinal': base + i, 'names': sorted(labels[i]), 'rva': hx(address),
                     'preferred_va': None if forwarder else hx(pe.image_base + address),
                     'forwarder': forwarder})
    return rows


def source_strings(pe: PEImage) -> dict[int, str]:
    result = {}
    for match in SOURCE_RE.finditer(pe.data):
        try:
            va = pe.offset_to_va(match.start(1))
        except PEFormatError:
            continue
        result[va] = match.group(1).decode('latin1')
    return result


def version_fields(pe: PEImage) -> dict:
    """Read RT_VERSION resource blocks, not arbitrary lookalike string pairs."""
    root, size = directory(pe, 2)
    result = {}
    if not root:
        return result

    def resource_read(offset, length):
        if offset < 0 or offset + length > size:
            raise PEFormatError('resource offset outside directory')
        return checked_read(pe, root + offset, length)

    def entries(offset):
        h = resource_read(offset, 16)
        count = sum(struct.unpack_from('<HH', h, 12))
        if count > 4096:
            raise PEFormatError('resource entry limit')
        return [struct.unpack('<II', resource_read(offset + 16 + i * 8, 8)) for i in range(count)]

    def parse_value(data, pos, end, depth=0):
        if depth > 8 or pos + 6 > end:
            return
        length, value_length, kind = struct.unpack_from('<HHH', data, pos)
        stop = pos + length
        if length < 6 or stop > end:
            raise PEFormatError('invalid version block length')
        key_end = pos + 6
        while key_end + 2 <= stop and data[key_end:key_end + 2] != b'\0\0':
            key_end += 2
        if key_end + 2 > stop:
            raise PEFormatError('unterminated version key')
        key = data[pos + 6:key_end].decode('utf-16le', 'replace')
        value_pos = (key_end + 5) & ~3
        value_end = value_pos + value_length * (2 if kind else 1)
        if value_end > stop:
            raise PEFormatError('version value outside block')
        if kind == 1 and value_length and key in {
            'CompanyName', 'ProductName', 'FileDescription', 'FileVersion', 'ProductVersion', 'OriginalFilename'
        }:
            value = data[value_pos:value_end].decode('utf-16le', 'replace').rstrip('\0')
            result.setdefault(key, []).append(value)
        child = (value_end + 3) & ~3
        while child + 6 <= stop:
            child_length = struct.unpack_from('<H', data, child)[0]
            if not child_length:
                break
            parse_value(data, child, stop, depth + 1)
            child = (child + child_length + 3) & ~3

    def descend(offset, depth=0):
        if depth > 3:
            raise PEFormatError('resource tree depth exceeded')
        for _, child in entries(offset):
            if child & 0x80000000:
                descend(child & 0x7fffffff, depth + 1)
            else:
                address, length, _, _ = struct.unpack('<4I', resource_read(child, 16))
                data = checked_read(pe, address, length)
                parse_value(data, 0, len(data))

    for kind, child in entries(0):
        if kind == 16 and child & 0x80000000:
            descend(child & 0x7fffffff)
    return {key: sorted(set(values)) for key, values in sorted(result.items())}


def module_family(name: str) -> str:
    name = name.lower()
    if name in DIRECTX:
        return 'directx'
    if name in WINDOWS:
        return 'windows'
    return 'unclassified_external'


def component_inventory(game_dir: Path, imports: list[dict]) -> list[dict]:
    required = {r['module'].lower() for r in imports}
    by_name = defaultdict(list)
    for p in sorted(game_dir.rglob('*')):
        if p.is_file() and not p.is_symlink() and p.suffix.lower() == '.dll':
            by_name[p.name.lower()].append(p)
    rows = []
    for name in sorted(required | set(by_name)):
        paths = by_name.get(name) or [None]
        for path in paths:
            row = {'module': name, 'family': module_family(name), 'direct_import': name in required,
                   'packaged_path': str(path.relative_to(game_dir)) if path else None,
                   'availability': 'packaged' if path else 'not_in_supplied_package',
                   'identity_evidence': [], 'inspection_error': None}
            if path:
                data = path.read_bytes()
                row['sha256'] = hashlib.sha256(data).hexdigest()
                try:
                    pe = PEImage(path)
                    row['exports'] = export_records(pe)
                    row['imports'] = import_records(pe)
                    row['version_fields'] = version_fields(pe)
                    for text in ('Microsoft Blade Software Rasterizer',):
                        offset = data.find(text.encode('ascii') + b'\0')
                        if offset >= 0:
                            row['identity_evidence'].append({'kind': 'literal_identity_string',
                                                            'text': text, 'file_offset': hx(offset)})
                            row['family'] = 'microsoft_blade_rasterizer_candidate'
                    row['debug_path_strings'] = sorted(set(m.group().decode('ascii') for m in
                        re.finditer(rb'[A-Za-z]:\\[^\x00\r\n]{1,500}\.pdb', data)))
                except (ValueError, struct.error, OSError) as exc:
                    row['inspection_error'] = str(exc)
            rows.append(row)
    return rows


@dataclass(frozen=True)
class Instruction:
    va: int
    raw: bytes
    mnemonic: str
    operands: str

    @property
    def end(self):
        return self.va + len(self.raw)


def direct_target(ins: Instruction) -> int | None:
    m = re.match(r'^(?:0x)?([0-9a-fA-F]+)(?:\s|$)', ins.operands)
    return int(m.group(1), 16) if m else None


def decode(pe: PEImage, objdump='objdump') -> tuple[dict[int, Instruction], str]:
    version = subprocess.run([objdump, '--version'], check=True, capture_output=True, text=True, timeout=10).stdout.splitlines()[0]
    run = subprocess.run([objdump, '-d', '-w', '-Mintel', str(pe.path)], check=True,
                         capture_output=True, text=True, timeout=90)
    rows = {}
    for line in run.stdout.splitlines():
        parts = line.split('\t')
        if len(parts) < 3 or not re.fullmatch(r'\s*[0-9a-fA-F]+:', parts[0]):
            continue
        raw = bytes.fromhex(parts[1].strip())
        va = int(parts[0].strip()[:-1], 16)
        asm = parts[2].strip().split(None, 1)
        if not raw or not asm or not pe.is_code_va(va):
            continue
        if checked_read(pe, va - pe.image_base, len(raw)) != raw:
            raise PEFormatError('disassembler bytes differ from input image')
        rows[va] = Instruction(va, raw, asm[0], asm[1] if len(asm) > 1 else '')
    return rows, version


def instruction_sources(ins: Instruction, sources: dict[int, str]) -> list[dict]:
    # Only explicit immediates in push/mov, not arbitrary 4-byte occurrences.
    if ins.mnemonic not in ('push', 'mov'):
        return []
    operand = ins.operands.split(',')[-1].strip()
    if not re.fullmatch(r'0x[0-9a-fA-F]+', operand):
        return []
    va = int(operand, 16)
    if va not in sources:
        return []
    return [{'instruction_va': hx(ins.va), 'string_va': hx(va), 'path': sources[va],
             'evidence_kind': 'decoded_source_string_immediate'}]


def merge_ranges(ranges) -> list[list[int]]:
    out = []
    for lo, hi in sorted(ranges):
        if hi <= lo:
            continue
        if out and lo <= out[-1][1]:
            out[-1][1] = max(out[-1][1], hi)
        else:
            out.append([lo, hi])
    return out


def walk(start: int, instructions: dict[int, Instruction], seeds: set[int], limit=8192):
    pending, seen, stops = [start], set(), set()
    while pending:
        va = pending.pop()
        if va in seen:
            continue
        if va != start and va in seeds:
            stops.add('other_candidate_entry')
            continue
        if abs(va - start) > 0x20000:
            stops.add('span_limit')
            continue
        ins = instructions.get(va)
        if ins is None or ins.mnemonic in ('(bad)', '.byte'):
            stops.add('undecoded_address')
            continue
        seen.add(va)
        if len(seen) >= limit:
            stops.add('instruction_limit')
            break
        op = ins.mnemonic
        if op.startswith(('ret', 'iret')) or op in ('int3', 'ud2', 'hlt') or (op in ('repz', 'rep') and ins.operands.startswith('ret')):
            continue
        if op == 'jmp':
            target = direct_target(ins)
            if target is None:
                stops.add('indirect_jump')
            else:
                pending.append(target)
            continue
        if op.startswith('j') or op.startswith('loop'):
            target = direct_target(ins)
            if target is not None:
                pending.append(target)
            else:
                stops.add('unknown_branch')
        # Calls fall through, but their callees never become part of this body.
        pending.append(ins.end)
    return sorted(seen), sorted(stops)


def iat_reference(ins: Instruction, imports_by_va: dict[int, dict]) -> dict | None:
    if ins.mnemonic not in ('call', 'jmp'):
        return None
    m = re.fullmatch(r'DWORD PTR (?:ds:)?(?:\[)?(0x[0-9a-fA-F]+)(?:\])?', ins.operands)
    if not m:
        return None
    row = imports_by_va.get(int(m.group(1), 16))
    return {'instruction_va': hx(ins.va), 'operation': ins.mnemonic, **row} if row else None


def build_map(pe: PEImage, instructions: dict[int, Instruction], imports: list[dict]) -> dict:
    sources = source_strings(pe)
    seed_reasons = defaultdict(set)
    uses = defaultdict(list)
    roles = {}
    delete_targets = Counter()
    classes = parse_rtti(pe)
    vtables = []
    for cls in classes:
        for record in cls.vtable_records:
            for va in record.vtables:
                entries = []
                for slot in range(256):
                    try:
                        fn = u32(pe, va - pe.image_base + 4 * slot)
                    except (ValueError, struct.error):
                        break
                    if not pe.is_code_va(fn):
                        break
                    entries.append(fn)
                    uses[fn].append({'class': cls.name, 'slot': slot, 'vtable_va': hx(va),
                                     'object_offset': record.object_offset})
                    seed_reasons[fn].add('rtti_vtable_entry')
                vtables.append({'class': cls.name, 'object_offset': record.object_offset, 'entries': entries})
    if pe.entry_rva:
        seed_reasons[pe.image_base + pe.entry_rva].add('pe_entry_point')
    for fn in list(uses):
        blob = pe.bytes_at_va(fn, 40)
        artifact = scalar_deleting_destructor(fn, blob) or this_adjustor_thunk(fn, blob)
        if artifact:
            roles[fn] = artifact['kind']
            callee = artifact.get('destructor_va', artifact.get('target_va'))
            if callee and pe.is_code_va(callee):
                seed_reasons[callee].add('compiler_artifact_target')
            if artifact.get('operator_delete_va'):
                delete_targets[artifact['operator_delete_va']] += 1
    iat = {int(x['iat_va'], 16): x for x in imports}
    references, thunks = {}, {}
    for va, ins in instructions.items():
        if ins.mnemonic == 'call':
            target = direct_target(ins)
            if target in instructions:
                seed_reasons[target].add('direct_call_target_linear_disassembly')
        ref = iat_reference(ins, iat)
        if ref:
            references[va] = ref
            if ins.mnemonic == 'jmp' and len(ins.raw) == 6 and ins.raw[:2] == bytes((255, 37)):
                thunks[va] = ref
                roles[va] = 'import_thunk'
                seed_reasons[va].add('decoded_iat_jump_stub')
    primary = {v['class']: v['entries'] for v in vtables if v['object_offset'] == 0}
    bases = {c.name: c.direct_bases for c in classes}
    primary_base = {c.name: next((b.name for b in c.bases[1:]
                                 if b.name in c.direct_bases and b.mdisp == 0 and b.pdisp == -1), None)
                    for c in classes}
    direct_uses = defaultdict(list)
    for va, refs in uses.items():
        for ref in refs:
            if ref['object_offset'] != 0:
                continue
            base = primary_base[ref['class']]
            if bases[ref['class']] and base is None:
                continue  # Do not flatten ambiguous multiple/virtual inheritance.
            bv = primary.get(base, [])
            if ref['slot'] < len(bv) and bv[ref['slot']] == va:
                continue
            direct_uses[va].append(ref)
    filenames = defaultdict(list)
    for path in sorted(set(sources.values())):
        if path.lower().endswith('.cpp'):
            filenames[PureWindowsPath(path).stem.lower()].append(path)
    seed_set = set(seed_reasons)
    rows, all_ranges, assigned_sources = [], [], defaultdict(list)
    for start in sorted(seed_set):
        reached, stops = walk(start, instructions, seed_set)
        ranges = merge_ranges((va, instructions[va].end) for va in reached)
        all_ranges.extend(ranges)
        src = [item for va in reached for item in instruction_sources(instructions[va], sources)]
        paths = sorted({x['path'] for x in src})
        hints = sorted({p for ref in direct_uses[start] for p in filenames.get(ref['class'].lower(), [])})
        owner, confidence = 'unknown', 'unclassified'
        if any(p.lower().startswith(PROJECT_ROOT.lower()) for p in paths):
            owner, confidence = 'rainbow_project_associated', 'source_reference_observed'
        elif any(p.lower().startswith(PROJECT_ROOT.lower()) for p in hints):
            owner, confidence = 'rainbow_project_candidate', 'class_filename_hint_only'
        role = roles.get(start, 'ordinary_or_unknown')
        if start in delete_targets:
            role = 'operator_delete_candidate'
        if role == 'import_thunk':
            owner, confidence = 'linker_glue', 'decoded_iat_jump'
        ext = [references[va] for va in reached if va in references]
        for va in reached:
            ins = instructions[va]
            target = direct_target(ins) if ins.mnemonic == 'call' else None
            if target in thunks:
                ext.append({**thunks[target], 'instruction_va': hx(va), 'operation': 'call_via_import_thunk', 'thunk_va': hx(target)})
        row = {'entry_va': hx(start), 'discovery': sorted(seed_reasons[start]), 'owner': owner,
               'owner_evidence': confidence, 'code_role': role, 'original_translation_unit': None,
               'source_references': src, 'class_filename_hypotheses': hints,
               'class_uses': uses[start], 'noninherited_primary_uses': direct_uses[start],
               'decoded_ranges': [[hx(a), hx(b)] for a, b in ranges],
               'decoded_byte_count': sum(b - a for a, b in ranges),
               'traversal_stops': stops, 'external_calls': ext,
               'unresolved_indirect_calls': [hx(va) for va in reached if instructions[va].mnemonic == 'call'
                                              and direct_target(instructions[va]) is None and va not in references]}
        if start in delete_targets:
            row['runtime_hypothesis'] = {'provider': 'unknown', 'candidate': 'global operator delete / allocation bookkeeping',
                                         'wrapper_callers': delete_targets[start], 'signature_validated': False}
        rows.append(row)
        for path in paths:
            assigned_sources[path].append(hx(start))
    reached_union = merge_ranges(all_ranges)
    section_ranges = [[pe.image_base + s.virtual_address,
                       pe.image_base + s.virtual_address + min(s.raw_size, s.virtual_size or s.raw_size)]
                      for s in pe.sections if s.characteristics & 0x20000000]
    section_union = merge_ranges(section_ranges)
    executable_bytes = sum(b - a for a, b in section_union)
    covered = merge_ranges((max(a, x), min(b, y)) for a, b in reached_union for x, y in section_union if max(a, x) < min(b, y))
    reached_bytes = sum(b - a for a, b in covered)
    units = [{'path': p, 'kind': PureWindowsPath(p).suffix.lower().lstrip('.'),
              'string_vas': [hx(va) for va, path in sorted(sources.items()) if path == p],
              'referencing_candidates': sorted(assigned_sources[p]), 'range_assignment': 'not_established'}
             for p in sorted(set(sources.values()))]
    return {'functions': rows, 'source_units': units, 'summary': {
        'candidate_entries': len(rows), 'rtti_class_records': len(classes), 'vtables': len(vtables),
        'owners': dict(sorted(Counter(r['owner'] for r in rows).items())),
        'roles': dict(sorted(Counter(r['code_role'] for r in rows).items())),
        'source_paths': len(units), 'source_path_kinds': dict(sorted(Counter(x['kind'] for x in units).items())),
        'source_paths_referenced_by_candidates': sum(bool(x['referencing_candidates']) for x in units),
        'executable_section_bytes': executable_bytes, 'candidate_reachable_instruction_bytes': reached_bytes,
        'bytes_not_reached_from_current_candidates': executable_bytes - reached_bytes,
        'coverage_is_decomp_progress': False, 'exhaustive_function_inventory': False,
        'exact_translation_unit_ranges_established': 0},
        'executable_section_ranges': [[hx(a), hx(b)] for a, b in section_union]}
