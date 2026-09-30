#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PRIVATE_ROOT="${MCM2_PRIVATE_ROOT:-$HOME/.cache/mcm2-private}"

if ! command -v wine >/dev/null 2>&1 || ! command -v winepath >/dev/null 2>&1; then
  if ! command -v apt-get >/dev/null 2>&1; then
    echo "wine/winepath missing and apt-get unavailable" >&2
    exit 2
  fi
  SUDO=""
  if [[ "$(id -u)" != 0 ]]; then
    command -v sudo >/dev/null 2>&1 || { echo "sudo required to install Wine" >&2; exit 2; }
    SUDO=sudo
  fi
  $SUDO dpkg --add-architecture i386
  $SUDO apt-get update
  $SUDO apt-get install -y --no-install-recommends wine wine32:i386 binutils unzip ca-certificates
fi

python3 "$ROOT/tools/install_private_bundle.py"   --url-env MCM2_PRIVATE_BUNDLE_URL   --root "$PRIVATE_ROOT"

python3 "$ROOT/tools/with_private_env.py" --root "$PRIVATE_ROOT" --   python3 "$ROOT/tools/vc6_acceptance.py" --root "$PRIVATE_ROOT"

echo
printf 'VC6 private worker ready.\n'
printf 'Use: python3 tools/with_private_env.py --root %q -- <command>\n' "$PRIVATE_ROOT"
