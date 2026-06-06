#!/data/data/com.termux/files/usr/bin/bash
set -e

CC=${CC:-clang}

for f in rafaelia_b3.S rafaelia_bench.S rafaelia_final.S rafaelia_core_armv7.S rafaelia_math_bench.S rafaelia_core_armv7_bench.S rafaelia_core_v8.S; do
  [ -f "$f" ] || continue
  out="${f%.S}"
  echo "[ASM] $f -> $out"
  $CC -O3 -march=armv7-a -mfpu=neon -mfloat-abi=softfp "$f" -o "$out" || echo "falhou: $f"
  chmod +x "$out" 2>/dev/null || true
done

ls -lh rafaelia_b3 rafaelia_bench rafaelia_final rafaelia_core_armv7 2>/dev/null || true
