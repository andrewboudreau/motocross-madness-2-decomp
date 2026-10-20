#!/usr/bin/env python3
"""Compile the reconstructed game and attempt a normal VC6 Windows link.

This is a source build, independent of the retail EXE and matching bindings.
Missing providers are link errors; a successful link still needs runtime testing.
Each invocation uses a fresh directory so failed builds cannot reuse old output.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from mcm2tool.coff import CoffObject
from mcm2tool.toolchain import (find_child_ci, find_vc6_bin, infer_vc98_root,
                               sha256_file)
from mcm2tool.vc6_runtime import executable_command, runner_kind, windows_path


def source_files(root: Path, spec: dict) -> list[Path]:
    sources = {p.resolve() for name in spec['source_roots']
               for p in (root / name).rglob('*.cpp')}
    for candidate in spec.get('candidate_sources', []):
        path = (root / candidate['path']).resolve()
        try:
            relative = path.relative_to(root.resolve())
        except ValueError:
            raise ValueError('candidate source escapes repository') from None
        if (relative.parts[0] != 'samples' or path.suffix != '.cpp'
                or not path.is_file()):
            raise ValueError(f'candidate must name an existing samples C++ file: {relative}')
        if (candidate.get('status') != 'reviewed-nonexact'
                or not candidate.get('reason') or not candidate.get('evidence')):
            raise ValueError(f'candidate lacks nonexact review evidence: {relative}')
        sources.add(path)
    if not sources:
        raise ValueError('no C++ sources found')
    # Original source xrefs support alphabetical unit order. Relative paths
    # break ties; this does not claim recovered original TU composition.
    return sorted(sources, key=lambda p: (p.name.casefold(), p.as_posix().casefold()))


def verify_toolchain(vc6: Path) -> dict:
    """Use the installed private bundle's pinned compiler/linker/CRT hashes."""
    expected = json.loads((ROOT / 'config/private_bundle_expected.json').read_text())
    vc98 = infer_vc98_root(vc6)
    hashes = {}
    for relative, digest in expected['core_files'].items():
        path = vc98
        for part in Path(relative).parts[1:]:
            child = find_child_ci(path, part)
            if child is None:
                raise ValueError(f'missing VC6 SP3 component: {relative}')
            path = child
        actual = sha256_file(path)
        if actual != digest:
            raise ValueError(f'VC6 SP3 component hash mismatch: {relative}')
        hashes[relative] = actual
    return hashes


def run_command(command: list[str], cwd: Path, env: dict, log: Path) -> dict:
    try:
        result = subprocess.run(command, cwd=cwd, env=env, capture_output=True,
                                text=True, encoding='latin-1')
        output = result.stdout + result.stderr
        code = result.returncode
    except OSError as exc:
        output, code = str(exc), -1
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_text(output, encoding='utf-8')
    return {'command': command, 'returncode': code, 'log': str(log)}


def valid_object(path: Path) -> bool:
    if not path.is_file():
        return False
    obj = CoffObject(path)
    # A globals/GUID provider can contain data without executable code. Common
    # symbols reserve zero-filled storage without contributing section bytes.
    return obj.machine == 0x14c and (
        any(sec.raw_size and sec.characteristics & 0xe0 for sec in obj.sections)
        or any(sym.storage_class == 2 and sym.section_number == 0 and sym.value > 0
               for sym in obj.symbols))


def compile_source(source: Path, build: Path, vc6: Path, includes: list[Path],
                   profile: str, env: dict) -> dict:
    relative = source.relative_to(ROOT)
    obj = (build / 'objects' / relative).with_suffix('.obj')
    obj.parent.mkdir(parents=True, exist_ok=True)
    command = [sys.executable, str(ROOT / 'tools/compile.py'), str(source),
               '-o', str(obj), '--compiler', 'vc6', '--vc6-root', str(vc6),
               '--profile', profile]
    for include in includes:
        command.extend(['--include', str(include)])
    row = run_command(command, ROOT, env, obj.with_suffix('.log'))
    row.update(source=relative.as_posix(), source_sha256=sha256_file(source),
               object=str(obj), compiled=False)
    if row['returncode'] == 0:
        try:
            row['compiled'] = valid_object(obj)
        except (ValueError, OSError) as exc:
            row['object_error'] = str(exc)
    return row


def response_file(path: Path, args: list[str]) -> None:
    # LINK's response format accepts quoted paths with spaces; it is not a shell.
    if any(any(c in arg for c in '\r\n"') for arg in args):
        raise ValueError('unsupported quote/newline in linker argument')
    path.write_text('\n'.join('"' + arg + '"' for arg in args) + '\n',
                    encoding='ascii')


