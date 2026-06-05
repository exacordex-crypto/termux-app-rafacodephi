# Vectras-VM-Android Knowledge Archive

## Propósito

Este arquivo organiza as sementes soltas da sessão **Vectras-VM-Android** em uma navegação auditável. Ele preserva metáforas como parábolas didáticas, mas separa metáfora, hipótese, contrato técnico e prova executável.

A regra central é simples: quando não há evidência, o arquivo deve dizer `DOC_ONLY`, `NEEDS_EVIDENCE` ou `NEEDS_BENCHMARK`; não deve transformar intenção em fato implementado.

## Entrada canônica

- Catálogo legível por máquina: [`catalog.json`](./catalog.json).
- Expansão por blocos: [`EXPANSION_MATRIX.md`](./EXPANSION_MATRIX.md).
- Gates de promoção de claims: [`CLAIM_GATES.md`](./CLAIM_GATES.md).
- Validador local: [`../../../scripts/verification/validate_vectras_archive.py`](../../../scripts/verification/validate_vectras_archive.py).

## Invariante geométrica coerente

A sessão carrega a pergunta: **"O que carrega o conhecimento que entendeu?"**

Resposta operacional neste repositório:

```text
I = Phi(s, S, H, C, G)
s = estado toroidal compacto
S = sinal ou sequência observada
H = entropia/ruído/incerteza
C = coerência/confiança operacional
G = geometria/grafo/caminho de navegação
```

Essa fórmula não é tratada como prova física. Ela é tratada como contrato de organização: todo bloco deve indicar estado, sinal, entropia, coerência e geometria de navegação.

## Como usar em trabalho real

1. Escolha um registro do catálogo (`E20`, `E13`, `S11`).
2. Verifique o `status` antes de escrever código.
3. Se o status for `DOC_ONLY`, escreva documentação, teste proposto e condição de falsificação.
4. Se o status for `NEEDS_EVIDENCE`, crie primeiro um teste/benchmark ou um fixture mínimo.
5. Só promova para `CODE_BACKED` quando houver arquivo executável, comando e resultado reproduzível.

## Níveis de navegação

| Nível | Nome | Conteúdo | Resultado esperado |
|---:|---|---|---|
| 0 | Entrada | Este README | humano entende o mapa |
| 1 | Catálogo | `catalog.json` | agente valida estrutura |
| 2 | Expansão | `EXPANSION_MATRIX.md` | blocos viram tarefas delimitadas |
| 3 | Gates | `CLAIM_GATES.md` | claims não sobem sem prova |
| 4 | Código/teste | scripts, C, ASM, Gradle | somente quando há evidência |
| 5 | Artefato | log, APK, benchmark, relatório | PASS/FAIL/SKIPPED rastreável |

## Parábola didática

O arquivo é como uma biblioteca em um barco: a vela é a metáfora, o casco é o contrato técnico, o leme é o teste e a âncora é o rollback. Sem casco e leme, a vela não prova viagem; sem vela, a biblioteca ainda pode navegar devagar, mas com verdade.
