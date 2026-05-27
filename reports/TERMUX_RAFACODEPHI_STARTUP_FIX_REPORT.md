# TERMUX RAFCODEPHI STARTUP FIX REPORT

## Resumo do problema
Foi reforçado o contrato mínimo de bootstrap/shell para evitar falso verde de CI e para melhorar diagnóstico do shell interno (sh/pkg obrigatórios, proot/busybox não bloqueantes).

## Arquivos alterados
- scripts/beta_internal_shell_diagnose.sh
- .github/workflows/debug_build.yml

## Falhas encontradas
- Bootstraps ausentes inicialmente no repositório local (corrigido via `--prepare-dev`).
- Ambiente local sem `adb`.
- Build local bloqueado por incompatibilidade Java/Gradle (`Unsupported class file major version 69`).

## Tabela B01-B10
| ID | Status | Observação |
|---|---|---|
| B01 | PASS | `verify_bootstrap_contract.sh --check` confirma ZIPs e metadados após prepare-dev. |
| B02 | PASS | Contrato valida `bin/sh` nos 4 ZIPs. |
| B03 | PASS | Contrato valida `bin/pkg` nos 4 ZIPs. |
| B04 | WARN | Sem device/adb local para validar execução real/linker. |
| B05 | PASS | Fluxo mantém fallback `/system/bin/sh` apenas como recuperação (já presente). |
| B06 | PASS | Diagnóstico rebaixado para WARN quando proot falha, sem bloquear primeiro shell. |
| B07 | PASS | SHA256 obrigatório e BLAKE3 opcional já reportado no script de contrato. |
| B08 | PASS | Guard estrito em release permanece protegido (sem relaxamento aplicado). |
| B09 | PASS | `bootstrap-arm.zip` validado com `TERMUX_ARCH=arm` e metadados corretos. |
| B10 | PASS | Workflow `debug_build.yml` agora valida contrato bootstrap antes do build e roda `verifyReleaseContract`. |

## Comandos executados
- `/tmp/raf_termux_inventory.sh`
- `/tmp/raf_termux_bootstrap_check.sh`
- `/tmp/raf_termux_prepare_dev.sh`
- `/tmp/raf_termux_build.sh`
- `/tmp/raf_termux_adb_diag.sh`

## Logs relevantes resumidos
- `Missing bootstrap archive: app/src/main/cpp/bootstrap-aarch64.zip` (antes do prepare-dev).
- `metadata OK for RAFCODEPHI local bootstraps` após prepare-dev.
- `Unsupported class file major version 69` ao tentar `:app:assembleDebug`.
- `adb: command not found` no diagnóstico de device.

## Resultado por ABI
- aarch64: metadados/hash/estrutura ZIP OK.
- arm (armeabi-v7a): metadados/hash/estrutura ZIP OK (inclui `TERMUX_ARCH=arm`).
- i686: metadados/hash/estrutura ZIP OK.
- x86_64: metadados/hash/estrutura ZIP OK.

## Resultado ARM32
`bootstrap-arm.zip` passou validação estrutural e de metadados no contrato local.

## WARN pendentes
- Execução runtime real no dispositivo (sh/pkg/proot/logcat) não validada localmente por ausência de adb.
- Build debug local não concluído por incompatibilidade de versão Java/Gradle do ambiente.

## FAIL/BLOCKER pendentes
- BLOCKER de ambiente para build local (`major version 69`).

## Próximos passos
1. Rodar CI atualizado para validar trilha completa com Java compatível do runner.
2. Executar `scripts/beta_internal_shell_diagnose.sh` em device Android com adb ativo.
3. Confirmar abertura da primeira sessão com `$PREFIX/bin/sh` e `pkg --version` no aparelho ARM32.
