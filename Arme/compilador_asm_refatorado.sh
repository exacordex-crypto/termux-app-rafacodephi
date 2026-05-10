#!/usr/bin/env bash
set -euo pipefail

OUT_DIR="${1:-/tmp/arme_refatorado}"
mkdir -p "$OUT_DIR"

cat > "$OUT_DIR/README_BUILD.md" << 'DOC'
# Pipeline ASM Refatorado

## Objetivo
Gerar artefatos textuais coerentes para evolução de C/ASM com trilha rastreável.

## Etapas
1. Normalizar contratos
2. Gerar headers base
3. Gerar stubs C/ASM
4. Validar sintaxe mínima
DOC

cat > "$OUT_DIR/raf_contract.h" << 'HDR'
#ifndef RAF_CONTRACT_H
#define RAF_CONTRACT_H

#define RAF_ABI_ARM32 1
#define RAF_ABI_ARM64 1
#define RAF_Q16_SCALE 65536

#endif
HDR

cat > "$OUT_DIR/raf_stub.c" << 'SRC'
#include "raf_contract.h"
int raf_identity(int x) { return x; }
SRC

cat > "$OUT_DIR/raf_stub_arm64.S" << 'ASM'
.text
.global raf_identity_asm64
raf_identity_asm64:
    ret
ASM

printf 'Artefatos gerados em: %s\n' "$OUT_DIR"
