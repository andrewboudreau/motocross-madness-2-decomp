# Cloud / container environment

`tools/cloud_setup.sh` provisions a fresh Ubuntu container (Claude Code on the
web, CI, a throwaway VM) with everything needed for VC6 byte matching. The
repository still ships no proprietary bytes: the two private inputs are fetched
from locations you configure outside Git.

| Variable | Required | Meaning |
|---|---|---|
| `MCM2_PRIVATE_BUNDLE_URL` | preferred | URL or local path of the private bundle ZIP (`toolchains/vc6sp3/VC98` + `work/game/mcm2.exe` + `work/private-inputs/SHA256SUMS.json`) |
| `MCM2_PRIVATE_BUNDLE_SHA256` | no | overrides the bundle hash pinned in `config/private_bundle_expected.json` |
| `MCM2_PRIVATE_ROOT` | no | install location for the bundle, default `~/.cache/mcm2-private` |
| `MCM2_INSTALLER_URL` | yes* | URL or local path of your `MCM2PCG.exe` |
| `MCM2_INSTALLER_SHA256` | no | expected hash of the installer; download is rejected on mismatch |
| `VC6_ARCHIVE_URL` | yes* | URL or local path of an archive (`.zip`, `.tar.*`, `.7z`) containing an SP3-patched `VC98` tree |
| `VC6_ARCHIVE_SHA256` | no | expected hash of that archive |
| `DOWNLOAD_AUTH_HEADER` | no | extra curl header for private hosting, e.g. `Authorization: Bearer …` |

The bundle replaces the installer + VC6 archive pair below. It is downloaded
with curl and installed by `tools/install_private_bundle.py` (archive hash,
every manifest entry, target `mcm2.exe` hash; see `docs/PRIVATE_TOOLCHAIN.md`),
so `tools/with_private_env.py` and `make private-ready` see the same install. It
lives outside the checkout, and symlinked into `toolchains/vc6sp3`
and `work/game`, so a git worktree gets the same files by running the script
again. With the bundle present the script also runs `make analyze` (restoring
the two tracked summary docs it rewrites).

\* Alternatively pre-place `input/MCM2PCG.exe` / `toolchains/vc6sp3/VC98`.

What it does (idempotent; missing inputs warn, never fail):

1. apt: `wine`, `wine32:i386`, `clang` (+ `clang-cl` symlink), `lld`, `cabextract`, `p7zip-full`, `unzip`, `libarchive-dev`;
2. 32-bit Wine prefix at `work/wine-vc6`;
3. fetch installer → `input/MCM2PCG.exe` → `tools/bootstrap.py` (extraction, analysis, clang gates), then checks `work/game/mcm2.exe` against the documented SHA-256;
4. fetch VC6 archive → `tools/import_vc6.py` → `toolchains/vc6sp3` → `tools/probe_vc6.py` → `analysis/vc6_probe.json`;
5. writes `work/cloud-env.sh` (and `$CLAUDE_ENV_FILE` when run as a hook) exporting `PYTHONPATH`, `WINEPREFIX`, `WINEARCH`, `VC6_ROOT`.

## Claude Code on the web

`.claude/hooks/session-start.sh` runs the script on every remote session start
(synchronously). Set the variables above in the cloud environment's settings,
and make sure the environment's network access allows the host(s) serving the
two files. Package installs use the Ubuntu archive, which the default policy
allows. Afterwards:

```bash
make status
make vc6-gate VC6_ROOT="$VC6_ROOT"
```

Host the archives somewhere private to you (a private release asset, a
presigned bucket URL, etc.); the Microsoft and game binaries must not be
published or committed (see `docs/REPOSITORY_POLICY.md`).
