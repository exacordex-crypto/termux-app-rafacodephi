# Bootstrap Gap Report — Termux/RAFCODEΦ

Data da auditoria: 2026-05-14 (UTC)

## Escopo
Auditoria focada no bootstrap real (Termux) e modo local de desenvolvimento RAFCODEΦ, sem substituir trilha oficial por `bootstrap_rafaelia`.

## Resultado por item

1. **Arquivos bootstrap obrigatórios** (`app/src/main/cpp/bootstrap-{aarch64,arm,i686,x86_64}.zip`)
   - **resolvido** (após geração local para auditoria)
   - Reprodução: `bash scripts/generate_developer_bootstraps.sh`

2. **Cada arquivo é ZIP válido**
   - **resolvido**
   - Validação lowlevel via `scripts/bootstrap_zip_contract_check.c` compilado e executado por `scripts/verify_bootstrap_contract.sh --check`

3. **Cada arquivo gera SHA256**
   - **resolvido**
   - `scripts/verify_bootstrap_contract.sh --check` emite SHA256 para os 4 arquivos

4. **Cada arquivo gera BLAKE3 quando b3sum existir**
   - **bloqueado por dependência externa** (ausência de `b3sum` neste ambiente)
   - Comportamento correto: fallback sem quebrar build, com SHA256 mantido

5. `./gradlew :app:downloadBootstraps --no-daemon`
   - **bloqueado por dependência externa**
   - Falha real: Android SDK não configurado (`SDK location not found`)

6. `bash scripts/verify_bootstrap_contract.sh --prepare`
   - **bloqueado por dependência externa**
   - Falha real: depende de `:app:downloadBootstraps`, que falha por Android SDK ausente

7. `bash scripts/verify_bootstrap_contract.sh --check`
   - **resolvido**
   - Executa e valida zip/hashes/metadata corretamente

8. `RAF_BOOTSTRAP_SOURCE=local ./gradlew :app:ensureBootstrapArchives --no-daemon`
   - **faltando** neste ambiente para execução completa (SDK ausente)
   - **correção aplicada**: dependências de `validateRewrittenBootstraps` deixam de ser forçadas no modo local

9. Fontes mínimos do bootstrap local
   - `bootstrap_src/common/bin/sh`: **resolvido**
   - `bootstrap_src/common/bin/pkg`: **resolvido**
   - `bootstrap_src/common/etc/motd`: **resolvido**

10. Validação runtime de `$PREFIX`, `$PREFIX/bin/sh`, `$PREFIX/bin/pkg`
   - **bloqueado por dependência externa/contexto**
   - Script reconhece corretamente; neste ambiente `PREFIX/TERMUX_PREFIX` não está setado e o check é pulado por design

11. Checker C `scripts/bootstrap_zip_contract_check.c` compilado/uso correto sem heap/malloc
   - **resolvido**
   - Compilado por `cc -O2 -std=c11 -Wall -Wextra -Werror ...` e usado no `--check`

12. Build release bloqueia ausência de bootstrap válido
   - **resolvido (contrato de código)**
   - `verifyBootstrapZipsPresent` falha quando faltar ZIP obrigatório; tarefas críticas dependem de `ensureBootstrapArchives`
   - Execução end-to-end bloqueada neste ambiente por Android SDK ausente

## Correções mínimas aplicadas

A. Gradle preparar/invocar bootstrap antes de builds críticos
- Mantido `ensureBootstrapArchives` em preBuild/javaCompile/externalNativeBuild.
- Ajuste mínimo: não forçar `validateRewrittenBootstraps` quando `RAF_BOOTSTRAP_SOURCE=local`.

B. CI falhar se faltar bootstrap obrigatório
- Contrato já presente via `verify_bootstrap_contract.sh --check` e `verifyBootstrapZipsPresent`.
- Sem relaxamento aplicado.

C. SHA256 sempre emitido
- Confirmado no checker (`emit_hashes_lowlevel`) sem alteração adicional.

D. BLAKE3 opcional sem quebrar build
- Confirmado no checker: ausência de `b3sum` não quebra.

E. Modo local gera arquivos válidos para desenvolvimento
- Confirmado com `scripts/generate_developer_bootstraps.sh` + `--check`.
- Ajuste no Gradle evita dependência indevida de rewrite upstream no modo local.

F. Status claro com reprodução
- Este relatório e o JSON anexo listam `resolvido`, `faltando`, `bloqueado por dependência externa` e comandos exatos.

## Comandos executados (reprodução)

- `./gradlew :app:downloadBootstraps --no-daemon`
- `bash scripts/verify_bootstrap_contract.sh --prepare`
- `bash scripts/verify_bootstrap_contract.sh --check`
- `RAF_BOOTSTRAP_SOURCE=local ./gradlew :app:ensureBootstrapArchives --no-daemon`
- `bash scripts/generate_developer_bootstraps.sh`
- `./gradlew assembleDebug --no-daemon`
- `./gradlew verifyReleaseContract --no-daemon`

