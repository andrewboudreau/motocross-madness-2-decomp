#!/usr/bin/env bash
# Provision a Linux / Claude Code cloud container with everything needed to
# byte-match reconstructed source against MCM2:
#
#   * system packages: wine + wine32 (i386), clang-cl/lld, cabextract, 7z, libarchive
#   * a private 32-bit Wine prefix for VC6          -> work/wine-vc6
#   * the user-owned MCM2PCG.exe installer          -> input/MCM2PCG.exe
#   * bootstrap (extract + analysis + clang gates)  -> work/game/, analysis/
#   * a user-owned VC6 SP3 VC98 tree                -> toolchains/vc6sp3/
#   * VC6 probe                                     -> analysis/vc6_probe.json
#
# No proprietary bytes live in this repository. The two private inputs are
# fetched from locations you configure as environment variables (set them in
# the cloud environment's settings, not in git):
#
#   MCM2_PRIVATE_BUNDLE_URL     preferred: one ZIP with toolchains/vc6sp3/VC98 and
#                               work/game/mcm2.exe (+ work/private-inputs/SHA256SUMS.json).
#                               Installed once to $MCM2_PRIVATE_ROOT (default
#                               ~/.cache/mcm2-private) and symlinked into the checkout,
#                               so worktrees can share it (run this script in each).
#   MCM2_PRIVATE_BUNDLE_SHA256  optional override of the pinned bundle hash
#
# or, separately:
#
#   MCM2_INSTALLER_URL     URL (or local path) of MCM2PCG.exe
#   MCM2_INSTALLER_SHA256  optional; expected SHA-256 of that file
#   VC6_ARCHIVE_URL        URL (or local path) of an archive (.zip/.tar.*/.7z)
#                          containing an SP3-patched VC98 tree
#   VC6_ARCHIVE_SHA256     optional; expected SHA-256 of that archive
#   DOWNLOAD_AUTH_HEADER   optional; extra curl header for private hosting,
#                          e.g. "Authorization: Bearer <token>"
#
# Instead of URLs you can pre-place input/MCM2PCG.exe or toolchains/vc6sp3/.
# Every step is idempotent; missing inputs are reported and skipped, never fatal.
#
# Usage: tools/cloud_setup.sh [--no-apt] [--env-file PATH]
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

DO_APT=1
ENV_FILE=""
while [ $# -gt 0 ]; do
  case "$1" in
    --no-apt) DO_APT=0 ;;
    --env-file) ENV_FILE="$2"; shift ;;
    *) echo "unknown argument: $1" >&2; exit 2 ;;
  esac
  shift
done

RETAIL_EXE_SHA256=31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874
INSTALLER="$ROOT/input/MCM2PCG.exe"
VC6_ROOT="$ROOT/toolchains/vc6sp3"
BUNDLE_SHA256="${MCM2_PRIVATE_BUNDLE_SHA256:-ce25eecdb4e0b55020847a32c9bd2b6449dbd82ceb3c83e7b4de8b27cc48b209}"
PRIVATE_ROOT="${MCM2_PRIVATE_ROOT:-$HOME/.cache/mcm2-private}"
WINEPREFIX="$ROOT/work/wine-vc6"
export WINEARCH=win32 WINEDEBUG=-all WINEPREFIX PYTHONPATH="$ROOT"

log()  { printf '[cloud-setup] %s\n' "$*"; }
warn() { printf '[cloud-setup] WARNING: %s\n' "$*" >&2; }

fetch() {  # fetch <url-or-path> <dest> [sha256]
  local src="$1" dest="$2" want="${3:-}" tmp="$2.part"
  mkdir -p "$(dirname "$dest")"
  if [ -f "$src" ]; then
    cp -f "$src" "$tmp"
  else
    local hdr=()
    [ -n "${DOWNLOAD_AUTH_HEADER:-}" ] && hdr=(-H "$DOWNLOAD_AUTH_HEADER")
    curl -fL --retry 4 --retry-delay 2 "${hdr[@]}" -o "$tmp" "$src"
  fi
  if [ -n "$want" ]; then
    local got; got="$(sha256sum "$tmp" | cut -d' ' -f1)"
    if [ "$got" != "${want,,}" ]; then
      rm -f "$tmp"; warn "SHA-256 mismatch for $dest (got $got, want $want)"; return 1
    fi
  fi
  mv -f "$tmp" "$dest"
}

