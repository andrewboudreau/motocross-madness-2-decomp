#!/usr/bin/env python3
"""Verify EcoSystem's reviewed body and strictly compare ordinary C++ probes."""
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
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mcm2tool.pe import PEImage
from mcm2tool.allocation import ascii_at, decode_image, direct_call_target, hx, read_va, reviewed_body
from mcm2tool.category_lifetimes import normal_exit_paths
from mcm2tool.coff import CoffObject
from mcm2tool.resolved_match import match_object
from mcm2tool.rtti import parse_rtti


def digest(data):
    return hashlib.sha256(data).hexdigest()


def validate_range(pe, spec):
    va, size = int(spec['va'], 16), spec['size']
    if type(size) is not int or not 0 < size <= 65536:
        raise ValueError('invalid reviewed size')
    if digest(read_va(pe, va, size)) != spec['sha256']:
        raise ValueError(f'reviewed body hash mismatch at {va:#x}')


def validate_review(pe, config, instructions):
    if config['schema_version'] != 1 or pe.machine != 0x14c or digest(pe.data) != config['input_sha256']:
        raise ValueError('unsupported schema or input build')
    target, helper = config['target'], config['helper']
    validate_range(pe, target); validate_range(pe, helper)
    start, end = int(target['va'], 16), int(target['va'], 16) + target['size']
    body = reviewed_body(instructions, start, end)
    for ins in body:
        if read_va(pe, ins.va, len(ins.raw)) != ins.raw:
            raise ValueError('decoded instruction bytes differ from input')
    classes = [c for c in parse_rtti(pe) if c.name == target['class']]
    vt = int(target['vtable'], 16)
    if len(classes) != 1 or not any(vt in r.vtables and r.object_offset == 0 for r in classes[0].vtable_records):
        raise ValueError('primary class/vtable evidence mismatch')
    if struct.unpack('<I', read_va(pe, vt + 4*target['slot'], 4))[0] != start:
        raise ValueError('virtual slot target mismatch')
    block_rows, cursor, ids = [], start, set()
    boundaries = {i.va for i in body} | {end}
    for block in config['blocks']:
        lo, hi = int(block['va'], 16), int(block['end'], 16)
        if lo != cursor or hi <= lo or hi not in boundaries or block['id'] in ids:
            raise ValueError('block partition has a gap, overlap, duplicate ID or split instruction')
        if digest(read_va(pe, lo, hi-lo)) != block['sha256']:
            raise ValueError('block hash mismatch')
        ids.add(block['id']); cursor = hi
        block_rows.append({**block, 'instruction_count': sum(lo <= i.va < hi for i in body)})
    if cursor != end:
        raise ValueError('block partition does not cover the whole target')
    calls = [{'site': hx(i.va), 'target': hx(direct_call_target(i))} for i in body if direct_call_target(i) is not None]
    if calls != config['expected_calls']:
        raise ValueError('reviewed direct call sites differ')
    callees = {int(c['target'], 16) for c in calls}
    refs = {int(s, 16) for i in body for s in re.findall(r'0x[0-9a-fA-F]+', i.operands)}
    bindings = {}
    for b in config['bindings']:
        va = int(b['va'], 16)
        if b['symbol'] in bindings or b['kind'] not in ('direct_call', 'address_reference'):
            raise ValueError('duplicate symbol or unknown binding kind')
        if va not in (callees if b['kind'] == 'direct_call' else refs):
            raise ValueError('binding lacks a reviewed instruction reference')
        bindings[b['symbol']] = va
    for lit in config['literals']:
        if ascii_at(pe, int(lit['va'], 16)) != lit['value']:
            raise ValueError('literal mismatch')
    constants = []
    for c in config['constants']:
        if c['format'] not in ('f','d'):
            raise ValueError('unsupported constant format')
        raw = read_va(pe, int(c['va'],16), struct.calcsize('<'+c['format']))
        if raw.hex() != c['hex']:
            raise ValueError('numeric constant mismatch')
        constants.append({**c, 'value': struct.unpack('<'+c['format'],raw)[0]})
    history = normal_exit_paths(body, 0x4a2d00, 0x4a2d90)
    return {'input_sha256': config['input_sha256'], 'target': target, 'helper': helper,
            'blocks': block_rows, 'direct_calls': calls, 'numeric_constants': constants,
            'instructions': [{'va':hx(i.va),'size':len(i.raw),'mnemonic':i.mnemonic,'operands':i.operands} for i in body],
            'normal_cfg': history, 'bindings': config['bindings'], 'instruction_count':len(body),
            'original_symbol_names_recovered':False, 'numeric_equivalence_proven':False,
            'original_game_executed':False, 'historical_vc6_executed':False}, bindings


def compile_probes(pe, config, bindings):
    compiler = shutil.which('clang-cl')
    if not compiler:
        raise ValueError('clang-cl required for --compile-probes')
    version = subprocess.run([compiler,'--version'],check=True,text=True,capture_output=True,timeout=10).stdout.strip()
    results = []
    with tempfile.TemporaryDirectory() as temp:
        for name in ('target','helper'):
            spec = config[name]
            obj = Path(temp)/(name+'.obj')
            cmd = [compiler, *config['compile_flags'], str(ROOT/spec['source']), '/Fo'+str(obj)]
            subprocess.run(cmd,check=True,text=True,capture_output=True,timeout=45)
            va = int(spec['va'],16)
            results.append({'role':name, **match_object(CoffObject(obj),spec['symbol'],va,read_va(pe,va,spec['size']),bindings)})
    return {'version':version,'flags':config['compile_flags'],'historical_vc6':False}, results


