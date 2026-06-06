#!/data/data/com.termux/files/usr/bin/bash

echo "=== RAFAELIA/VECTRA EXEC SCAN ==="
echo "data=$(date)"
echo

OUT=exec_scan_results.txt
: > "$OUT"

for x in rafaelia_* vectra_* blink_* crc_* bitraf_* exacordex* geolm* raf_bench vectra; do
  [ -f "$x" ] || continue
  [ -x "$x" ] || continue

  echo "=== $x ===" | tee -a "$OUT"

  timeout 8s ./"$x" 2>&1 | head -40 | tee -a "$OUT"

  echo | tee -a "$OUT"
done

echo "=== RESUMO CANDIDATOS ==="
grep -Ei "MB/s|MBps|ns=|median|avg|throughput|MCR|REGIME|DONE|CRC|winner" "$OUT" | tail -200

echo
echo "Arquivo salvo: $OUT"
