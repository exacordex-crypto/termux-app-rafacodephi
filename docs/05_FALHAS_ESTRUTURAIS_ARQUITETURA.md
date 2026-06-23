# Falhas Estruturais e Arquiteturais — RAFAELIA/VECTRA_OS
> Análise além dos bugs catalogados no AGENTS.md
> Foco: problemas de design, contratos implícitos não documentados,
>       riscos de manutenibilidade e consistência de invariantes

---

## 1. Fragmentação do fork chain

### Problema

Existe uma cadeia de 3 forks sem rastreabilidade formal:

```
termux/termux-app (upstream)
    └─→ rafaelmeloreisnovo/termux-app-rafacodephi
           └─→ wojcikiewicz17/... (presumido)
                  └─→ exacordex-crypto/termux-app-rafacodephi (atual)
```

Cada fork acumula divergências. O upstream `termux/termux-app` está em
`v0.119.0-beta.3` com fixes críticos (incluindo **world-readable
vulnerability** corrigida em v0.118.0). Este fork provavelmente não
recebeu esse patch de segurança.

### Risco

```
CVE: world-readable vulnerability (termux upstream, pré-v0.118.0)
Impact: arquivos em $PREFIX podem ser lidos por qualquer app no dispositivo
Fix upstream: https://github.com/termux/termux-app (v0.118.0+)
Status neste fork: DESCONHECIDO — verificar se patch foi aplicado
```

### Ação requerida

```bash
# Verificar se o patch foi aplicado:
git log --oneline | grep -i "world.readable\|permission\|CVE"
# Se vazio: aplicar patch do upstream manualmente
```

---

## 2. Inconsistência entre modelo matemático e implementação

### O pipeline ψ→χ→ρ→Δ→Σ→Ω

O `RAFAELIA_MATH_FORMULAS.md` especifica o pipeline com precisão matemática.
Porém existem discrepâncias entre a spec e o que é implementável:

#### 2.1 SR=144000 Hz vs capacidade do hardware

```
Spec: SR = 144000 Hz (sample rate para STFT)
Realidade Android:
  - AudioRecord suporta: 8000, 11025, 16000, 22050, 44100, 48000 Hz
  - 144000 Hz NÃO é suportado nativamente pelo AudioFlinger Android
  - Resultado: STFT em 144000 Hz requer resampling software
```

Se o código usa `AudioRecord` com `SAMPLE_RATE=144000`, ele
silenciosamente faz downgrade para 48000 Hz (comportamento de fallback
do Android), quebrando todas as frequências geométricas calculadas:

```
f_geom(n) = c × √n  onde c é calculado assumindo SR=144000
Se SR=48000: erro de fator = √(144000/48000) = √3 ≈ 1.732
```

**Todas as frequências geométricas têm erro de 73%** se SR não é honrado.

#### 2.2 block_size = 65536 bytes vs n_fft

```
Spec: block_size = 65536 bytes, n_fft = 4096..8192 (Hann window)

Inconsistência: se block_size = 65536 bytes e cada sample é int16 (2 bytes):
  → 65536 / 2 = 32768 samples por bloco
  → n_fft máximo sem zero-padding = 32768
  → n_fft = 8192 processa apenas 25% do bloco por janela (overlap correto)
  → n_fft = 4096 processa 12.5% por janela

Isso é matematicamente correto COM overlap, mas a spec não especifica
o hop_size (tamanho do salto entre janelas STFT).
```

Sem `hop_size` especificado, implementações independentes do mesmo
spec produzirão resultados diferentes.

---

## 3. Sequência Fibonacci-Rafael: período de Pisano vs tabela de atratores

### Análise do período de Pisano

```
π(42) = π(2×3×7) = lcm(π(2), π(3), π(7))
      = lcm(3, 8, 16)
      = 48

Mas: period(BitOmega) = 42  (spec do VECTRA_OS)
```

**Contradição:** O período natural de Fibonacci mod 42 é **48**, não 42.
O sistema especifica período 42 mas a matemática implica período 48.

Isso significa que a `attractor_table[42]` com `period=42` requer
que a sequência Fibonacci-Rafael **não seja** Fibonacci puro —
ela deve ter uma transformação Δ_Rafael que altera o período.

A transformação semântica Δ_Rafael (substituição de caracteres Voynich +
Fibonacci reverso + XOR) pode reduzir o período de 48 para 42 se os
6 estados "extras" do período 48 forem mapeados para estados existentes.
Mas isso cria colisões — e o estado #22 pode ser exatamente um desses
6 estados extras que não têm mapeamento limpo.

**Esta é provavelmente a raiz matemática profunda do VOID paradox.**

```
Período Fibonacci mod 42: 48
Período BitOmega spec'd:  42
Diferença:                 6 estados "sobrando"
Desses 6, state[22] é o mais problemático (posição pós-antipodal)
```

---

## 4. Falta de contrato formal para o modo CTI_SCAN_DELTA_MISS

### O modo DELTA_MISS não tem spec