# ---------------------------------------------------------------- packages --
if [ "$DO_APT" = 1 ]; then
  need=()
  command -v wine >/dev/null        || need+=(wine)
  dpkg -s wine32:i386 >/dev/null 2>&1 || need+=(wine32:i386)
  command -v cabextract >/dev/null  || need+=(cabextract)
  command -v 7z >/dev/null          || need+=(p7zip-full)
  command -v unzip >/dev/null       || need+=(unzip)
  command -v clang >/dev/null       || need+=(clang)
  command -v lld-link >/dev/null    || need+=(lld)
  command -v make >/dev/null        || need+=(make)
  dpkg -s libarchive-dev >/dev/null 2>&1 || need+=(libarchive-dev)
  if [ ${#need[@]} -gt 0 ]; then
    SUDO=""; [ "$(id -u)" = 0 ] || SUDO="sudo"
    log "installing: ${need[*]}"
    dpkg --print-foreign-architectures | grep -qx i386 || $SUDO dpkg --add-architecture i386
    # Third-party PPAs may be unreachable behind the proxy; ignore their errors.
    $SUDO apt-get update -qq || true
    apt_install() { DEBIAN_FRONTEND=noninteractive $SUDO apt-get install -y -qq --no-install-recommends "$@"; }
    # A newer amd64 libgd3 from a PPA (e.g. ondrej/php on the cloud image) can
    # block wine32:i386 resolution; installing the archive's libgd3:i386 first
    # lets apt settle both architectures on the same version.
    apt_install "${need[@]}" || { apt_install libgd3:i386 && apt_install "${need[@]}"; }
  fi
fi

# Ubuntu ships clang-cl only as clang-cl-<N>; expose an unversioned name.
if ! command -v clang-cl >/dev/null; then
  cc="$(ls /usr/bin/clang-cl-* /usr/lib/llvm-*/bin/clang-cl 2>/dev/null | sort -V | tail -1 || true)"
  if [ -n "$cc" ]; then
    mkdir -p "$HOME/.local/bin"; ln -sf "$cc" "$HOME/.local/bin/clang-cl"
    export PATH="$HOME/.local/bin:$PATH"
  else
    warn "clang-cl not found; clang gates will be skipped"
  fi
fi

# --------------------------------------------------------------- wine prefix --
if command -v wine >/dev/null && [ ! -f "$WINEPREFIX/system.reg" ]; then
  log "initialising 32-bit Wine prefix at $WINEPREFIX"
  python3 tools/init_wine_prefix.py --prefix "$WINEPREFIX" >/dev/null 2>&1 \
    || warn "wineboot failed; VC6 compiles will not work until Wine is fixed"
fi

# ------------------------------------------------------------ private bundle --
if [ -n "${MCM2_PRIVATE_BUNDLE_URL:-}" ] && [ ! -f "$PRIVATE_ROOT/.installed-$BUNDLE_SHA256" ]; then
  log "fetching private bundle (URL not printed)"
  zip="$PRIVATE_ROOT.download.zip"
  if fetch "$MCM2_PRIVATE_BUNDLE_URL" "$zip" "$BUNDLE_SHA256"; then
    rm -rf "$PRIVATE_ROOT.tmp" && mkdir -p "$PRIVATE_ROOT.tmp"
    if unzip -Z1 "$zip" | grep -qvE '^(toolchains/vc6sp3/|work/(game|private-inputs)/)|(^|/)\.\.(/|$)'; then
      warn "bundle contains unexpected paths; refusing to install"
    elif unzip -q "$zip" -d "$PRIVATE_ROOT.tmp" && python3 - "$PRIVATE_ROOT.tmp" "$RETAIL_EXE_SHA256" <<'EOF'
import hashlib, json, sys
from pathlib import Path
root, want_exe = Path(sys.argv[1]), sys.argv[2]
h = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
sums = json.loads((root / 'work/private-inputs/SHA256SUMS.json').read_text())
bad = [k for k, v in sums.items() if not (root / k).is_file() or h(root / k) != v]
if h(root / 'work/game/mcm2.exe') != want_exe: bad.append('work/game/mcm2.exe (target hash)')
if bad: sys.exit('bundle verification failed: ' + ', '.join(bad[:5]))
print(f'[cloud-setup] bundle verified: {len(sums)} files')
EOF
    then
      rm -rf "$PRIVATE_ROOT" && mv "$PRIVATE_ROOT.tmp" "$PRIVATE_ROOT"
      touch "$PRIVATE_ROOT/.installed-$BUNDLE_SHA256"
    else
      warn "bundle extraction/verification failed"
    fi
    rm -rf "$PRIVATE_ROOT.tmp"
  else
    warn "could not fetch MCM2_PRIVATE_BUNDLE_URL"
  fi
  rm -f "$zip"
fi

if [ -f "$PRIVATE_ROOT/.installed-$BUNDLE_SHA256" ]; then
  # Link (never overwrite) the shared install into this checkout/worktree.
  mkdir -p toolchains work
  [ -e toolchains/vc6sp3 ] || ln -s "$PRIVATE_ROOT/toolchains/vc6sp3" toolchains/vc6sp3
  [ -e work/game ]         || ln -s "$PRIVATE_ROOT/work/game" work/game
  if [ ! -f analysis/function_manifest.json ] && [ -f work/game/mcm2.exe ]; then
    log "generating analysis from bundled mcm2.exe"
    # analyze also rewrites two tracked summary docs; keep the checkout clean
    # unless they already had local edits.
    docs=(docs/CLASS_DOSSIERS.md docs/WORK_QUEUE.md)
    clean=0; git diff --quiet -- "${docs[@]}" 2>/dev/null && clean=1
    make -s analyze >/dev/null || warn "make analyze failed"
    [ "$clean" = 1 ] && git checkout -q -- "${docs[@]}" 2>/dev/null || true
  fi
fi

# ---------------------------------------------------------------- installer --
if [ ! -f "$INSTALLER" ] && [ -n "${MCM2_INSTALLER_URL:-}" ]; then
  log "fetching MCM2 installer"
  fetch "$MCM2_INSTALLER_URL" "$INSTALLER" "${MCM2_INSTALLER_SHA256:-}" \
    || warn "could not fetch MCM2_INSTALLER_URL"
fi

if [ -f "$INSTALLER" ] && [ ! -f work/game/mcm2.exe ]; then
  log "bootstrapping from $INSTALLER"
  python3 tools/bootstrap.py "$INSTALLER" || warn "bootstrap failed"
fi

if [ -f work/game/mcm2.exe ]; then
  got="$(sha256sum work/game/mcm2.exe | cut -d' ' -f1)"
  [ "$got" = "$RETAIL_EXE_SHA256" ] \
    || warn "work/game/mcm2.exe SHA-256 $got differs from the documented target $RETAIL_EXE_SHA256"
else
  warn "no game executable: set MCM2_PRIVATE_BUNDLE_URL or MCM2_INSTALLER_URL"
fi

# ---------------------------------------------------------------------- VC6 --
if ! ls "$VC6_ROOT"/VC98/[Bb][Ii][Nn]/[Cc][Ll].[Ee][Xx][Ee] >/dev/null 2>&1 && [ -n "${VC6_ARCHIVE_URL:-}" ]; then
  name="$(basename "${VC6_ARCHIVE_URL%%\?*}")"
  case "$name" in *.zip|*.tar|*.tar.gz|*.tgz|*.tar.bz2|*.tbz2|*.tar.xz|*.txz|*.7z|*.iso) ;; *) name="vc6sp3.zip" ;; esac
  archive="$ROOT/work/downloads/$name"
  log "fetching VC6 archive"
  if fetch "$VC6_ARCHIVE_URL" "$archive" "${VC6_ARCHIVE_SHA256:-}"; then
    python3 tools/import_vc6.py "$archive" --out "$VC6_ROOT" --overwrite || warn "VC6 import failed"
    rm -f "$archive"
  else
    warn "could not fetch VC6_ARCHIVE_URL"
  fi