def link_game(build: Path, objects: list[Path], libraries: list[Path],
              vc6: Path, runner: str, env: dict) -> dict:
    linker = find_child_ci(find_vc6_bin(vc6), 'link.exe')
    if linker is None:
        raise ValueError('VC6 LINK.EXE missing')
    exe = build / 'mcm2.exe'
    args = ['/nologo', '/MACHINE:I386', '/SUBSYSTEM:WINDOWS', '/INCREMENTAL:NO',
            '/OUT:' + windows_path(exe, runner),
            '/MAP:' + windows_path(build / 'mcm2.map', runner),
            *[windows_path(p, runner) for p in objects + libraries]]
    rsp = build / 'link.rsp'
    response_file(rsp, args)
    row = run_command([*executable_command(linker, runner),
                       '@' + windows_path(rsp, runner)], build, env, build / 'link.log')
    row['linked'] = row['returncode'] == 0 and exe.is_file() and exe.stat().st_size > 0
    row['executable'] = str(exe) if row['linked'] else None
    if row['linked']:
        row['sha256'] = sha256_file(exe)
    return row


def build_game(vc6: Path, jobs: int, compile_only: bool = False) -> tuple[dict, Path]:
    config = ROOT / 'config/game_build.json'
    spec = json.loads(config.read_text())
    output = ROOT / 'work/game-build'
    output.mkdir(parents=True, exist_ok=True)
    build = Path(tempfile.mkdtemp(prefix='build-', dir=output))
    report = {'stage': 'toolchain', 'compiled': False, 'linked': False,
              'runtime_verified': False, 'build_directory': str(build),
              'config_sha256': sha256_file(config)}
    try:
        report['toolchain_hashes'] = verify_toolchain(vc6)
        runner = runner_kind()
        vc98 = infer_vc98_root(vc6)
        libdir = find_child_ci(vc98, 'lib')
        if libdir is None:
            raise ValueError('VC6 library directory missing')
        env = dict(os.environ)
        env['PYTHONPATH'] = str(ROOT)
        env['LIB'] = windows_path(libdir, runner)
        env['PATH'] = str(find_vc6_bin(vc6)) + os.pathsep + env.get('PATH', '')
        sources = source_files(ROOT, spec)
        includes = [ROOT / name for name in spec['includes']]
        report['profile'] = spec['profile']
        report['candidate_sources'] = spec.get('candidate_sources', [])
        report['profile_sha256'] = sha256_file(ROOT / 'config/compile_profiles.json')
        report['headers'] = {p.relative_to(ROOT).as_posix(): sha256_file(p)
                             for p in sorted((ROOT / 'src').rglob('*'))
                             if p.suffix in ('.h', '.hpp', '.inl')}
        report['stage'] = 'compile'
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            report['sources'] = list(pool.map(
                lambda source: compile_source(source, build, vc6, includes,
                                              spec['profile'], env), sources))
        passed = sum(row['compiled'] for row in report['sources'])
        print(f'VC6 compiled {passed}/{len(sources)} C++ translation units')
        report['compiled'] = passed == len(sources)
        if not report['compiled'] or compile_only:
            return report, build
        report['stage'] = 'import_libraries'
        librarian = find_child_ci(find_vc6_bin(vc6), 'lib.exe')
        if librarian is None:
            raise ValueError('VC6 LIB.EXE missing')
        libraries = []
        report['import_libraries'] = []
        for name in spec['import_definitions']:
            definition = ROOT / name
            library = build / (definition.stem + '-imports.lib')
            row = run_command([*executable_command(librarian, runner), '/nologo',
                               '/MACHINE:I386', '/DEF:' + windows_path(definition, runner),
                               '/OUT:' + windows_path(library, runner)],
                              build, env, build / (definition.stem + '-imports.log'))
            row['definition_sha256'] = sha256_file(definition)
            report['import_libraries'].append(row)
            if row['returncode'] or not library.is_file():
                return report, build
            libraries.append(library)
        for name in spec['libraries']:
            library = find_child_ci(libdir, name)
            if library is None:
                raise ValueError(f'missing import library: {name}')
            libraries.append(library)
        report['stage'] = 'link'
        report['link'] = link_game(build, [Path(r['object']) for r in report['sources']],
                                   libraries, vc6, runner, env)
        report['linked'] = report['link']['linked']
        if report['linked']:
            report['stage'] = 'linked_runtime_unverified'
        return report, build
    except (ValueError, OSError) as exc:
        report['error'] = str(exc)
        return report, build
    finally:
        (build / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
        (output / 'latest.json').write_text(json.dumps(report, indent=2) + '\n')


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('--vc6-root', type=Path,
                        default=os.environ.get('VC6_ROOT', str(ROOT / 'work/vc6sp3-disc')))
    parser.add_argument('-j', '--jobs', type=int, default=4)
    parser.add_argument('--compile-only', action='store_true')
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error('--jobs must be positive')
    report, build = build_game(args.vc6_root.resolve(), args.jobs, args.compile_only)
    print(f'Report: {build / "report.json"}')
    if report.get('error'):
        print(report['error'], file=sys.stderr)
    if args.compile_only and report['compiled']:
        print('Compilation passed; linking and gameplay remain unverified.')
        return 0
    if report['linked']:
        print(f'Linked {report["link"]["executable"]}; gameplay remains unverified.')
        return 0
    print(f'Game build failed during {report["stage"]}; see the report and logs.', file=sys.stderr)
    return 1


if __name__ == '__main__':
    raise SystemExit(main())
