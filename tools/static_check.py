#!/usr/bin/env python3
from __future__ import annotations
import ast, json, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FORBIDDEN_SUFFIXES = {'.exe','.dll','.cab','.iso','.msi','.lib','.pdb','.idb','.ilk','.zip','.7z','.rar'}
IGNORED_DIRS = {'.git','work','toolchains','input','__pycache__'}
REQUIRED = [
    'tools/bootstrap.py','tools/extract_installer.py','tools/analyze.py',
    'tools/build_class_evidence.py','tools/analyze_msvc_artifacts.py','tools/find_vtable_writes.py',
    'tools/build_function_manifest.py','tools/build_class_dossiers.py','tools/build_work_queue.py',
    'tools/compile.py','tools/match.py','tools/run_samples.py','tools/generate_easy_probes.py',
    'tools/run_easy_probes.py','tools/run_calibration.py','tools/status.py','tools/selftest.py',
    'tools/import_vc6.py','tools/probe_vc6.py','tools/vc6_gate.py','tools/init_wine_prefix.py',
    'config/compile_profiles.json','config/vc6_sp3_expected.json','AGENTS.md','README.md',
]

def fail(msg: str) -> None:
    print(f'[fail] {msg}', file=sys.stderr)
    raise SystemExit(1)

def main() -> None:
    missing=[p for p in REQUIRED if not (ROOT/p).is_file()]
    if missing: fail('missing required repository files: '+', '.join(missing))

    forbidden=[]
    py_files=[]
    json_files=[]
    for p in ROOT.rglob('*'):
        if not p.is_file(): continue
        rel=p.relative_to(ROOT)
        if any(part in IGNORED_DIRS for part in rel.parts): continue
        if p.suffix.lower() in FORBIDDEN_SUFFIXES: forbidden.append(rel.as_posix())
        if p.suffix.lower()=='.py': py_files.append(p)
        if p.suffix.lower()=='.json' and 'analysis' not in rel.parts: json_files.append(p)
    if forbidden: fail('proprietary/binary archive extensions committed: '+', '.join(forbidden))

    for p in py_files:
        try: ast.parse(p.read_text(encoding='utf-8'), filename=str(p))
        except Exception as e: fail(f'python syntax error in {p.relative_to(ROOT)}: {e}')
    for p in json_files:
        try: json.loads(p.read_text(encoding='utf-8'))
        except Exception as e: fail(f'invalid JSON in {p.relative_to(ROOT)}: {e}')

    # Cheap unit checks for the recognizers added to the bootstrap. These do not
    # require the game executable and keep CI useful on a clean public clone.
    from mcm2tool.easy import classify_easy_bytes
    ui = bytes.fromhex('c7 81 c0 01 00 00 00 00 00 00 c7 81 bc 01 00 00 03 00 00 00 c3')
    camera = bytes.fromhex('33 c0 89 81 2c 02 00 00 89 81 34 02 00 00 89 81 20 02 00 00 c3')
    p1=classify_easy_bytes(ui); p2=classify_easy_bytes(camera)
    if not p1 or p1.kind!='set_i32_constants': fail('set_i32_constants recognizer regression')
    if not p2 or p2.kind!='zero_i32_fields': fail('zero_i32_fields recognizer regression')

    print(f'static-check: PASS ({len(py_files)} Python files, {len(json_files)} JSON files)')

if __name__=='__main__': main()