fi

if ls "$VC6_ROOT"/VC98/[Bb][Ii][Nn]/[Cc][Ll].[Ee][Xx][Ee] >/dev/null 2>&1; then
  if [ ! -f analysis/vc6_probe.json ] && command -v wine >/dev/null; then
    log "probing VC6"; mkdir -p analysis
    python3 tools/probe_vc6.py --vc6-root "$VC6_ROOT" > analysis/vc6_probe.json \
      || warn "VC6 probe failed (see analysis/vc6_probe.json)"
  fi
else
  warn "no VC6 tree: set MCM2_PRIVATE_BUNDLE_URL or VC6_ARCHIVE_URL"
fi

# -------------------------------------------------------------- environment --
exports=(
  "export PYTHONPATH=\"$ROOT\""
  "export WINEARCH=win32"
  "export WINEDEBUG=-all"
  "export WINEPREFIX=\"$WINEPREFIX\""
  "export VC6_ROOT=\"$VC6_ROOT\""
  "export PATH=\"$HOME/.local/bin:\$PATH\""
)
mkdir -p work
printf '%s\n' "${exports[@]}" > work/cloud-env.sh
if [ -n "$ENV_FILE" ]; then printf '%s\n' "${exports[@]}" >> "$ENV_FILE"; fi

log "done. Next: make status; make vc6-gate VC6_ROOT=\"$VC6_ROOT\""
