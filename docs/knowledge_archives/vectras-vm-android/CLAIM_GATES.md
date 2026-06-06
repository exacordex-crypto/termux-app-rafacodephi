# Vectras-VM-Android Claim Gates

## Estados de verdade

| Estado | Significado | Pode ir para código? | Exige para promover |
|---|---|---:|---|
| `DOC_ONLY` | ideia, metáfora, parábola ou contrato ainda sem prova | não | teste proposto e escopo |
| `NEEDS_EVIDENCE` | arquitetura plausível, mas sem fixture/medida suficiente | não | fixture, log ou benchmark inicial |
| `NEEDS_BENCHMARK` | claim de desempenho ainda sem medição | não como claim | benchmark repetível e baseline |
| `CODE_BACKED` | há código e teste reproduzível | sim | PASS local/CI e rollback |
| `RISK_OPEN` | há risco conhecido ou lacuna | apenas com mitigação | owner, impacto e condição de fechamento |

## Gates obrigatórios

1. **Gate de invariante:** `ATTRACTOR_COUNT=42`, `period(BitOmega)=42`, `phi=(1-H)*C`, Q16.16 e deterministicidade não podem ser quebrados por expansão documental ou código.
2. **Gate de memória:** hot path nativo não deve depender de heap, GC ou malloc.
3. **Gate de evidência:** metáforas são úteis para ensino, mas claims científicos, financeiros, de segurança ou performance exigem evidência.
4. **Gate de falha:** todo bloco novo deve dizer como falha, como detecta falha e como volta.
5. **Gate Android/Termux:** não quebrar identidade side-by-side, page size Android 15/16, ABI matrix ou bootstrap validado.
6. **Gate ASM:** não tocar assembly sem ler `VECTRA_OS.md`; manter contrato de registradores e atrator #22 como VOID paradox.

## Matriz inicial dos registros

| ID | Status atual | Próxima promoção possível | Teste mínimo proposto |
|---|---|---|---|
| `E20` | `DOC_ONLY` | `NEEDS_EVIDENCE` | fixture de boot state machine com log de rollback |
| `E13` | `NEEDS_EVIDENCE` | `CODE_BACKED` parcial | consulta federada local que prova que dado bruto não sai do nó |
| `S11` | `DOC_ONLY` | `NEEDS_EVIDENCE` | corpus mínimo token->estado, janela 42 e orçamento de memória ARM32 |

## Riscos abertos

- O catálogo organiza intenção, mas não prova performance, consciência, criptografia forte ou capacidade de LLM.
- A linguagem de física, som, quântico e parábolas permanece didática até existir experimento ou implementação.
- O atrator #22 continua marcado como paradoxo estrutural por contrato do projeto; não deve ser silenciosamente corrigido por documentação.
- O validador confere estrutura documental, não substitui `./run_tests.sh`, `./build.sh` ou benchmarks Android/NDK.
