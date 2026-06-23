# BUG-03 — vectra_pulse.S: 4 Bugs AArch64 Abertos
> Severidade: 🟠 ALTO — Falhas em assembly bare-metal, sem fallback
> Componente: `asm/vectra_pulse.S` (ou `Arme/vectra_pulse.S`)
> Detectado em: `AGENTS.md` → "vectra_pulse.S: 4 open AArch64 bugs"
> Dependência: Parcialmente bloqueado por BUG-01 (attractor_table incompleta)
> ABI canônica: x0=state ptr, x1=C, x2=H, x3=phase, x4=attractor

---

## Contrato ABI obrigatório (VECTRA_OS / CODEX POLIMATA)

```asm
// Registro de entrada (não modificar sem save/restore):
// x0  = state ptr          (pointer para AttractorState atual)
// x1  = C   (coerência)    Q16.16 fixo
// x2  = H   (entropia)     Q16.16 fixo
// x3  = phase              uint32, 0..41
// x4  = attractor index    uint8, 0..41

// Proibições absolutas:
//   BL para símbolos externos       → não permitido em hot path
//   malloc / libc / printf          → nunca
//   branches com predição imprevisível → usar csel/csinc
//   loops sem prova de terminação via GCD → não permitido

// Invariante de saída:
//   φ = (1-H)·C em Q16.16 carregado em x5 (retorno)
//   state ptr atualizado atomicamente via str + dmb ish
```

---

## BUG-03-A: Load-use hazard em phase transition path

### Descrição

O lookup do atrator usa `ldr` seguido imediatamente de `csel`.
Em microarquiteturas ARM Cortex-A (especialmente A55/A76),
o `ldr` para registrador que alimenta um `csel` na instrução
seguinte cria **stall de 4 ciclos** ou, pior, resultado
especulativo incorreto se o endereço de `ldr` depende de
cálculo anterior com latência pendente.

### Código problemático (inferido)

```asm
// BUGGY:
vectra_pulse_transition:
    ldr     x6, [x0, #8]          // carrega coherence de AttractorState
    csel    x1, x6, x1, ne        // HAZARD: x6 pode não estar pronto
    // continua com x1 "atualizado"
```

### Fix

```asm
// CORRETO: inserir instrução independente entre ldr e csel
vectra_pulse_transition:
    ldr     x6, [x0, #8]          // carrega coherence
    and     x7, x3, #0x3F         // instrução independente (phase & 63)
    // x6 pronto após 4 ciclos — ok agora
    csel    x1, x6, x1, ne        // seguro
```

### Alternativa: `ldar` (load-acquire)

```asm
    ldar    x6, [x0]              // load com acquire barrier — garante
                                   // visibilidade antes de qualquer
                                   // instrução subsequente dependente
```

**Nota:** `ldar` tem overhead de ~2-4 ciclos extra mas elimina o
hazard em qualquer implementação ARM64.

---

## BUG-03-B: Overflow silencioso no índice do atrator

### Descrição

O índice do atrator (`x4`) é um `uint8` (0..41). O load da tabela
calcula o offset como:

```asm
// BUGGY:
    lsl     x4, x4, #4            // × 16 bytes (assume sizeof=16)
    ldr     x6, [x9, x4]          // x9 = &attractor_table
```

Problema 1: `sizeof(AttractorState)` = **20 bytes** (não 16).
`lsl #4` = ×16 → offset errado para qualquer estado além do [0].

Problema 2: Sem verificação de bounds. Se `x4 > 41` por corrupção
ou state nulo do BUG-01, o load acessa memória fora da tabela.

### Fix

```asm
// CORRETO: usar multiplicação explícita com sizeof=20
// e bound check via csel
vectra_pulse_attractor_load:
    // bound check
    cmp     x4, #42
    csel    x4, xzr, x4, hs       // se x4 >= 42: usa state[0] como fallback

    // offset = id × 20
    mov     x7, #20
    mul     x8, x4, x7            // x8 = x4 × 20
    ldr     x9, =attractor_table  // endereço da tabela (via literal pool)
    add     x9, x9, x8            // x9 → AttractorState[x4]
    ldr     x6, [x9, #4]          // carrega delta_r (offset +4 na struct)
```

**Alternativa preferred:** usar `ubfx` para isolar 6 bits e garantir
que `x4 ∈ [0..41]` structuralmente:

```asm
    ubfx    x4, x4, #0, #6        // extrai bits [5:0] → máximo valor = 63
    cmp     x4, #42               // ainda precisa verificar 42..63
    csel    x4, xzr, x4, hs
```

---

## BUG-03-C: Ausência de `dmb ish` após write de estado

### Descrição

Em sistemas multi-core (Android em SoCs com múltiplos clusters),
a escrita do estado atualizado em `[x0]` não tem barreira de memória.
O estado pode ficar no store buffer do core atual e ser invisível
para outros cores que leem o mesmo `state ptr`.

```asm
// BUGGY:
    str     x1, [x0, #8]          // escreve coherence atualizado
    // nenhuma barreira — outro core pode ver estado inconsistente
    ret
```

### Fix

```asm
// CORRETO: dmb ish garante ordering para todos os cores no Inner
// Shareable domain (padrão em Android/ARM big.LITTLE)
vectra_pulse_commit:
    str     x1, [x0, #8]          // escreve coherence
    str     x2, [x0, #12]         // escreve entropy
    str     x5, [x0, #16]         // escreve lyapunov φ
    dmb     ish                   // barrier: tudo anterior visível antes de ret
    ret
```

