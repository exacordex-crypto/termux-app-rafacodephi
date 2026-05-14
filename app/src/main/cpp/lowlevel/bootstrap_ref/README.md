# ABI bootstrap reference (ARM32 + ARM64)

Este diretório é a **fonte de referência textual** extraída de `asm.sh`.

- `asm.sh` **não é script de build**; ele contém um heredoc com especificação/blueprint.
- A extração separa as seções por ABI em arquivos dedicados para uso em bootstrap nativo.

## Regenerar

```bash
python3 tools/bootstrap/extract_abi_bootstrap.py
```

Arquivos gerados:
- `rafaelia_abi_header.txt`
- `bootstrap_arm64_adaptive.s.txt`
- `bootstrap_arm32_adaptive.s.txt`
