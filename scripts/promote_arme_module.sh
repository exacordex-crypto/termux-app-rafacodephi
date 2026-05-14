#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 3 ]]; then
  echo "Uso: $0 <origem_em_Arme/Add> <destino_canonico_em_Arme/> <id_teste_equivalencia>" >&2
  exit 2
fi

SRC="$1"
DST="$2"
TEST_ID="$3"
MANIFEST="Arme/manifest.json"
AUDIT_LOG="Arme/reports/promotion_audit.log"

if [[ ! -f "$SRC" ]]; then
  echo "Erro: arquivo de origem inexistente: $SRC" >&2
  exit 1
fi
if [[ ! "$SRC" =~ ^Arme/Add/ ]]; then
  echo "Erro: origem deve estar em Arme/Add (staging): $SRC" >&2
  exit 1
fi
if [[ ! "$DST" =~ ^Arme/(spec|include|src/c|src/asm/arm32|src/asm/arm64|tests|bench|reports)/ ]]; then
  echo "Erro: destino fora dos diretórios canônicos: $DST" >&2
  exit 1
fi
if [[ ! -f "$MANIFEST" ]]; then
  echo "Erro: manifesto ausente: $MANIFEST" >&2
  exit 1
fi

python3 - "$MANIFEST" "$SRC" <<'PY'
import json,sys
manifest=sys.argv[1]; target=sys.argv[2]
with open(manifest, encoding='utf-8') as f:
    data=json.load(f)
items=data.get('itens',[])
entry=next((i for i in items if i.get('arquivo')==target), None)
if not entry:
    print(f"Erro: {target} sem entrada no manifesto", file=sys.stderr); sys.exit(1)
if entry.get('status')!='ativo' or not entry.get('pode_extrair',False):
    print(f"Erro: {target} bloqueado pelo manifesto", file=sys.stderr); sys.exit(1)
print('Manifesto OK')
PY

TEST_SCRIPT="Arme/tests/equivalence/${TEST_ID}.sh"
if [[ ! -x "$TEST_SCRIPT" ]]; then
  echo "Erro: teste mínimo de equivalência ausente ou sem permissão de execução: $TEST_SCRIPT" >&2
  exit 1
fi
"$TEST_SCRIPT" "$SRC" "$DST"

mkdir -p "$(dirname "$DST")" "$(dirname "$AUDIT_LOG")"
cp "$SRC" "$DST"

printf '%s | promoted | src=%s | dst=%s | test=%s | by=%s\n' \
  "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$SRC" "$DST" "$TEST_ID" "${GIT_AUTHOR_NAME:-$(whoami)}" >> "$AUDIT_LOG"

echo "Promoção concluída: $SRC -> $DST"
