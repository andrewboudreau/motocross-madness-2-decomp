#!/usr/bin/env python3
"""Inspect reviewed allocation layers; optionally compile and strictly match probes."""
from __future__ import annotations
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mcm2tool.pe import PEImage
from mcm2tool.coff import CoffObject
from mcm2tool.allocation import analyze, decode_image, read_va
from mcm2tool.resolved_match import match_object


def report(result):
    s = result['summary']
    lines = ['# Allocation subsystem — reviewed evidence', '',
             f"Input SHA-256: `{result['input_sha256']}`", '',
             'The labels in this report are reconstructed behavior names, not original symbols or proven library identities.', '',
             '## Application-side bookkeeping', '',
             'The shared deleting-destructor target at `0x004a30c0` is a tracked-deallocation wrapper. It queries allocation size, subtracts from the active category or fallback counter, then calls the lower-level free-like entry at `0x00537929`.', '',
             'Two other reviewed entry points (`0x004a2e60`, `0x004a3060`) have the same body once their direct call destinations are resolved and checked. No caller target, pointer constant, or non-relocation byte is ignored.', '',
             f"Validated same-body pairs: **{s['same_body_pairs_validated']}**. This does not prove original free/delete/delete[] symbol spellings.", '',
             f"The category-selection entry has **{s['observed_category_select_calls']}** decoded direct-call sites; **{s['category_calls_with_literal_arguments']}** have a locally recoverable literal argument.", '',
             'Literal category candidates: ' + ', '.join('`'+x+'`' for x in s['distinct_literal_categories']) + '.', '',
             'The tracker initializes category zero from the literal `Unclaimed`, uses 128-byte label records, and keeps separate dword arrays at +0x08 and +0x0c. Its active category index is +0x10. These names and the provisional structure name are ours, not recovered RTTI names.', '',
             '## Reviewed functions', '', '| VA | Size | Behavioral label | Layer hypothesis |', '|---|---:|---|---|']
    lines += [f"| `{r['va']}` | {r['size']} | {r['label']} | {r['layer_hypothesis']} |" for r in result['functions']]
    lines += ['', '## External API boundary', '',
              'The size-like and free-like entries both consult a small-block region lookup and use indexed locks. Their fallback branches call named Windows imports. The allocator also has a small-block path before a heap allocation fallback.', '',
              '| Reviewed entry | Observed direct external API | Local IAT slot | Call site |', '|---|---|---|---|']
    for r in result['functions']:
        for a in r['imported_calls']:
            lines.append(f"| `{r['va']}` | `{a['module']}!{a['name'] or a['ordinal']}` | `{a['iat_va']}` | `{a['site_va']}` |")
    lines += ['', 'The may-reach paths in allocation.json include initialization, retry, and locking branches. They do not assert every API is called on every allocation. Unreviewed callees remain outside the analyzed graph.', '',
              'Runtime candidates are consistent with malloc/free/_msize-style roles. No authentic VC6 .lib signature corpus was available; **zero original library identities are established**.', '',
              '## Candidate source validation', '']
    if result.get('probe_matches'):
        lines += ['| Target | Candidate symbol | Retail / candidate size | Strict exact |', '|---|---|---:|---|']
        for m in result['probe_matches']:
            lines.append(f"| `{m['target_va']}` | `{m['symbol']}` | {m['retail_size']} / {m['candidate_size']} | {m['strict_exact']} |")
        lines += ['', 'Every supported COFF relocation is applied at the target VA using the explicit reviewed binding map. Unsupported or unresolved relocations fail closed. No relocation masking is used and size differences count as mismatches.', '',
                  'The exact category-index setter has no relocations. All non-exact wrappers remain compiler-calibration candidates, not new matches. Native model tests exercise candidate behavior only; the original game is never executed.', '']
    else:
        lines += ['Probes were not compiled in this run. Pass `--compile-probe` or `--probe-object`.', '']
    lines += ['## Semantics deliberately preserved', '',
              '- Null deallocation skips the size query but still calls the lower-level free routine.',
              '- Category selection is global/current, not inferred from a per-allocation owner tag.',
              '- The category pointer/index are reloaded after the size query in the category branch.',
              '- `0x004a2e20` charges requested bytes only on successful allocation; `0x004a3010` charges before attempting allocation, including a failed attempt.',
              '- The calloc-like wrapper charges count*size with 32-bit wraparound, on success.',
              '- Deallocation subtracts the backend-reported size, which need not equal the originally charged request. Do not "repair" this behavior during reconstruction.', '',
              '## Limitations and next gate', '',
              'These are 21 manually reviewed ranges tied to an exact input hash. Linear disassembly call counts are discovery evidence, not an exhaustive call graph. Callbacks, indirect targets and remaining small-block internals are not fully traced. No original class name or translation-unit ownership has been established for this accounting subsystem.', '',
              'Next: supply privately held VC6 libraries, compare objects with relocation/callee verification, and corroborate runtime identities. Keep the accounting wrappers in the reconstruction scope even after the runtime beneath them is identified.', '',
              '## Primary API references', '',
              '- https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapsize',
              '- https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapfree',
              '- https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc',
              '- https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/msize', '',
              'These API contracts support interpretation of the named imports. They do not prove the historical CRT object version.', '']
    return '\n'.join(lines)


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--exe', type=Path, default=ROOT / 'work/game/mcm2.exe')
    ap.add_argument('--config', type=Path, default=ROOT / 'config/allocation_targets.json')
    ap.add_argument('--out', type=Path, default=ROOT / 'analysis/allocation')
    ap.add_argument('--objdump', default='objdump')
    group = ap.add_mutually_exclusive_group()
    group.add_argument('--compile-probe', action='store_true', help='use clang-cl i686 MSVC ABI; not historical VC6')
    group.add_argument('--probe-object', type=Path, help='use a previously compiled i386 COFF probe')
    args = ap.parse_args()
    config = json.loads(args.config.read_text(encoding='utf-8'))
    pe = PEImage(args.exe)
    digest = hashlib.sha256(pe.data).hexdigest()
    if pe.machine != 0x14c or digest != config['input_sha256']:
        raise ValueError('Input does not match the reviewed MCM2 build; hard-coded ranges will not be applied.')
    instructions, decoder = decode_image(pe, args.objdump)
    result = analyze(pe, config, instructions)
    result.update({'schema_version': 1, 'input_sha256': digest, 'decoder': decoder,
                   'probe_matches': [], 'compiler': None,
                   'validation_scope': 'static target inspection plus optional candidate compilation; target never executed'})
    with tempfile.TemporaryDirectory() as temp:
        obj_path = args.probe_object
        if args.compile_probe:
            compiler = shutil.which('clang-cl')
            if not compiler:
                raise ValueError('clang-cl is required for --compile-probe')
            version = subprocess.run([compiler, '--version'], check=True, capture_output=True, text=True, timeout=10).stdout
            obj_path = Path(temp) / 'allocation.obj'
            flags = ['--target=i686-pc-windows-msvc', '/nologo', '/c', '/O2', '/GR', '/EHsc']
            cmd = [compiler, *flags, str(ROOT / config['probe']['source']), '/Fo'+str(obj_path)]
            subprocess.run(cmd, check=True, capture_output=True, text=True, timeout=60)
            result['compiler'] = {'version': version.strip(), 'flags': flags, 'historical_vc6': False}
        if obj_path:
            obj = CoffObject(obj_path)
            bindings = {k: int(v, 16) for k, v in config['probe']['bindings'].items()}
            for target in config['probe']['targets']:
                va = int(target['va'], 16)
                result['probe_matches'].append(match_object(obj, target['symbol'], va,
                                                           read_va(pe, va, target['size']), bindings))
    files = ['mcm2tool/allocation.py', 'mcm2tool/resolved_match.py', 'mcm2tool/pe.py',
             'mcm2tool/msvc.py', 'mcm2tool/rtti.py', 'mcm2tool/coff.py',
             'tools/analyze_allocation.py', config['probe']['source']]
    result['tool_sha256'] = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in files}
    result['config_sha256'] = hashlib.sha256(args.config.read_bytes()).hexdigest()
    args.out.mkdir(parents=True, exist_ok=True)
    (args.out / 'allocation.json').write_text(json.dumps(result, indent=2, sort_keys=True)+'\n', encoding='utf-8')
    (args.out / 'REPORT.md').write_text(report(result), encoding='utf-8')
    print(json.dumps(result['summary'], indent=2))
    if result['probe_matches']:
        print(f"Strict source matches: {sum(m['strict_exact'] for m in result['probe_matches'])}/{len(result['probe_matches'])}; remaining candidates are calibration data")
    print(f"Report: {args.out / 'REPORT.md'}")


if __name__ == '__main__':
    try:
        main()
    except (ValueError, OSError, subprocess.SubprocessError) as exc:
        print(f'allocation: {exc}', file=sys.stderr)
        if isinstance(exc, subprocess.CalledProcessError) and exc.stderr:
            print(exc.stderr, file=sys.stderr)
        raise SystemExit(2)
