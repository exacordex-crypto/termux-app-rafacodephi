#!/usr/bin/env bash
set -euo pipefail

log(){ printf '[runtime-shell-diagnose] %s\n' "$*"; }
fail(){ printf '[runtime-shell-diagnose] ERROR: %s\n' "$*" >&2; exit 1; }

prefix="${PREFIX:-${TERMUX_PREFIX:-}}"
abi="$(getprop ro.product.cpu.abi 2>/dev/null || uname -m || true)"

log "ABI: ${abi:-unknown}"
log "PREFIX=${PREFIX:-<unset>}"
log "TERMUX_PREFIX=${TERMUX_PREFIX:-<unset>}"

[[ -n "$prefix" ]] || fail "PREFIX/TERMUX_PREFIX not set. Export PREFIX before running diagnostics."
[[ -d "$prefix" ]] || fail "PREFIX directory not found: $prefix"

check_candidates(){
  local label="$1"; shift
  local found=""
  local candidate
  for candidate in "$@"; do
    if [[ -e "$prefix/$candidate" ]]; then
      log "$label candidate found: $prefix/$candidate"
      if [[ -x "$prefix/$candidate" ]]; then
        found="$prefix/$candidate"
        break
      fi
      log "$label candidate exists but is not executable: $prefix/$candidate"
    fi
  done
  [[ -n "$found" ]] || fail "Missing executable $label (checked: $*)"
  printf '%s\n' "$found"
}

shell_path="$(check_candidates shell bin/sh usr/bin/sh)"
pkg_path="$(check_candidates pkg bin/pkg usr/bin/pkg)"
check_candidates proot bin/proot usr/bin/proot >/dev/null
check_candidates busybox bin/busybox usr/bin/busybox >/dev/null

log "Running shell smoke test via: $shell_path"
"$shell_path" -lc 'echo RAFAELIA_RUNTIME_SHELL_OK'

log "Runtime shell diagnostics passed. shell=$shell_path pkg=$pkg_path"
