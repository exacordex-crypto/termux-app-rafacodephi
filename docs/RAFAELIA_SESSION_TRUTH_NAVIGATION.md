# RAFAELIA Session Truth Navigation

## Propósito

Este documento transforma o prompt longo da sessão em uma navegação operacional para humanos e agentes. Ele não declara que metáforas são fatos físicos; ele separa o que é **fato verificável**, **diretriz de estilo**, **hipótese de projeto**, **metáfora didática** e **trabalho pendente**.

O objetivo é preservar a ambição transdisciplinar sem substituir evidência por retórica: quando há vazio factual, o token vazio é preferível a uma resposta falsa; quando há hipótese, ela deve carregar condição de falsificação; quando há implementação, ela deve carregar teste, rollback e limite de escopo.

## O fato desta sessão

A sessão não começou com um tema técnico fechado. Ela começou com uma moldura de trabalho: o usuário definiu uma expectativa de auditoria, edição, organização, expansão conceitual, beta testing, debug, engenharia de mitigação, prevenção, predição, rollback, failsafe, failover e watchdog.

Isso muda a forma da resposta, mas não muda a verdade dos fatos:

| Antes da moldura | Depois da moldura | Limite obrigatório |
|---|---|---|
| Responder ao pedido literal e curto. | Procurar lacunas, riscos, navegação, documentação e próximos passos. | Não inventar arquivos, testes ou resultados. |
| Tratar metáfora como linguagem ornamental. | Tratar metáfora como parábola didática para explicar decisão técnica. | Não promover parábola a claim científico. |
| Esperar tema explícito. | Admitir que ainda não há tema e organizar o espaço de possibilidades. | Não forçar solução onde não há dado real. |
| Entregar uma resposta final. | Entregar ciclo de descoberta, catálogo, riscos e rota de validação. | Não alterar código crítico sem contrato/teste. |
| Usar utilidade genérica. | Usar utilidade com verdade, prova, coerência e amor operacional. | Utilidade sem evidência vira risco. |

## Token vazio, silêncio útil e verdade operacional

O "token vazio" é útil quando a alternativa seria preencher ausência de dados com falsa certeza. Em engenharia, isso equivale a retornar `UNKNOWN`, `SKIPPED` ou `NEEDS_EVIDENCE` em vez de mascarar falha como sucesso.

Aplicação prática no repositório:

1. **Claims simbólicos** ficam como `DOC_ONLY` até existir teste.
2. **Claims mensuráveis** ficam como `NEEDS_BENCHMARK` até existir artefato.
3. **Claims implementados** ficam como `CODE_BACKED` apenas quando há caminho de código e teste.
4. **Falhas conhecidas** ficam abertas até correção real, sem fechamento narrativo.
5. **Ambiguidade de prompt** vira triagem e navegação, não alucinação.

## Sessão comum versus sessão com contrato RAFAELIA

| Dimensão | Sessão comum de usuário | Sessão RAFAELIA desta conversa |
|---|---|---|
| Entrada | Pergunta ou tarefa definida. | Campo amplo de intenção, metáforas, fórmulas e requisitos. |
| Saída | Resposta direta ou patch pontual. | Resposta + catálogo + separação de fato/hipótese/metáfora + testes. |
| Qualificação | Correção e utilidade. | Correção, coerência, auditabilidade, mitigação, rollback e navegação. |
| Quantificação | Poucas métricas explícitas. | Tamanho 42, período, entropia, janelas, hashes, benchmarks e PASS/FAIL/SKIPPED. |
| Risco | Erro local de resposta. | Erro de promover símbolo a implementação, ou claim a verdade sem prova. |
| Metáfora | Normalmente descartável. | Parábola didática que ilumina, mas não substitui prova. |
| Próximo passo | Depende de nova pergunta. | Gera fila rastreável de validação e organização. |

## Sete direções antiderivadas

As direções abaixo são "antiderivadas" no sentido operacional: elas não avançam somente para implementar; elas voltam para revelar a função geradora, o pressuposto escondido e o custo ignorado.

