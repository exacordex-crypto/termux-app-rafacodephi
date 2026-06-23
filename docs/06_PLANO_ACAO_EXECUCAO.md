# Plano de Ação — RAFAELIA/VECTRA_OS Bug Resolution
> Ordem de execução baseada no grafo de dependências
> Todos os itens abaixo devem ser executados na ordem indicada

---

## FASE 0 — Triagem imediata (hoje, sem código)

### 0.1 Verificar CVE world-readable
```bash
git log --oneline | grep -iE "world.readable|CVE|permission|security"
# Se nenhum resultado: APLICAR PATCH DO UPSTREAM IMEDIATAMENTE
```

### 0.2 Verificar ZrManifest em stack
```bash
grep -rn "ZrManifest " . --include="*.c" --include="*.h" \
  | grep -v "static\|extern\|\*\|&\|typedef\|struct"
# Cada linha é um stack allocation — BUG CRÍTICO
```

### 0.3 Verificar BLAKE3 exit code
```bash
grep -n "b3sum\|blake3\|BLAKE3" hotfix_ate_compilar.sh build_apk_matrix.sh
# Verificar se há exit 1 após mismatch
```

---

## FASE 1 — Resolução teórica BUG-02 (Semana 1)

**Nenhum código antes desta fase estar concluída.**

### 1.1 Decisão sobre Resolução do VOID paradox #22

Escolher UMA das resoluções documentadas em `02_BUG_VOID_PARADOX_ATRATOR_22.md`:

