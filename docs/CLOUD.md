# Cloud / container environment

`tools/cloud_setup.sh` provisions a fresh Ubuntu container (Claude Code on the
web, a throwaway VM) for VC6 byte matching. In Claude Code on the web it runs
on every session start and resume through `.claude/hooks/session-start.sh`.

It uses the same private bundle and tooling as `docs/PRIVATE_TOOLCHAIN.md`;
the repository still ships no proprietary bytes.

## Configure the environment once

In the cloud environment's settings:

1. Environment variable (quoted, because signed URLs contain `&` and `%`):
   ```
   MCM2_PRIVATE_BUNDLE_URL="https://…signed bundle URL…"
   ```
2. Network access: allow the bundle's host.

Settings reach sessions started after the change, not the current one.

Optional variables: `MCM2_PRIVATE_ROOT` (install location, default
`~/.cache/mcm2-private`) and `MCM2_PRIVATE_BUNDLE_SHA256` (overrides the hash
pinned in `config/private_bundle_expected.json`).

## What a session start does

Every step warns and continues on failure; the environment is written first.

1. Writes `work/cloud-env.sh` and the session environment (once):
   `PYTHONPATH`, `MCM2_PRIVATE_ROOT`, `VC6_ROOT`, `MCM2_EXE`, `WINEPREFIX`,
   `WINEARCH`, `WINEDEBUG`. The paths match `tools/with_private_env.py`.
2. apt: `wine`, `wine32:i386`, `clang` (+ `clang-cl` symlink), `lld`, `unzip`,
   `binutils`, `make`, only when missing.
3. If the bundle is not installed: downloads it with curl (URL passed on stdin,
   never printed) and installs it with `tools/install_private_bundle.py`, which
   checks the archive hash, every manifest entry and the `mcm2.exe` hash.
4. Symlinks `toolchains/vc6sp3` and `work/game` to the install, so the Makefile
   defaults and every git worktree use the same verified files. A real
   directory at either path is left alone with a warning.
5. Once per install: `tools/vc6_acceptance.py`, a real `CL.EXE` compile under
   Wine (this also creates the Wine prefix). Result in `work/vc6-acceptance.json`.
6. Once per checkout: `make analyze`, with the translation-unit skeletons
   written to `generated/krusty2-skeletons` so `src/krusty2` only holds
   promoted code, and the two tracked summary docs it rewrites restored.
   `analysis/` and `generated/` are added to the local `.git/info/exclude`.

A first start takes about 45 seconds; later starts and resumes take well under
a second.

Afterwards:

```bash
make status
make vc6-gate VC6_ROOT="$VC6_ROOT"
make private-ready          # same acceptance check, through with_private_env.py
```

Treat the container's cache as private: it holds the compiler and game binary
(see `docs/REPOSITORY_POLICY.md`).
