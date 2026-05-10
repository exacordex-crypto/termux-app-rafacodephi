#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="${ROOT_DIR}/dist/apk-matrix"
UNSIGNED_DIR="${OUT_DIR}/unsigned"
SIGNED_DIR="${OUT_DIR}/signed"
SHA_FILE="${OUT_DIR}/SHA256SUMS.txt"

TRACK="${RELEASE_TRACK:-official}"
BOOTSTRAP_STRICT="${BOOTSTRAP_BAREMETAL_STRICT:-true}"

info() { printf '\n[verify_release_contract] %s\n' "$*"; }
fail() { printf '\n[verify_release_contract] ERROR: %s\n' "$*" >&2; exit 1; }

[[ -d "${OUT_DIR}" ]] || fail "Missing ${OUT_DIR}. Run scripts/build_apk_matrix.sh first."
[[ -d "${UNSIGNED_DIR}" ]] || fail "Missing ${UNSIGNED_DIR}."
[[ -d "${SIGNED_DIR}" ]] || fail "Missing ${SIGNED_DIR}."
[[ -f "${SHA_FILE}" ]] || fail "Missing ${SHA_FILE}."

if [[ "${TRACK}" == "official" && "${BOOTSTRAP_STRICT}" != "true" ]]; then
  fail "Official release requires BOOTSTRAP_BAREMETAL_STRICT=true."
fi

required_release_abis=("armeabi-v7a" "arm64-v8a")
for abi in "${required_release_abis[@]}"; do
  signed_count="$(find "${SIGNED_DIR}" -maxdepth 1 -type f -name "*release*${abi}*-signed.apk" | wc -l | tr -d ' ')"
  [[ "${signed_count}" -gt 0 ]] || fail "Missing signed release APK for ${abi}."
done

if [[ "${TRACK}" == "official" ]]; then
  for abi in "${required_release_abis[@]}"; do
    unsigned_release_count="$(find "${UNSIGNED_DIR}" -maxdepth 1 -type f -name "*release*${abi}*.apk" | wc -l | tr -d ' ')"
    [[ "${unsigned_release_count}" -eq 0 ]] || fail "Unsigned release APK for ${abi} is forbidden on official track."
  done
else
  info "Internal track: unsigned artifacts are allowed."
fi

while IFS= read -r -d '' apk; do
  base="$(basename "${apk}")"
  [[ "${base}" == *.apk ]] || fail "Unexpected artifact extension: ${base}"
  if [[ "${apk}" == *"/signed/"* ]]; then
    [[ "${base}" == *"-signed.apk" ]] || fail "Signed artifact missing -signed suffix: ${base}"
    [[ "${base}" == *"release"* ]] || fail "Signed dir must contain release APKs only: ${base}"
  fi
  if [[ "${apk}" == *"/unsigned/"* ]]; then
    [[ "${base}" != *"-signed.apk" ]] || fail "Unsigned dir contains signed naming: ${base}"
  fi
  grep -Fq "  ${apk#${OUT_DIR}/}" "${SHA_FILE}" || fail "Missing SHA256SUMS entry for ${apk#${OUT_DIR}/}"
done < <(find "${UNSIGNED_DIR}" "${SIGNED_DIR}" -maxdepth 1 -type f -name '*.apk' -print0)

info "Release contract validated for track=${TRACK}."
