#!/usr/bin/env bash
# Provision an Ubuntu cloud worker for VC6 byte matching.
# --strict fails setup on errors and refreshes readiness/analysis on every run.
# The Claude session hook uses the default best-effort mode.
#
#   * apt: wine + wine32 (i386), clang-cl/lld, unzip, binutils, make
#   * private bundle (VC98 tree + mcm2.exe) from MCM2_PRIVATE_BUNDLE_URL,
#     verified and installed by tools/install_private_bundle.py into
#     $MCM2_PRIVATE_ROOT (default ~/.cache/mcm2-private) - see docs/TOOLCHAIN.md
#   * toolchains/vc6sp3 and work/game symlinked into this checkout/worktree
#   * tools/vc6_acceptance.py: real CL.EXE compile under Wine (creates the prefix)
#   * make analyze, so analysis/ exists when the session starts
#
# The bundle URL is a secret: set it in the environment settings, never in git.
# It is passed to curl through stdin, so it does not appear in logs or `ps`.
#
# Usage: bash tools/cloud_setup.sh [--strict] [--no-apt] [--env-file PATH]
# Cached startup: bash tools/cloud_setup.sh --strict --no-apt
set +x # Never trace commands that expand the private download URL.
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

DO_APT=1
STRICT=0
ENV_FILE=""
while [ $# -gt 0 ]; do
  case "$1" in
    --strict) STRICT=1 ;;
    --no-apt) DO_APT=0 ;;
    --env-file)
      [ $# -ge 2 ] && [ -n "$2" ] || { echo '--env-file requires a path' >&2; exit 2; }
      ENV_FILE="$2"; shift ;;
    *) echo "unknown argument: $1" >&2; exit 2 ;;
  esac
  shift
done
if [ "$STRICT" = 1 ]; then set -e; fi

log()  { printf '[cloud-setup] %s\n' "$*"; }
warn() { printf '[cloud-setup] WARNING: %s\n' "$*" >&2; }
fail() { warn "$*"; if [ "$STRICT" = 1 ]; then exit 1; fi; }

PRIVATE_ROOT="${MCM2_PRIVATE_ROOT:-$HOME/.cache/mcm2-private}"
MARKER="$PRIVATE_ROOT/private-inputs-state.json"
BUNDLE_SHA256="${MCM2_PRIVATE_BUNDLE_SHA256:-$(python3 -c 'import json;print(json.load(open("config/private_bundle_expected.json"))["archive_sha256"])' 2>/dev/null)}"
installed() {
  python3 - "$MARKER" "$BUNDLE_SHA256" <<'PY'
import json, sys
from pathlib import Path
marker = Path(sys.argv[1])
try:
    state = json.loads(marker.read_text())
    valid = bool(sys.argv[2]) and state.get('archive_sha256') == sys.argv[2]
    valid = valid and (marker.parent / 'work/game/mcm2.exe').is_file()
    valid = valid and (marker.parent / 'toolchains/vc6sp3/VC98/BIN/CL.EXE').is_file()
except (OSError, ValueError, AttributeError):
    valid = False
sys.exit(0 if valid else 1)
PY
}

# ---------------------------------------------------------------- environment --
# Written first so the session gets it even if a later step fails. Paths match
# tools/with_private_env.py, so both routes share one install and Wine prefix.
exports=(
  "export PYTHONPATH=$(printf '%q' "$ROOT")"
  "export MCM2_PRIVATE_ROOT=$(printf '%q' "$PRIVATE_ROOT")"
  "export VC6_ROOT=$(printf '%q' "$PRIVATE_ROOT/toolchains/vc6sp3")"
  "export MCM2_EXE=$(printf '%q' "$PRIVATE_ROOT/work/game/mcm2.exe")"
  "export WINEPREFIX=$(printf '%q' "$PRIVATE_ROOT/wine-vc6")"
  "export WINEARCH=win32"
  "export WINEDEBUG=-all"
  "export PATH=$(printf '%q' "$HOME/.local/bin"):\"\$PATH\""
)
mkdir -p work
printf '%s\n' "${exports[@]}" > work/cloud-env.sh
if [ -n "$ENV_FILE" ] && ! grep -qsxF "${exports[0]}" "$ENV_FILE"; then
  printf '%s\n' "${exports[@]}" >> "$ENV_FILE"
fi
# shellcheck disable=SC1091
. work/cloud-env.sh