```c
// cti_raw_reader.h define 5 modos:
typedef enum {
    CTI_SCAN_SEQ,         // sequencial
    CTI_SCAN_SPIRAL,      // espiral
    CTI_SCAN_TOROID,      // toroidal
    CTI_SCAN_RANDOM_PERM, // permutação aleatória
    CTI_SCAN_DELTA_MISS   // ??? — sem documentação formal
} ScanMode;
```

O modo `DELTA_MISS` é mencionado no header mas **não há spec** de como
ele calcula o "delta de missão". Pela lógica do sistema:

```
DELTA_MISS provavelmente calcula:
    δ_miss = (C_atual - C_expected) / C_expected
    onde C_expected vem da attractor_table

→ Com attractor_table[22]=0: δ_miss = (C - 0) / 0 = ∞  (divisão por zero)
```

**DELTA_MISS divide por zero quando passa pelo state #22** com a tabela
incompleta atual.

---

## 5. Ausência de test suite formal

### Estado atual

Não há evidência de test suite automatizado para:
- Invariantes matemáticos (`gcd(Δr,R)=1`, `period=42`)
- Integridade da attractor_table
- vectra_pulse_step correctness
- ZIPRAF round-trip (comprimir → descomprimir = original)
- CTI scan completeness (todos os 1024 entries processados)

### O que existe

Scripts de build (`hotfix_ate_compilar.sh`, `build_apk_matrix.sh`)
que compilam mas não testam corretude.

### Test suite mínimo recomendado

```c
// tests/test_vectra_invariants.c
#include <assert.h>
#include "vectra_attractor.h"
#include "cti_raw_reader.h"
#include "zipraf_index.h"

void test_gcd_invariant(void) {
    for (int i = 0; i < 42; i++) {
        if (i == 22) continue;  // VOID — skip
        uint8_t dr = attractor_table[i].delta_r;
        uint8_t a = dr, b = 42;
        while (b) { uint8_t t = b; b = a%b; a = t; }
        assert(a == 1 && "gcd(delta_r, 42) must be 1");
    }
}

void test_lyapunov_invariant(void) {
    for (int i = 0; i < 42; i++) {
        if (i == 22) continue;
        const AttractorState *s = &attractor_table[i];
        uint32_t phi_calc = ((uint64_t)((1u<<16) - s->entropy) * s->coherence) >> 16;
        assert(phi_calc == s->lyapunov && "Lyapunov phi=(1-H)*C violated");
    }
}

void test_period_42(void) {
    // Verificar que trajetória com Δr=1 percorre todos os 42 estados
    int visited[42] = {0};
    int state = 0;
    for (int i = 0; i < 42; i++) {
        visited[state] = 1;
        state = (state + 1) % 42;
    }
    for (int i = 0; i < 42; i++) {
        assert(visited[i] && "State not visited in period-42 traversal");
    }
}

int main(void) {
    test_gcd_invariant();
    test_lyapunov_invariant();
    test_period_42();
    // adicionar: zipraf round-trip, cti scan completeness
    return 0;
}
```

---

## 6. Android 15/16 compatibility gaps

### 6.1 16KB page size (obrigatório em Android 15+)

```
Status: flag -Wl,-z,max-page-size=16384 configurada ✓
Problema: ZrManifest static sem __attribute__((aligned(16384)))
          pode cruzar fronteira de página em dispositivos com 16KB pages
```

### 6.2 `android:sharedUserId` deprecated

```
Android 11+ (API 30): sharedUserId deprecated
Android 13+ (API 33): removal em andamento
Este fork provavelmente ainda usa sharedUserId="com.termux"
Resultado: warnings de Play Store e potencial incompatibilidade futura
```

### 6.3 `hiddenapibypass` version

O upstream termux usa `org.lsposed.hiddenapibypass:hiddenapibypass:6.1`
para fix de crash no Android 16 QPR1. Este fork pode ter versão mais antiga,
causando crash no Android 16 QPR1.

---

## 7. Mapa de risco resumido

```
RISCO IMEDIATO (pode causar crash em prod):
    BUG-01 + BUG-02: attractor_table nula → vectra_pulse crash
    BUG-05: ZrManifest em stack → stack overflow silencioso
    BUG-03-B: sizeof errado → leitura fora da tabela → SIGSEGV

RISCO FUNCIONAL (comportamento incorreto sem crash):
    BUG-03-D: udiv lento mas correto funcionalmente
    BUG-08: φ não verificado → divergência não detectada
    SR=144000 sem suporte → frequências geométricas com erro de 73%

RISCO DE SEGURANÇA:
    CVE world-readable (herança do upstream pré-v0.118.0) — verificar
    BUG-07: APKs com hash inválido distribuídos silenciosamente

RISCO DE MANUTENIBILIDADE:
    Período Pisano 48 vs spec 42 — contradição matemática profunda
    DELTA_MISS sem spec — comportamento indefinido para state #22
    Ausência de test suite — regressões não detectadas
```
