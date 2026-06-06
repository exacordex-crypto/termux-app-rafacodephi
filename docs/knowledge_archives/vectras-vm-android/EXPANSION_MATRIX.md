# Vectras-VM-Android Expansion Matrix

## Regra de expansão

Cada semente deve ser expandida em blocos pequenos, determinísticos e auditáveis. Em caminhos nativos/hot path, a expansão deve respeitar: sem heap, sem GC, sem malloc, Q16.16, branchless quando isso reduzir risco real, fallback C quando houver ASM/SIMD, e rollback claro.

## E20 — Sistema operacional cognitivo completo

| Bloco | Contrato | Entrega mínima | Failsafe / rollback |
|---|---|---|---|
| `boot_contract` | `RF_ID -> IDENTIFY -> SELECT_KERNEL -> freestanding` | diagrama + fixture de estado de boot | voltar para bootstrap upstream/local validado |
| `state_protocol_network` | rede por estado, não por narrativa | schema de estados e tags | rejeitar estado desconhecido como `UNKNOWN` |
| `audit_chain` | Bitraf + CRC32C + Merkle como rastreabilidade | log com hash, timestamp e etapa | não promover checksum a criptografia forte |
| `rollback_failover_watchdog` | toda promoção exige volta segura | matriz de falha e comando de reversão | feature flag, commit revert ou fallback C |

## E13 — Plataforma de dados federados

| Bloco | Contrato | Entrega mínima | Failsafe / rollback |
|---|---|---|---|
| `local_node_contract` | dado bruto não sai do nó | fixture local com consulta agregada | negar consulta que vaze payload |
| `route_tag_index` | `route_tag` indexa caminho distribuído | schema de tag e colisões | fallback para consulta local isolada |
| `privacy_boundary` | identidade sem conteúdo | threat model mínimo | mascarar ou bloquear campos sensíveis |
| `coherence_aggregation` | resultado agregado por coerência | métrica `C/H/phi` documentada | retornar `NEEDS_EVIDENCE` se métrica faltar |

## S11 — LLM sem pesos GeoLM

| Bloco | Contrato | Entrega mínima | Failsafe / rollback |
|---|---|---|---|
| `token_state_transition` | tokens viram transições de estado | tabela token -> estado | rejeitar token fora do dicionário |
| `attractor_memory` | memória por atratores estáveis | janela de 42 posições | manter atrator #22 marcado como VOID paradox |
| `decay_forgetting` | esquecimento por decaimento | fórmula Q16.16 e teste de monotonia | saturar valores em faixa válida |
| `arm32_no_gpu_limits` | 4GB RAM, Cortex-A7, sem GPU | orçamento de memória e ciclos | fallback para modo doc-only se exceder orçamento |

## Eixos transdisciplinares catalogados

| Eixo | Uso seguro | Limite |
|---|---|---|
| Linguagem/som | metadados de alfabeto, direção, acento, cadência e fonética | não afirmar efeito neurológico sem fonte/teste |
| Física/quântico/toro | metáfora ou formalismo matemático documentado | não declarar prova física sem experimento |
| Mercado/dados | taxonomia de variáveis, métricas e backtests | não emitir recomendação financeira |
| Segurança | threat model, checksum, hash, Merkle e logs | não confundir integridade com sigilo completo |
| Low-level | C/ASM, NEON, registradores, cache e page size | não tocar `.S` sem ler `VECTRA_OS.md` |
| Semântica/ética | parábolas didáticas e rotulagem de verdade | não substituir teste por retórica |
| Enterprise | auditoria, SLA, rollback, failover, ownership | não prometer produto sem artefato |
