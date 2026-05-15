#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

if [[ -x "./scripts/ci_android_preflight.sh" ]]; then
  ./scripts/ci_android_preflight.sh
fi

# Build both signed (when keys configured) and unsigned validation artifacts.
./scripts/build_apk_matrix.sh
