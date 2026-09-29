"""Category-to-source navigation evidence; never a source ownership classifier.

Decoded call arguments, containing CFG candidates and one-hop call context stay
separate. No transitive label propagation or original-TU assignment is made.
"""
from __future__ import annotations
from collections import Counter, defaultdict, deque
import hashlib
from pathlib import PureWindowsPath
import re
import struct

from .allocation import (Insn, ascii_at, decode_image, direct_call_target, hx,
                         iat_symbols, imported_target, read_va, rel32_target)
from .msvc import scalar_deleting_destructor, this_adjustor_thunk
from .rtti import parse_rtti

SOURCE_RE = re.compile(rb'([A-Za-z]:\\[^\x00\r\n]{1,500}?\.(?:cpp|cxx|c|hpp|h|inl))\x00', re.I)
REGISTERS = {'eax', 'ecx', 'edx', 'ebx', 'ebp', 'esi', 'edi'}
LIMITATIONS = [
    'Categories name memory-accounting contexts, not necessarily original source folders or modules.',
    'Candidate function boundaries come from linear disassembly and bounded CFG traversal; they are not exhaustive.',
    'Source paths are decoded immediate references, including possible headers/inlined code; no original TU is assigned.',
    'One-hop calls are reachable before a local select/restore boundary, not proof of the runtime category at the callee.',
    'Unknown callees can change global accounting state; exception edges, jump tables and indirect callees are not resolved.',
    'Multiple category contexts are preserved, not collapsed into a single owner or propagated transitively.',
    'Vptr stores are class construction/destruction/member clues, not proof that the containing function is a class method.',
    'No source files are moved or renamed, and no new byte-match or CRT-identity claim is made by this pass.',
]


def branch_target(ins: Insn) -> int | None:
    raw = ins.raw
    if len(raw) == 2 and (raw[0] == 0xeb or 0x70 <= raw[0] <= 0x7f or 0xe0 <= raw[0] <= 0xe3):
        return (ins.end + struct.unpack('<b', raw[1:])[0]) & 0xffffffff
    if len(raw) == 6 and raw[0] == 0x0f and 0x80 <= raw[1] <= 0x8f:
        return (ins.end + struct.unpack('<i', raw[2:])[0]) & 0xffffffff
    if raw[:1] == b'\xe9':
        return rel32_target(ins)
    return None


def successors(ins: Insn) -> tuple[list[int], str | None]:
    op = ins.mnemonic
    if op.startswith(('ret', 'iret')) or op in ('int3', 'ud2', 'hlt'):
        return [], 'return_or_trap'
    if op in ('rep', 'repz') and ins.operands.startswith('ret'):
        return [], 'return_or_trap'
    if op.startswith('j') or op.startswith('loop'):
        target = branch_target(ins)
        if target is None:
            return [], 'unresolved_branch'
        return ([target] if op == 'jmp' else [ins.end, target]), None
    if op in ('(bad)', '.byte'):
        return [], 'invalid_decode'
    # Calls fall through; never traverse into their callees.
    return [ins.end], None


def walk(start: int, instructions: dict[int, Insn], entries: set[int],
         boundaries: dict[int, str] | None = None, limit: int = 8192):
    pending, seen, stops = deque([start]), set(), set()
    boundaries = boundaries or {}
    while pending:
        va = pending.popleft()
        if va in seen:
            continue
        if va in boundaries:
            stops.add((hx(va), boundaries[va])); continue
        if va != start and va in entries:
            stops.add((hx(va), 'other_candidate_entry')); continue
        if abs(va - start) > 0x20000:
            stops.add((hx(va), 'span_limit')); continue
        ins = instructions.get(va)
        if ins is None or ins.mnemonic in ('(bad)', '.byte'):
            stops.add((hx(va), 'undecoded')); continue
        seen.add(va)
        if len(seen) >= limit:
            stops.add((hx(va), 'instruction_limit')); break
        nxt, reason = successors(ins)
        if reason:
            stops.add((hx(va), reason))
        pending.extend(nxt)
    return sorted(seen), [{'va': v, 'reason': r} for v, r in sorted(stops)]


def preserves_stack_argument(ins: Insn) -> bool:
    if ins.mnemonic != 'mov' or ',' not in ins.operands:
        return False
    dest = ins.operands.split(',', 1)[0].strip()
    if dest in REGISTERS:
        return True
    # Permit only explicit positive ESP-relative stores beyond the top dword.
    # Arbitrary pointer stores may alias the argument, so they stop the search.
    m = re.fullmatch(r'(?:BYTE|WORD|DWORD) PTR \[esp\+(0x[0-9a-f]+)\]', dest)
    return bool(m and 4 <= int(m.group(1), 16) <= 0x10000)


