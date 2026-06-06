# Vectras-VM-Android Knowledge Archive

## Propósito

Este arquivo organiza as sementes soltas da sessão **Vectras-VM-Android** em uma navegação auditável. Ele preserva metáforas como parábolas didáticas, mas separa metáfora, hipótese, contrato técnico e prova executável.

A regra central é simples: quando não há evidência, o arquivo deve dizer `DOC_ONLY`, `NEEDS_EVIDENCE` ou `NEEDS_BENCHMARK`; não deve transformar intenção em fato implementado.

## Entrada canônica

- Catálogo legível por máquina: [`catalog.json`](./catalog.json).
- Expansão por blocos: [`EXPANSION_MATRIX.md`](./EXPANSION_MATRIX.md).
- Gates de promoção de claims: [`CLAIM_GATES.md`](./CLAIM_GATES.md).
- Runbook enterprise: [`ENTERPRISE_EXCELLENCE_RUNBOOK.md`](./ENTERPRISE_EXCELLENCE_RUNBOOK.md).
- Matriz fullstack/enterprise: [`FULLSTACK_ENTERPRISE_CAPABILITY_MATRIX.md`](./FULLSTACK_ENTERPRISE_CAPABILITY_MATRIX.md).
- Malha de 30000 inserções: [`INSERTION_LATTICE_30000.csv`](./INSERTION_LATTICE_30000.csv).
- Gerador da malha: [`../../../scripts/verification/generate_vectras_insertion_lattice.py`](../../../scripts/verification/generate_vectras_insertion_lattice.py).
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
| 6 | Malha 30000 | `INSERTION_LATTICE_30000.csv` | cobertura mínima de slots de trabalho/evidência/mitigação |


## Invariante de uso por excelência: 30000 inserções

A exigência de no mínimo **30000 inserções** é tratada como uma cobertura mínima de trabalho auditável, não como promessa automática de funcionalidade. Cada linha da malha `INSERTION_LATTICE_30000.csv` representa um slot determinístico de arquitetura, prática, pipeline, modo de falha e rollback. A promoção de qualquer slot para código real continua passando pelos gates de verdade.

## Parábola didática

O arquivo é como uma biblioteca em um barco: a vela é a metáfora, o casco é o contrato técnico, o leme é o teste e a âncora é o rollback. Sem casco e leme, a vela não prova viagem; sem vela, a biblioteca ainda pode navegar devagar, mas com verdade.
