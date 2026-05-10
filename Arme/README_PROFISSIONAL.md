# ARMÉ — União de Forças para um Ecossistema de Refatoração (C/ASM Freestanding)

## Missão do diretório `Arme/`
`Arme/` deixa de ser apenas pasta de rascunhos e passa a ser o **núcleo de integração** entre:
- arquitetura low-level (C/ASM);
- cadeia Android (Gradle + CMake + NDK + JNI);
- release engineering (signed/unsigned);
- validação contínua (CI/CD + artefatos auditáveis).

Objetivo técnico central:
- transformar componentes isolados em **ecossistema único, coerente e executável** para arm32 e arm64.

## Fonte de verdade (single source of truth)
A refatoração deve respeitar esta ordem de autoridade:
1. contratos técnicos versionados (`ABI`, memória, bootstrap, assinaturas);
2. implementação (`C` freestanding e `ASM`);
3. automação (`scripts/` e `.github/workflows/`);
4. documentação operacional de release.

Se qualquer camada divergir, a correção ocorre na causa estrutural e não em ajuste pontual.

## Estado atual
Os arquivos em `Arme/` seguem como fonte de requisitos e hipóteses técnicas. O avanço de fase exige converter texto em contrato verificável por script/workflow.

## Diretrizes de refatoração sistêmica
1. **Baseline por módulo**
   - catalogar entradas, saídas, invariantes e critérios de falha;
   - registrar impacto em arm32/arm64 e JNI, quando aplicável.
2. **Normalização de contratos**
   - definir ABI, calling convention, alinhamento, registradores, símbolos exportados e flags mínimas.
3. **Migração incremental controlada**
   - primeiro C freestanding validado;
   - depois ASM apenas onde houver ganho real (latência, tamanho, determinismo, auditabilidade).
4. **Separação explícita de trilha de release**
   - unsigned: validação interna/reprodutibilidade;
   - signed: trilha oficial com segurança preservada (sem atalho de conveniência).
5. **Paridade arquitetural obrigatória**
   - arm32 e arm64 devem manter equivalência de comportamento e contratos.

## Estrutura alvo sugerida
- `Arme/spec/` — contratos canônicos (ABI, memória, bootstrap, assinatura)
- `Arme/src/c/` — transição C freestanding
- `Arme/src/asm/arm32/` — ASM armv7
- `Arme/src/asm/arm64/` — ASM aarch64
- `Arme/tests/contracts/` — testes de contrato e equivalência
- `Arme/docs/` — operação, release e troubleshooting

## Política de qualidade e merge gate
- Nenhuma mudança entra sem contrato explícito de entrada/saída.
- Toda entrega indica alvo arquitetural e impacto em build/release.
- Divergência entre documentação, script e workflow bloqueia merge.
- Erro real não pode ser mascarado por fallback silencioso.

## Fluxo CI/CD mínimo para ecossistema
- Lint e consistência de contratos/documentação.
- Build matrix Android (arm32 + arm64).
- Geração de APK unsigned para validação interna.
- Geração de APK signed em trilha oficial com secrets.
- Upload de artefatos com naming padronizado e metadados de rastreio.

## Entregáveis mínimos por etapa
- **E1**: inventário técnico e mapa de dependências cruzadas.
- **E2**: contratos ABI/memória/símbolos versionados e verificáveis.
- **E3**: bootstrap freestanding compilando arm32+arm64.
- **E4**: ASM crítico com testes de equivalência.
- **E5**: release auditável com assinados + não assinados e trilhas separadas.

## Definição de pronto da refatoração
A etapa só fecha quando houver evidência verificável de:
- compilação funcional;
- upload de artefatos configurado;
- documentação mínima alinhada;
- diff claro e rastreável no histórico Git.
