#!/data/data/com.termux/files/usr/bin/bash

OUT="DOSSIÊ_RAFAELIA_VECTRA_C_ASM.txt"
: > "$OUT"

add_section(){
  title="$1"
  shift
  {
    echo
    echo "============================================================"
    echo "## $title"
    echo "============================================================"
    echo
  } >> "$OUT"

  for f in "$@"; do
    [ -f "$f" ] || continue
    {
      echo
      echo "------------------------------------------------------------"
      echo "### ARQUIVO: $f"
      echo "------------------------------------------------------------"
      echo
      cat "$f"
      echo
    } >> "$OUT"
  done
}

add_section "RAFAELIA C — EDGE / MVP / BITRAF" \
  rafaelia_edge_v7_auto.c \
  rafaelia_edge_v6_unroll.c \
  rafaelia_edge_v5.c \
  rafaelia_edge_v4.c \
  rafaelia_edge_v2.c \
  rafaelia_mvp_bench.c \
  rafaelia_bitraf.c \
  rafaelia_b3_android.c

add_section "RAFAELIA ASM — ARM32 / RAW" \
  rafaelia_core_armv7.S \
  rafaelia_b3.S \
  rafaelia_bench.S \
  rafaelia_final.S \
  rafaelos_armv7.s \
  crc_armv7.S \
  crc_raw.S

add_section "VECTRA C — CANDIDATOS FORTES" \
  vectra_raw.c \
  vectra_quantum_total_sentinel.c \
  vectra_quantum_sentinel.c \
  vectra_quantum_observer.c \
  vectra_quantum_final.c \
  vectra_sync.c \
  vectra_portable.c \
  vectra_zero.c

add_section "BLINK / NEON / MULTICORE" \
  blink_neon_multicore.c \
  blink_neon_multistream.c \
  blink_neon_feedback_fix.c \
  blink_multicore.c \
  blink_neon.c

add_section "RESULTADO DO SCAN" \
  exec_scan_results.txt

echo "OK: $OUT"
ls -lh "$OUT"
