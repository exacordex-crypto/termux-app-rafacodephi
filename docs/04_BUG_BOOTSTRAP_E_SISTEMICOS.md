# BUG-04..08 — Bootstrap Hardcode e Bugs Sistêmicos
> Repositório: `exacordex-crypto/termux-app-rafacodephi`
> Documento cobre: BUG-04 (bootstrap), BUG-05 (ZrManifest), 
>                  BUG-06 (CTI race), BUG-07 (BLAKE3), BUG-08 (Lyapunov)

---

## BUG-04 — Bootstrap paths hardcoded para `com.termux`

### Severidade: 🟡 MÉDIO — Funcional mas bloqueia release oficial

### Descrição

O fork `termux-app-rafacodephi` usa o package name `com.termux.rafacodephi`
(ou similar), mas os paths de bootstrap estão codificados para o package
name do Termux original:

```java
// Localizações prováveis (inferidas):
// termux-shared/src/main/java/.../TermuxConstants.java
public static final String TERMUX_PACKAGE_NAME = "com.termux";
public static final String TERMUX_FILES_DIR_PATH =
    "/data/data/com.termux/files";   // ← HARDCODED
public static final String TERMUX_PREFIX_DIR_PATH =
    "/data/data/com.termux/files/usr"; // ← HARDCODED
```

### Impacto

1. **Side-by-side install impossível:** instalar `com.termux.rafacodephi`
   ao lado do `com.termux` original causa conflito de paths — ambos
   tentam usar `/data/data/com.termux/files/`.

2. **SharedUserId conflict:** Ambos usam `android:sharedUserId="com.termux"`,
   mas APKs assinados com chaves diferentes não podem compartilhar o mesmo
   `sharedUserId`. Resultado: crash no install.

3. **Bootstrap não extrai para path correto:** O bootstrap zip é extraído
   para `/data/data/com.termux/files/` independente do package instalado.

### Fix

```java
// TermuxConstants.java — patch mínimo
public static final String TERMUX_PACKAGE_NAME = BuildConfig.APPLICATION_ID;

// No Gradle (app/build.gradle):
android {
    defaultConfig {
        applicationId "com.termux.rafacodephi"
    }
}

// Nos paths: usar BuildConfig ao invés de hardcode
public static final String TERMUX_FILES_DIR_PATH =
    "/data/data/" + BuildConfig.APPLICATION_ID + "/files";
```

### Scripts de bootstrap afetados

```bash
# prepare_bootstrap_env.sh — linha problemática (inferida)
TERMUX_PREFIX=/data/data/com.termux/files/usr  # ← mudar para variável

# Fix:
TERMUX_PKG="${TERMUX_PACKAGE_NAME:-com.termux.rafacodephi}"
TERMUX_PREFIX="/data/data/${TERMUX_PKG}/files/usr"
```

### Critério de fechamento

- [ ] `TermuxConstants.java` usa `BuildConfig.APPLICATION_ID`
- [ ] `prepare_bootstrap_env.sh` usa variável configurável
- [ ] Build matrix (`build_apk_matrix.sh`) gera APKs com package name correto
- [ ] Install testado side-by-side com Termux original no mesmo dispositivo
- [ ] `android:sharedUserId` removido ou atualizado (Android 11+: deprecated)

---

## BUG-05 — `ZrManifest` em stack de thread (Stack Overflow)

### Severidade: 🔴 CRÍTICO (silencioso)

### Descrição

O `ZrManifest` da ZIPRAF tem ~59KB de tamanho. Se instanciado como
variável local em qualquer função/thread, causa stack overflow silencioso.

```c
// BUGGY — comum em código não revisado:
void some_zipraf_operation(void) {
    ZrManifest manifest;  // 59KB na stack!!! OVERFLOW
    zr_manifest_init(&manifest, ...);
}
```

Stack default em threads Android NDK = 1MB.
Thread principal do app = 8MB (mas Termux pode ter menos).
59KB de stack frame único não parece muito, mas combinado com
frames de outras funções na call stack pode causar overflow.

