# Fullstack Enterprise Capability Matrix

## Leitura correta

Esta matriz descreve **capacidade efetiva desejada** e sua rota de promoção. Ela não afirma que todas as camadas já são código completo. O objetivo é impedir fricção: cada camada sabe qual contrato segue, qual teste exige e qual rollback protege o sistema.

| ID | Capacidade | Contrato sem fricção | Teste/failtest | Rollback/failover |
|---|---|---|---|---|
| FE-01 | Navegação documental | README -> catálogo -> matriz -> gates -> manifestos | links existem | voltar para README canônico |
| FE-02 | Catálogo de seeds | IDs únicos, status válido e falsificação | validador Python | status `DOC_ONLY` |
| FE-03 | Malha 30000 | 30000 slots determinísticos | contar CSV | regenerar CSV |
| FE-04 | Boot contract | `RF_ID -> IDENTIFY -> SELECT_KERNEL` | fixture de estados | bootstrap validado |
| FE-05 | Scheduler BitOmega | período 42 preservado | log period-42 | desativar promoção |
| FE-06 | Attractors | `|A|=42`, #22 VOID | contagem + risco | não corrigir silenciosamente |
| FE-07 | Q16.16 math | sem float em hot path | vetores fixos | fallback C |
| FE-08 | Branchless native | flags/máscaras quando aplicável | benchmark e revisão | caminho genérico |
| FE-09 | ARM32 fallback | Sem GPU, 4GB RAM | orçamento de bytes | modo reduzido |
| FE-10 | AArch64 NEON | SIMD com contrato ABI | comparativo C/NEON | fallback C |
| FE-11 | Federated data | dado bruto não sai do nó | consulta agregada local | negar consulta |
| FE-12 | Identity | identidade sem revelar conteúdo | threat model | bloquear campo sensível |
| FE-13 | Audit chain | hash/log/Merkle para integridade | fixture de adulteração | invalidar cadeia |
| FE-14 | Security boundary | checksum != criptografia | teste adversarial | downgrade de claim |
| FE-15 | Watchdog | falha vira sinal | teste de falha induzida | fail-safe state |
| FE-16 | Rollback | toda promoção reversível | script ou comando documentado | revert/flag |
| FE-17 | Build Android | API 28+, Termux sem root | `./build.sh` | matriz SKIPPED se ambiente incompatível |
| FE-18 | Page size | max-page-size 16384 | inspeção build nativo | ajustar linker |
| FE-19 | Low memory | Cortex-A53 como limite | medição de RSS/ciclos | modo doc-only |
| FE-20 | Linguagem/som | metadados, não claim físico | corpus + métrica | rotular metáfora |
| FE-21 | Mercado/dados | sem recomendação financeira | split temporal | remover claim |
| FE-22 | Supply chain | variáveis rastreáveis | fixture | isolar domínio |
| FE-23 | Molecular/DNA | campos catalogados | dataset validado | doc-only |
| FE-24 | CI/report | PASS/FAIL/SKIPPED explícito | comando exato | não ocultar falha |
| FE-25 | Enterprise delivery | cada camada com artefato | release checklist | bloquear release |

## Morph-on-runtime seguro

A seleção de caminho deve seguir esta ordem: genérico seguro -> ARM32 -> NEON -> AArch64 otimizado. A promoção só é aceita quando o mesmo vetor de teste passa em todos os caminhos habilitados. Se um caminho falhar, o failover volta ao genérico seguro e registra `RISK_OPEN`.

## Falsificação

Falha se qualquer capacidade acima for apresentada como entregue em produção sem teste/failtest, artefato, risco e rollback.
