# RAFAELIA Two-Cycle Enterprise Protocol

## Status and intent

This document turns the latest multidomain prompt into an operational protocol for RAFCODEΦ documentation and implementation work. It is **methodical, coherent, systematic, and conservative**: symbolic language may guide naming and review, but only measured behavior may be promoted to runtime claims.

The protocol is not a replacement for the low-level contracts in `AGENTS.md`. Native or hot-path work must remain branch-aware without heap allocation, garbage collection, hidden libc dependency, or unbounded loops. Documentation may describe metaphors, but implementation gates must stay falsifiable.

## Non-negotiable constraints

| Constraint | Required handling | Evidence gate |
|---|---|---|
| No heap in hot path | Prefer static buffers, caller-owned memory, fixed windows, and compile-time sizes. | Review for `malloc`, `new`, allocator wrappers, and GC-managed hot loops. |
| Branchless where useful | Use masks, flags, table selection, `csel/csinc`, or deterministic dispatch only when it reduces risk. | C/ASM parity test and benchmark, not aesthetic preference. |
| Freestanding native core | Keep assembly and low-level C independent from libc assumptions. | Link/build check for target ABI. |
| AArch64 primary, ARM32 fallback | Promote optimized path only after generic and ARM32-safe behavior is preserved. | Same vectors pass on generic, ARM32 fallback, and AArch64/NEON path. |
| Q16.16 fixed point | Do not introduce float into hot-path math. | Static scan and vector tests. |
| Attractor count 42 | Preserve `|A| = 42` and keep attractor #22 VOID paradox explicit. | Count check plus risk note. |
| Toroidal traversal | Require `gcd(Δr,R)=1` and `gcd(Δc,C)=1` for full traversal claims. | GCD proof or test vector. |
| Truth over usefulness | Empty/VOID tokens are valid when evidence is absent. | Claims matrix labels unknown as `VOID`, `DOC_ONLY`, or `NEEDS_BENCHMARK`. |

## Two-cycle operating model

### Cycle 1 — Omega intake and coherence

Cycle 1 receives formulas, metaphors, domains, and architecture requests without pretending they are already executable. It classifies each item into a deterministic state:

1. **FACT** — already represented by code, configuration, tests, or documented repository policy.
2. **HYPOTHESIS** — plausible design direction that needs a test or benchmark.
3. **METAPHOR** — useful conceptual language, not a physical or performance claim.
4. **VOID** — insufficient evidence; must remain explicit instead of being filled with a false claim.
5. **RISK_OPEN** — known bug, failing gate, incomplete proof, or missing rollback.

Cycle 1 output is a compact ledger: concept, layer, evidence, falsification condition, and next gate.

### Cycle 2 — Feedback, promotion, and rollback

Cycle 2 promotes only the subset that survives verification. Every promoted item must have:

1. a runnable or reviewable artifact;
2. a fail-safe behavior;
3. a failover route;
4. a rollback command or feature flag;
5. an explicit mitigation for known failure modes;
6. a status label: `PASS`, `FAIL`, `SKIPPED`, `DOC_ONLY`, or `NEEDS_BENCHMARK`.

If Cycle 2 fails, the system must return to the generic safe path and keep the richer concept as documentation only.

## Multidomain mapping

| Domain | Accepted as | Runtime promotion condition | Rollback/failover |
|---|---|---|---|
| Quantum, torus, hidden vectors | Mathematical metaphor or model scaffold. | Deterministic fixed-point model with reproducible vectors. | Revert to scalar toroidal map. |
| Alphabets, dictionaries, sound symbols | Unicode and phonetic metadata. | Corpus fixture proves normalization preserves intended fields. | Store raw token plus normalized token. |
| Sacred multilingual reading and cadence | Semantic layer requiring humility and provenance. | No promotion to truth claim without source, language, and translation policy. | Mark `DOC_ONLY` or `VOID`. |
| Hz, waveform, 1 kHz, timbre, reverberation | Signal-processing candidate. | Captured signal, sampling rate, FFT/window definition, and metrics. | Disable audio-derived feature. |
| Cognitive/neuroscience claims | External scientific claim. | Cite source and define measurable proxy. | Remove from implementation claim. |
| Market variables and ratios | Data schema / analytics vocabulary. | Time-split backtest, leakage check, fee/tax/slippage model. | Disable strategy output; keep audit-only report. |
| Supply chain and molecular/DNA fields | Schema extension only. | Valid dataset, units, bounds, and privacy review. | Isolate module and mark `DOC_ONLY`. |
| RAFAELIA tag14/rafbit10/omega state | Internal symbolic telemetry. | Serialization, deterministic update rule, and invariant tests. | Drop to previous schema version. |

## Formula handling policy

The 50 formulas are retained as a taxonomy, not automatically as executable physics. They map to four implementation lanes:

1. **Core invariants:** toroidal state, EMA for `C/H`, `φ=(1-H)·C`, attractor count 42, period 42, entropy milli, coprimality, XOR/FNV-like hash, CRC/Merkle integrity.
2. **Candidate metrics:** correlations, Hurst, fractal entropy, PCA/ICA, autoencoders, genetic algorithms, market ratios, nonlinear causality, anomaly tests.
3. **Schema vocabularies:** matrix, pair, geometry, temporal, market, event, supply-chain, molecular, and RAFAELIA fields.
4. **Metaphor-only until evidenced:** quantum virtual mechanics, sacred multilingual resonance, hidden fractal vectors, syntropy, and parabolic teaching models.

A formula moves lanes only when its falsification condition is written first.

## FAILSAFE, FAILOVER, ROLLBACK, and mitigation checklist

| Gate | Required question | PASS condition | Failure behavior |
|---|---|---|---|
| FAILSAFE | What is the safest state if the module lies, crashes, or lacks data? | State becomes `VOID`, `DOC_ONLY`, or generic fallback. | Block promotion. |
| FAILOVER | What alternate route preserves service? | Generic C/scalar path remains available. | Disable optimized path. |
| ROLLBACK | How is the change undone? | Git revert, feature flag, or config switch is documented. | Mark release blocked. |
| MITIGATION | What risk is reduced now? | Known risk is named with test or manual check. | Keep `RISK_OPEN`. |
| EVIDENCE | What proves it? | Exact command, fixture, benchmark, or citation exists. | Downgrade claim. |

## Branchless low-friction implementation guidance

Branchless work is a means, not a slogan. Use it when the branch predictor, side-channel profile, or loop structure benefits from it. Do not obscure correctness.

Preferred order:

1. generic safe implementation;
2. fixed-size tables and caller-owned buffers;
3. static dispatch by architecture capability;
4. ARM32 NEON/SIMD fallback when available;
5. AArch64 NEON specialization;
6. benchmark-backed promotion;
7. rollback flag or compile-time switch.

Never add an abstraction layer merely to rename a condition. If one conditional is enough, keep one conditional outside the hot loop and make the hot loop deterministic.

## Enterprise delivery expectations

A fullstack enterprise-ready delivery must include:

1. documentation route and ownership;
2. schema and variables with units or encoding;
3. build path and target ABI assumptions;
4. tests for success and induced failure;
5. low-memory behavior;
6. security boundary and integrity check;
7. rollback plan;
8. open risks;
9. next action list.

## Falsification condition

This protocol fails if any future document or module presents metaphor, intuition, or multidisciplinary vocabulary as delivered production capability without an artifact, exact test, risk statement, and rollback route.
