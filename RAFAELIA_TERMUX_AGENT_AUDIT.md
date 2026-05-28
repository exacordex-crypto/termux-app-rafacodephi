# RAFAELIA TERMUX AGENT AUDIT

## Objetivo
Garantir bootstrap funcional por ABI real e shell operacional em `$PREFIX/bin/sh` (ou fallback em `$PREFIX/usr/bin/sh`).

## Checklist executável

```bash
bash scripts/verify_bootstrap_contract.sh --prepare-dev
bash scripts/verify_bootstrap_contract.sh --check
PREFIX="$PWD/_fake_prefix" bash scripts/verify_bootstrap_contract.sh --runtime-prefix-only
bash scripts/diagnose-runtime-shell.sh
```

## Diagnóstico runtime

O script `scripts/diagnose-runtime-shell.sh` valida:

- ABI atual (`getprop ro.product.cpu.abi` ou `uname -m`);
- valores de `PREFIX` e `TERMUX_PREFIX`;
- presença executável de `bin/sh` ou `usr/bin/sh`;
- presença executável de `bin/pkg` ou `usr/bin/pkg`;
- presença executável de `bin/proot`/`usr/bin/proot` e `bin/busybox`/`usr/bin/busybox`;
- smoke test de shell com `-lc 'echo RAFAELIA_RUNTIME_SHELL_OK'`.

## Variáveis seguras de assinatura

Use somente variáveis de ambiente (não commitar segredos no Git):

- `TERMUX_SIGNING_STORE_FILE`
- `TERMUX_SIGNING_STORE_PASSWORD`
- `TERMUX_SIGNING_KEY_ALIAS`
- `TERMUX_SIGNING_KEY_PASSWORD`

Recomendação:

- manter keystore fora do repositório;
- usar `.gitignore` para caminhos locais de assinatura;
- injetar variáveis no CI/CD via secret manager.
