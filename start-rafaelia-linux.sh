#!/usr/bin/env bash
set -euo pipefail
PREFIX_DEFAULT="${PREFIX:-$HOME/.termux}"
LAUNCHER="${LAUNCHER_PATH:-$PREFIX_DEFAULT/bin/start-rafaelia-linux.sh}"
if [[ ! -x "$LAUNCHER" ]]; then
  echo "[ERRO] launcher não encontrado: $LAUNCHER"
  echo "Execute antes: ./install-rafaelia-linux.sh"
  exit 1
fi
exec "$LAUNCHER" "$@"
