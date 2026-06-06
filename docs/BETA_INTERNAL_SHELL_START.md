# BETA INTERNAL SHELL START (RAFCODEΦ)

## Cadeia de start
1. `TermuxActivity` inicia fluxo UI e solicita bootstrap/sessão.
2. `TermuxService` mantém ciclo de vida de sessões.
3. `TermuxInstaller.setupBootstrapIfNeeded()` valida e instala `$PREFIX`.
4. `createInitialSession()` dispara primeira sessão.
5. `TermuxSession.execute()` seleciona shell e executa processo final.

## Por que bash não é obrigatório
O bootstrap mínimo para primeiro start **não exige bash**. O seletor de shell tenta candidatos (`bash`, `zsh`, `fish`, `sh`) e, se não houver shell no prefixo, cai em `/system/bin/sh` para recuperação.

## Contrato mínimo
Para bootstrap saudável do shell interno:
- `$PREFIX/bin/sh` (obrigatório)
- `$PREFIX/bin/pkg` (obrigatório)

`bash` é recomendado para experiência completa, mas não hard dependency.

## Papel de busybox e proot
- `busybox`: utilitários auxiliares; útil, porém não bloqueia shell básico.
- `proot`: ambiente/chroot userspace; ausência deve ser **WARN** no diagnóstico, não bloqueio do primeiro shell.

## ABI/kernel/page size
Diagnóstico beta deve capturar:
- `ro.product.cpu.abi`
- `ro.product.cpu.abilist`
- `ro.build.version.sdk`
- `uname -a`
- `getconf PAGE_SIZE`

Esses dados ajudam a validar compatibilidade ABI, linker, kernel e constraints como page-size (ex.: 16384 em Android 15).

## Logcat para investigação
Usar filtro:
- `TermuxInstaller`
- `BootstrapIntegrity`
- `BootstrapBaremetalGuard`
- `TermuxActivity`
- `TermuxService`
- `TermuxSession`
- `linker|exec|RuntimeException`

Comando sugerido:
```bash
adb logcat -d | grep -E "TermuxInstaller|BootstrapIntegrity|BootstrapBaremetalGuard|TermuxActivity|TermuxService|TermuxSession|linker|exec|RuntimeException"
```

## Strict mode: diferença debug x release
`BOOTSTRAP_BAREMETAL_STRICT`:
- `false` (debug/internal beta): permite diagnóstico e coleta de dados mesmo sem hash/validação completa.
- `true` (release/strict): falha forte em violações de integridade/guard.

## Interpretação de falhas
- **PASS**: esperado, executável e funcional.
- **WARN**: problema não-bloqueante para primeiro shell (ex.: proot ausente).
- **FAIL**: erro funcional que afeta requisito direto (`sh/pkg` quebrado).
- **BLOCKER**: sem pré-requisito de diagnóstico/execução (ADB indisponível, app ausente).
