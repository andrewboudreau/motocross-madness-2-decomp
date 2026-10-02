# Build and private inputs

Use Python 3.10+ and authentic VC6 SP3. Native Windows and x86-64 Linux/wibo
execution are verified. Linux setup installs the pinned wibo runner. Wine remains
an alternative on hosts that support 32-bit Linux executables. GNU `objdump` is
needed for disassembly tools. Clang is optional for bootstrap comparisons; a
native C++ compiler enables behavior-model tests.

## Private bundle

Archive, EXE and component hashes are pinned in `config/private_bundle_expected.json`
and `config/vc6_sp3_expected.json`. Installation checks archive membership, all
1,463 payload hashes and the EXE, and rejects path traversal/symlinks. The bundle
contains an installed VC98 tree plus the target EXE, so no full installer is needed.
The bundle keeps six standard C++ headers under their 8.3 CD names (`XCEPTION`,
`ALGRITHM`, `FCTIONAL`, `STDXCEPT`, `STREAMBF`, `STRSTREM`). VC6 setup installs
them as `exception`, `algorithm`, etc., which `<typeinfo.h>` and the STL
include, so the installer (and `tools/cloud_setup.sh`, for older installs)
adds byte-identical long-name copies next to the verified files.

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

### Codex Cloud

Create an environment for this GitHub repository on `main`. Use an x86-64 Ubuntu
worker with Python 3.10+ and root/sudo package-install access. Keep the environment
private: its prepared filesystem includes the game EXE and Microsoft toolchain.

| Setting | Value |
|---|---|
| Install script | `bash tools/cloud_setup.sh --strict` |
| Start skill instructions | Run `bash tools/cloud_setup.sh --strict --no-apt`, then `python3 tools/with_private_env.py -- make status` from the repository root. |
| Direct environment variable | `MCM2_PRIVATE_BUNDLE_URL`: the private ZIP download URL |
| Network | Allow Ubuntu package repositories, PyPI, GitHub release downloads and the ZIP host during installation. |

The current [Codex Cloud interface](https://learn.chatgpt.com/docs/environments/cloud-environments)
uses **Install script** and **Start skill**. Configure the full signed URL as a
direct variable (optionally from Personal vault); the downloader must read a real
URL, not a network-secret placeholder. This value is accessible to environment
processes. The setup script does not print it or save it in its generated exports.
Review the setup results and publish the prepared environment. After changing
installed dependencies, edit and republish it for new tasks.

If your interface instead shows the [legacy setup/maintenance fields](https://learn.chatgpt.com/docs/environments/cloud-environment),
put `bash tools/cloud_setup.sh --strict` in **Setup script**,
`bash tools/cloud_setup.sh --strict --no-apt` in **Maintenance script**, and the
URL in a **Secret** named `MCM2_PRIVATE_BUNDLE_URL`. A cached startup uses installed
inputs without requiring the download secret again.

Setup installs x86-64 wibo, Clang, GNU disassembly/build tools, Capstone and a native
C++ compiler; verifies and installs the pinned private bundle; runs a real VC6
readiness compile; and generates analysis. `--strict` returns failure when a
required step fails. Logs are `work/vc6-acceptance.log` and
`work/cloud-analyze.log`; compiler details are in `work/vc6-acceptance.json`.
Readiness does not claim every reconstructed function matches. For byte checks:

```bash
python3 tools/with_private_env.py -- make vc6-gate
make static-check test
```

Setup exports do not carry into a separate task shell. Use the wrapper above,
or `source work/cloud-env.sh` in each shell before direct `make` commands. The
default cache is `~/.cache/mcm2-private`; set `MCM2_PRIVATE_ROOT` in environment
settings if a different persistent location is needed. Startup regenerates
analysis instead of trusting reports from the previously checked-out revision.

The Claude remote-session hook uses the same script in best-effort mode. Its
completion alone is not readiness; inspect the acceptance report. Linux setup
through `tools/setup_vc6_linux.sh` delegates to strict mode.

### Runner compatibility

If an older setup reports `/usr/lib/wine/wine: Exec format error`, update to
current `main` and reset the environment cache, then rerun the same setup script.
That failure occurs while launching the 32-bit Linux Wine executable, before
CL.EXE runs. Setup now uses [wibo 1.2.0](https://github.com/decompals/wibo/releases/tag/1.2.0),
an x86-64 host runner for the authentic 32-bit Windows compiler. Its download
hash is pinned in `tools/cloud_setup.sh`. Allow `github.com` and
`release-assets.githubusercontent.com` if setup networking is restricted.
`VC6_RUNNER=wine` retains the Wine route on compatible hosts; set it in environment
settings if needed. No clang fallback is used for VC6 checks.

### Docker

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
