#!/usr/bin/env bash
set -euo pipefail

BASE_REF="${1:-origin/main}"
MANIFEST="Arme/manifest.json"

if [[ ! -f "$MANIFEST" ]]; then
  echo "Erro: manifesto não encontrado: $MANIFEST" >&2
  exit 1
fi

mapfile -t added_files < <(git diff --name-only --diff-filter=A "$BASE_REF"...HEAD -- 'Arme/Add/*' | rg '\.(c|h|S)$' || true)

if [[ ${#added_files[@]} -eq 0 ]]; then
  echo "Sem novos .c/.h/.S em Arme/Add/."
  exit 0
fi

python3 - "$MANIFEST" "${added_files[@]}" <<'PY'
import json,sys
manifest=sys.argv[1]
added=sys.argv[2:]
with open(manifest, encoding='utf-8') as f:
    data=json.load(f)
files={item.get('arquivo') for item in data.get('itens',[]) if isinstance(item,dict)}
missing=[p for p in added if p not in files]
if missing:
    print('Erro: arquivos novos em Arme/Add sem entrada no manifesto:')
    for p in missing:
        print(f' - {p}')
    sys.exit(1)
print('Manifesto cobre todos os novos arquivos em Arme/Add.')
PY