**Alternativa:** usar `stlr` (store-release) que inclui barrier implícito:

```asm
    stlr    x5, [x0]              // store-release: barrier + write atômico
```

**Nota importante:** O `stlr` funciona apenas para o registrador de
**base** do ponteiro (x0 + offset 0). Para múltiplos campos, `dmb ish`
após os `str`s é a abordagem correta.

---

## BUG-03-D: Phase wrapping com divisão lenta (udiv/msub)

### Descrição

O wrapping de phase no range [0..41] usa divisão inteira AArch64:

```asm
// BUGGY (lento + desnecessário):
    add     x3, x3, x6            // x3 = phase + delta_r
    mov     x7, #42
    udiv    x8, x3, x7            // x8 = (phase + delta_r) / 42
    msub    x3, x8, x7, x3        // x3 = phase - 42*(phase/42) = phase mod 42
```

`udiv` tem latência de **13-39 ciclos** em Cortex-A. Em hot path
de pulso, isso é inaceitável.

A condição `gcd(Δr, 42) = 1` **garante** que o wrapping é simples:
`phase + Δr` nunca excede `2×42 - 1 = 83` quando `phase ∈ [0..41]`
e `Δr ∈ [1..41]`. Logo, uma única subtração condicional é suficiente.

### Fix (branchless, 2 ciclos)

```asm
// CORRETO: subtração condicional (sem divisão)
vectra_pulse_phase_wrap:
    add     x3, x3, x6            // x3 = phase + delta_r (máx = 41+41 = 82)
    mov     x7, #42
    subs    x8, x3, x7            // tenta x3 - 42
    csel    x3, x8, x3, hs        // se x3 >= 42: usa x8, senão mantém x3
    // phase agora garantidamente ∈ [0..41]
```

**Nota:** A condição `hs` (higher or same, unsigned ≥) é correta aqui
porque `x3` e `x7` são unsigned. `cs` também funciona (mesmo encoding).

**Prova de corretude:**
```
phase ∈ [0..41], Δr ∈ [1..41] (coprime com 42)
→ phase + Δr ∈ [1..82]
→ se phase + Δr ≥ 42: result = (phase + Δr) - 42 ∈ [0..40]  ✓
→ se phase + Δr < 42: result = phase + Δr ∈ [1..41]          ✓
→ Uma subtração condicional é suficiente (não é possível exceder 83)
```

---

## 5. Assembly completo corrigido (vectra_pulse_step)

```asm
// vectra_pulse.S — vectra_pulse_step (VERSÃO CORRIGIDA)
// AArch64, sem libc, sem malloc, macros only
// Entrada: x0=state_ptr, x1=C, x2=H, x3=phase, x4=attractor_idx

    .section    .text.vectra_pulse, "ax"
    .align      4
    .global     vectra_pulse_step
    .type       vectra_pulse_step, %function

vectra_pulse_step:
    // === BOUND CHECK attractor_idx ===
    cmp     x4, #42
    csel    x4, xzr, x4, hs          // fallback state[0] se out of bounds

    // === LOAD AttractorState[x4] ===
    mov     x7, #20                   // sizeof(AttractorState) = 20
    mul     x8, x4, x7               // offset
    adrp    x9, attractor_table
    add     x9, x9, :lo12:attractor_table
    add     x9, x9, x8               // x9 → state[attractor_idx]
    and     x10, x3, #0x3F           // instrução independente (anti-hazard)
    ldr     x6, [x9, #2]             // load delta_r (offset +2, uint16)
    ubfx    x6, x6, #0, #8           // isola byte baixo (delta_r é uint8)

    // === PHASE WRAP (branchless) ===
    add     x3, x3, x6               // phase + delta_r
    mov     x7, #42
    subs    x10, x3, x7
    csel    x3, x10, x3, hs          // wrap [0..41]

    // === LYAPUNOV φ = (1-H)·C em Q16.16 ===
    mov     x11, #0x10000            // 1.0 em Q16.16
    sub     x12, x11, x2             // (1 - H)
    mul     x13, x12, x1             // (1-H) × C (produzirá Q32.32)
    lsr     x5, x13, #16             // shift para Q16.16 → φ em x5

    // === COMMIT com barrier ===
    str     w1, [x0, #8]             // coherence
    str     w2, [x0, #12]            // entropy
    str     w5, [x0, #16]            // lyapunov
    str     w3, [x0, #20]            // phase atualizado
    dmb     ish                      // visibility barrier (multi-core)

    // retorna: x5 = φ, x3 = phase_new
    ret

    .size vectra_pulse_step, . - vectra_pulse_step
```

---

## 6. Critério de fechamento do BUG-03

- [ ] BUG-03-A resolvido: instrução independente ou `ldar` inserida
- [ ] BUG-03-B resolvido: sizeof=20 correto, bound check ativo
- [ ] BUG-03-C resolvido: `dmb ish` ou `stlr` após writes de estado
- [ ] BUG-03-D resolvido: `udiv/msub` substituído por `subs/csel`
- [ ] Assembly testado em QEMU AArch64 com `qemu_rafaelia` integration test
- [ ] `verify_attractor_table()` retorna 0 (requer BUG-01 fechado)
- [ ] Cycle count do hot path ≤ 30 ciclos (verificar com `perf` ou PMU)