# ------------------------------------------------------------------- packages --
if [ "$DO_APT" = 1 ]; then
  need=()
  command -v wine >/dev/null             || need+=(wine)
  dpkg -s wine32:i386 >/dev/null 2>&1    || need+=(wine32:i386)
  command -v winepath >/dev/null         || need+=(wine)
  command -v unzip >/dev/null            || need+=(unzip)
  command -v objdump >/dev/null          || need+=(binutils)
  command -v make >/dev/null             || need+=(make)
  command -v g++ >/dev/null              || need+=(g++)
  command -v curl >/dev/null             || need+=(curl)
  dpkg -s ca-certificates >/dev/null 2>&1 || need+=(ca-certificates)
  command -v clang >/dev/null            || need+=(clang)
  command -v lld-link >/dev/null         || need+=(lld)
  if [ ${#need[@]} -gt 0 ]; then
    SUDO=""; [ "$(id -u)" = 0 ] || SUDO="sudo"
    log "installing: ${need[*]}"
    dpkg --print-foreign-architectures | grep -qx i386 || $SUDO dpkg --add-architecture i386
    # Third-party PPAs on the image may be unreachable behind the proxy.
    $SUDO apt-get update -qq >/dev/null 2>&1 || true
    apt_install() { $SUDO env DEBIAN_FRONTEND=noninteractive apt-get install -y -qq --no-install-recommends "$@" >/dev/null; }
    # A newer amd64 libgd3 from a PPA (e.g. ondrej/php) can block wine32:i386;
    # installing the archive's libgd3:i386 first lets apt settle both arches.
    apt_install "${need[@]}" || { apt_install libgd3:i386 && apt_install "${need[@]}"; } \
      || fail "apt install failed: ${need[*]}"
  fi
fi

# tools/disasm_fn.py needs capstone (PyPI is reachable through the proxy).
if [ "$DO_APT" = 1 ] && ! python3 -c 'import capstone' 2>/dev/null; then
  python3 -m pip install -q capstone >work/cloud-python.log 2>&1 \
    || fail "pip install capstone failed; see work/cloud-python.log"
fi

# Ubuntu ships clang-cl only as clang-cl-<N>; expose an unversioned name.
if ! command -v clang-cl >/dev/null; then
  cc="$(ls /usr/bin/clang-cl-* 2>/dev/null | sort -V | tail -1 || true)"
  if [ -n "$cc" ]; then mkdir -p "$HOME/.local/bin" && ln -sfn "$cc" "$HOME/.local/bin/clang-cl"
  else fail "clang-cl not found; install clang"; fi
fi

if [ "$STRICT" = 1 ]; then
  for command in wine winepath objdump make g++ curl clang-cl; do
    command -v "$command" >/dev/null || fail "missing $command; rerun without --no-apt"
  done
fi

# ------------------------------------------------------------- private bundle --
if [ -n "${MCM2_PRIVATE_BUNDLE_URL:-}" ] && ! installed; then
  log "fetching private bundle (URL not printed)"
  zip="$PRIVATE_ROOT.download.zip"
  mkdir -p "$(dirname "$zip")"
  if printf 'url = "%s"\n' "$MCM2_PRIVATE_BUNDLE_URL" \
       | curl -fsSL --retry 4 --retry-delay 2 -K - -o "$zip"; then
    python3 tools/install_private_bundle.py --archive "$zip" --archive-sha256 "$BUNDLE_SHA256" \
      --root "$PRIVATE_ROOT" --overwrite >/dev/null \
      && log "bundle verified and installed" || { rm -f "$zip"; fail "bundle verification/install failed"; }
  else
    rm -f "$zip"
    fail "could not download MCM2_PRIVATE_BUNDLE_URL (check network access for its host)"
  fi
  rm -f "$zip"
elif [ -z "${MCM2_PRIVATE_BUNDLE_URL:-}" ] && ! installed; then
  fail "MCM2_PRIVATE_BUNDLE_URL is not set; VC6 and mcm2.exe unavailable (docs/TOOLCHAIN.md)"
fi

if installed; then
  # Link the shared install into this checkout/worktree. Replace stale or
  # dangling links; leave real directories (e.g. a manual unzip) alone.
  mkdir -p toolchains work
  for pair in "toolchains/vc6sp3:toolchains/vc6sp3" "work/game:work/game"; do
    link="${pair%%:*}"; target="$PRIVATE_ROOT/${pair#*:}"
    if [ -L "$link" ] || [ ! -e "$link" ]; then ln -sfn "$target" "$link"
    else warn "$link is a real directory, not the verified install; remove it to use $target"; fi
  done

  # Strict startup rechecks execution; a cached marker is not current evidence.
  if [ "$STRICT" = 1 ] || [ ! -f "$PRIVATE_ROOT/.accepted-$BUNDLE_SHA256" ]; then
    log "VC6 acceptance compile"
    if python3 tools/with_private_env.py --root "$PRIVATE_ROOT" -- \
         python3 tools/vc6_acceptance.py --root "$PRIVATE_ROOT" --out work/vc6-acceptance.json >work/vc6-acceptance.log 2>&1; then
      touch "$PRIVATE_ROOT/.accepted-$BUNDLE_SHA256"
    else
      tail -n 20 work/vc6-acceptance.log >&2
      fail "VC6 acceptance failed; see work/vc6-acceptance.log and work/vc6-acceptance.json"
    fi
  fi

  # Generated reports and filename skeletons stay outside tracked docs/source.
  if [ "$STRICT" = 1 ] || [ ! -f analysis/.cloud-analyze-ok ]; then
    log "generating analysis from mcm2.exe"
    if make -s analyze >work/cloud-analyze.log 2>&1; then touch analysis/.cloud-analyze-ok
    else
      tail -n 20 work/cloud-analyze.log >&2
      fail "make analyze failed; see work/cloud-analyze.log"
    fi
  fi
fi

log "done. Next: source work/cloud-env.sh; make status; make vc6-gate"
exit 0
