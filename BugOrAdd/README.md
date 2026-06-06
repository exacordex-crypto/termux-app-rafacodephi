# BugOrAdd — Catálogo Completo + Plano de Refatoração Técnica

Este README substitui o inventário parcial e agora **cataloga 100% dos arquivos (98/98)**, com classificação técnica e plano de remodelação focado em:
- **Leveza operacional** (baixo uso de memória e caminhos quentes sem heap).
- **Versões paralelas**: ASM low-level, C freestanding sem malloc, e C syscall-only.
- **Convergência arquitetural** para Android/Termux API 28+ com foco ARM32/ARM64.

## 1) Inventário atual

- Total de arquivos: **98**.
- Distribuição por tipo: **Archive: 8**, **Assembly: 24**, **Binary/Other: 5**, **Build: 2**, **C: 34**, **Doc/Data: 7**, **Header: 5**, **Java: 1**, **Script: 12**.

### 1.1 Catálogo de arquivos (com classificação)

| # | Arquivo | Classe |
|---:|---|---|
| 1 | `1.zip` | Archive |
| 2 | `42.c` | C |
| 3 | `Android_nomalloc.mk` | Build |
| 4 | `Application.mk` | Build |
| 5 | `DOSSIÊ_RAFAELIA_VECTRA_C_ASM.txt` | Doc/Data |
| 6 | `RAFAELIA_MATH_FORMULAS.md` | Doc/Data |
| 7 | `README.md` | Doc/Data |
| 8 | `RafaeliaCore.java` | Java |
| 9 | `Readme.md` | Doc/Data |
| 10 | `asm.zip` | Archive |
| 11 | `baremetal_nomalloc.c` | C |
| 12 | `baremetal_nomalloc.h` | Header |
| 13 | `bench.c` | C |
| 14 | `bench.s` | Assembly |
| 15 | `bitstack.c` | C |
| 16 | `bitstack.h` | Header |
| 17 | `blink_crc_bench.c` | C |
| 18 | `blink_dual_ilp.c` | C |
| 19 | `blink_multicore.c` | C |
| 20 | `blink_neon.c` | C |
| 21 | `blink_neon_feedback.c` | C |
| 22 | `blink_neon_feedback_fix.c` | C |
| 23 | `blink_neon_multicore.c` | C |
| 24 | `blink_neon_multistream.c` | C |
| 25 | `buffer.c` | C |
| 26 | `build_all.sh` | Script |
| 27 | `build_asm.sh` | Script |
| 28 | `build_asm32_2.sh` | Script |
| 29 | `build_b3_nopie.sh` | Script |
| 30 | `build_b3_start.sh` | Script |
| 31 | `core.s` | Assembly |
| 32 | `core_entropy.s` | Assembly |
| 33 | `core_ilp.s` | Assembly |
| 34 | `core_microkernel.s` | Assembly |
| 35 | `core_neon.s` | Assembly |
| 36 | `core_scalar.s` | Assembly |
| 37 | `dia2.md` | Doc/Data |
| 38 | `diagnose.sh` | Script |
| 39 | `diagnose_termux.sh` | Script |
| 40 | `exec_scan_results.txt` | Doc/Data |
| 41 | `final.s` | Assembly |
| 42 | `fix_asm32.sh` | Script |
| 43 | `geo_seed_core.s` | Assembly |
| 44 | `geo_seed_core_v2.s` | Assembly |
| 45 | `geo_seed_core_v3.s` | Assembly |
| 46 | `gerar_dossie_rafaelia.sh` | Script |
| 47 | `hyperforms.json` | Doc/Data |
| 48 | `main.c` | C |
| 49 | `maio.zip` | Archive |
| 50 | `maio_S.zip` | Archive |
| 51 | `maio_h.zip` | Archive |
| 52 | `maio_py.zip` | Archive |
| 53 | `maio_sS.zip` | Archive |
| 54 | `maio_sh.zip` | Archive |
| 55 | `poly_link.c` | C |
| 56 | `poly_opt.c` | C |
| 57 | `raf_asm_b1.S` | Assembly |
| 58 | `raf_bench.c` | C |
| 59 | `rafaelia_arena.h` | Header |
| 60 | `rafaelia_b1.S` | Assembly |
| 61 | `rafaelia_b2.S` | Assembly |
| 62 | `rafaelia_b3` | Binary/Other |
| 63 | `rafaelia_b3.S` | Assembly |
| 64 | `rafaelia_b3.S.bak` | Binary/Other |
| 65 | `rafaelia_b3_android` | Binary/Other |
| 66 | `rafaelia_b3_android.c` | C |
| 67 | `rafaelia_b3_pie.S` | Assembly |
| 68 | `rafaelia_b4.S` | Assembly |
| 69 | `rafaelia_b5.S` | Assembly |
| 70 | `rafaelia_b6.S` | Assembly |
| 71 | `rafaelia_b7.S` | Assembly |
| 72 | `rafaelia_b8.S` | Assembly |
| 73 | `rafaelia_bench.S` | Assembly |
| 74 | `rafaelia_bench.S.fixbak` | Binary/Other |
| 75 | `rafaelia_bitraf.c` | C |
| 76 | `rafaelia_core.c` | C |
| 77 | `rafaelia_core_armv7_bench.S` | Assembly |
| 78 | `rafaelia_core_armv7_bench.S.bak` | Binary/Other |
| 79 | `rafaelia_edge_v2.c` | C |
| 80 | `rafaelia_edge_v3.c` | C |
| 81 | `rafaelia_edge_v4.c` | C |
| 82 | `rafaelia_edge_v5.c` | C |
| 83 | `rafaelia_edge_v6_unroll.c` | C |
| 84 | `rafaelia_edge_v7_auto.c` | C |
| 85 | `rafaelia_final.S` | Assembly |
| 86 | `rafaelia_glue.c` | C |
| 87 | `rafaelia_gpu_mid.c` | C |
| 88 | `rafaelia_gpu_mid.h` | Header |
| 89 | `rafaelia_jni_direct.c` | C |
| 90 | `rafaelia_master.sh` | Script |
| 91 | `rafaelia_mvp_bench.c` | C |
| 92 | `rafaelia_orchestrator.c` | C |
| 93 | `rafaelia_sigma_omega.c` | C |
| 94 | `rafaelia_types.h` | Header |
| 95 | `scan_execs.sh` | Script |
| 96 | `termux_arm32_build.sh` | Script |
| 97 | `vectra_brenck_industrial.c` | C |
| 98 | `vectras_bbs.c` | C |

