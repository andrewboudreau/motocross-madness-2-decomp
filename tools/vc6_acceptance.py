#!/usr/bin/env python3
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from mcm2tool.coff import CoffObject
from mcm2tool.toolchain import fingerprint_toolchain
from mcm2tool.vc6_runtime import runner_kind

EXPECTED_EXE_SHA = '31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874'

def sha(path: Path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()

def run(cmd, env=None):
    r = subprocess.run(
        cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env
    )
    return {'command': cmd, 'returncode': r.returncode, 'output': r.stdout}

def main():
    ap = argparse.ArgumentParser(
        description='Prove the private VC6 SP3 payload can compile i386 COFF, then optionally run the historical MCM2 gate.'
    )
    ap.add_argument(
        '--root',
        type=Path,
        default=Path(
            os.environ.get('MCM2_PRIVATE_ROOT', '~/.cache/mcm2-private')
        ).expanduser(),
    )
    ap.add_argument('--full-gate', action='store_true')
    ap.add_argument('--out', type=Path, default=Path('work/vc6-acceptance.json'))
    args = ap.parse_args()
    root = args.root.resolve()
    vc6 = root / 'toolchains/vc6sp3'
    exe = root / 'work/game/mcm2.exe'

    if not vc6.exists() or not exe.exists():
        raise SystemExit(
            'private inputs missing; run tools/install_private_bundle.py first'
        )
    if sha(exe) != EXPECTED_EXE_SHA:
        raise SystemExit('mcm2.exe hash mismatch')

    fp = fingerprint_toolchain(vc6)
    expected = json.loads((ROOT / 'config/vc6_sp3_expected.json').read_text())
    bundle_expected = json.loads(
        (ROOT / 'config/private_bundle_expected.json').read_text()
    )

    by_name = {f['name'].upper(): f for f in fp['files']}
    checks = []
    for name, prefixes in expected['core_files'].items():
        versions = (by_name.get(name.upper()) or {}).get('version_strings', [])
        checks.append(
            {
                'file': name,
                'versions': versions,
                'matches': any(
                    any(v.startswith(p) for p in prefixes) for v in versions
                ),
            }
        )
    if not checks or not all(x['matches'] for x in checks):
        raise SystemExit('VC6 SP3 core version check failed')

    for relative, expected_hash in bundle_expected.get('core_files', {}).items():
        actual = vc6 / relative
        if not actual.exists() or sha(actual) != expected_hash:
            raise SystemExit(f'private core file hash mismatch: {relative}')

    platform = runner_kind()
    required = {'windows': (), 'wine': ('wine', 'winepath'), 'wibo': ('wibo',)}[platform]
    missing = [tool for tool in required if not shutil.which(tool)]
    if missing:
        payload = {
            'ready': False,
            'stage': 'compiler_execution',
            'reason': '/'.join(missing) + ' unavailable',
            'platform': platform,
            'toolchain': fp,
            'sp3_checks': checks,
            'mcm2_sha256': EXPECTED_EXE_SHA,
        }
        args.out.parent.mkdir(parents=True, exist_ok=True)
        args.out.write_text(json.dumps(payload, indent=2) + '\n')
        print(json.dumps(payload, indent=2))
        raise SystemExit(3)

    env = os.environ.copy()
    env.update(
        {
            'VC6_ROOT': str(vc6),
            'WINEPREFIX': str(root / 'wine-vc6'),
            'WINEARCH': 'win32',
            'WINEDEBUG': '-all',
        }
    )

    if platform == 'wine' and not (root / 'wine-vc6').exists():
        init = run(
            [
                sys.executable,
                str(ROOT / 'tools/init_wine_prefix.py'),
                '--prefix',
                str(root / 'wine-vc6'),
            ],
            env,
        )
        if init['returncode'] != 0:
            raise SystemExit(init['output'])

    with tempfile.TemporaryDirectory(prefix='mcm2-vc6-ready-') as td:
        obj = Path(td) / 'readiness.obj'
        compile_result = run(
            [
                sys.executable,
                str(ROOT / 'tools/compile.py'),
                str(ROOT / 'samples/vc6/Readiness.cpp'),
                '-o',
                str(obj),
                '--compiler',
                'vc6',
                '--vc6-root',
                str(vc6),
            ],
            env,
        )
        if compile_result['returncode'] != 0:
            payload = {
                'ready': False,
                'stage': 'compile',
                'compile': compile_result,
                'toolchain': fp,
                'sp3_checks': checks,
                'mcm2_sha256': EXPECTED_EXE_SHA,
            }
            args.out.parent.mkdir(parents=True, exist_ok=True)
            args.out.write_text(json.dumps(payload, indent=2) + '\n')
            print(json.dumps(payload, indent=2))
            raise SystemExit(4)

        coff = CoffObject(obj)
        symbol = coff.find_symbol('Vc6Readiness')
        body, size, rels = coff.symbol_extent(symbol)
        if coff.machine != 0x14C or not body:
            raise SystemExit('compiler output is not nonempty i386 COFF')
        object_info = {
            'machine': f'0x{coff.machine:04x}',
            'symbol': symbol.name,
            'function_size': size,
            'relocations': len(rels),
            'sha256': sha(obj),
        }

    gate = None
    if args.full_gate:
        gate = run(
            [
                sys.executable,
                str(ROOT / 'tools/vc6_gate.py'),
                '--vc6-root',
                str(vc6),
                '--exe',
                str(exe),
                '--out',
                str(root / 'vc6_gate.json'),
            ],
            env,
        )

    payload = {
        'ready': True,
        'platform': platform,
        'toolchain': fp,
        'sp3_checks': checks,
        'mcm2_sha256': EXPECTED_EXE_SHA,
        'readiness_compile': compile_result,
        'object': object_info,
        'full_gate': gate,
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(payload, indent=2) + '\n')
    print(json.dumps(payload, indent=2))
    # Readiness stays true in the report, but an explicitly requested matching
    # gate must propagate its failure to make/CI/the caller.
    if gate and gate['returncode'] != 0:
        raise SystemExit(gate['returncode'])

if __name__ == '__main__':
    main()