def recover_argument(call_va: int, instructions: dict[int, Insn], by_end: dict[int, Insn],
                     branch_entries: set[int], read_string, limit=16) -> dict:
    pos, proof = call_va, []
    for _ in range(limit):
        if pos in branch_entries:
            return {'label': None, 'reason': 'control_flow_entry_before_push'}
        previous = by_end.get(pos)
        if previous is None:
            return {'label': None, 'reason': 'decode_gap'}
        proof.append(hx(previous.va))
        if len(previous.raw) == 5 and previous.raw[0] == 0x68:
            va = struct.unpack_from('<I', previous.raw, 1)[0]
            value = read_string(va)
            if value:
                return {'label': value, 'reason': 'literal_stack_argument',
                        'push_va': hx(previous.va), 'string_va': hx(va),
                        'proof_instructions': list(reversed(proof))}
            return {'label': None, 'reason': 'argument_not_printable_literal'}
        if not preserves_stack_argument(previous):
            return {'label': None, 'reason': 'dynamic_argument_or_unmodelled_write', 'stopped_at': hx(previous.va)}
        pos = previous.va
    return {'label': None, 'reason': 'lookback_limit'}


def immediate(ins: Insn) -> int | None:
    if ins.mnemonic not in ('mov', 'push'):
        return None
    operand = ins.operands.split(',')[-1].strip()
    return int(operand, 16) if re.fullmatch(r'0x[0-9a-fA-F]+', operand) else None


def rtti_evidence(pe):
    classes = parse_rtti(pe)
    vtables, uses, roles, extra_entries = {}, defaultdict(list), {}, set()
    for cls in classes:
        for rec in cls.vtable_records:
            for vt in rec.vtables:
                vtables[vt] = {'class': cls.name, 'object_offset': rec.object_offset, 'vtable_va': hx(vt)}
                for slot in range(256):
                    try:
                        target = struct.unpack('<I', read_va(pe, vt + slot * 4, 4))[0]
                        if not pe.is_code_va(target):
                            break
                        uses[target].append({**vtables[vt], 'slot': slot})
                    except (ValueError, struct.error):
                        break
    for fn in uses:
        try:
            body = read_va(pe, fn, 40)
            artifact = scalar_deleting_destructor(fn, body) or this_adjustor_thunk(fn, body)
        except (ValueError, struct.error):
            artifact = None
        if artifact:
            roles[fn] = artifact['kind']
            extra_entries.add(artifact.get('destructor_va', artifact.get('target_va')))
    return vtables, uses, roles, extra_entries


def source_map(pe):
    paths = {}
    for match in SOURCE_RE.finditer(pe.data):
        try:
            paths[pe.offset_to_va(match.start(1))] = match.group(1).decode('latin1')
        except ValueError:
            continue
    return paths


def validate_build(pe, config):
    if pe.machine != 0x14c or hashlib.sha256(pe.data).hexdigest() != config['input_sha256']:
        raise ValueError('Category addresses require the reviewed MCM2 PE32/i386 build')
    chosen = {}
    for label in ('select_category_return_previous', 'restore_category_index'):
        specs = [x for x in config['functions'] if x['label'] == label]
        if len(specs) != 1:
            raise ValueError(f'missing/ambiguous category function: {label}')
        spec = specs[0]; va = int(spec['va'], 16)
        if hashlib.sha256(read_va(pe, va, spec['size'])).hexdigest() != spec['sha256']:
            raise ValueError(f'Category function bytes differ: {label}')
        chosen[label] = va
    return chosen['select_category_return_previous'], chosen['restore_category_index']


