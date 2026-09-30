# Build and private inputs

Use Python 3.10+ and authentic VC6 SP3. Native Windows execution is verified;
Linux requires 32-bit Wine and the same readiness check. GNU `objdump` is needed
for disassembly tools. Clang is optional for bootstrap comparisons; a native
C++ compiler enables behavior-model tests.

## Private bundle

Archive, EXE and component hashes are pinned in `config/private_bundle_expected.json`
and `config/vc6_sp3_expected.json`. Installation checks archive membership, all
1,463 payload hashes and the EXE, and rejects path traversal/symlinks. The bundle
contains an installed VC98 tree plus the target EXE, so no full installer is needed.

Windows:

```powershell
$env:PYTHONPATH = '.'
$env:MCM2_PRIVATE_ROOT = Join-Path $env:LOCALAPPDATA 'mcm2-private'
powershell -ExecutionPolicy Bypass -File tools/setup_vc6_windows.ps1 -Archive C:\private\mcm2-vc6sp3-private-inputs.zip
$env:VC6_ROOT = Join-Path $env:MCM2_PRIVATE_ROOT 'toolchains/vc6sp3'
$env:MCM2_EXE = Join-Path $env:MCM2_PRIVATE_ROOT 'work/game/mcm2.exe'
python tools/analyze.py $env:MCM2_EXE
python tools/build_class_evidence.py
python tools/analyze_msvc_artifacts.py
python tools/find_vtable_writes.py
python tools/build_function_manifest.py
python tools/build_class_dossiers.py
python tools/build_work_queue.py
python tools/vc6_gate.py --exe $env:MCM2_EXE --vc6-root $env:VC6_ROOT
```

Linux, with `MCM2_PRIVATE_BUNDLE_URL` in the setup environment:

```bash
bash tools/setup_vc6_linux.sh
python3 tools/with_private_env.py -- make analyze
make private-ready
make vc6-private-gate
```

For a local archive: `make private-install PRIVATE_BUNDLE=/private/inputs.zip`.
Linux defaults to `~/.cache/mcm2-private`; use `MCM2_PRIVATE_ROOT` or `--root`
to select another cache. The wrapper supplies EXE, compiler and Wine paths to
child processes. Never store signed URLs/tokens in tracked files or logs.

Readiness requires authenticated SP3 components and a real compiler process
producing nonempty i386 COFF. `--full-gate` additionally propagates byte-gate
failure. Source mismatch and environment failure are separate; no clang fallback
is allowed when VC6 acceptance fails.

## Cloud and containers

`tools/cloud_setup.sh` provisions Ubuntu workers and generates analysis. The
Claude remote-session hook calls it automatically. Configure the private bundle
URL and allow its host. Steps warn on failure, so inspect
`work/vc6-acceptance.json`; hook completion alone is not readiness. Reusable path
settings are in `work/cloud-env.sh`. Outputs stay outside tracked docs/source.

Docker accepts an owned installer and a mounted compiler tree:

```bash
docker compose build decomp
VC6_ROOT_HOST=/private/vc6sp3 docker compose run --rm decomp
VC6_ROOT_HOST=/private/vc6sp3 docker compose run --rm decomp make vc6-gate VC6_ROOT=/toolchains/vc6sp3
```

Its default command reads `input/MCM2PCG.exe`; override it for an extracted EXE.
Workers/caches containing private inputs must remain private.

## Existing installations and toolchain identity

`tools/import_vc6.py /path/to/VC98 --out toolchains/vc6sp3` accepts an installed
tree or archive. On Linux use `tools/init_wine_prefix.py`; inspect components
with `tools/probe_vc6.py --vc6-root ...`.

`python tools/bootstrap.py /path/to/MCM2PCG.exe` extracts game files as data and
runs analysis. Filename skeletons go to `generated/krusty2-skeletons/`, not `src/`.
Clang comparisons run only when available.

Retail Rich records include `Utc12_CPP / 8447 / 197` and `Linker600 / 8447 / 2`.
The supplied SP3 frontend is build 8472; backend and linker are build 8447.
The CL driver banner alone is insufficient. On the SP3 disc, the full backend
is `os/system/msvcep.dll` (original filename C2.DLL); `msse.dll` and `intro.dll`
are different edition variants.

[CRT object matching](VC6_CRT_ATLAS.md) corroborates toolchain family and linked
runtime. [Function matching](VC6_MATCHING.md) constrains compiler profiles;
original per-file switches are not established by a matching banner.
