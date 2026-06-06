# RAFAELIA Loose Files Map

| Arquivo | Função | Status | Compila | Build oficial | Risco | Valor técnico | Promoção |
|---|---|---|---|---|---|---|---|
| mvp/rafaelia_opcodes.hex | catálogo opcode raw | SOLTO/EXPERIMENTAL | n/a | não | médio | referência ISA | parser/test vector |
| mvp/rafaelia_mvp_puro.s | MVP asm puro | SOLTO/EXPERIMENTAL | parcial | não | alto | bootstrapping low-level | portar bloco útil |
| rmr/Rrr/rafaelia_core.c | core legado arm32 | SOLTO/EXPERIMENTAL | parcial | não | alto | contém vcpu/hz/layers | reimplementar modular |
| rmr/Rrr/rafaelia_jni_direct.c | JNI legado | SOLTO/EXPERIMENTAL | parcial | não | alto | contratos antigos | mapear gaps |
| rmr/Rrr/rafaelia_arena.h | arena legado | SOLTO/EXPERIMENTAL | sim | não | médio | estratégia nomalloc | convergir com baremetal |
| rmr/Rrr/rafaelia_b1.S | asm legado | SOLTO/EXPERIMENTAL | parcial | não | médio | otimização | validar ABI/NDK |
| rafaelia/old/* | histórico | SOLTO/EXPERIMENTAL | variável | não | médio | rastreabilidade | documentar sem promover cego |

## Entrada de navegação para prompts amplos

Quando o pedido do usuário vier como campo sem tema técnico fechado, usar `docs/RAFAELIA_SESSION_TRUTH_NAVIGATION.md` antes de promover arquivos soltos. Esse documento define como separar verdade verificável, hipótese, metáfora/parábola, token vazio, risco e próxima ação.

Regra prática: arquivo solto não vira build oficial por entusiasmo semântico; ele precisa passar por catálogo, contrato, teste, mitigação e rollback.
