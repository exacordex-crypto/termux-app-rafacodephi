#!/data/data/com.termux/files/usr/bin/bash
set -u

CC=clang
FLAGS="-O3 -march=armv7-a -mfpu=neon -mfloat-abi=softfp"

echo "[B3 START] rafaelia_b3.S -> rafaelia_b3"
$CC $FLAGS -nostdlib -Wl,-e,_start rafaelia_b3.S -o rafaelia_b3 \
  && chmod +x rafaelia_b3 \
  && echo "OK: ./rafaelia_b3" \
  || echo "FALHOU b3"

[ -x ./rafaelia_b3 ] && ./rafaelia_b3