def render_report(data):
    lines = ['# EcoSystem top-level reconstruction', '',
             f"Input SHA-256: `{data['input_sha256']}`", '',
             f"Reviewed target: `{data['target']['va']}`, {data['target']['size']} bytes, {data['instruction_count']} instructions.",
             f"All {len(data['blocks'])} contiguous review blocks and {len(data['direct_calls'])} direct-call sites verified.", '',
             'The shared C++98 algorithm replaces the opaque iteration callback at the top level. Existing callees remain external, with provisional declarations. No original method names or full object sizes are claimed.', '',
             '## Numeric constants', '', '| Address | Format | Stored value |', '|---|---|---:|']
    for c in data['numeric_constants']:
        lines.append(f"| `{c['va']}` | {c['format']} | {c['value']!r} |")
    lines += ['', 'The bound comparison selects the second operand for less/equal or unordered. The vertical lift remains separate from the rounded bound. Host double/long-double models do not prove retail x87 precision, exception masks or return-register behavior.', '',
              '## Normal exits', '', '| Return | Local select/restore history |','|---|---|']
    for e in data['normal_cfg']['normal_returns']:
        lines.append(f"| `{e['return_va']}` | {e['local_history']} |")
    lines += ['', '## Strict compiled comparisons', '']
    if data.get('probe_results'):
        lines += ['| Target | Retail size | Candidate size | Matching / compared positions | Resolved fixups | Exact |','|---|---:|---:|---:|---:|---|']
        for p in data['probe_results']:
            lines.append(f"| `{p['target_va']}` | {p['retail_size']} | {p['candidate_size']} | {p['matching_positions']}/{p['compared_positions']} | {len(p['relocations_applied'])} | {p['strict_exact']} |")
    else:
        lines.append('Probes were not compiled in this run.')
    lines += ['', 'No bytes are masked. A comparison is exact only when lengths and all bytes agree after explicit relocation application. Address bindings do not authenticate original symbol identity.', '',
              '## Scope', '',
              'Record classification, coordinate conversion, geometry call, two list-growth paths and timing/category exits are implemented. Helpers for classification, record application, geometry testing, traversal, memory management and category tracking remain separately callable boundaries.', '',
              'The first list grows by 100 slots, the second by 20; capacities increase only if the returned pointer differs. The candidate deliberately does not add a failure guard absent from the reviewed code. Native test storage is larger than its simulated capacities; that tests the branch, not allocator safety.', '',
              'No original EXE/DLL execution, differential target emulation, VC6 run, complete engine reconstruction or proven floating-point equivalence is claimed.', '']
    return '\n'.join(lines)


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--exe',type=Path,default=ROOT/'work/game/mcm2.exe')
    ap.add_argument('--config',type=Path,default=ROOT/'config/ecosystem_pass.json')
    ap.add_argument('--out',type=Path,default=ROOT/'work/ecosystem')
    ap.add_argument('--objdump',default='objdump')
    ap.add_argument('--compile-probes',action='store_true')
    args=ap.parse_args();config=json.loads(args.config.read_text());pe=PEImage(args.exe)
    if pe.machine!=0x14c or digest(pe.data)!=config['input_sha256']:
        raise ValueError('unknown input: reviewed addresses will not be applied')
    instructions,decoder=decode_image(pe,args.objdump)
    data,bindings=validate_review(pe,config,instructions);data['decoder']=decoder
    data['probe_results']=[]
    if args.compile_probes:data['compiler'],data['probe_results']=compile_probes(pe,config,bindings)
    paths=['samples/ecosystem/EcoSystemPass.h','samples/ecosystem/EcoSystemProbe.cpp','samples/ecosystem/EpochProbe.cpp',
           'tools/review_ecosystem.py','mcm2tool/pe.py','mcm2tool/rtti.py','mcm2tool/msvc.py','mcm2tool/allocation.py',
           'mcm2tool/coff.py','mcm2tool/resolved_match.py','mcm2tool/category_lifetimes.py','mcm2tool/category_contexts.py']
    data['tool_sha256']={p:digest((ROOT/p).read_bytes()) for p in paths}
    data['config_sha256']=digest(args.config.read_bytes())
    args.out.mkdir(parents=True,exist_ok=True)
    (args.out/'ecosystem.json').write_text(json.dumps(data,indent=2,sort_keys=True)+'\n')
    (args.out/'REPORT.md').write_text(render_report(data))
    print(render_report(data))


if __name__=='__main__':
    try: main()
    except (ValueError,OSError,KeyError,TypeError,struct.error,subprocess.SubprocessError) as exc:
        print(f'ecosystem: {exc}',file=sys.stderr)
        if isinstance(exc,subprocess.CalledProcessError) and exc.stderr:print(exc.stderr,file=sys.stderr)
        raise SystemExit(2)
