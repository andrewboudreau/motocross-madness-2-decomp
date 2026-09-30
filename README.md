# Motocross Madness 2 decomp bootstrap — v0.6

Linux-first tooling for reconstructing and byte-matching the native x86 C++ code in **Motocross Madness 2** from a user-owned `MCM2PCG.exe` installer.

```text
retail installer
  -> embedded CAB / mcm2.exe
  -> PE + Rich header + RTTI + all primary/secondary vtables
  -> source-unit / class / layout evidence
  -> candidate C++
  -> VC6 SP3 under Wine
  -> COFF .obj
  -> relocation-aware exact matcher
```

## Current verified state

Native Windows VC6 SP3 matching now passes **19/19 hand-written samples** without
relocation masking and **39/39 generated probes** with address relocations resolved.
Calibration passes **8/16**. See [the executed checks and remaining gaps](docs/VC6_MATCHING.md).
The bootstrap inventory below also includes earlier clang results.

For the supplied installer:

- 104 shipped files recovered from the embedded CAB
- retail `mcm2.exe` SHA-256 `31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`
- native PE32/i386, image base `0x00400000`, linker `6.00`
- build timestamp `2000-05-31 21:39:06 UTC`
- original source root `D:\aardvark\VC\krusty2\`
- **107 `.cpp` + 4 headers** recovered by original filename
- **252 RTTI type descriptors / 249 logical class hierarchies**
- **271 vtables**, including **22 secondary base-subobject vtables**
- **17** multiple-inheritance / multi-vtable classes
- **145** canonical VC6 scalar deleting-destructor wrappers
- **28** vtable `this`-adjustor thunks
- **492** decoded vtable-write sites
- **57** mechanically classified small functions
- **39/39** automatically generated high-confidence C++ probes exact under clang-cl's x86 MSVC ABI
- **19/19** hand-written bootstrap samples exact
- **40 unique exact retail functions** across the two gates
- **16** explicit historical-compiler calibration targets
- dominant Rich record `Utc12_CPP / build 8447 / 197 objects`, consistent with **Visual C++ 6.0 SP3 generation**

The important v0.4+ correction is preservation of every MSVC `CompleteObjectLocator`. v0.3 could collapse secondary vtables on multiple-inheritance classes; v0.4 does not.

## One-command bootstrap

Requirements are Python 3 and one CAB extraction path (`cabextract`, `7z`, or libarchive headers + a C compiler). No Python packages are required.

```bash
python3 tools/bootstrap.py /path/to/MCM2PCG.exe
# or
make bootstrap INSTALLER=/path/to/MCM2PCG.exe
```

The bootstrap extracts the retail files, analyzes the executable, generates class evidence, recognizes MSVC destructor/thunk artifacts, finds vtable writes, runs the hand-written and generated clang gates, records compiler-calibration baselines, and rebuilds the agent function manifest.

If you already have the private input ZIP, extract it into the checkout and use
its `work/private-inputs/README.md` and `bootstrap_analysis.py`. It contains the
target EXE and compiler inputs; the full installer is unnecessary for analysis.

## Fast status / gates

```bash
make status
make smoke          # hand-written exact samples
make easy-smoke     # generated high-confidence probes
make calibration    # compiler-sensitive targets; mismatch under clang is expected
```

## Agent queue

Generated analysis artifacts are intentionally not committed. Bootstrap a user-owned installer locally, then use:

```bash
make status
make selftest
make easy
make manifest
make work-queue

PYTHONPATH=. python3 tools/discover_easy_targets.py --class UIControl
PYTHONPATH=. python3 tools/inspect_target.py --class UIControl --slot 34
PYTHONPATH=. python3 tools/nearest_source.py 0x4703c0
```

Nearby `__FILE__` references are hints, not proof of translation-unit ownership.

## Bring your own VC6 SP3 tree

The repo deliberately does **not** redistribute Microsoft compiler binaries.

```bash
make import-vc6 VC6_SOURCE=/path/to/vc98-or-archive
make wine-init

export VC6_ROOT=$PWD/toolchains/vc6sp3
export WINEPREFIX=$PWD/work/wine-vc6
export WINEARCH=win32

make probe-vc6 VC6_ROOT="$VC6_ROOT"
make vc6-gate VC6_ROOT="$VC6_ROOT"
```

See `docs/TOOLCHAIN.md`. For a fresh cloud container, see `docs/CLOUD.md` (`tools/cloud_setup.sh`).

## Evidence policy

RTTI names, source-path strings, COL/vtable offsets, target bytes, and decoded machine behavior are evidence. Semantic names and types remain provisional until corroborated. Do not use inline assembly, copied bytes, naked functions, or linker tricks just to manufacture matches.

See `AGENTS.md` and `docs/CLASS_MODEL.md`.


## Promoted reconstruction

`src/reconstructed/BaseObject.{h,cpp}` is the first promoted class slice.
Class identity, primary vtable, `this+0x04` storage, constructor initialization,
destructor core, AddRef behavior, and GetRefCount behavior are directly evidenced.
Release remains a historical-compiler calibration target.

The active camera slice now has exact FollowCamera preset methods plus a state
machine and a strong MSVC hidden-return-buffer interpretation for slot 69. See
`docs/V06_CAMERA_UI.md`, `docs/V06_FOLLOW_CAMERA_MODE.md`, and
`docs/RECONSTRUCTION_STATUS.md`.
