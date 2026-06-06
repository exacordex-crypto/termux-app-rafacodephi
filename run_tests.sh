#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT_DIR"

python3 scripts/validate_side_by_side_contract.py

./gradlew --no-daemon :app:testDebugUnitTest :terminal-emulator:testDebugUnitTest :terminal-view:testDebugUnitTest

if [[ -x "./scripts/validate_top42_periodicity.sh" ]]; then
  ./scripts/validate_top42_periodicity.sh
fi

if [[ -x "./scripts/validate_blake3_rmr.sh" ]]; then
  ./scripts/validate_blake3_rmr.sh
fi
