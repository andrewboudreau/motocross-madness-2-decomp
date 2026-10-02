#!/usr/bin/env bash
# Linux and cloud workers share one provisioning path.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec bash "$ROOT/tools/cloud_setup.sh" --strict "$@"
