#!/data/data/com.termux/files/usr/bin/bash
set -u

CC=clang
FLAGS="-O3 -march=armv7-a -mfpu=neon -mfloat-abi=softfp"

echo "[B3 NO-PIE]"

$CC $FLAGS \
  -nostdlib \
  -fno-pic -fno-pie \
  -Wl,-no-pie \
  -Wl,-e,_start \
  rafaelia_b3.S -o rafaelia_b3 \
  && chmod +x rafaelia_b3 \
  && echo "OK: ./rafaelia_b3" \
  || echo "FALHOU b3 nopie"

file ./rafaelia_b3 2>/dev/null || true
