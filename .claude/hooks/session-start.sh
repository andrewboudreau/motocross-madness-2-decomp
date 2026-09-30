#!/bin/bash
# Claude Code on the web: provision Wine/clang-cl, fetch the private MCM2
# EXE + VC6 SP3 tree (see tools/cloud_setup.sh for the env vars), and
# export WINEPREFIX/VC6_ROOT/PYTHONPATH into the session.
set -euo pipefail
if [ "${CLAUDE_CODE_REMOTE:-}" != "true" ]; then
  exit 0
fi
cd "$CLAUDE_PROJECT_DIR"
tools/cloud_setup.sh ${CLAUDE_ENV_FILE:+--env-file "$CLAUDE_ENV_FILE"}
