# Vectras-VM-Android Enterprise Excellence Runbook

## Objetivo real do sistema inteiro

Este runbook transforma o arquivo conceitual em um roteiro operacional para uso real: organizar conhecimento, reduzir fricção, preservar determinismo, criar evidência antes de claim e manter rotas de rollback/failover. Ele não declara que o sistema inteiro já está implementado; ele define como cada parte deve ser promovida com excelência computacional.

## Invariante de 30000 inserções

A sessão exige **mínimo de 30000 inserções** como invariante de uso por excelência. Neste repositório, isso é codificado como uma malha determinística de 30000 slots auditáveis em [`INSERTION_LATTICE_30000.csv`](./INSERTION_LATTICE_30000.csv).

Cada inserção é um slot de trabalho, teste, mitigação, documentação ou navegação. O slot só vira funcionalidade quando passa por evidência. Assim, 30000 não vira volume vazio; vira cobertura mensurável de arquitetura.

## Mais de 20 maneiras de trabalhar com boas práticas

| # | Prática | Aplicação concreta | Evidência mínima |
|---:|---|---|---|
| 1 | Catalogar antes de implementar | Todo conceito entra com `status` e falsificação. | `catalog.json` válido. |
| 2 | Separar metáfora de prova | Parábolas ensinam, testes comprovam. | Gate `DOC_ONLY`/`CODE_BACKED`. |
| 3 | Hot path sem heap | Código nativo crítico não usa malloc/GC. | Revisão + teste nativo. |
| 4 | Q16.16 por padrão | Evitar float em kernels determinísticos. | Teste de saturação. |
| 5 | Branchless quando reduz risco | Preferir flags, máscaras, `csel/csinc` em ASM. | Benchmark e revisão. |
| 6 | Fallback C | SIMD/ASM precisa caminho portável. | Mesmo vetor de teste nos dois caminhos. |
| 7 | Failover explícito | Cada módulo tem rota segura. | Teste de falha induzida. |
| 8 | Rollback explícito | Cada promoção sabe voltar. | Commit revert, flag ou fallback. |
| 9 | Watchdog verificável | Erro não fica silencioso. | Log, contador ou teste CI. |
| 10 | GCD em loops toroidais | Terminação provada por coprimalidade. | Validação de `gcd(stride, radius)=1`. |
| 11 | Atrator #22 preservado | VOID paradox é risco aberto. | Registro em riscos. |
| 12 | 42 atratores | Não promover tabela incompleta. | Validador de contagem. |
| 13 | Android API 28+ | Compatibilidade Termux sem root. | Build/fixture por API. |
| 14 | Page size 16 KiB | Compatibilidade Android moderno. | Linker flag em build nativo. |
| 15 | Baixa memória | Cortex-A53/Moto E7 como limite. | Orçamento de bytes. |
| 16 | Sem abstração desnecessária em ASM | Macros pequenas, sem camadas. | Revisão de `.S`. |
| 17 | Threat model antes de segurança | Hash/checksum não vira sigilo. | Modelo de ameaça. |
| 18 | Dados federados locais | Dado bruto não sai do nó. | Fixture de vazamento negado. |
| 19 | Navegação 5 níveis deep | Índice por árvore até profundidade 5. | `find . -maxdepth 5`. |
| 20 | Log de auditoria | Decisão deixa rastro. | Relatório PASS/FAIL/SKIPPED. |
| 21 | Testes pequenos primeiro | Prova mínima antes de suíte pesada. | Validador local. |
| 22 | Benchmark só com baseline | Performance exige comparação. | Baseline armazenado. |
| 23 | Claims científicos com fonte/experimento | Física/som/quântico ficam didáticos até medição. | Experimento reprodutível. |
| 24 | Enterprise sem prometer magia | Fullstack é roadmap rastreável. | Matriz de capacidade. |
| 25 | Token vazio quando faltar verdade | Melhor declarar desconhecido que inventar. | Campo `unknowns`. |
| 26 | Rota de promoção clara | `DOC_ONLY -> NEEDS_EVIDENCE -> CODE_BACKED`. | Histórico de status. |
| 27 | Módulo de uma coisa | Bloco pequeno com contrato único. | Nome de bloco e teste. |
| 28 | Flags antes de forks | Ativar/desativar sem quebrar fluxo. | Flag documentada. |
| 29 | Morph-on-runtime controlado | Selecionar caminho por capacidade detectada. | Matriz genérico/ARM32/NEON/AArch64. |
| 30 | Não ocultar falha | Falha documentada e explicada. | Saída final com FAIL/SKIPPED. |

## Capacidade fullstack/enterprise por camadas

| Camada | Função | Status seguro atual | Próxima evidência |
|---|---|---|---|
| Arquivo | Guardar sementes, fórmulas e navegação. | Documentado. | Validação estrutural. |
| Catálogo | Fonte machine-readable dos registros. | Validável. | Mais registros reais. |
| Planejamento 30000 | Cobrir slots de trabalho e teste. | Manifesto gerado. | Promover lotes com fixtures. |
| Android/Termux | Execução alvo sem root. | Roadmap. | `./build.sh` em JDK/NDK compatível. |
| Nativo C/ASM | Kernels determinísticos. | Risco aberto. | Ler `VECTRA_OS.md` antes de `.S`. |
| Dados federados | Consulta local agregada. | `NEEDS_EVIDENCE`. | Fixture que prova não vazamento. |
| Segurança | Integridade e auditoria. | Roadmap. | Threat model + teste adversarial. |
| Observabilidade | Log, watchdog e rollback. | Planejado. | Teste de falha induzida. |

## Failsafe, failover e rollback

1. **Failsafe:** entrada inválida vira `UNKNOWN`, `VOID`, `NEEDS_EVIDENCE` ou erro auditado; nunca vira claim silencioso.
2. **Failover:** caminho SIMD/ASM cai para C; caminho distribuído cai para local; claim enterprise cai para `DOC_ONLY` se faltar prova.
3. **Rollback:** commit revert, feature flag, fixture anterior ou remoção controlada do claim.
4. **Mitigação:** risco ganha owner técnico, condição de fechamento e teste mínimo.
5. **Failtest:** cada bloco novo deve ter teste que injeta falha e prova que a falha não atravessa a fronteira.

## Falsificação geral

Esta arquitetura falha se algum item for promovido a funcionalidade real sem: arquivo, comando, saída reproduzível, limite conhecido, rollback e status atualizado no catálogo.
