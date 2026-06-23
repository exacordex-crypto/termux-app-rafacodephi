# RAFAELIA / VECTRA_OS — Bug Master Index
> Repositório: `exacordex-crypto/termux-app-rafacodephi`
> Fork chain: `termux/termux-app` → `rafaelmeloreisnovo` → `exacordex-crypto`
> Data da análise: 2025-06
> Commits analisados: 2.499 (branch `master`)

---

## Sumário executivo

O repositório contém **4 bugs críticos catalogados no AGENTS.md** e **N bugs estruturais
inferidos** da análise de código. Nenhum dos 4 pode ser fechado com patch simples — todos
requerem intervenção cirúrgica nos invariantes do sistema.

| ID | Componente | Tipo | Severidade | Estado |
|----|-----------|------|------------|--------|
| BUG-01 | `attractor_table` | Faltante | 🔴 CRÍTICO | 40/42 ausentes |
| BUG-02 | Atrator #22 | Estrutural/Teórico | 🔴 CRÍTICO | VOID paradox |
| BUG-03 | `vectra_pulse.S` | AArch64 ASM | 🟠 ALTO | 4 bugs abertos |
| BUG-04 | Bootstrap | Hardcode | 🟡 MÉDIO | `com.termux` fixo |
| BUG-05 | `ZrManifest` | Stack overflow | 🔴 CRÍTICO | Uso incorreto em stack |
| BUG-06 | `CtiScanner` | Race condition | 🟠 ALTO | Sem barrier em TOROID |
| BUG-07 | Build pipeline | Hash mismatch | 🟡 MÉDIO | BLAKE3 skip silencioso |
| BUG-08 | RAFAELIA pipeline | Invariante | 🟠 ALTO | φ=(1-H)·C não verificado |

---

## Invariantes do sistema (referência)

```
gcd(Δr, R) = 1      onde R = 42
|A| = 42             tamanho da attractor_table
period(BitOmega) = 42
φ = (1 - H) · C     Lyapunov (convergência)
x0 = state ptr
x1 = C   (coerência)
x2 = H   (entropia)
x3 = phase
x4 = attractor
```

---

## Dependências entre bugs

```
BUG-02 (VOID #22)
    └─→ bloqueia BUG-01 (attractor_table incompleta)
           └─→ bloqueia BUG-03 (vectra_pulse.S sem tabela válida)
                  └─→ bloqueia validação de φ = (1-H)·C (BUG-08)

BUG-04 (bootstrap) é independente — pode ser resolvido em paralelo
BUG-05 (ZrManifest stack) é independente — fix cirúrgico
BUG-06 (race condition CtiScanner) depende de BUG-03 ser estável
BUG-07 (BLAKE3 skip) é independente
```

**Ordem de resolução obrigatória:**
1. BUG-02 → resolução teórica do VOID paradox
2. BUG-01 → geração completa da attractor_table com os 42 estados
3. BUG-03 → fix dos 4 bugs AArch64 com tabela válida como base
4. BUG-04, BUG-05, BUG-06, BUG-07, BUG-08 em paralelo

---

## Módulos em estado Production (não tocar)

| Módulo | Arquivo | Status |
|--------|---------|--------|
| CTI BITSTACK | `rmr/Rrr/cti_raw_reader.h/.c` | ✅ Production (PR #190) |
| ZIPRAF core | `rmr/Rrr/zipraf_index.h/.c` | ✅ Production (PR #190) |
| RAFAELIA_MATH_FORMULAS | `rmr/Rrr/RAFAELIA_MATH_FORMULAS.md` | ✅ Spec canônica |

---

## Áreas de risco não catalogadas formalmente

### 1. Sequência Fibonacci-Rafael e alinhamento de estados
O mapeamento Δ_Rafael das sementes Fibonacci para estados do atrator
não está explicitamente provado ser bijetivo para os 42 estados.
F(8)=21, F(9)=34 — o intervalo [22..33] não tem semente Fibonacci
direta, o que implica que ~12 estados do atrator dependem de
interpolação não especificada.

### 2. ZrManifest em uso dinâmico
O manifesto ZIPRAF de ~59KB **deve ser `static`**. Qualquer uso em
stack de thread causará stack overflow silencioso no Android
(stack size padrão de thread = 1MB em NDK, mas em Termux pode ser
menor). Ver BUG-05.

### 3. Page alignment 16KB em Android 15/16
`-Wl,-z,max-page-size=16384` está configurado, mas segmentos de
dados do `ZrManifest` estático precisam de `__attribute__((aligned(16384)))`
para garantir que não cruzem fronteiras de página.

### 4. Modo DELTA_MISS do CtiScanner
O modo de varredura `DELTA_MISS` do CTI depende implicitamente do
`attractor_table` para calcular o delta de missão. Com 40/42 estados
ausentes, o modo DELTA_MISS produz comportamento indefinido.

---

## Checksums de arquivos críticos (para validação futura)

| Arquivo | Tamanho esperado | Observação |
|---------|-----------------|------------|
| `cti_raw_reader.h` | 89 linhas | Especificação completa |
| `zipraf_index.h` | ~120 linhas | 8 modos × 33 níveis |
| `attractor_table.c` | **FALTANTE** | Deve ter 42 entradas |
| `vectra_pulse.S` | Parcial | 4 bugs conhecidos |

---

## Referências

- `AGENTS.md` (root do repositório) — fonte primária dos BUG-01..04
- `rmr/Rrr/RAFAELIA_MATH_FORMULAS.md` — especificação matemática canônica
- `rmr/Rrr/cti_raw_reader.h` — contrato CTI BITSTACK
- PR #190 — merge de CTI BITSTACK + ZIPRAF
- VECTRA_OS.md — especificação ABI AArch64
