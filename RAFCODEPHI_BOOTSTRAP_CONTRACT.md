# RAFCODEΦ Bootstrap Contract

Este documento fixa o bootstrap como requisito obrigatório da fork `termux-app-rafacodephi`.

## Decisão técnica

O app só deve ser considerado operacional quando houver bootstrap íntegro, verificável e compatível com a ABI alvo. Um build que compila mas não entrega shell funcional não é considerado entrega válida.

## Fonte de verdade

Os arquivos bootstrap esperados são gerados pela task Gradle existente:

```bash
./gradlew :app:downloadBootstraps --no-daemon
```

Os arquivos esperados ficam em:

```text
app/src/main/cpp/bootstrap-aarch64.zip
app/src/main/cpp/bootstrap-arm.zip
app/src/main/cpp/bootstrap-i686.zip
app/src/main/cpp/bootstrap-x86_64.zip
```

Esses arquivos podem ser artefatos de build e não precisam ser versionados em Git. O contrato obrigatório é: eles precisam existir, ser ZIPs válidos e ter hashes rastreáveis durante build, CI ou preparação local.

## Requisitos mínimos

### Build-time

- `app/src/main/cpp/bootstrap-aarch64.zip` existe e é ZIP válido.
- `app/src/main/cpp/bootstrap-arm.zip` existe e é ZIP válido.
- `app/src/main/cpp/bootstrap-i686.zip` existe e é ZIP válido.
- `app/src/main/cpp/bootstrap-x86_64.zip` existe e é ZIP válido.
- SHA256 deve ser emitido para cada arquivo.
- BLAKE3 deve ser emitido quando `b3sum` estiver disponível no ambiente.
- Espaço livre mínimo padrão: `1024 MB`, ajustável por `MIN_FREE_MB`.

### Runtime Termux

Quando executado dentro de uma instalação Termux/RAFCODEΦ, o contrato exige:

```text
$PREFIX
$PREFIX/bin/sh ou $PREFIX/usr/bin/sh
$PREFIX/bin/pkg ou $PREFIX/usr/bin/pkg
$PREFIX/bin/proot ou $PREFIX/usr/bin/proot
$PREFIX/bin/busybox ou $PREFIX/usr/bin/busybox
```

Se `PREFIX` ou `TERMUX_PREFIX` não estiver definido, a checagem de runtime é tratada como ambiente de build, não como falha.

## Comandos canônicos

Preparar e validar bootstrap:

```bash
bash scripts/verify_bootstrap_contract.sh --prepare
```


Gerar bootstrap developer local em C lowlevel (sem download remoto):

```bash
bash scripts/verify_bootstrap_contract.sh --prepare-dev
```

Validar bootstrap já existente:

```bash
bash scripts/verify_bootstrap_contract.sh --check
```

Validar somente runtime instalado:

```bash
bash scripts/verify_bootstrap_contract.sh --runtime-prefix-only
```

Reduzir ou elevar o limite de espaço livre:

```bash
MIN_FREE_MB=2048 bash scripts/verify_bootstrap_contract.sh --prepare
```


## Implementação lowlevel

A verificação usa um checker lowlevel em C (`scripts/bootstrap_zip_contract_check.c`) compilado no ato e executado sem heap/malloc para validar assinatura local + EOCD do ZIP, junto de `sha256sum` e `b3sum` opcional.

## Bootstrap próprio (source)

Fonte bootstrap developer no repositório:

```text
bootstrap_src/common/bin/sh
bootstrap_src/common/bin/pkg
bootstrap_src/common/etc/motd
```

Para build Gradle usando bootstrap local (sem download remoto):

```bash
RAF_BOOTSTRAP_SOURCE=local ./gradlew :app:ensureBootstrapArchives --no-daemon
```


## Assinatura segura

Nunca versionar keystore ou segredos. Use variáveis:

- `TERMUX_SIGNING_STORE_FILE`
- `TERMUX_SIGNING_STORE_PASSWORD`
- `TERMUX_SIGNING_KEY_ALIAS`
- `TERMUX_SIGNING_KEY_PASSWORD`