def analyze_categories(pe, config, instructions, decoder):
    select, restore = validate_build(pe, config)
    vtables, uses, roles, extra = rtti_evidence(pe)
    imports = iat_symbols(pe)
    paths = source_map(pe)
    entries = set(uses) | extra | {pe.image_base + pe.entry_rva}
    entries.update(direct_call_target(i) for i in instructions.values() if direct_call_target(i) in instructions)
    entries.discard(None)
    branch_entries = {branch_target(i) for i in instructions.values() if branch_target(i) is not None} | entries
    by_end = {i.end: i for i in instructions.values()}
    boundaries = {i.va: ('select' if direct_call_target(i) == select else 'restore')
                  for i in instructions.values() if direct_call_target(i) in (select, restore)}
    events = []
    for site, boundary in sorted(boundaries.items()):
        if boundary == 'select':
            events.append({'call_va': hx(site), **recover_argument(site, instructions, by_end, branch_entries,
                                                                  lambda va: ascii_at(pe, va, 128))})
    anchor_sites = {int(e['call_va'], 16) for e in events}
    containing, bodies, stops = defaultdict(list), {}, {}
    for entry in sorted(entries):
        reached, reasons = walk(entry, instructions, entries)
        bodies[entry], stops[entry] = reached, reasons
        for site in anchor_sites.intersection(reached):
            containing[site].append(entry)
    cache = {}

    def describe(entry):
        if entry not in cache:
            src, writes, calls = [], [], []
            reached = bodies.get(entry, [])
            for va in reached:
                ins = instructions[va]; value = immediate(ins)
                if value in paths:
                    src.append({'path': paths[value], 'instruction_va': hx(va), 'string_va': hx(value)})
                if value in vtables and ins.mnemonic == 'mov' and 'PTR [' in ins.operands.split(',')[0]:
                    writes.append({**vtables[value], 'instruction_va': hx(va),
                                   'destination': ins.operands.split(',')[0]})
                target = direct_call_target(ins)
                if target is not None:
                    calls.append({'call_va': hx(va), 'target_va': hx(target)})
            cache[entry] = {'entry_va': hx(entry), 'source_references': src, 'vtable_uses': uses.get(entry, []),
                            'vptr_store_clues': writes, 'direct_calls': calls,
                            'contained_category_selects': [hx(v) for v in sorted(anchor_sites.intersection(reached))],
                            'traversal_stops': stops.get(entry, []), 'original_translation_unit': None,
                            'code_role': roles.get(entry, 'ordinary_or_unknown')}
        return cache[entry]

    reviewed_support = {int(x['va'], 16) for x in config['functions']}
    contexts = defaultdict(list)
    for event in events:
        site = int(event['call_va'], 16)
        parents = containing.get(site, [])
        event['containing_candidates'] = [hx(x) for x in parents]
        event['container_status'] = 'unique_candidate' if len(parents) == 1 else ('ambiguous' if parents else 'unresolved')
        for parent in parents:
            describe(parent)
        event['window_calls'] = []
        event['window_stops'] = []
        if event['label'] is None:
            continue
        reached, reasons = walk(instructions[site].end, instructions, entries, boundaries, limit=4096)
        event['window_stops'] = reasons
        for va in reached:
            ins = instructions[va]; target = direct_call_target(ins)
            api = imported_target(ins, imports)
            if api:
                event['window_calls'].append({'call_va': hx(va), 'kind': 'external_boundary', 'api': api})
            elif target is not None:
                is_thunk = target in instructions and imported_target(instructions[target], imports) is not None
                support = target in reviewed_support or is_thunk or target in roles
                kind = 'support_boundary' if support else 'one_hop_context_only'
                event['window_calls'].append({'call_va': hx(va), 'target_va': hx(target), 'kind': kind})
                if not support:
                    contexts[target].append({'category': event['label'], 'select_call_va': event['call_va'],
                                             'call_va': hx(va)})
                    describe(target)
            elif ins.mnemonic == 'call':
                event['window_calls'].append({'call_va': hx(va), 'kind': 'unresolved_indirect_call', 'operand': ins.operands})
    category_rows = []
    for label in sorted({e['label'] for e in events if e['label']}):
        anchors = [e for e in events if e['label'] == label]
        parents = sorted({p for e in anchors for p in e['containing_candidates']})
        targets = sorted({t for t, cs in contexts.items() if any(c['category'] == label for c in cs)})
        links = []
        for parent in parents:
            desc = describe(int(parent, 16))
            for s in desc['source_references']:
                links.append({**s, 'candidate_va': parent, 'evidence': 'same_candidate_as_select',
                              'container_select_count': len(desc['contained_category_selects'])})
        for target in targets:
            for s in describe(target)['source_references']:
                links.append({**s, 'candidate_va': hx(target), 'evidence': 'one_hop_context_not_ownership'})
        category_rows.append({'category': label, 'anchor_calls': [e['call_va'] for e in anchors],
                              'containing_candidates': parents, 'one_hop_candidates': [hx(t) for t in targets],
                              'source_links': sorted(links, key=lambda x: (x['path'], x['evidence'], x['candidate_va'], x['instruction_va']))})
    sources = []
    for path in sorted(set(paths.values())):
        links = [{**s, 'category': c['category']} for c in category_rows for s in c['source_links'] if s['path'] == path]
        sources.append({'path': path, 'filename': PureWindowsPath(path).name, 'links': links,
                        'observed_categories': sorted({x['category'] for x in links}), 'original_module': None})
    context_rows = [{'entry_va': hx(v), 'observed_contexts': sorted({x['category'] for x in cs}),
                     'sites': sorted(cs, key=lambda x: (x['category'], x['select_call_va'], x['call_va'])),
                     'owner_assignment': None} for v, cs in sorted(contexts.items())]
    return {'schema_version': 1, 'input_sha256': config['input_sha256'], 'decoder': decoder,
            'select_va': hx(select), 'restore_va': hx(restore), 'limitations': LIMITATIONS,
            'events': events, 'categories': category_rows, 'sources': sources,
            'candidates': [cache[v] for v in sorted(cache)], 'call_contexts': context_rows,
            'summary': {'category_select_calls': len(events), 'literal_category_calls': sum(e['label'] is not None for e in events),
                        'unresolved_category_calls': sum(e['label'] is None for e in events), 'distinct_categories': len(category_rows),
                        'candidate_entry_count': len(entries), 'container_status': dict(sorted(Counter(e['container_status'] for e in events).items())),
                        'source_paths_with_links': sum(bool(x['links']) for x in sources),
                        'one_hop_context_candidates': len(context_rows),
                        'multi_category_context_candidates': sum(len(x['observed_contexts']) > 1 for x in context_rows),
                        'ownership_assignments': 0, 'source_files_moved': 0}}
