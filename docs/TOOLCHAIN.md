# Toolchain fingerprint and VC6 import

## Verified local setup

For new workers, use the pinned installer and process wrapper described in
[PRIVATE_TOOLCHAIN.md](PRIVATE_TOOLCHAIN.md). This verifies the archive into a
private cache outside the checkout and proves that the authentic compiler runs.
The older manual import and bundle-overlay workflows below remain usable.

The assembled SP3 tree has passed native Windows compile/link/run checks and
the function checks in [VC6_MATCHING.md](VC6_MATCHING.md). Compiler acquisition
is complete for this pass; remaining work is source/profile calibration and
relocation validation. Linux/Wine execution remains a separate check.

The private `mcm2-vc6sp3-private-inputs.zip` overlays an existing checkout with
`toolchains/vc6sp3/`, `work/game/mcm2.exe`, setup helpers, and per-file hashes.
Read `work/private-inputs/README.md` after extraction. No installer is needed
when this exact extracted EXE is already present. Do not use the default Docker
Compose command with this bundle: it invokes the installer-based bootstrap;
override it with the analysis helper or a gate command.

The SP3 disc stores the full compiler backend as `os/system/msvcep.dll`:
its original filename is `C2.DLL`, version `12.0.8447.0`. The installer selects
this file for the full compiler; `msse.dll` and `intro.dll` are different edition
variants. The verified tree combines the Professional build-8168 disc with its
German SP3 update and places `MSPDB60.DLL` beside `CL.EXE` for DLL discovery.
The unchanged driver banner is 8168; C1/C1XX are 8472 and C2 is 8447.
The German LINK text version is stale (8168), while its fixed version/build
and banner identify 8447. Inspect the components, not just a single string.

## Retail executable evidence

The MCM2 executable contains a Microsoft Rich header. Relevant records include:

| Product | Build | Count |
|---|---:|---:|
| Utc12_CPP | 8168 | 15 |
| Utc12_C | 8168 | 153 |
| Masm613 | 7299 | 36 |
| Linker600 | 8447 | 2 |
| **Utc12_CPP** | **8447** | **197** |
| Cvtres500 | 1735 | 1 |

The dominant game C++ record is therefore `Utc12_CPP / 8447`, strongly identifying the **Visual C++ 6.0 SP3 toolchain generation**.

Archived Microsoft KB **Q230733**, “Visual Studio 6.0 SP3 Readme: Part 9 - File Versions,” lists these key Visual C++ SP3 files:

- `c1.dll` -> `12.0.8472.0`
- `c1xx.dll` -> `12.0.8472.0`
- `c2.dll` -> `12.0.8447.0`
- `cvtres.exe` -> `5.0.1736.1`

MCM2's own Rich header independently records linker build `8447`. The `cl.exe` driver/banner alone is not a reliable SP-level discriminator, so the bootstrap fingerprints the component files and then uses emitted-code matching as the final authority.

Historical reference: https://helparchive.huntertur.net/document/104797

## Import a privately owned VC6 installation

Microsoft compiler binaries are intentionally **not** included in this repository or Docker image.

If you have a VC6 installation already patched to SP3, copy/archive the `VC98` tree and import it:

```bash
make import-vc6 VC6_SOURCE=/path/to/vc6-or-vc98.zip
```

Equivalent direct command:

```bash
PYTHONPATH=. python3 tools/import_vc6.py /path/to/source \
  --out toolchains/vc6sp3
```

The importer accepts an installed Visual Studio directory, a `VC98` directory, or a normal archive containing one. It normalizes the private cache to:

```text
toolchains/vc6sp3/
  VC98/
    Bin/
      CL.EXE
      C1.DLL
      C1XX.DLL
      C2.DLL
      LINK.EXE
      ...
    Include/
    Lib/
  toolchain-fingerprint.json
```

`toolchains/` is ignored by Git and Docker build context.

A practical workflow, also used by modern MSVC-under-Wine wrapper projects, is to install Visual Studio on a licensed Windows machine/VM and copy the installed compiler tree to Linux rather than fighting the old IDE installer under Wine. See https://github.com/fekir/wine-cl and https://github.com/mstorsjo/msvc-wine for modern examples of the general pattern.

## Probe and verify

```bash
export VC6_ROOT=$PWD/toolchains/vc6sp3
PYTHONPATH=. python3 tools/probe_vc6.py --vc6-root "$VC6_ROOT"
```

The probe records SHA-256 hashes and embedded version strings for the important compiler components, and executes `cl.exe` through Wine when Wine is available.

Initialize a dedicated 32-bit Wine prefix on a Linux host:

```bash
make wine-init
export WINEPREFIX=$PWD/work/wine-vc6
export WINEARCH=win32
```

Or use Docker Compose, which already configures a private VC6 mount at `/toolchains/vc6sp3`.

## One-command historical compiler gate

After bootstrap + VC6 import:

```bash
make vc6-gate VC6_ROOT=$PWD/toolchains/vc6sp3
```

That writes `analysis/vc6_gate.json` containing:

1. toolchain hashes/version probe;
2. all hand-written smoke targets compiled with VC6;
3. all generated high-confidence easy probes compiled with VC6;
4. compiler-calibration results including BaseObject special members and `BaseObject::Release`.

The gate fails if either the hand-written smoke corpus or generated high-confidence probe corpus is not exact. Calibration mismatches remain data until the source shape/profile is resolved.

## Flags

Only `/GR` is strongly indicated by the abundant MSVC RTTI. The initial profiles are hypotheses. Larger matching functions should empirically settle:

- `/O2` vs `/O1`;
- `/G5` vs `/G6`;
- `/ML` vs `/MT`;
- `/GX` on/off;
- `/Gy` function-level linking;
- inlining and frame-pointer behavior.

Do not select these from convention alone; use byte-match evidence.

VC6 profiles also include `/Z7` to expose COFF function lengths to the matcher.
This is measurement metadata, not a claimed original flag. See VC6_MATCHING.md
for the paired codegen checks and the distinction between code and alignment.
