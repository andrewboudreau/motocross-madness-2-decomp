"""Category/source evidence joins. Allocation context is NOT source ownership.

Re-decode category arguments and source references from the input image. The
provenance snapshot supplies candidate ranges only; class uses are re-read from
RTTI. Never spread categories to callees, siblings, or entire translation units.
"""
from __future__ import annotations

from collections import Counter, defaultdict
import hashlib
import re
import struct

from .allocation import Insn, ascii_at, direct_call_target, hx, read_va
from .pe import PEImage
from .rtti import parse_rtti

REGISTERS = {'eax', 'ecx', 'edx', 'ebx', 'ebp', 'esi', 'edi'}
SOURCE_PATH = re.compile(r'^[A-Za-z]:\\[^\r\n]+\.(?:cpp|cxx|c|hpp|h|inl)$', re.I)
CAVEATS = [
    'Category names describe allocation context, not exclusive code ownership.',
    'Candidate ranges come from the hash-bound provenance snapshot; they are not proven original function boundaries.',
    'Source references are decoded observations within the same candidate, not original translation-unit assignments.',
    'RTTI uses refer only to non-inherited primary slots; no category is propagated to other methods or derived classes.',
    'Different categories in one candidate can represent phases of loading or orchestration, not conflicting evidence.',
    'Backward argument recovery is bounded and direct-branch-aware; indirect incoming edges and aliasing are not fully solved.',
    'No active-category lifetime, restore pairing, callee ownership, static-library identity, or decomp progress is inferred.',
]


class CategoryError(ValueError):
    pass


def number(value) -> int:
    if isinstance(value, bool):
        raise CategoryError('boolean is not an address')
    result = int(value, 0) if isinstance(value, str) else value
    if not isinstance(result, int) or not 0 <= result <= 0xffffffff:
        raise CategoryError('invalid i386 address')
    return result


def branch_entries(instructions: dict[int, Insn]) -> set[int]:
    entries = set()
    for ins in instructions.values():
        if ins.mnemonic.startswith('j') or ins.mnemonic.startswith('loop'):
            if re.fullmatch(r'0x[0-9a-fA-F]+', ins.operands):
                entries.add(int(ins.operands, 16))
    return entries


def safe_argument_gap(ins: Insn) -> bool:
    # Only explicit MOVs which leave ESP and the top argument untouched.
    # Arbitrary [eax]/[ebp] stores could alias the argument: reject them.
    if ins.mnemonic != 'mov' or ',' not in ins.operands:
        return False
    destination = ins.operands.split(',', 1)[0].strip()
    if destination in REGISTERS:
        return True
    stack = re.fullmatch(r'DWORD PTR \[esp\+(0x[0-9a-fA-F]+)\]', destination)
    return bool(stack and 4 <= int(stack.group(1), 16) <= 0x7fffffff)


def recover_argument(call: Insn, by_end: dict[int, Insn], incoming: set[int], pe: PEImage,
                     max_instructions=16, max_bytes=96) -> dict:
    pos, gap = call.va, []
    for _ in range(max_instructions):
        if pos in incoming:
            return {'label': None, 'reason': 'incoming_branch_can_bypass_argument'}
        previous = by_end.get(pos)
        if previous is None:
            return {'label': None, 'reason': 'decoding_gap'}
        if call.va - previous.va > max_bytes:
            return {'label': None, 'reason': 'backward_byte_limit'}
        if len(previous.raw) == 5 and previous.raw[0] == 0x68:
            string_va = struct.unpack_from('<I', previous.raw, 1)[0]
            label = ascii_at(pe, string_va, 128)
            if label is None:
                return {'label': None, 'reason': 'argument_not_a_printable_literal'}
            return {'label': label, 'reason': 'literal_argument_observed',
                    'push_va': hx(previous.va), 'string_va': hx(string_va),
                    'intervening_instructions': list(reversed(gap))}
        if not safe_argument_gap(previous):
            return {'label': None, 'reason': 'unsupported_argument_or_intervening_instruction',
                    'barrier_va': hx(previous.va)}
        gap.append(hx(previous.va))
        pos = previous.va
    return {'label': None, 'reason': 'backward_instruction_limit'}


