#!/usr/bin/env python3
"""Join allocation category call sites with hash-bound source/class evidence."""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mcm2tool.allocation import decode_image
from mcm2tool.categories import build_categories, render_report, validate_inputs
from mcm2tool.pe import PEImage


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--exe', type=Path, default=ROOT/'work/game/mcm2.exe')
    ap.add_argument('--provenance', type=Path, default=ROOT/'analysis/provenance/provenance.json')
    ap.add_argument('--config', type=Path, default=ROOT/'config/allocation_targets.json')
    ap.add_argument('--out', type=Path, default=ROOT/'analysis/categories')
    ap.add_argument('--objdump', default='objdump')
    ap.add_argument('--query', help='query an existing map by exact category, filename, class or candidate/call VA')
    a = ap.parse_args()
    if a.query is not None:
        data = json.loads((a.out/'category_map.json').read_text(encoding='utf-8'))
        query = a.query.casefold()
        from mcm2tool.categories import source_basename
        found = [c for c in data['candidates'] if query in {
            c['entry_va'].casefold(), *[x.casefold() for x in c['category_calls']],
            *[x.casefold() for x in c['category_labels']],
            *[x['class'].casefold() for x in c['class_uses']],
            *[source_basename(x['path']).casefold() for x in c['source_references']],
            *[x['path'].casefold() for x in c['source_references']]}]
        if query.startswith('0x'):
            canonical = f'0x{int(query, 16):08x}'
            found = [c for c in data['candidates'] if canonical == c['entry_va'] or canonical in c['category_calls']]
        print(json.dumps({'input_sha256': data['input_sha256'], 'query': a.query, 'candidates': found,
                          'note': 'This queries category-anchor candidates, not every game function.'}, indent=2))
        return
    pe = PEImage(a.exe)
    config = json.loads(a.config.read_text(encoding='utf-8'))
    snapshot = json.loads(a.provenance.read_text(encoding='utf-8'))
    validate_inputs(pe, config, snapshot)  # Reject wrong image before disassembly.
    instructions, decoder = decode_image(pe, a.objdump)
    data = build_categories(pe, config, snapshot, instructions)
    data['decoder'] = decoder
    data['provenance_snapshot_sha256'] = hashlib.sha256(a.provenance.read_bytes()).hexdigest()
    data['config_sha256'] = hashlib.sha256(a.config.read_bytes()).hexdigest()
    paths = ['mcm2tool/categories.py', 'mcm2tool/allocation.py', 'mcm2tool/pe.py',
             'mcm2tool/rtti.py', 'mcm2tool/msvc.py', 'tools/build_categories.py']
    data['tool_sha256'] = {p: hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in paths}
    a.out.mkdir(parents=True, exist_ok=True)
    def write(name, value):
        (a.out/name).write_text(json.dumps(value, indent=2, sort_keys=True)+'\n', encoding='utf-8')
    write('category_map.json', data)
    write('review_queue.json', {'schema_version': 1, 'input_sha256': data['input_sha256'],
                              'candidates': data['review_queue'], 'caveats': data['caveats']})
    (a.out/'REPORT.md').write_text(render_report(data), encoding='utf-8')
    print(json.dumps(data['summary'], indent=2))
    print(f'Report: {a.out / "REPORT.md"}')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, KeyError, TypeError, struct.error, subprocess.SubprocessError) as exc:
        print(f'categories: {exc}', file=sys.stderr)
        raise SystemExit(2)
