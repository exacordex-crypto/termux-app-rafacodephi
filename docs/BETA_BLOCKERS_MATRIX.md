# BETA BLOCKERS MATRIX — Internal Shell Start

| ID | fase | sintoma | causa provável | arquivo/função | comando de diagnóstico | correção provável | status |
|---|---|---|---|---|---|---|---|
| B01 | files-access | `files/usr` ausente | app sem bootstrap completo ou falha de permissão | `TermuxInstaller.setupBootstrapIfNeeded` | `adb shell ls -la /data/data/com.termux.rafacodephi/files` | corrigir criação de diretórios e permissões | OPEN |
| B02 | prefix-check | `bin/sh` ausente | payload bootstrap incompleto | `TermuxInstaller.verifyRuntimeBinary` | `adb shell ls -la /data/data/com.termux.rafacodephi/files/usr/bin/sh` | reinstalar bootstrap e validar zip | OPEN |
| B03 | verify-pkg | `pkg --version` falha | `pkg` ausente/corrompido | `TermuxInstaller` / `TermuxSession.execute` | `adb shell /data/data/com.termux.rafacodephi/files/usr/bin/pkg --version` | reextrair bootstrap e checar hash | OPEN |
| B04 | verify-sh | shell não inicia | bit de execução ausente ou linker incompatível ABI | `TermuxInstaller.verifyRuntimeBinary` | `adb shell /data/data/com.termux.rafacodephi/files/usr/bin/sh -c 'echo RAF_SHELL_OK'` | ajustar chmod/payload ABI correto | OPEN |
| B05 | shell-select | fallback para `/system/bin/sh` sempre | nenhum shell candidato executável em `$PREFIX/bin` | `TermuxSession.execute` | `adb shell ls -la /data/data/com.termux.rafacodephi/files/usr/bin/{bash,zsh,fish,sh}` | instalar/reparar shells no prefixo | OPEN |
| B06 | verify-proot | proot existe mas falha | binário incompatível/sem permissão | `TermuxInstaller.verifyRuntimeBinary` | `adb shell /data/data/com.termux.rafacodephi/files/usr/bin/proot --version` | rebuild proot para ABI alvo | OPEN |
| B07 | blake3-check | erro de integridade bootstrap | hash esperado divergente/ausente | `BootstrapIntegrityVerifier` | `adb logcat -d | grep BootstrapIntegrity` | corrigir hash buildConfig ou payload | OPEN |
| B08 | baremetal-guard | crash em strict | validação baremetal detectou violação | `BootstrapBaremetalGuard.validateAfterBootstrap` | `adb logcat -d | grep BootstrapBaremetalGuard` | corrigir bootstrap + manter strict release | OPEN |