| Opção | Descrição | Complexidade | Recomendação |
|-------|-----------|--------------|--------------|
| A | Quantum bypass (pula #22) | Baixa | ❌ Viola period=42 |
| B | State merge com #13 | Média | ⚠️ Requer prova de hash |
| C | Dual-mode state | Alta | ✅ Preserva todos invariantes |
| D | Redefinir R=43 | Máxima | ❌ Quebra tudo |

**Recomendação: Resolução C (Dual-mode state)**

```c
// Especificação de state[22] com Resolução C:
attractor_table[22] = {
    .id         = 22,
    .delta_r    = 1,      // modo FIBONACCI (padrão)
    .phase_mask = 0x8000, // bit 15: indica dual-mode
    .coherence  = Q16(0.500),
    .entropy    = Q16(0.500),
    .lyapunov   = Q16(0.250),
    .fnv_hash   = FNV1A(22, 1) ^ FNV1A(22, 13),
};
// vectra_pulse.S: verificar phase_mask[15] para state[22] e usar Δr=13 em TOROID_MODE
```

### 1.2 Formalizar matematicamente
Escrever prova de que com Resolução C:
- `|A| = 42` ✓ (42 estados, nenhum fundido)
- `period(BitOmega) = 42` ✓ (state[22] participativo)
- `gcd(Δr, 42) = 1` ✓ (Δr=1 e Δr=13 ambos coprimos com 42)
- Hash chain Merkle sem colisões ✓

---

## FASE 2 — Implementação BUG-01 (Semana 1-2)

### 2.1 Gerar attractor_table.c completa

Usar como base o código em `01_BUG_ATTRACTOR_TABLE_INCOMPLETA.md`,
completando state[22] com a resolução escolhida na Fase 1.

### 2.2 Implementar verify_attractor_table()

```c
// vectra_attractor_verify.c
int verify_attractor_table(void);  // retorna 0 se OK, negativo se erro
```

### 2.3 Rodar verificação no build pipeline

```bash
# Adicionar em hotfix_ate_compilar.sh:
./tools/verify_attractor < /dev/null
if [ $? -ne 0 ]; then
    echo "FATAL: attractor_table inválida" >&2
    exit 1
fi
```

---

## FASE 3 — Fix BUG-03: vectra_pulse.S (Semana 2)

Aplicar os 4 fixes em ordem:

```
3-A: load-use hazard         → inserir instrução independente
3-B: sizeof=20 + bounds      → fix offset e bound check
3-C: dmb ish barrier         → adicionar após writes de estado
3-D: udiv → subs/csel        → eliminar divisão do hot path
```

### Assembly completo de referência

Ver seção 5 do arquivo `03_BUG_VECTRA_PULSE_AARCH64.md`.

### Teste em QEMU

```bash
# qemu_rafaelia integration test:
cd qemu_rafaelia/
./run_vectra_test.sh --state-count=42 --cycles=10000
# Deve completar sem SIGILL, SIGSEGV, ou assertion failure
```

---

## FASE 4 — Fixes independentes em paralelo (Semanas 1-2)

Podem ser feitos em paralelo com Fases 1-3:

### 4.1 BUG-05: ZrManifest static
```bash
# Encontrar e corrigir todos os stack allocations
grep -rn "ZrManifest " . --include="*.c" | grep -v static
# Para cada ocorrência: adicionar 'static' ou mover para arena
```

### 4.2 BUG-07: BLAKE3 exit 1
```bash
# hotfix_ate_compilar.sh e build_apk_matrix.sh
# Substituir "echo WARN" por "echo FATAL; exit 1"
```

### 4.3 BUG-04: Bootstrap package name
```bash
# TermuxConstants.java:
# Substituir "com.termux" hardcoded por BuildConfig.APPLICATION_ID
```

---

## FASE 5 — Fixes que requerem Fase 3 concluída (Semana 3)

### 5.1 BUG-06: CtiScanner race condition
```c
// Após vectra_pulse.S estável:
// Adicionar _Atomic int scan_idx em CtiScanner
// Testar com ThreadSanitizer
```

### 5.2 BUG-08: Lyapunov assert em runtime
```c
// Adicionar VECTRA_ASSERT_LYAPUNOV após cada vectra_pulse_step
// Testar com attractor_table completa (Fase 2)
```

---

## FASE 6 — Problemas estruturais (Semana 3-4)

### 6.1 SR=144000 Hz — resolução da discrepância

Duas opções:
- **A:** Aceitar SR=48000 e recalcular constantes `f_geom` para esse SR
- **B:** Implementar resampling software de 48000→144000 no pipeline STFT

```c
// Opção A (mais simples):
#define RAFAELIA_SR         48000u    // SR real do hardware Android
#define RAFAELIA_SR_SPEC    144000u   // SR da especificação matemática
#define SR_SCALE_FACTOR     ((float)RAFAELIA_SR_SPEC / RAFAELIA_SR)
// Multiplicar f_geom(n) por SR_SCALE_FACTOR ao calcular
```

### 6.2 Test suite mínimo

Implementar `tests/test_vectra_invariants.c` (ver seção 5 do arquivo 05).

### 6.3 Pisano period mismatch

Documentar formalmente em `RAFAELIA_MATH_FORMULAS.md`:
- Por que `period(BitOmega) = 42` apesar de `π(42) = 48`
- Quais 6 estados do período Pisano são fundidos/mapeados
- Confirmação de que Resolução C do BUG-02 cobre esses 6 estados

---

## Checklist final de release

```
□ verify_attractor_table() retorna 0 para todos os 42 estados
□ vectra_pulse_step: 4 bugs corrigidos, cycle count ≤ 30
□ ZrManifest: zero stack allocations (grep limpo)
□ Bootstrap: BuildConfig.APPLICATION_ID usado em todo lugar
□ BLAKE3: todo mismatch causa exit 1
□ CtiScanner: _Atomic int scan_idx
□ Lyapunov: VECTRA_ASSERT_LYAPUNOV em todo vectra_pulse_step
□ CVE world-readable: patch do upstream aplicado
□ SR discrepância: documentada e resolvida
□ Test suite: test_vectra_invariants passa sem falhas
□ QEMU integration test: 10000 ciclos sem crash
□ Page alignment: ZrManifest com aligned(16384)
□ Period Pisano mismatch: documentado em RAFAELIA_MATH_FORMULAS.md
□ hiddenapibypass: versão 6.1+ (crash Android 16 QPR1)
□ AGENTS.md atualizado: bugs 01-04 marcados como resolvidos
```

---

## Timeline estimado

```
Semana 1: FASE 0 (triagem) + FASE 1 (decisão teórica) + início FASE 4
Semana 2: FASE 2 (attractor_table) + FASE 3 (vectra_pulse.S)
Semana 3: FASE 5 (CTI race, Lyapunov) + FASE 6 início
Semana 4: FASE 6 completo + QA + release candidate
```
