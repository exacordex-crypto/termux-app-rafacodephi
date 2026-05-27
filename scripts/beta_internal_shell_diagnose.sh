#!/usr/bin/env bash
set -u

PACKAGE_NAME="com.termux.rafacodephi"
REPORT_PATH="reports/beta_internal_shell_report.md"
mkdir -p reports

STATUS_PASS=0
STATUS_WARN=0
STATUS_FAIL=0
STATUS_BLOCKER=0

now_utc() { date -u +"%Y-%m-%dT%H:%M:%SZ"; }

append() { printf "%s\n" "$1" >> "$REPORT_PATH"; }

record() {
  local level="$1" title="$2" detail="$3"
  case "$level" in
    PASS) STATUS_PASS=$((STATUS_PASS+1));;
    WARN) STATUS_WARN=$((STATUS_WARN+1));;
    FAIL) STATUS_FAIL=$((STATUS_FAIL+1));;
    BLOCKER) STATUS_BLOCKER=$((STATUS_BLOCKER+1));;
  esac
  append "- **${level}** ${title}: ${detail}"
}

run_adb() {
  local cmd="$1"
  adb shell "$cmd" 2>&1
}

: > "$REPORT_PATH"
append "# Beta Internal Shell Diagnose Report"
append ""
append "- Generated at (UTC): $(now_utc)"
append "- Package: $PACKAGE_NAME"
append ""
append "## Checks"

if ! command -v adb >/dev/null 2>&1; then
  record "BLOCKER" "ADB" "adb command not found in host environment."
  append ""
  append "## Summary"
  append "- PASS: $STATUS_PASS"
  append "- WARN: $STATUS_WARN"
  append "- FAIL: $STATUS_FAIL"
  append "- BLOCKER: $STATUS_BLOCKER"
  echo "Report generated: $REPORT_PATH"
  exit 0
fi

if ! adb get-state >/dev/null 2>&1; then
  record "BLOCKER" "ADB device" "No authorized device connected."
else
  record "PASS" "ADB device" "Device connected and authorized."
fi

pkg_installed=$(adb shell pm list packages 2>/dev/null | tr -d '\r' | grep -Fx "package:${PACKAGE_NAME}" || true)
if [ -n "$pkg_installed" ]; then
  record "PASS" "Package installed" "$PACKAGE_NAME is installed."
else
  record "BLOCKER" "Package installed" "$PACKAGE_NAME not installed."
fi

check_prop() {
  local prop="$1"
  local val
  val=$(run_adb "getprop $prop" | tr -d '\r')
  if [ -n "$val" ]; then
    record "PASS" "$prop" "$val"
  else
    record "WARN" "$prop" "Empty property."
  fi
}

check_prop ro.product.cpu.abi
check_prop ro.product.cpu.abilist
check_prop ro.build.version.sdk

uname_out=$(run_adb "uname -a" | tr -d '\r')
[ -n "$uname_out" ] && record "PASS" "uname -a" "$uname_out" || record "WARN" "uname -a" "No output."

page_size=$(run_adb "getconf PAGE_SIZE" | tr -d '\r')
if echo "$page_size" | grep -Eq '^[0-9]+$'; then
  record "PASS" "PAGE_SIZE" "$page_size"
else
  record "WARN" "PAGE_SIZE" "Could not parse PAGE_SIZE: $page_size"
fi

pkg_dump=$(adb shell dumpsys package "$PACKAGE_NAME" 2>/dev/null | tr -d '\r' || true)
for key in nativeLibraryDir primaryCpuAbi dataDir; do
  val=$(printf "%s\n" "$pkg_dump" | awk -F= -v k="$key" '$1 ~ k {print $2; exit}')
  if [ -n "$val" ]; then
    record "PASS" "$key" "$val"
  else
    record "WARN" "$key" "Not found in dumpsys package."
  fi
done

BASE="/data/data/${PACKAGE_NAME}/files/usr"
for p in "$BASE" "$BASE/bin/sh" "$BASE/bin/pkg" "$BASE/bin/bash" "$BASE/bin/proot" "$BASE/bin/busybox"; do
  out=$(run_adb "if [ -e '$p' ]; then ls -l '$p'; else echo MISSING; fi" | tr -d '\r')
  if echo "$out" | grep -q "MISSING"; then
    if [ "$p" = "$BASE/bin/proot" ] || [ "$p" = "$BASE/bin/busybox" ]; then
      record "WARN" "Path exists: $p" "Missing optional component."
    else
      record "FAIL" "Path exists: $p" "Missing required component."
    fi
  else
    record "PASS" "Path exists: $p" "$out"
    if echo "$out" | awk '{print $1}' | grep -q x; then
      record "PASS" "Executable bit: $p" "Executable permission present."
    else
      record "FAIL" "Executable bit: $p" "Not executable."
    fi
  fi
done

sh_exec=$(run_adb "run-as $PACKAGE_NAME $BASE/bin/sh -c 'echo RAF_SHELL_OK'" | tr -d '\r' || true)
if echo "$sh_exec" | grep -q "RAF_SHELL_OK"; then
  record "PASS" "sh execution" "$sh_exec"
else
  sh_exec_fb=$(run_adb "$BASE/bin/sh -c 'echo RAF_SHELL_OK'" | tr -d '\r' || true)
  if echo "$sh_exec_fb" | grep -q "RAF_SHELL_OK"; then
    record "PASS" "sh execution" "$sh_exec_fb"
  else
    record "FAIL" "sh execution" "Failed to run sh. Output: ${sh_exec} / fallback: ${sh_exec_fb}"
  fi
fi

pkg_ver=$(run_adb "$BASE/bin/pkg --version" | tr -d '\r' || true)
if [ -n "$pkg_ver" ] && ! echo "$pkg_ver" | grep -qiE "not found|No such file|Permission denied"; then
  record "PASS" "pkg --version" "$pkg_ver"
else
  record "FAIL" "pkg --version" "Could not execute pkg: $pkg_ver"
fi

if run_adb "[ -e '$BASE/bin/proot' ] && [ -x '$BASE/bin/proot' ]" >/dev/null 2>&1; then
  proot_ver=$(run_adb "$BASE/bin/proot --version" | tr -d '\r' || true)
  if [ -n "$proot_ver" ] && ! echo "$proot_ver" | grep -qiE "not found|No such file|Permission denied"; then
    record "PASS" "proot --version" "$proot_ver"
  else
    record "WARN" "proot --version" "proot exists but failed to execute (non-blocking for first shell): $proot_ver"
  fi
else
  record "WARN" "proot --version" "proot not present or not executable (non-blocking for first shell)."
fi

append ""
append "## Filtered logcat"
append '```'
adb logcat -d 2>/dev/null | grep -E "TermuxInstaller|BootstrapIntegrity|BootstrapBaremetalGuard|TermuxActivity|TermuxService|TermuxSession|linker|exec|RuntimeException" | tail -n 400 >> "$REPORT_PATH" || true
append '```'

append ""
append "## Summary"
append "- PASS: $STATUS_PASS"
append "- WARN: $STATUS_WARN"
append "- FAIL: $STATUS_FAIL"
append "- BLOCKER: $STATUS_BLOCKER"

echo "Report generated: $REPORT_PATH"
