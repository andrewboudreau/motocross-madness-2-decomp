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
import re
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from mcm2tool.coff import CoffObject
from mcm2tool.toolchain import (find_child_ci, find_vc6_bin, infer_vc98_root,
                               sha256_file)
from mcm2tool.vc6_runtime import executable_command, runner_kind, windows_path


# Unit-wide flags a reviewed unit may add. `/QIfist` stands for retail's inline
# fistp helper; `/Op` for a binary32 spill that helper forces. `/Oy-` stands for
# the EBP frame the helper's `__asm` gives each function that contains it, so it
# is accepted only together with `/QIfist`, and only for a unit whose every
# function is framed in retail (docs/GAME_BUILD.md#frame-pointer-units).
CANDIDATE_FLAGS = ('/QIfist', '/Op', '/Oy-')
BRINGUP_FLAGS = ('/QIfist', '/Oy-')


def valid_unit_flags(flags, allowed: tuple[str, ...]) -> bool:
    return (isinstance(flags, list) and all(flag in allowed for flag in flags)
            and len(set(flags)) == len(flags)
            and ('/Oy-' not in flags or '/QIfist' in flags))


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
        if (candidate.get('status') not in ('reviewed-nonexact', 'best-guess-unreviewed')
                or not candidate.get('reason') or not candidate.get('evidence')):
            raise ValueError(f'candidate lacks nonexact review evidence: {relative}')
        if not valid_unit_flags(candidate.get('extra_flags', []), CANDIDATE_FLAGS):
            raise ValueError(f'unsupported candidate compiler flags: {relative}')
        sources.add(path)
    # A source file that another compiled source #includes is part of that
    # translation unit (e.g. a method body shared by a calibration case and its
    # owning unit); compiling it on its own as well would define it twice.
    # Quoted includes resolve against the owner's directory, then the
    # configured include paths, as CL.EXE does.
    include_dirs = [(root / name).resolve() for name in spec.get('includes', [])]
    for fragment in spec.get('included_sources', []):
        path = (root / fragment['path']).resolve()
        owner = (root / fragment.get('included_by', '')).resolve()
        if path not in sources or owner not in sources or path == owner:
            raise ValueError(f"included source must name two compiled sources: {fragment['path']}")
        if not fragment.get('reason'):
            raise ValueError(f"included source lacks a reason: {fragment['path']}")
        includes = re.findall(r'^\s*#\s*include\s+"([^"]+)"', owner.read_text(encoding='latin1'), re.M)
        if not any((base / name).resolve() == path
                   for name in includes for base in [owner.parent] + include_dirs):
            raise ValueError(f"{fragment['included_by']} does not #include {fragment['path']}")
        sources.discard(path)
    if not sources:
        raise ValueError('no C++ sources found')
    # Original source xrefs support alphabetical unit order. Relative paths
    # break ties; this does not claim recovered original TU composition.
    return sorted(sources, key=lambda p: (p.name.casefold(), p.as_posix().casefold()))


def compile_flags(root: Path, spec: dict) -> dict[Path, tuple[str, ...]]:
    """Per-source extra compiler flags: reviewed candidates' `extra_flags` and
    `bringup_flags` entries for src/bringup units. Bring-up stand-ins are compiled
    only by this build (calibration and the physics runner compile canonical
    source and samples), so a unit-wide flag there changes no scored object."""
    flags = {(root / candidate['path']).resolve(): tuple(candidate.get('extra_flags', []))
             for candidate in spec.get('candidate_sources', [])}
    for entry in spec.get('bringup_flags', []):
        path = (root / entry['path']).resolve()
        try:
            relative = path.relative_to(root.resolve())
        except ValueError:
            raise ValueError('bring-up flag source escapes repository') from None
        if (relative.parts[:2] != ('src', 'bringup') or path.suffix != '.cpp'
                or not path.is_file()):
            raise ValueError(f'bring-up flags must name an existing src/bringup C++ file: {relative}')
        unit_flags = entry.get('extra_flags')
        if not unit_flags or not valid_unit_flags(unit_flags, BRINGUP_FLAGS):
            raise ValueError(f'unsupported bring-up compiler flags: {relative}')
        if not entry.get('reason'):
            raise ValueError(f'bring-up flags lack a reason: {relative}')
        if path in flags:
            raise ValueError(f'bring-up flags listed twice: {relative}')
        flags[path] = tuple(unit_flags)
    return flags


def header_hashes(root: Path) -> dict[str, str]:
    """Conservative inventory including headers shared by admitted candidates."""
    return {p.relative_to(root).as_posix(): sha256_file(p)
            for name in ('src', 'samples')
            for p in sorted((root / name).rglob('*'))
            if p.is_file() and p.suffix.lower() in ('.h', '.hpp', '.inl')}


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
                   profile: str, env: dict, extra_flags: tuple[str, ...] = ()) -> dict:
    relative = source.relative_to(ROOT)
    obj = (build / 'objects' / relative).with_suffix('.obj')
    obj.parent.mkdir(parents=True, exist_ok=True)
    command = [sys.executable, str(ROOT / 'tools/compile.py'), str(source),
               '-o', str(obj), '--compiler', 'vc6', '--vc6-root', str(vc6),
               '--profile', profile]
    for include in includes:
        command.extend(['--include', str(include)])
    for flag in extra_flags:
        command.extend(['--extra', flag])
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


def write_alias_object(path: Path, aliases: dict) -> None:
    """Emit a COFF object of weak externals: each alias resolves to its target.

    The retail binary has one object where reconstructed sources spell the same
    address with two C++ types (two mangled names). Run-only link aid; it adds
    no code or data and never changes a matched translation unit.
    """
    strings = bytearray(4)
    def name_field(name: str) -> bytes:
        raw = name.encode('ascii')
        if len(raw) <= 8:
            return raw.ljust(8, bytes(1))
        offset = len(strings)
        strings.extend(raw + bytes(1))
        return struct.pack('<II', 0, offset)
    symbols = bytearray()
    for index, (alias, target) in enumerate(sorted(aliases.items())):
        symbols += name_field(target) + struct.pack('<IhHBB', 0, 0, 0, 2, 0)
        symbols += name_field(alias) + struct.pack('<IhHBB', 0, 0, 0, 105, 1)
        symbols += struct.pack('<II10x', index * 3, 3)
    struct.pack_into('<I', strings, 0, len(strings))
    header = struct.pack('<HHIIIHH', 0x14c, 0, 0, 20, len(symbols) // 18, 0, 0)
    path.write_bytes(header + bytes(symbols) + bytes(strings))


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
        source_flags = compile_flags(ROOT, spec)
        outside = sorted(str(path) for path in source_flags if path not in sources)
        if outside:
            raise ValueError(f'compiler flags name sources outside the build: {outside}')
        includes = [ROOT / name for name in spec['includes']]
        report['profile'] = spec['profile']
        report['candidate_sources'] = spec.get('candidate_sources', [])
        report['bringup_flags'] = spec.get('bringup_flags', [])
        report['profile_sha256'] = sha256_file(ROOT / 'config/compile_profiles.json')
        report['headers'] = header_hashes(ROOT)
        report['stage'] = 'compile'
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            report['sources'] = list(pool.map(
                lambda source: compile_source(source, build, vc6, includes,
                                              spec['profile'], env, source_flags.get(source, ())), sources))
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
        alias_objects = []
        if spec.get('link_aliases'):
            alias_object = build / 'link-aliases.obj'
            write_alias_object(alias_object, spec['link_aliases'])
            alias_objects.append(alias_object)
        report['link'] = link_game(build, [Path(r['object']) for r in report['sources']] + alias_objects,
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
