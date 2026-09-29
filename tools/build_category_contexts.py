#!/usr/bin/env python3
"""Build hash-bound category navigation, or query an existing local snapshot."""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path, PureWindowsPath
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mcm2tool.pe import PEImage
from mcm2tool.allocation import decode_image
from mcm2tool.category_contexts import analyze_categories, validate_build


def pilot_queue(snapshot):
    labels = {}
    for event in snapshot['events']:
        if event['label'] and event['container_status'] == 'unique_candidate':
            labels.setdefault(event['containing_candidates'][0], set()).add(event['label'])
    rows = []
    for candidate in snapshot['candidates']:
        va = candidate['entry_va']
        if va not in labels:
            continue
        sources = sorted({x['path'] for x in candidate['source_references']})
        classes = sorted({x['class'] for x in candidate['vtable_uses'] + candidate['vptr_store_clues']})
        # Scheduling priorities only; no inferred filename is promoted to fact.
        agrees = any(c.casefold() == PureWindowsPath(p).stem.casefold() == label.casefold()
                     for c in classes for p in sources for label in labels[va])
        tier = 0 if agrees else (1 if sources and classes else (2 if sources else (3 if classes else 4)))
        rows.append({'entry_va': va, 'categories': sorted(labels[va]), 'source_paths': sources,
                     'class_clues': classes, 'navigation_tier': tier, 'owner_assignment': None,
                     'next_action': 'Review CFG boundaries and category save/restore; corroborate class/source before reconstructing.'})
    return sorted(rows, key=lambda r: (r['navigation_tier'], r['entry_va']))


def report(snapshot):
    s = snapshot['summary']
    lines = ['# Memory categories: source-navigation atlas', '',
             f"Input SHA-256: `{snapshot['input_sha256']}`", '',
             '**Category context is not source ownership. No files were moved or renamed.**', '',
             f"Resolved {s['literal_category_calls']} of {s['category_select_calls']} category-selection call arguments into {s['distinct_categories']} literal categories.",
             f"The graph links {s['source_paths_with_links']} source/header filenames and {s['one_hop_context_candidates']} one-hop candidates; {s['multi_category_context_candidates']} candidates appear in multiple category contexts.", '',
             '| Category | Literal call sites | Containing candidates | One-hop candidates | Source/header links |',
             '|---|---:|---:|---:|---|']
    for row in snapshot['categories']:
        names = sorted({PureWindowsPath(x['path']).name for x in row['source_links']})
        lines.append(f"| {row['category']} | {len(row['anchor_calls'])} | {len(row['containing_candidates'])} | {len(row['one_hop_candidates'])} | {', '.join(names) or 'Unresolved'} |")
    lines += ['', 'The last column combines two explicitly separate evidence classes: a path referenced in the same CFG candidate as the selector, or a path referenced in a one-hop callee before a local category boundary. It is not a directory reorganization plan.', '',
              '## Strong starting points', '']
    for row in snapshot['pilot_queue']:
        if row['navigation_tier'] == 0:
            lines.append(f"- `{row['entry_va']}`: labels {', '.join(row['categories'])}; source anchors {', '.join(PureWindowsPath(p).name for p in row['source_paths'])}; class clues {', '.join(row['class_clues'])}.")
    lines += ['', 'These are review leads, not recovered function names or exact original TU assignments. Constructor/destructor vptr stores can also initialize members. A containing candidate may select many categories.', '',
              '## Unresolved selection arguments', '']
    for event in snapshot['events']:
        if event['label'] is None:
            lines.append(f"- `{event['call_va']}`: {event['reason']}; not filled from nearby text.")
    lines += ['', '## Shared context example', '']
    shared = sorted(snapshot['call_contexts'], key=lambda x: (-len(x['observed_contexts']), x['entry_va']))
    for row in shared[:5]:
        lines.append(f"- `{row['entry_va']}`: {', '.join(row['observed_contexts'])}. Owner remains unassigned.")
    lines += ['', '## Evidence safeguards', ''] + ['- '+x for x in snapshot['limitations']]
    lines += ['', f"Decoder: `{snapshot['decoder']}`. The input and reviewed select/restore bodies are hash-checked. The tool hashes are retained in context_map.json.", '',
              '## Technical references', '',
              '- Microsoft x86 thiscall: https://learn.microsoft.com/en-us/cpp/cpp/thiscall',
              '- Predefined macros, including __FILE__: https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros', '']
    return '\n'.join(lines)