## 2) Diretrizes de refatoração completa (target state)

### 2.1 Três trilhas oficiais do mesmo núcleo
1. **asm-lowlevel/**: assembly puro (AArch64/ARMv7), macros, sem dependências libc, syscalls diretas.
2. **c-freestanding/**: C `-ffreestanding -fno-builtin`, sem malloc no hot path, opcional inline asm.
3. **c-syscall/**: C minimalista com wrappers syscall-only (write/mmap/clone/wait4/gettimeofday/exit).

### 2.2 Layout proposto
```text
BugOrAdd/
  include/              # headers compartilhados, tipos Q16.16, contratos
  kernels/
    asm-lowlevel/       # .S/.s produtivos
    c-freestanding/     # .c sem libc no caminho quente
    c-syscall/          # .c com syscalls explícitas
  bench/                # microbench e validações de performance
  tools/                # diagnose/scan/scripts de suporte
  archive/              # .zip/.bak e artefatos históricos
  docs/                 # fórmulas, dossiês e decisões arquiteturais
```

### 2.3 Regras técnicas para excelência
- **Q16.16 obrigatório** em dinâmica toroidal (evitar float no núcleo).
- **Sem heap no hot path**; arena fixa + alinhamento de cache-line.
- **ASM**: contrato de registradores e loops com término verificável.
- **C freestanding**: `-nostdlib` no binário final quando aplicável.
- **Syscall profile** único por arquitetura para reduzir divergência.
- **42 atratores** e período 42 preservados como invariantes de regressão.

## 3) Plano de execução por fases

1. **Higienização**: mover `.zip`, `.bak`, binários e duplicados para `archive/`.
2. **Normalização de build**: unificar scripts em `build_all.sh` + presets por alvo.
3. **Modularização**: separar kernel/bench/docs/tools com includes canônicos.
4. **Tripla implementação**: garantir paridade funcional ASM vs C-freestanding vs C-syscall.
5. **Validação**: benchmark + regressão dos invariantes (42 atratores, período, CRC/BLAKE3).
6. **Hardening**: checagem de tamanho de binário, latência, footprint e fallback ARM32.

## 4) Mapeamento conceitual solicitado (linguagem, toro, entropia)

A modelagem com `T^7`, 42 atratores, mistura EMA (`C`,`H`) e coerência espectral pode ser tratada como:
- **Camada de estado**: representação toroidal e dinâmica discreta (`x_{n+1}=f(x_n)`).
- **Camada semântica**: tokens/símbolos/fonética com pesos por idioma e prosódia.
- **Camada de coerência**: projeção espectral por domínio (linguístico/fisiológico/sinal).
- **Camada de integridade**: hash/CRC/Merkle para verificar estabilidade entre traduções e variações de cadência.

Em termos práticos de engenharia, isso vira **pipeline multi-camada**: aquisição → mapeamento toroidal → atualização de estado → colapso em atrator → verificação criptográfica/estatística.
