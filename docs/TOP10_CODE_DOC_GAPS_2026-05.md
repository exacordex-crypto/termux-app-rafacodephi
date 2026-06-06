# Top 10 MD — sincronização código ↔ documentação (2026-05-14)

Objetivo: registrar **o que já existe no código** e ainda não está explícito (ou está incompleto) na documentação principal.

## Método objetivo

Foram revisados 10 markdowns de operação/build/release e comparados com scripts/workflows ativos do repositório.

## 1) README.md

### Gap encontrado
- README não explicita que existe gate formal de contrato de release via `verifyReleaseContract` antes de upload no fluxo de matriz.

### Evidência em código
- Workflow `apk_matrix_build.yml` executa `./gradlew verifyReleaseContract` antes do upload de artefatos.

### Ação recomendada no MD
- Adicionar bloco curto “Gate obrigatório de release” citando a task Gradle.

## 2) RAFCODEPHI_BOOTSTRAP_CONTRACT.md

### Gap encontrado
- Documento cobre `--prepare` e `--check`, mas não destaca que o builder emite também artefatos de release por script dedicado (`build_release_artifacts.sh`) com validações extras.

### Evidência em código
- `scripts/build_release_artifacts.sh` faz preflight Android, valida contrato de bootstrap e roda assemble/test.

### Ação recomendada no MD
- Incluir seção “Pipeline local recomendado” com `prepare_bootstrap_env.sh` + `build_release_artifacts.sh`.

## 3) VALIDATION_COMMANDS.md

### Gap encontrado
- Foco excessivo em Android 15; faltam comandos canônicos já usados no CI para contrato de release e consistência ABI.

### Evidência em código
- Existem verificadores dedicados: `scripts/validate_release_pipeline_contract.sh`, `scripts/validate_abi_policy_consistency.sh`.

### Ação recomendada no MD
- Acrescentar seção “Release/ABI gates” com esses comandos.

## 4) docs/BUILD_APK_MATRIX.md

### Gap encontrado
- Documento descreve saída `dist/apk-matrix/`, mas não menciona a distinção contratual entre trilha `official` e `internal` presente no workflow.

### Evidência em código
- `apk_matrix_build.yml` separa regras por trilha e exige assinatura oficial sob condição de input/secrets.

### Ação recomendada no MD
- Inserir tabela de diferenças entre trilhas oficial/interna (o que quebra em cada uma).

## 5) docs/BETA_CI_ARTIFACTS.md

### Gap encontrado
- Não documenta com precisão todos os workflows de artefatos hoje existentes (ex.: variantes, attach debug, runtime smoke).

### Evidência em código
- Workflows ativos: `apk_matrix_artifacts_variants.yml`, `attach_debug_apks_to_release.yml`, `device-runtime-smoke.yml`.

### Ação recomendada no MD
- Atualizar inventário de workflows e respectivos artefatos publicados.

## 6) docs/BETA_BOOTSTRAP_VALIDATION.md

### Gap encontrado
- Não explicita o modo `--prepare-dev` para gerar bootstrap local sem download remoto.

### Evidência em código
- `scripts/verify_bootstrap_contract.sh` possui opção `--prepare-dev`.

### Ação recomendada no MD
- Documentar cenário offline/dev bootstrap e limitações.

## 7) docs/BETA_BUILD_FLAGS.md

### Gap encontrado
- Falta mapear variáveis de assinatura release já suportadas no fluxo local.

### Evidência em código
- `scripts/setup_android_signing.sh` e `README` usam `TERMUX_ENABLE_RELEASE_SIGNING` e variáveis de keystore.

### Ação recomendada no MD
- Adicionar matriz “flag -> efeito -> trilha (internal/oficial)”.

## 8) docs/CI_WORKFLOW_OWNERSHIP.md

### Gap encontrado
- Não inclui ownership explícito dos workflows recentes de governança ABI e compatibilidade arm32.

### Evidência em código
- Workflows presentes: `abi_policy_consistency.yml`, `compatibility-arm32.yml`, `compatibility-arm32-ndk29.yml`.

### Ação recomendada no MD
- Atualizar tabela de ownership e SLA por workflow crítico.

## 9) docs/ENGINEERING_SYSTEM_RUNBOOK.md

### Gap encontrado
- Runbook não traz sequência operacional mínima reproduzível ponta-a-ponta (preflight -> bootstrap -> build matrix -> contrato).

### Evidência em código
- Scripts existem e são encadeáveis: `ci_android_preflight.sh`, `prepare_bootstrap_env.sh`, `build_apk_matrix.sh`, `validate_release_pipeline_contract.sh`.

### Ação recomendada no MD
- Incluir seção “Comandos em ordem de execução” com tempo estimado e critérios de sucesso.

## 10) docs/STATUS.md

### Gap encontrado
- Status não referencia explicitamente o gate de runtime device (`device_runtime_smoke.sh`) para fechar ciclo de validação além de build.

### Evidência em código
- Script e workflow dedicados: `scripts/device_runtime_smoke.sh` e `.github/workflows/device-runtime-smoke.yml`.

### Ação recomendada no MD
- Incluir indicador de saúde “build green + runtime smoke green”.

---

## Comandos canônicos para fechar o delta de documentação

```bash
./scripts/ci_android_preflight.sh
bash scripts/verify_bootstrap_contract.sh --check
./scripts/build_apk_matrix.sh
bash scripts/validate_release_pipeline_contract.sh
bash scripts/validate_abi_policy_consistency.sh
```

## Critério de done desta revisão

- Os 10 MDs acima foram mapeados com lacunas concretas e evidências em código.
- O foco ficou em coerência de build/release/CI/ABI/runtime, sem alterar contrato de segurança da trilha oficial.
