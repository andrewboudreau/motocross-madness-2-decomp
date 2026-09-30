# Private VC6 SP3 execution environment

The public repository never contains Microsoft VC6 files or the retail game binary.
A private bundle is installed outside the checkout, verified byte-for-byte, and then
exposed to project commands through a wrapper.

## Pinned bundle

Current private bundle ID: `mcm2-vc6sp3-private-inputs-2026-09-29`

Archive SHA-256:

```text
ce25eecdb4e0b55020847a32c9bd2b6449dbd82ceb3c83e7b4de8b27cc48b209
```

The archive's own manifest contains 1,463 payload hashes. Installation refuses
path traversal/symlinks, verifies archive membership and every manifest hash,
then verifies the retail `mcm2.exe` hash. The URL/token are never committed.

The bundle contains an installed `VC98` tree, including headers and libraries,
and the known retail MCM2 executable. Key pinned hashes are kept in
`config/private_bundle_expected.json` so a future bundle change is explicit.

## Local Windows

Native Windows removes Wine as a variable and is the preferred first historical
compiler validation:

```powershell
# Either use a local archive...
powershell -ExecutionPolicy Bypass -File tools/setup_vc6_windows.ps1 `
  -Archive C:\private\mcm2-vc6sp3-private-inputs.zip

# ...or set the private URL only in the process/session and omit -Archive.
$env:MCM2_PRIVATE_BUNDLE_URL = '<private signed URL>'
powershell -ExecutionPolicy Bypass -File tools/setup_vc6_windows.ps1
```

The setup performs a real `CL.EXE` compilation and parses the result as nonempty
i386 COFF. A banner/fingerprint alone is not considered ready.

## Linux / Codex cloud setup

Use these setup-only secrets/environment values:

```text
MCM2_PRIVATE_BUNDLE_URL       private signed/object URL
MCM2_PRIVATE_BUNDLE_TOKEN     optional Bearer token; omit for signed URL
MCM2_PRIVATE_BUNDLE_SHA256    optional override; repository already pins current hash
MCM2_PRIVATE_ROOT             optional; defaults to ~/.cache/mcm2-private
```

Point the environment setup command at:

```bash
bash tools/setup_vc6_linux.sh
```

That script installs 32-bit Wine dependencies when necessary, downloads the
bundle without printing its URL, verifies it, initializes a dedicated win32 Wine
prefix, and runs the same real-compiler acceptance test.

Setup-time secrets do not belong in `.env`, GitHub Actions, issue comments, or
logs. Downloaded proprietary files persist in the worker filesystem, so treat
that worker/cache as private too.

## Running project commands with the private paths

Environment variables set in one setup shell are not assumed to persist. Use the
wrapper so commands always receive the correct paths:

```bash
python3 tools/with_private_env.py -- make vc6-gate
python3 tools/with_private_env.py -- python3 tools/vc6_acceptance.py --full-gate
```

Or through Make:

```bash
make private-ready
make vc6-private-gate
```

The wrapper provides:

```text
VC6_ROOT=<private root>/toolchains/vc6sp3
MCM2_EXE=<private root>/work/game/mcm2.exe
WINEPREFIX=<private root>/wine-vc6
WINEARCH=win32
WINEDEBUG=-all
```

## Readiness vs historical matching

Two gates remain separate:

1. **Private readiness**: bundle integrity + SP3 fingerprints + an authentic
   `CL.EXE` process producing parseable i386 COFF.
2. **MCM2 historical gate**: compile the existing smoke/easy/calibration corpus
   with that compiler and compare against retail.

A failed MCM2 byte match does not mean the private environment is broken. It is
compiler/profile/source-shape evidence. Conversely, a successful static bundle
check does not claim the compiler has executed.

When `--full-gate` is requested, its failing exit status propagates to the
caller while the report can still say `ready: true`. Run the analysis bootstrap
before the full gate: generated probes require `analysis/easy_targets.json`.
The historical gate now requires executed SP3 identity and strict generated
probe comparisons, including resolution of both global-load addresses.

## Current bundle facts verified outside Wine

The supplied archive was independently verified before these scripts were added:

- 1,463 manifest entries, 0 hash failures.
- Exact retail MCM2 SHA-256:
  `31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874`.
- `C1.DLL` SHA-256 `fb0234ad...e7dc95`.
- `C1XX.DLL` SHA-256 `69fc98ba...317262`.
- `C2.DLL` SHA-256 `22d3dd19...dc09c`.
- `LINK.EXE` SHA-256 `df858ec7...5e837`.
- Both `LIBC.LIB` and `LIBCMT.LIB` are present for later runtime-object analysis.

The initial Linux verification environment had no Wine, so that pass established
bundle integrity only. A subsequent native Windows run passed installation,
real compiler readiness, and the full manual/generated gate with the local COFF
function-length and strict-relocation changes. See `VC6_MATCHING.md` for the
results and remaining calibration failures. Linux/Wine execution is still
unverified; the acceptance script reports a missing Wine environment as not ready.
