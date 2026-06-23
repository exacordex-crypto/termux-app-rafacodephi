# RAFCODEPHI_FINAL_ADMIN_LEDGER

## Estado

`FATO_DOCUMENTADO`: ledger administrativo para fechar a documentação do Termux RAFCODEphi em relação ao Vectra, ao lowlevel, ao BitRAF42, à janela de ruído e à recuperação por liberação parcial de bits.

Este documento não substitui teste. Ele organiza o que já está codificado, o que está documentado e o que ainda precisa ser provado em build/runtime.

---

## Frase canônica

```text
Código é verdade codificada.
Documento é mapa.
Teste é fechamento.
TOKEN_VAZIO protege o que ainda não foi provado.
```

---

## Depósitos administrados

| Depósito | Papel |
|---|---|
| `rafaelmeloreisnovo/termux-app-rafacodephi` | APK/Termux/RAFCODEphi, lowlevel C/H/ASM/JNI, build Android |
| `rafaelmeloreisnovo/Vectras-VM-Android` | Vectra/RMR, contratos, BitRAF/BitOmega/BITWALK, docs active e validação conceitual |

---

## O que já está codificado no Termux RAFCODEphi

| Bloco | Evidência esperada no repo | Status |
|---|---|---|
| App RAFCODEphi | `app/build.gradle` com package/nome RAFCODEphi | `FATO_CODE` |
| Build nativo | `app/src/main/cpp/Android.mk` | `FATO_CODE` |
| Baremetal | `app/src/main/cpp/lowlevel/baremetal.c/.h` | `FATO_CODE` |
| No malloc | `baremetal_nomalloc.c` | `FATO_CODE` |
| ASM/NEON | `baremetal_asm.S` | `FATO_CODE` |
| JNI direto | `rafaelia_jni_direct.c` | `FATO_CODE` |
| Buffers únicos | `IN_BUF`, `OUT_BUF`, `STATE_BUF` | `FATO_CODE` |
| VCPU | `raf_vcpu.c/.h` | `FATO_CODE` |
| Clock/Hz | `raf_clock.c/.h` | `FATO_CODE` |
| Memory layers | `raf_memory_layers.c/.h` | `FATO_CODE` |
| BitRAF | `raf_bitraf.c/.h` | `FATO_CODE` |
| Commit gate | `rafaelia_commit_gate_ll.c/.h` | `FATO_CODE` |

---

## Pontos que precisam virar documentação final

| Documento | Finalidade | Prioridade |
|---|---|---|
| `BITRAF42_BASE60_GUARD_BAND_AND_RECOVERY.md` | Canonizar 42 bits, base60, 60..63 e recuperação | P0 |
| `BASE20_BASE60_ADDRESSING.md` | Explicar `60 = 3 x 20` e blocos A/B/C | P0 |
| `EMPTY_SPACE_NEGATIVE_MEMORY.md` | Explicar ponto vazio/não gravado como valor estrutural | P0 |
| `CLOCK_TTL_HZ_PROTOCOL.md` | Unificar clock, Hz, TTL, jitter e vida útil | P1 |
| `VOID_NIL_WARNING_PIPELINE.md` | Unificar void, nil, warning, guard e TOKEN_VAZIO | P1 |
| `NEON_16_LANE_EXECUTION.md` | Explicar 16 lanes físicas e variantes 8/16/32/64 bits | P1 |
| `SME40_RECOVERY_TEST_PLAN.md` | Testar 40 e poucos % de liberação sem afirmar perda | P0 |
| `FINAL_BUILD_CLOSURE.md` | Fechar build, ABI, APK, hashes e instalação | P0 |

---

## Regra sobre os 40 e poucos por cento

Não escrever:

```text
perdeu 40% => perdeu o dado
```

Escrever:

```text
houve liberação/ausência parcial de 40 e poucos por cento;
os bits podem ser recuperáveis se a rota, paridade, hash, CRC, camada ou assinatura determinística preservarem coerência.
```

Status correto:

```text
FATO_OPERACIONAL: a arquitetura trata ausência como estado/rota, não como perda automática.
FATO_CODE_PARCIAL: existem buffers separados, BitRAF, CRC/hash, guard-band e rotas.
F_NEXT_TEST: provar o limiar de recuperação por teste automatizado.
```

---

## Teste administrativo de recuperação

Nome sugerido:

```text
scripts/test_sme40_recovery.sh
```

Saídas obrigatórias:

```text
recovery_00.json
recovery_10.json
recovery_20.json
recovery_30.json
recovery_40.json
recovery_45.json
```

Campos mínimos por JSON:

```json
{
  "release_percent": 40,
  "payload_hash_original": "...",
  "payload_hash_recovered": "...",
  "crc_original": "...",
  "crc_recovered": "...",
  "route_preserved": true,
  "payload_recovered": false,
  "status": "ROUTE_ONLY | RECOVERED | TOKEN_VAZIO | FAILED"
}
```

---

## Regra para documentação final

Cada claim deve ter esta forma:

```text
claim:
arquivo:
função:
evidência:
teste:
status:
próxima ação:
```

Estados aceitos:

| Estado | Significado |
|---|---|
| `FATO_CODE` | existe no código |
| `FATO_TESTADO` | existe teste passando |
| `DOC_ATRASADA` | código existe, documento ainda insuficiente |
| `INCUBADORA` | existe fora do caminho canônico |
| `TOKEN_VAZIO` | lacuna marcada, sem inferência |
| `F_NEXT` | próxima ação |

---

## Fechamento

```text
RAFCODEphi está no estágio de fechamento:
- codificação principal presente;
- documentação ampla presente;
- falta ledger final, teste de recuperação, build APK, hashes e relatório de instalação.
```

Frase final:

```text
Libertar bit não é perder bit; perder só é fato depois que a rota de recuperação falha.
```