def validate_inputs(pe: PEImage, config: dict, provenance: dict) -> str:
    digest = hashlib.sha256(pe.data).hexdigest()
    if pe.machine != 0x14c or config.get('input_sha256') != digest:
        raise CategoryError('input is not the reviewed i386 build')
    if provenance.get('schema_version') != 1 or provenance.get('input', {}).get('sha256') != digest:
        raise CategoryError('provenance snapshot schema or input hash mismatch')
    if number(provenance['input']['preferred_image_base']) != pe.image_base:
        raise CategoryError('provenance image base mismatch')
    select = number(config['category_select_va'])
    spec = next((f for f in config['functions'] if number(f['va']) == select), None)
    if spec is None:
        raise CategoryError('selector has no reviewed body hash')
    size = spec['size']
    if not isinstance(size, int) or isinstance(size, bool) or not 0 < size <= 65536:
        raise CategoryError('invalid selector extent')
    if hashlib.sha256(read_va(pe, select, size)).hexdigest() != spec['sha256']:
        raise CategoryError('reviewed selector bytes differ')
    return digest


def candidate_ranges(provenance: dict, instructions: dict[int, Insn]) -> list[tuple]:
    result, seen = [], set()
    for row in provenance['functions']:
        entry = number(row['entry_va'])
        if entry in seen:
            raise CategoryError('duplicate candidate entry')
        seen.add(entry)
        ranges, last = [], -1
        for lo, hi in row['decoded_ranges']:
            lo, hi = number(lo), number(hi)
            if hi <= lo or lo < last or lo not in instructions:
                raise CategoryError('invalid candidate range')
            ranges.append((lo, hi)); last = hi
        result.append((entry, ranges))
    return sorted(result)


def primary_class_uses(pe: PEImage) -> dict[int, list[dict]]:
    classes = parse_rtti(pe)
    tables = {}
    for cls in classes:
        for record in cls.vtable_records:
            if record.object_offset != 0:
                continue
            for va in record.vtables:
                entries = []
                for slot in range(256):
                    fn = struct.unpack('<I', read_va(pe, va + 4 * slot, 4))[0]
                    if not pe.is_code_va(fn):
                        break
                    entries.append(fn)
                if cls.name in tables:
                    # Multiple primary records are not silently collapsed.
                    tables[cls.name] = None
                else:
                    tables[cls.name] = (va, entries)
    uses = defaultdict(list)
    for cls in classes:
        own = tables.get(cls.name)
        if own is None:
            continue
        bases = [b for b in cls.bases[1:] if b.name in cls.direct_bases and b.mdisp == 0 and b.pdisp == -1]
        if cls.direct_bases and (len(bases) != 1 or tables.get(bases[0].name) is None):
            continue
        parent = tables[bases[0].name][1] if bases else []
        for slot, fn in enumerate(own[1]):
            if slot < len(parent) and parent[slot] == fn:
                continue
            uses[fn].append({'class': cls.name, 'slot': slot, 'vtable_va': hx(own[0]),
                             'evidence': 'noninherited_primary_vtable_use'})
    return uses


def decode_sources(pe: PEImage, body: list[Insn]) -> list[dict]:
    result = []
    for ins in body:
        if ins.mnemonic not in ('push', 'mov'):
            continue
        immediate = ins.operands.split(',')[-1].strip()
        if not re.fullmatch(r'0x[0-9a-fA-F]+', immediate):
            continue
        va = int(immediate, 16)
        path = ascii_at(pe, va)
        if path and SOURCE_PATH.fullmatch(path):
            result.append({'instruction_va': hx(ins.va), 'string_va': hx(va), 'path': path,
                           'evidence': 'decoded_source_literal_in_same_candidate'})
    return result


def range_body(ranges: list[tuple], instructions: dict[int, Insn]) -> list[Insn]:
    body = []
    for lo, hi in ranges:
        pos = lo
        while pos < hi:
            ins = instructions.get(pos)
            if ins is None or ins.end > hi or ins.mnemonic in ('(bad)', '.byte'):
                raise CategoryError(f'candidate range has a decoding gap at {pos:#x}')
            body.append(ins); pos = ins.end
    return body