### Fix

```c
// CORRETO 1: static (mais simples)
static ZrManifest g_manifest;  // BSS/data segment, não stack
void some_zipraf_operation(void) {
    zr_manifest_init(&g_manifest, ...);
}

// CORRETO 2: Se múltiplas instâncias forem necessárias:
// Alocar em heap via sistema de pool pré-alocado (sem malloc direto)
// O VECTRA_OS tem um pool system baseado em arena estática

// CORRETO 3: __attribute__((aligned(16384))) para 16KB page alignment
static ZrManifest __attribute__((aligned(16384))) g_manifest_aligned;
```

### Detecção

```bash
# Verificar no código se há ZrManifest como variável local:
grep -rn "ZrManifest " . | grep -v "static\|extern\|*\|&"
# Qualquer linha sem static/extern/pointer é um stack allocation
```

### Critério de fechamento

- [ ] `grep` acima retorna zero linhas
- [ ] Todas as instâncias de `ZrManifest` são `static` ou via arena
- [ ] `__attribute__((aligned(16384)))` aplicado para Android 15/16

---

## BUG-06 — `CtiScanner` modo TOROID: Race condition sem barrier

### Severidade: 🟠 ALTO

### Descrição

O modo de varredura `CTI_SCAN_TOROID` do `CtiScanner` usa uma
variável de índice compartilhada sem proteção de concorrência.
Em uso multi-thread (comum em Android via ART), dois threads
podem iniciar varreduras TOROID simultâneas e corromper o índice.

```c
// cti_raw_reader.c — trecho problemático (inferido)
typedef struct {
    CtiEntry entries[CTI_MAX_ENTRIES];  // 1024 entradas
    int      scan_idx;                  // ← SEM ATOMIC
    ScanMode mode;
    // ...
} CtiScanner;

// Thread A e Thread B chamam simultaneamente:
int cti_scan_next(CtiScanner *s) {
    // RACE: dois threads lêem s->scan_idx = 512,
    // ambos incrementam para 513, mas só um processa 512
    int idx = s->scan_idx++;  // ← não atômico
    return process_entry(&s->entries[idx % CTI_MAX_ENTRIES]);
}
```

### Fix

```c
// cti_raw_reader.h — fix com _Atomic (C11)
#include <stdatomic.h>

typedef struct {
    CtiEntry       entries[CTI_MAX_ENTRIES];
    _Atomic int    scan_idx;      // ← atômico
    ScanMode       mode;
    // ...
} CtiScanner;

// cti_raw_reader.c — uso correto
int cti_scan_next(CtiScanner *s) {
    // fetch_add atômico — thread-safe
    int idx = atomic_fetch_add_explicit(
        &s->scan_idx, 1, memory_order_acq_rel);
    return process_entry(&s->entries[idx % CTI_MAX_ENTRIES]);
}
```

**Alternativa para zero-overhead single-thread:** marcar explicitamente
que `CtiScanner` é single-threaded e adicionar `assert` no header:

```c
#define CTI_ASSERT_SINGLE_THREAD  // se definido, desabilita _Atomic
```

### Critério de fechamento

- [ ] `scan_idx` declarado `_Atomic int` ou proteção explícita documentada
- [ ] Modo TOROID testado com 2+ threads simultâneos sem race
- [ ] ASan + TSan habilitados em build de debug para confirmar

---

## BUG-07 — BLAKE3 hash skip silencioso no build pipeline

### Severidade: 🟡 MÉDIO

### Descrição

O script `hotfix_ate_compilar.sh` verifica hash BLAKE3 dos artefatos
mas em caso de falha faz **log silencioso sem abort**:

