#!/usr/bin/env python3
"""Build or query an evidence-preserving MCM2 provenance snapshot."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys

# Allow direct invocation from any working directory, without PYTHONPATH.
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mcm2tool.pe import PEImage
from mcm2tool.provenance import (CAVEATS, build_map, component_inventory, decode,
                                 directory, import_records, module_family)

EXPECTED_SHA256 = '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874'


def write_json(path: Path, value):
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + '\n', encoding='utf-8')


def report(snapshot: dict, components: list[dict], imports: list[dict]) -> str:
    s = snapshot['summary']
    lines = ['# MCM2 provenance map — first static pass', '',
             f"Input: `{snapshot['input']['filename']}`", '',
             f"SHA-256: `{snapshot['input']['sha256']}`", '',
             '**This is an evidence inventory, not a recovered original C++ project or a decomp completion percentage.**', '',
             '## External implementation boundaries', '',
             '| Module | Import slots | Package availability | Family / evidence |',
             '|---|---:|---|---|']
    counts = {}
    for imp in imports:
        key = imp['module'].lower()
        counts[key] = counts.get(key, 0) + 1
    for c in components:
        if c['direct_import']:
            lines.append(f"| `{c['module']}` | {counts[c['module']]} | {c['availability']} | {c['family']} |")
    lines += ['', 'IAT addresses in imports.json belong to this executable. They are not runtime addresses inside the DLLs.', '',
              '### Blade correction', '']
    for c in components:
        if c['module'] != 'blade.dll':
            continue
        for e in c.get('identity_evidence', []):
            lines.append(f"`blade.dll` contains the identity string **{e['text']}** at file offset `{e['file_offset']}`.")
        used = [i['name'] or f"ordinal:{i['ordinal']}" for i in imports if i['module'].lower() == 'blade.dll']
        lines += [f"MCM2 imports: {', '.join('`'+n+'`' for n in used)}.", '',
                  'Treat Blade as a separate rasterizer component with Microsoft self-identification, not as a proven Rainbow engine DLL.', '']
    rm = [i['name'] for i in imports if i['module'].lower() == 'd3drm.dll']
    lines += [f"Named d3drm imports: `{', '.join(n for n in rm if n)}`. A vector-helper import alone does not establish use of the Retained Mode renderer.", '',
              '### Other packaged DLLs', '']
    for c in components:
        if c['packaged_path'] and not c['direct_import']:
            lines.append(f"- `{c['packaged_path']}`: packaged, but not in the EXE's normal import table. It may be dynamically loaded or installer/resource support; no role is assumed.")
    lines += ['', '## Inside the executable', '',
              f"- {s['candidate_entries']} candidate entry addresses, seeded from RTTI vtables, direct calls, compiler artifacts, IAT jump stubs, and the PE entry point.",
              f"- {s['rtti_class_records']} parsed RTTI class records; {s['vtables']} concrete vtables.",
              f"- {s['source_paths']} embedded source/header paths; {s['source_paths_referenced_by_candidates']} referenced by instructions reached from the current candidates.", '',
              '| Ownership evidence | Candidate entries |', '|---|---:|']
    lines += [f'| {k} | {v} |' for k, v in s['owners'].items()]
    lines += ['', 'These are evidence labels, not mutually complete original-library allocations. Class/filename hints are not confirmed authorship.', '',
              '| Code role (independent of ownership) | Candidate entries |', '|---|---:|']
    lines += [f'| {k} | {v} |' for k, v in s['roles'].items()]
    lines += ['', 'Canonical deleting destructors and adjustor thunks should be reproduced through C++ declarations and the compiler. They are not discarded from the matching scope.', '',
              '### Runtime candidate', '']
    for row in snapshot['functions']:
        if 'runtime_hypothesis' in row:
            h = row['runtime_hypothesis']
            lines.append(f"`{row['entry_va']}` is the common delete target of {h['wrapper_callers']} recognized wrappers. The operator-delete role is a strong hypothesis; Microsoft CRT identity remains **unverified** until library/signature comparison.")
    lines += ['', '## Coverage accounting', '',
              f"Executable-section file-backed virtual bytes: **{s['executable_section_bytes']}**.",
              f"Union of instructions reached from candidate entries: **{s['candidate_reachable_instruction_bytes']}** bytes.",
              f"Bytes not reached from the current candidates: **{s['bytes_not_reached_from_current_candidates']}**.", '',
              'The remainder may include undiscovered code, alignment, inline data, switch tables or decoding gaps. The reached set can also contain false-positive code candidates. Neither number is an authorship or decomp-progress percentage.', '',
              '**Exact original translation-unit ranges established: 0.** Source paths are anchors and lower-bound filename evidence, not a complete project file list.', '',
              '## Reproducibility and limitations', '',
              f"Decoder: `{snapshot['decoder']}`. All decoded instruction bytes were checked against the supplied image.", '',
              'The report is regenerated directly from the binary; it does not trust cached smoke-test percentages, dossiers, or old analysis files.', '']
    lines += [f'- {c}' for c in CAVEATS]
    lines += ['', '## Next evidence to collect', '',
              '1. Compare runtime candidates with privately supplied VC6 library objects, checking relocations and callees rather than just masking them.',
              '2. Recover additional function/CFG boundaries with a disassembler project and switch-table analysis.',
              '3. Resolve COM interface identities and dynamic DLL loads; preserve unresolved dispatches until evidence exists.',
              '4. Promote source ownership only where independent evidence agrees. Preserve shared code and multiple possible source origins.', '',
              '## Format references', '',
              '- Microsoft PE/COFF: https://learn.microsoft.com/en-us/windows/win32/debug/pe-format',
              '- COM interface layout: https://learn.microsoft.com/en-us/windows/win32/com/interface-pointers-and-interfaces',
              '- Link inputs: https://learn.microsoft.com/en-us/cpp/build/reference/link-input-files', '']
    return '\n'.join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--exe', type=Path, default=ROOT / 'work/game/mcm2.exe')
    ap.add_argument('--game-dir', type=Path, help='defaults to the executable parent; inspected, never executed')
    ap.add_argument('--out', type=Path, default=ROOT / 'analysis/provenance')
    ap.add_argument('--objdump', default='objdump', help='GNU objdump executable')
    ap.add_argument('--allow-unknown-build', action='store_true')
    ap.add_argument('--address', type=lambda value: int(value, 0), help='query an existing snapshot instead of rebuilding')
    args = ap.parse_args()
    if args.address is not None:
        snap = json.loads((args.out / 'provenance.json').read_text(encoding='utf-8'))
        exact = [r for r in snap['functions'] if int(r['entry_va'], 16) == args.address]
        containing = [r for r in snap['functions'] if any(int(a, 16) <= args.address < int(b, 16) for a, b in r['decoded_ranges'])]
        imports = json.loads((args.out / 'imports.json').read_text(encoding='utf-8'))
        slots = [r for r in imports if int(r['iat_va'], 16) <= args.address < int(r['iat_va'], 16) + 4]
        print(json.dumps({'input': snap['input'], 'import_slots': slots, 'address': f'0x{args.address:08x}',
                          'exact_entry_matches': exact, 'containing_candidates': containing,
                          'note': 'No match means unclassified by this snapshot, not necessarily data.'}, indent=2))
        return
    digest = hashlib.sha256(args.exe.read_bytes()).hexdigest()
    if digest != EXPECTED_SHA256 and not args.allow_unknown_build:
        raise ValueError('Unsupported input SHA-256. Use the supplied MCM2 build or explicitly pass --allow-unknown-build.')
    pe = PEImage(args.exe)
    if pe.machine != 0x14c:
        raise ValueError('Only PE32 i386 images are supported')
    if not shutil.which(args.objdump):
        raise ValueError('GNU objdump is required (binutils); it analyzes but never executes the input')
    imports = import_records(pe)
    components = component_inventory(args.game_dir or args.exe.parent, imports)
    families = {r['module']: r['family'] for r in components}
    for imp in imports:
        imp['family'] = families.get(imp['module'].lower(), module_family(imp['module']))
    instructions, decoder = decode(pe, args.objdump)
    snapshot = build_map(pe, instructions, imports)
    known_modules = {r['module'].lower() for r in imports}
    dll_strings = sorted({m.group().decode('ascii') for m in re.finditer(rb'[A-Za-z0-9_.-]+\.dll(?=\x00)', pe.data, re.I)})
    snapshot.update({'schema_version': 1, 'input': {'filename': args.exe.name, 'sha256': digest,
                     'known_build': digest == EXPECTED_SHA256, 'preferred_image_base': f'0x{pe.image_base:08x}'},
                     'decoder': decoder, 'caveats': CAVEATS,
                     'tool_hashes': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                                    for p in [ROOT / 'mcm2tool/provenance.py', Path(__file__).resolve(),
                                              ROOT / 'mcm2tool/pe.py', ROOT / 'mcm2tool/rtti.py', ROOT / 'mcm2tool/msvc.py']},
                     'dynamic_linking': {'delay_import_directory_present': bool(directory(pe, 13)[0]),
                                         'loader_api_imports': sorted({r['name'] for r in imports if r['name'] and
                                                                     (r['name'].startswith('LoadLibrary') or r['name'] == 'GetProcAddress')}),
                                         'additional_dll_strings_not_proven_loaded': [s for s in dll_strings if s.lower() not in known_modules]}})
    snapshot['summary']['direct_import_slots'] = len(imports)
    snapshot['summary']['direct_import_modules'] = len(known_modules)
    args.out.mkdir(parents=True, exist_ok=True)
    write_json(args.out / 'provenance.json', snapshot)
    write_json(args.out / 'imports.json', imports)
    write_json(args.out / 'components.json', components)
    write_json(args.out / 'source_units.json', snapshot['source_units'])
    (args.out / 'REPORT.md').write_text(report(snapshot, components, imports), encoding='utf-8')
    print(json.dumps(snapshot['summary'], indent=2))
    print(f'Report: {args.out / "REPORT.md"}')


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, KeyError, RuntimeError, struct.error, subprocess.SubprocessError) as exc:
        print(f'provenance: {exc}', file=sys.stderr)
        raise SystemExit(2)