def source_basename(path: str) -> str:
    return path.replace('\\', '/').rsplit('/', 1)[-1]


def build_categories(pe: PEImage, config: dict, provenance: dict,
                     instructions: dict[int, Insn]) -> dict:
    digest = validate_inputs(pe, config, provenance)
    select = number(config['category_select_va'])
    candidates = candidate_ranges(provenance, instructions)
    by_end = {ins.end: ins for ins in instructions.values()}
    incoming = branch_entries(instructions) | {entry for entry, _ in candidates}
    class_uses = primary_class_uses(pe)
    sites, contexts = [], {}
    for call in sorted(instructions.values(), key=lambda i: i.va):
        if direct_call_target(call) != select:
            continue
        # Verification is independent of any cached allocation category labels.
        if read_va(pe, call.va, len(call.raw)) != call.raw:
            raise CategoryError('selector call bytes differ from input')
        row = {'call_va': hx(call.va), 'selector_va': hx(select),
               **recover_argument(call, by_end, incoming, pe)}
        containers = [entry for entry, ranges in candidates if any(lo <= call.va and call.end <= hi for lo, hi in ranges)]
        row['containing_candidates'] = [hx(x) for x in containers]
        row['container_status'] = 'unique_candidate' if len(containers) == 1 else 'ambiguous_candidates' if containers else 'unmapped'
        sites.append(row)
        for entry, ranges in candidates:
            if entry not in containers or entry in contexts:
                continue
            body = range_body(ranges, instructions)
            contexts[entry] = {'entry_va': hx(entry), 'decoded_ranges': [[hx(a), hx(b)] for a, b in ranges],
                               'source_references': decode_sources(pe, body), 'class_uses': class_uses.get(entry, []),
                               'category_calls': [], 'category_labels': [], 'original_translation_unit': None,
                               'exclusive_category': None, 'evidence': 'category_call_in_candidate_reachable_ranges'}
    for site in sites:
        for entry in site['containing_candidates']:
            contexts[number(entry)]['category_calls'].append(site['call_va'])
    by_call = {s['call_va']: s for s in sites}
    source_edges, class_edges = defaultdict(set), defaultdict(set)
    for context in contexts.values():
        labels = sorted({by_call[c]['label'] for c in context['category_calls'] if by_call[c]['label'] is not None})
        context['category_labels'] = labels
        context['review_lane'] = 'mixed_category_orchestrator' if len(labels) > 1 else 'single_category_anchor' if labels else 'unresolved_category_argument'
        # Only a unique container supplies co-occurrence edges. Other membership
        # is kept for review without assigning the evidence to every candidate.
        for call in context['category_calls']:
            site = by_call[call]
            if site['label'] is None or site['container_status'] != 'unique_candidate':
                continue
            for path in {s['path'] for s in context['source_references']}:
                source_edges[(site['label'], path)].add((context['entry_va'], call))
            for cls in {s['class'] for s in context['class_uses']}:
                class_edges[(site['label'], cls)].add((context['entry_va'], call))
    def edges(mapping, key_name):
        return [{'category': label, key_name: key, 'evidence': 'same_candidate_cooccurrence',
                 'candidate_entries': sorted({e for e, _ in pairs}),
                 'call_sites': sorted({c for _, c in pairs}), 'exclusive_ownership': False}
                for (label, key), pairs in sorted(mapping.items())]
    source_rows, class_rows = edges(source_edges, 'source_path'), edges(class_edges, 'class')
    labels = sorted({s['label'] for s in sites if s['label'] is not None})
    categories = []
    for label in labels:
        own = [s for s in sites if s['label'] == label]
        categories.append({'label': label, 'call_sites': [s['call_va'] for s in own],
                           'candidate_entries': sorted({e for s in own for e in s['containing_candidates']}),
                           'source_edges': [e for e in source_rows if e['category'] == label],
                           'class_edges': [e for e in class_rows if e['category'] == label]})
    queue = sorted(contexts.values(), key=lambda c: (c['review_lane'] != 'single_category_anchor',
                                                    not bool(c['source_references']), c['entry_va']))
    return {'schema_version': 1, 'input_sha256': digest, 'selector_va': hx(select), 'caveats': CAVEATS,
            'summary': {'selector_call_sites': len(sites), 'literal_labeled_sites': sum(s['label'] is not None for s in sites),
                        'unresolved_argument_sites': sum(s['label'] is None for s in sites),
                        'categories': len(categories), 'containing_candidates': len(contexts),
                        'mixed_category_candidates': sum(len(c['category_labels']) > 1 for c in contexts.values()),
                        'sites_by_container_status': dict(sorted(Counter(s['container_status'] for s in sites).items())),
                        'source_category_edges': len(source_rows), 'distinct_source_paths': len({e['source_path'] for e in source_rows}),
                        'class_category_edges': len(class_rows), 'distinct_classes': len({e['class'] for e in class_rows}),
                        'original_translation_units_assigned': 0, 'callee_category_propagations': 0,
                        'new_byte_matches_claimed': 0},
            'categories': categories, 'call_sites': sites, 'candidates': sorted(contexts.values(), key=lambda c: c['entry_va']),
            'source_category_edges': source_rows, 'class_category_edges': class_rows, 'review_queue': queue}