```bash
# hotfix_ate_compilar.sh — trecho problemático (inferido)
EXPECTED_HASH="abc123..."
ACTUAL_HASH=$(b3sum output.apk | cut -d' ' -f1)
if [ "$EXPECTED_HASH" != "$ACTUAL_HASH" ]; then
    echo "WARN: hash mismatch"  # ← deveria ser FATAL
    # continua sem abort!!!
fi
```

Resultado: APKs com hash incorreto são distribuídos silenciosamente.
Em um sistema com BLAKE3 como verificação de integridade, isso é
uma violação do contrato de segurança.

### Fix

```bash
# hotfix_ate_compilar.sh — versão corrigida
verify_blake3() {
    local file="$1"
    local expected="$2"
    local actual

    actual=$(b3sum "$file" 2>/dev/null | cut -d' ' -f1)
    if [ $? -ne 0 ]; then
        echo "FATAL: b3sum falhou para $file" >&2
        exit 1
    fi

    if [ "$actual" != "$expected" ]; then
        echo "FATAL: BLAKE3 mismatch em $file" >&2
        echo "  Esperado: $expected" >&2
        echo "  Obtido:   $actual" >&2
        exit 1  # ← ABORT, não warn
    fi
    echo "OK: $file hash verificado"
}
```

### Critério de fechamento

- [ ] Todo hash mismatch causa `exit 1` (não warn)
- [ ] `build_apk_matrix.sh` verifica hashes de todos os artefatos
- [ ] CI/CD bloqueia release se verificação BLAKE3 falhar

---

## BUG-08 — Invariante φ=(1-H)·C não verificado em runtime

### Severidade: 🟠 ALTO

### Descrição

O Lyapunov `φ = (1-H)·C` é um invariante fundamental do VECTRA_OS.
Ele garante convergência do sistema. Porém, em nenhum ponto do código
há verificação de runtime que φ permanece positivo e consistente com
C e H após cada transição de estado.

Se φ → 0 (H → 1 com C baixo), o sistema está no limite de convergência.
Se φ < 0 (matematicamente impossível em Q16.16 sem overflow), há
corrupção de estado.

```c
// Falta em toda a codebase (inferido):
// Nenhuma chamada para verificação pós-transição
```

### Fix

```c
// vectra_invariant.h — macro de verificação
#define VECTRA_ASSERT_LYAPUNOV(C, H, phi) do {                    \
    uint32_t _one_minus_H = (1u << 16) - (H);                    \
    uint32_t _expected_phi = (uint32_t)(                          \
        ((uint64_t)_one_minus_H * (C)) >> 16);                   \
    /* tolerância: ±1 ULP em Q16.16 */                            \
    uint32_t _diff = (_expected_phi > (phi)) ?                    \
        (_expected_phi - (phi)) : ((phi) - _expected_phi);       \
    if (_diff > 1u) {                                             \
        vectra_panic("LYAPUNOV VIOLATION: phi=%u expected=%u",    \
            (phi), _expected_phi);                                \
    }                                                             \
} while(0)

// Uso após cada transição:
vectra_pulse_step(...);
VECTRA_ASSERT_LYAPUNOV(x1_C, x2_H, x5_phi);
```

### Critério de fechamento

- [ ] `VECTRA_ASSERT_LYAPUNOV` definido em `vectra_invariant.h`
- [ ] Chamado após cada `vectra_pulse_step` em debug build
- [ ] Logs de violação incluem state id e phase para rastreamento
- [ ] Build de release usa versão no-op da macro (zero overhead)

---

## Resumo de priorização

```
IMEDIATO (bloqueia tudo):
    BUG-02 → BUG-01 → BUG-03

PARALELO (não bloqueante entre si):
    BUG-05 (Stack ZrManifest)    — fix em 1 hora
    BUG-07 (BLAKE3 exit 1)       — fix em 30 min
    BUG-04 (Bootstrap package)   — fix em 2-4 horas

APÓS BUG-03 ESTÁVEL:
    BUG-06 (CTI race condition)  — requer vectra_pulse.S estável
    BUG-08 (Lyapunov assert)     — requer tabela completa para testar
```