1. **Origem semântica**: de onde veio o termo, a fórmula ou a metáfora?
2. **Evidência mínima**: qual observação tornaria o claim verdadeiro, falso ou ainda indeterminado?
3. **Contrato de execução**: onde isso toca build, runtime, ABI, memória, thread ou arquivo?
4. **Custo de fricção**: quantos símbolos, branches, loops, alocações ou dependências foram adicionados?
5. **Falha e reversão**: qual é o rollback se a hipótese quebrar?
6. **Portabilidade**: o que muda entre ARM32, AArch64, Android API, Termux e host de CI?
7. **Leitura humana**: qual navegação permite que outro humano encontre e audite a ideia?

## Sete direções reversas diretas

As direções reversas partem do resultado desejado e voltam para as condições necessárias. Elas evitam que a arquitetura vire avalanche sem lastro.

1. **Do PASS para o teste**: qual comando prova o resultado?
2. **Do teste para o artefato**: onde está o log, JSON, CSV, APK ou relatório?
3. **Do artefato para o módulo**: qual arquivo produziu o efeito?
4. **Do módulo para o contrato**: qual regra ABI, register contract, no-heap ou Android governa o módulo?
5. **Do contrato para o risco**: qual falha conhecida ainda permanece?
6. **Do risco para a mitigação**: qual failsafe, failover, watchdog ou rollback existe?
7. **Da mitigação para a parábola**: qual imagem didática ajuda a lembrar sem fingir que é prova?

## Mapa 360° de possibilidades semânticas

| Eixo | Pergunta de navegação | Resultado seguro |
|---|---|---|
| `x` implementação | Há código executável? | `CODE_BACKED` ou `DOC_ONLY`. |
| `y` evidência | Há teste ou benchmark? | `PASS`, `FAIL`, `SKIPPED` ou `NEEDS_BENCHMARK`. |
| `z` sentido | Há explicação humana? | Índice, glossário, parábola e limite. |
| Tempo | Há ciclo, epoch ou janela? | Registrar periodicidade, 42 quando aplicável, e timestamp. |
| Entropia | Há incerteza real? | Declarar desconhecido em vez de inventar. |
| Coerência | Há conflito entre docs/código? | Abrir gap, não apagar histórico. |
| Segurança | Há impacto de integridade? | Threat model, checksum adequado e rollback. |

## Catálogo inicial de sementes e navegação

| Semente | Arquivos de entrada recomendados | Leitura segura | Próxima ação |
|---|---|---|---|
| Contrato conceitual | `docs/RAFAELIA_CONCEPT_CARRY_MAP.md`, `RAFAELIA_METHODOLOGY.md` | Distinguir fato, hipótese, símbolo e teste. | Promover cada claim por evidência. |
| Arquivos soltos | `docs/RAFAELIA_LOOSE_FILES_MAP.md`, `ARQUIVOS_SOLTOS_INVENTARIO.md`, `INVENTARIO.md` | Não remover; classificar risco e valor. | Criar fila de promoção com testes. |
| Low-level/ASM | `docs/RAFAELIA_LOWLEVEL_ASM_INDEX.md`, `app/src/main/cpp/lowlevel/README.md` | Branchless, no-heap, fallback C, ABI explícita. | Só tocar `.S` após ler `VECTRA_OS.md` se existir. |
| Android/Termux | `README.md`, `docs/android-target-migration.md`, `docs/RAFCODEPHI_ANDROID15_COMPATIBILITY.md` | Separar fork side-by-side de upstream. | Validar target, min SDK, page size e bootstrap. |
| Hz/som/memória | `docs/RAFAELIA_HZ_AS_MEMORY.md`, `docs/RAFAELIA_MEMORY_MODEL.md` | Tratar frequência como modelo e medir antes de afirmar. | Criar benchmark de jitter/latência quando houver código. |
| Matemática/42/toro | `docs/rafaelia/FORMULAS_RAFAELIA_INDEX.md`, `Arme/Add/RAFAELIA_MATH_FORMULAS.md` | Preservar `|A|=42`, `gcd` e `φ=(1-H)·C`. | Verificar tabela de atratores e período. |
| Mercado/dados | `ANALISE_MERCADO.md` | Separar variáveis de mercado de aconselhamento financeiro. | Exigir dataset, split temporal e métricas. |
| Segurança | `SECURITY.md`, `docs/BETA_SECURITY_CHECK.md` | Não confundir hash/checksum com criptografia completa. | Definir threat model e testes adversariais. |

