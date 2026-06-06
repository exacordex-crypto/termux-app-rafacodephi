#!/data/data/com.termux/files/usr/bin/bash
set -u

for f in rafaelia_b3.S rafaelia_bench.S rafaelia_final.S rafaelia_core_armv7_bench.S; do
  [ -f "$f" ] || continue
  cp "$f" "$f.fixbak"

  perl -pi -e 's/\bmov\s+(r[0-9]+),\s*#(msg_[A-Za-z0-9_]+_len)\b/ldr $1, =$2/g' "$f"
  perl -pi -e 's/\band\s+r2,\s*r2,\s*#0xFFFF\b/uxth r2, r2/g' "$f"
  perl -pi -e 's/\badr\s+(r[0-9]+),\s*(msg_[A-Za-z0-9_]+)/ldr $1, =$2/g' "$f"
done

echo "patch aplicado"
grep -n "mov r[0-9], #msg_\|and r2, r2, #0xFFFF\|adr r[0-9], msg_" rafaelia_*.S 2>/dev/null || true