def render_report(result: dict) -> str:
    s = result['summary']
    lines = ['# Allocation categories and source clues', '',
             f"Input SHA-256: `{result['input_sha256']}`", '',
             '**Categories describe allocation context, not exclusive source ownership.**', '',
             f"{s['literal_labeled_sites']} of {s['selector_call_sites']} selector sites have literal labels; "
             f"{s['unresolved_argument_sites']} remain unresolved. There are {s['categories']} observed labels, "
             f"{s['containing_candidates']} containing candidates and {s['mixed_category_candidates']} mixed-category candidates.", '',
             '| Category | Labeled sites | Source paths observed in the same candidates | Non-inherited primary class uses |',
             '|---|---:|---|---|']
    for c in result['categories']:
        sources = ', '.join(sorted({source_basename(e['source_path']) for e in c['source_edges']})) or 'Not established'
        classes = ', '.join(sorted({e['class'] for e in c['class_edges']})) or 'Not established'
        lines.append(f"| {c['label']} | {len(c['call_sites'])} | {sources} | {classes} |")
    lines += ['', 'The source column means a decoded source-path reference and a category selection occur within the same candidate ranges. '
              'It does not mean the file implements only that category. Class evidence is likewise method-specific, not class-wide.', '',
              '## Mixed-category routines', '']
    for c in result['candidates']:
        if len(c['category_labels']) > 1:
            paths = ', '.join(sorted({source_basename(e['path']) for e in c['source_references']})) or 'No direct source reference'
            lines.append(f"- `{c['entry_va']}` ({paths}): {', '.join(c['category_labels'])}.")
    lines += ['', 'These are useful orchestration/loading investigation targets. Splitting a candidate into one file per category would be unsupported.', '',
              '## Unresolved arguments', '']
    for site in result['call_sites']:
        if site['label'] is None:
            lines.append(f"- `{site['call_va']}`: {site['reason']}.")
    lines += ['', 'The recognizer skips register MOVs and DWORD stack stores above the argument at [esp]. '
              'It rejects writes through unknown pointers, stack changes, nonliteral pushes, branch entrances and other unsupported instructions. '
              'It does not guess categories from nearby strings or from the enclosing source name.', '',
              '## Review queue', '',
              'review_queue.json contains the selected candidates, exact call sites, source-string reference addresses and RTTI slot uses. '
              'Mixed-category routines have a separate review lane. Ambiguous containers do not produce source/class co-occurrence edges.', '',
              '## Scope and limitations', '', *['- '+c for c in CAVEATS], '',
              'No source files were moved, no original translation-unit ownership was assigned, and no additional byte matches are claimed.', '']
    return '\n'.join(lines)
