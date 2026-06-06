#!/data/data/com.termux/files/usr/bin/bash
set -u

CC=clang
FLAGS="-O3 -march=armv7-a -mfpu=neon -mfloat-abi=softfp"

for f in rafaelia_core_armv7.S rafaelia_final.S; do
  [ -f "$f" ] || continue
  out="${f%.S}"
  echo "[START] $f -> $out"
  $CC $FLAGS -nostdlib -Wl,-e,_start "$f" -o "$out" && chmod +x "$out" || echo "falhou $f"
done

for f in rafaelia_b3.S; do
  [ -f "$f" ] || continue
  out="${f%.S}"
  echo "[ASM32] $f -> $out"
  $CC $FLAGS "$f" -o "$out" && chmod +x "$out" || echo "falhou $f"
done

ls -lh rafaelia_core_armv7 rafaelia_final rafaelia_b3 2>/dev/null || true