def category_packet(snapshot, category):
    rows = [e for e in snapshot['events'] if e['label'] == category['category']]
    vas = set(category['containing_candidates'] + category['one_hop_candidates'])
    return {'input_sha256': snapshot['input_sha256'], 'limitations': snapshot['limitations'],
            'category': category, 'events': rows,
            'candidates': [c for c in snapshot['candidates'] if c['entry_va'] in vas],
            'pilot_queue': [r for r in snapshot['pilot_queue'] if category['category'] in r['categories']]}


def query_snapshot(snapshot, category=None, source=None, address=None):
    if category is not None:
        found = [c for c in snapshot['categories'] if c['category'].casefold() == category.casefold()]
        return [category_packet(snapshot, c) for c in found]
    if source is not None:
        return [r for r in snapshot['sources'] if source.casefold() in (r['path'].casefold(), r['filename'].casefold())]
    return [{'candidate': c, 'call_contexts': [x for x in snapshot['call_contexts'] if x['entry_va'] == c['entry_va']],
             'category_events': [e for e in snapshot['events'] if c['entry_va'] in e['containing_candidates']]}
            for c in snapshot['candidates'] if int(c['entry_va'], 16) == address]


def write_snapshot(snapshot, output):
    output.mkdir(parents=True, exist_ok=True)
    def put(path, value):
        path.write_text(json.dumps(value, indent=2, sort_keys=True)+'\n', encoding='utf-8')
    put(output/'context_map.json', snapshot)
    put(output/'source_index.json', snapshot['sources'])
    put(output/'work_queue.json', snapshot['pilot_queue'])
    (output/'REPORT.md').write_text(report(snapshot), encoding='utf-8')
    packets = output/'packets'; packets.mkdir(exist_ok=True)
    index = []
    for i, category in enumerate(snapshot['categories']):
        filename = f"{i:02d}_"+re.sub(r'[^A-Za-z0-9_-]', '_', category['category'])+'.json'
        put(packets/filename, category_packet(snapshot, category))
        index.append({'category': category['category'], 'path': 'packets/'+filename})
    put(output/'packet_index.json', index)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--exe', type=Path, default=ROOT/'work/game/mcm2.exe')
    ap.add_argument('--config', type=Path, default=ROOT/'config/allocation_targets.json')
    ap.add_argument('--out', type=Path, default=ROOT/'analysis/category_contexts')
    ap.add_argument('--objdump', default='objdump')
    q = ap.add_mutually_exclusive_group()
    q.add_argument('--category'); q.add_argument('--source')
    q.add_argument('--address', type=lambda value: int(value, 0))
    args = ap.parse_args()
    if args.category is not None or args.source is not None or args.address is not None:
        snapshot = json.loads((args.out/'context_map.json').read_text(encoding='utf-8'))
        matches = query_snapshot(snapshot, args.category, args.source, args.address)
        print(json.dumps({'input_sha256': snapshot['input_sha256'], 'matches': matches,
                          'note': 'Query addresses are candidate entry VAs in this snapshot; absence is not an ownership classification.'}, indent=2))
        return 0 if matches else 1
    config = json.loads(args.config.read_text(encoding='utf-8'))
    pe = PEImage(args.exe)
    validate_build(pe, config)  # Reject wrong builds before running the decoder.
    instructions, decoder = decode_image(pe, args.objdump)
    snapshot = analyze_categories(pe, config, instructions, decoder)
    snapshot['pilot_queue'] = pilot_queue(snapshot)
    snapshot['config_sha256'] = hashlib.sha256(args.config.read_bytes()).hexdigest()
    files = ['mcm2tool/category_contexts.py', 'tools/build_category_contexts.py', 'mcm2tool/allocation.py',
             'mcm2tool/pe.py', 'mcm2tool/rtti.py', 'mcm2tool/msvc.py']
    snapshot['tool_sha256'] = {n: hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in files}
    write_snapshot(snapshot, args.out)
    print(json.dumps(snapshot['summary'], indent=2))
    print(f"Pilot queue: {len(snapshot['pilot_queue'])}; report: {args.out/'REPORT.md'}")
    return 0


if __name__ == '__main__':
    try:
        raise SystemExit(main())
    except (ValueError, OSError, KeyError, subprocess.SubprocessError) as exc:
        print(f'categories: {exc}', file=sys.stderr)
        raise SystemExit(2)
