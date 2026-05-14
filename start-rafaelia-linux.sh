#!/usr/bin/env bash
set -euo pipefail
BASE_DIR="${PREFIX:-$HOME/.rafaelia-linux}"
LAUNCHER="${LAUNCHER_PATH:-$BASE_DIR/start-rafaelia-linux.sh}"
if [[ ! -x "$LAUNCHER" ]]; then
  echo "[ERRO] launcher não encontrado: $LAUNCHER"
  echo "Execute antes: ./install-rafaelia-linux.sh"
  exit 1
fi
exec "$LAUNCHER" "$@"
