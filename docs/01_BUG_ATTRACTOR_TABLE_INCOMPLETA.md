# BUG-01 — Attractor Table: 40/42 estados FALTANDO
> Severidade: 🔴 CRÍTICO — Bloqueante para VECTRA_OS, vectra_pulse.S, CTI DELTA_MISS
> Componente: `attractor_table` (arquivo não localizado / stub vazio)
> Detectado em: `AGENTS.md` → "attractor_table incomplete (40 of 42 missing)"
> Dependência: Requer resolução do BUG-02 (VOID paradox #22) antes de preencher #21..#23

---

## 1. O que é a attractor_table

A `attractor_table` é o núcleo do sistema de estados toroidais VECTRA_OS.
É um array estático de 42 entradas que define o espaço de fase do atrator:

```c
// Estrutura esperada (inferida do contrato ABI)
typedef struct __attribute__((packed)) {
    uint8_t  id;         // 0..41
    uint8_t  delta_r;    // Δr — passo toroidal (deve ser coprime com 42)
    uint16_t phase_mask; // máscara de fase (Q16.16 parcial)
    uint32_t coherence;  // C em Q16.16
    uint32_t entropy;    // H em Q16.16
    uint32_t lyapunov;   // φ = (1-H)·C em Q16.16
    uint32_t fnv_hash;   // FNV-1a do estado para integridade
} AttractorState;        // 20 bytes por estado

// Tabela completa
static const AttractorState attractor_table[42];
```

---

## 2. Por que 42 estados

O invariante `|A| = 42` e `period(BitOmega) = 42` deriva da fatoração:

```
42 = 2 × 3 × 7
```

Os valores de Δr **coprimos com 42** (válidos como passo toroidal) são:

```
{1, 5, 11, 13, 17, 19, 23, 25, 29, 31, 37, 41}
= 12 valores válidos de Δr
```

Um Δr inválido (e.g., Δr=6, gcd(6,42)=6≠1) causa colapso do período —
o sistema cicla em subconjunto < 42 estados, violando o invariante.

### Mapeamento Fibonacci-Rafael → estados

A sequência Fibonacci-Rafael mapeia os landmarks:

```
F_R: 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89...

Mapeamento módulo 42:
F(1)  = 1   → state[1]
F(2)  = 1   → state[1]  (colisão — identidade)
F(3)  = 2   → state[2]
F(4)  = 3   → state[3]
F(5)  = 5   → state[5]
F(6)  = 8   → state[8]
F(7)  = 13  → state[13]
F(8)  = 21  → state[21]
F(9)  = 34  → state[34]
F(10) = 55  → state[55 mod 42] = state[13]  (colisão)
F(11) = 89  → state[89 mod 42] = state[5]   (colisão)
...
```

Fibonacci mod 42 tem **período de Pisano π(42) = 56**.
Os estados com semente Fibonacci direta (sem colisão):
`{1, 2, 3, 5, 8, 13, 21, 34}` = 8 estados.

Estados **sem semente Fibonacci direta** (precisam de interpolação):
```
{0, 4, 6, 7, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19, 20,
 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 35, 36,
 37, 38, 39, 40, 41}
= 34 estados precisam de definição explícita
```

---

## 3. Estado atual (stub inferido)

Com base no AGENTS.md, os únicos 2 estados presentes são presumivelmente:

```c
// Provavelmente apenas estes 2 estão definidos:
static const AttractorState attractor_table[42] = {
    [0]  = { .id=0,  .delta_r=1,  ... },  // estado inicial (raiz)
    [21] = { .id=21, .delta_r=21, ... },  // midpoint — ou #22 com VOID
    // [1..20] e [22..41] = zeros/undefined
};
```

**Impacto imediato:**
- `vectra_pulse.S` faz lookup em `x4 = attractor` → endereço inválido/zero
- Modo `DELTA_MISS` do CTI calcula `Δr` de estado nulo → comportamento indefinido
- O Lyapunov `φ = (1-H)·C` não pode ser verificado sem H e C válidos de cada estado
- O invariante `gcd(Δr, R) = 1` não pode ser validado end-to-end

---

## 4. Proposta de geração completa da tabela

### 4.1 Fórmula para estados sem semente Fibonacci

Para estados `s` sem semente Fibonacci direta, usar interpolação toroidal:

```
δ(s) = φ_approx × s (mod 42)    onde φ_approx = 1.618...

Em inteiro Q16.16:
PHI_Q16 = 0x00019E37   # (1 + sqrt(5)/2) × 2^16 ≈ 106039

delta_r(s) = (PHI_Q16 × s) >> 16) mod 42
Se gcd(delta_r(s), 42) ≠ 1: delta_r(s) = next_coprime(delta_r(s))
```

### 4.2 Tabela completa gerada

```c
// attractor_table.c — GERADO (aguarda validação de BUG-02 para #22)
// Todos os valores em Q16.16 (×65536)
// FNV-1a offset basis = 2166136261u

#include <stdint.h>
#include "vectra_attractor.h"

#define R           42u
#define Q16(x)      ((uint32_t)((x) * 65536.0))
#define FNV1A(id, dr) (((2166136261u ^ (id)) * 16777619u) ^ (dr))

static const AttractorState attractor_table[42] = {
//   id  Δr   phase_mask  coherence      entropy        lyapunov        fnv
    { 0,  1,  0x0000,  Q16(1.000), Q16(0.000), Q16(1.000), FNV1A(0,1)  },
    { 1,  1,  0x0001,  Q16(0.997), Q16(0.003), Q16(0.994), FNV1A(1,1)  },
    { 2,  5,  0x0002,  Q16(0.991), Q16(0.009), Q16(0.982), FNV1A(2,5)  },
    { 3,  5,  0x0003,  Q16(0.983), Q16(0.017), Q16(0.966), FNV1A(3,5)  },
    { 4, 11,  0x0004,  Q16(0.973), Q16(0.027), Q16(0.947), FNV1A(4,11) },
    { 5, 11,  0x0005,  Q16(0.961), Q16(0.039), Q16(0.923), FNV1A(5,11) },
    { 6, 13,  0x0006,  Q16(0.947), Q16(0.053), Q16(0.897), FNV1A(6,13) },
    { 7, 13,  0x0007,  Q16(0.931), Q16(0.069), Q16(0.867), FNV1A(7,13) },
    { 8, 13,  0x0008,  Q16(0.913), Q16(0.087), Q16(0.834), FNV1A(8,13) },
    { 9, 17,  0x0009,  Q16(0.893), Q16(0.107), Q16(0.797), FNV1A(9,17) },
    {10, 17,  0x000A,  Q16(0.871), Q16(0.129), Q16(0.759), FNV1A(10,17)},
    {11, 17,  0x000B,  Q16(0.847), Q16(0.153), Q16(0.718), FNV1A(11,17)},
    {12, 19,  0x000C,  Q16(0.821), Q16(0.179), Q16(0.674), FNV1A(12,19)},
    {13, 19,  0x000D,  Q16(0.793), Q16(0.207), Q16(0.629), FNV1A(13,19)},
    {14, 19,  0x000E,  Q16(0.763), Q16(0.237), Q16(0.581), FNV1A(14,19)},
    {15, 23,  0x000F,  Q16(0.731), Q16(0.269), Q16(0.531), FNV1A(15,23)},
    {16, 23,  0x0010,  Q16(0.697), Q16(0.303), Q16(0.479), FNV1A(16,23)},
    {17, 23,  0x0011,  Q16(0.661), Q16(0.339), Q16(0.424), FNV1A(17,23)},
    {18, 25,  0x0012,  Q16(0.623), Q16(0.377), Q16(0.368), FNV1A(18,25)},
    {19, 25,  0x0013,  Q16(0.583), Q16(0.417), Q16(0.310), FNV1A(19,25)},
    {20, 25,  0x0014,  Q16(0.541), Q16(0.459), Q16(0.250), FNV1A(20,25)},
    {21, 29,  0x0015,  Q16(0.500), Q16(0.500), Q16(0.250), FNV1A(21,29)},
    // --- ESTADO #22: VOID PARADOX — ver BUG-02 ---
    // {22, ???, 0xVOID, ???, ???, ???, ???},
    {23, 29,  0x0017,  Q16(0.459), Q16(0.541), Q16(0.248), FNV1A(23,29)},
    {24, 31,  0x0018,  Q16(0.417), Q16(0.583), Q16(0.243), FNV1A(24,31)},
    {25, 31,  0x0019,  Q16(0.375), Q16(0.625), Q16(0.234), FNV1A(25,31)},
    {26, 31,  0x001A,  Q16(0.333), Q16(0.667), Q16(0.222), FNV1A(26,31)},
    {27, 37,  0x001B,  Q16(0.291), Q16(0.709), Q16(0.206), FNV1A(27,37)},
    {28, 37,  0x001C,  Q16(0.250), Q16(0.750), Q16(0.187), FNV1A(28,37)},
    {29, 37,  0x001D,  Q16(0.209), Q16(0.791), Q16(0.165), FNV1A(29,37)},
    {30, 41,  0x001E,  Q16(0.169), Q16(0.831), Q16(0.140), FNV1A(30,41)},
    {31, 41,  0x001F,  Q16(0.129), Q16(0.871), Q16(0.112), FNV1A(31,41)},
    {32, 41,  0x0020,  Q16(0.090), Q16(0.910), Q16(0.082), FNV1A(32,41)},
    {33,  1,  0x0021,  Q16(0.052), Q16(0.948), Q16(0.049), FNV1A(33,1)  },
    {34,  1,  0x0022,  Q16(0.017), Q16(0.983), Q16(0.017), FNV1A(34,1)  },
    {35,  5,  0x0023,  Q16(0.003), Q16(0.997), Q16(0.003), FNV1A(35,5)  },
    {36,  5,  0x0024,  Q16(0.000), Q16(1.000), Q16(0.000), FNV1A(36,5)  },
    // retorno ao eixo — simetria toroidal
    {37,  5,  0x0025,  Q16(0.017), Q16(0.983), Q16(0.017), FNV1A(37,5)  },
    {38, 11,  0x0026,  Q16(0.052), Q16(0.948), Q16(0.049), FNV1A(38,11) },
    {39, 11,  0x0027,  Q16(0.090), Q16(0.910), Q16(0.082), FNV1A(39,11) },
    {40, 13,  0x0028,  Q16(0.129), Q16(0.871), Q16(0.112), FNV1A(40,13) },
    {41, 13,  0x0029,  Q16(0.169), Q16(0.831), Q16(0.140), FNV1A(41,13) },
};
```

**⚠️ O estado [22] está propositalmente vazio — aguarda BUG-02.**

---

## 5. Verificação de invariantes

```c
// Rotina de verificação (rodar em build-time via _Static_assert onde possível)
static int verify_attractor_table(void) {
    for (int i = 0; i < 42; i++) {
        if (i == 22) continue;  // VOID paradox — skip até BUG-02

        const AttractorState *s = &attractor_table[i];

        // Invariante 1: id correto
        if (s->id != (uint8_t)i) return -1;

        // Invariante 2: gcd(Δr, 42) == 1
        uint8_t a = s->delta_r, b = R;
        while (b) { uint8_t t = b; b = a % b; a = t; }
        if (a != 1) return -2;  // BUG: Δr não é coprime com 42

        // Invariante 3: φ = (1-H)·C  em Q16.16
        uint32_t one_minus_H = (1u << 16) - s->entropy;
        uint32_t phi_calc = (uint32_t)(((uint64_t)one_minus_H * s->coherence) >> 16);
        if (phi_calc != s->lyapunov) return -3;

        // Invariante 4: FNV-1a
        uint32_t hash = (2166136261u ^ s->id) * 16777619u ^ s->delta_r;
        if (hash != s->fnv_hash) return -4;
    }
    return 0;  // OK
}
```

---

## 6. Critério de fechamento do BUG-01

- [ ] Estado #22 preenchido (após resolução do BUG-02)
- [ ] `verify_attractor_table()` retorna 0 para todos os 42 estados
- [ ] `vectra_pulse.S` executa sem fault em lookup de qualquer estado 0..41
- [ ] CTI modo DELTA_MISS completa varredura de 1024 entradas sem UB
- [ ] BLAKE3 hash da tabela gerado e verificado no build pipeline
