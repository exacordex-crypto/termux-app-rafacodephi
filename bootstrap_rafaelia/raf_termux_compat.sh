#!/usr/bin/env bash
set -euo pipefail

RAF_TERMUX_PACKAGE="${RAF_TERMUX_PACKAGE:-com.termux.rafacodephi}"
RAF_TERMUX_APP_DIR="${RAF_TERMUX_APP_DIR:-/data/data/${RAF_TERMUX_PACKAGE}}"
PREFIX="${RAF_TERMUX_PREFIX:-${RAF_TERMUX_APP_DIR}/files/usr}"
OUT_DIR="${1:-$PWD/out-termux-compat}"

mkdir -p "$OUT_DIR/bin" "$OUT_DIR/share/bootstrap_rafaelia" "$OUT_DIR/lib/bootstrap_rafaelia"
cat > "$OUT_DIR/bin/bootstrap-rafaelia-selftest" <<SELFTEST_SH
#!/usr/bin/env sh
set -eu
PREFIX="\${RAF_TERMUX_PREFIX:-$PREFIX}"
if [ -x "\$PREFIX/lib/bootstrap_rafaelia/raf_selftest" ]; then
  exec "\$PREFIX/lib/bootstrap_rafaelia/raf_selftest"
fi
echo "raf_selftest not installed in \$PREFIX/lib/bootstrap_rafaelia"
exit 1
SELFTEST_SH
chmod +x "$OUT_DIR/bin/bootstrap-rafaelia-selftest"
cat > "$OUT_DIR/share/bootstrap_rafaelia/NOTICE.txt" <<'TXT'
bootstrap_rafaelia is experimental and auxiliary.
It does NOT replace TermuxInstaller bootstrap.zip flow.
It does NOT replace SYMLINKS.txt validation.
TXT
cp -f README.md "$OUT_DIR/share/bootstrap_rafaelia/README.md"
echo "Generated Termux-compatible auxiliary payload at: $OUT_DIR"