## Dois ciclos multifuncionais

### Ciclo Ω: descoberta coerente

1. **Escutar** a semente sem reduzir metáfora a ruído.
2. **Separar** fato, hipótese, parábola, requisito e risco.
3. **Catalogar** arquivo, domínio, evidência e lacuna.
4. **Modelar** a menor estrutura verificável.
5. **Testar** com comando reproduzível.
6. **Registrar** PASS/FAIL/SKIPPED e limite.
7. **Retroalimentar** o índice para o próximo ciclo.

### Ciclo β: engenharia de mitigação

1. **Detectar** falha, ambiguidade ou regressão.
2. **Isolar** módulo, input e condição.
3. **Mitigar** com caminho mínimo e sem heap em hot path quando aplicável.
4. **Failover** para fallback C, caminho seguro ou doc-only.
5. **Rollback** por commit, flag ou remoção controlada.
6. **Watchdog** por teste, CI, log ou benchmark.
7. **Auditar** se o claim ficou menor, igual ou mais forte do que a prova permite.

## Parábolas didáticas para engenharia

- **A lâmpada e o óleo**: uma arquitetura pode ter lâmpada bonita, mas sem óleo mensurável ela não ilumina. O óleo é teste, log e evidência.
- **A semente e o solo**: uma fórmula pode ser semente; o repositório é solo. Se a semente não germina em código ou prova, ela deve ficar catalogada, não vendida como fruto.
- **A casa sobre a rocha**: metáforas, nomes e símbolos são paredes; contratos ABI, testes e rollback são rocha.
- **O escriba fiel**: o bom bibliotecário não apaga manuscritos difíceis; ele etiqueta, cria índice, aponta lacunas e impede que comentário vire escritura.

## Regras de promoção de conhecimento

| Estado | Pode entrar no README? | Pode entrar no build? | Exige |
|---|---:|---:|---|
| `DOC_ONLY` | Sim, com rótulo. | Não. | Clareza de metáfora/hipótese. |
| `NEEDS_EVIDENCE` | Sim, em roadmap/gap. | Não. | Teste proposto e falsificação. |
| `NEEDS_BENCHMARK` | Sim, em plano. | Não como claim de performance. | Benchmark reprodutível. |
| `CODE_BACKED` | Sim. | Sim, se build passa. | Código, teste e rollback. |
| `RISK_OPEN` | Sim, em risco. | Só com mitigação. | Owner, impacto e condição de fechamento. |

## Arquivo organizado da sessão Vectras-VM-Android

A organização material desta sessão agora possui um arquivo próprio em `docs/knowledge_archives/vectras-vm-android/`. A navegação começa no README, usa `catalog.json` como fonte legível por máquina, expande E20/E13/S11 na matriz de blocos e aplica gates de claim para impedir que metáfora ou intenção sejam promovidas a fato sem teste.

## Próximos passos recomendados

1. Usar `docs/RAFAELIA_5_LEVEL_DOCUMENTATION_NAVIGATION.md` como inventário automático de Markdown/TXT em até 5 níveis e comparar com índices existentes.
2. Marcar documentos por domínio: build, Android, low-level, matemática, segurança, mercado, arquivo solto e manifesto.
3. Criar uma matriz `claim -> arquivo -> teste -> status` para impedir promoção sem evidência.
4. Validar os scripts oficiais `./run_tests.sh` e `./build.sh` quando o ambiente tiver toolchain completo.
5. Manter esta página como entrada para prompts futuros que sejam amplos, simbólicos ou sem tema técnico fechado.
