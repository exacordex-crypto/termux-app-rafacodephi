# AGENTS.md — VECTRA / RafaelOS

## Build
run: ./build.sh
target: aarch64-linux-android28
toolchain: Android NDK r26+ / Termux clang

## Tests
run: ./run_tests.sh
validate: bitomega.log → period-42 confirmed
validate: BLAKE3/RMR → N=200 runs vs upstream

## Architecture
- AArch64 primary, ARM32 fallback
- Android API 28+ (Termux, no root)
- Fixed-point Q16.16 throughout (NO float)
- NEON intrinsics where available
- Single-file kernels preferred

## Register Contract
DO NOT touch x0..x4 outside designated modules.
x0 = state ptr  x1 = C  x2 = H  x3 = phase  x4 = attractor

## Assembly Rules
- Macros only — no BL to unknown symbols
- csel/csinc — no unpredictable branches
- Every loop must have gcd-proven termination
- COLLAPSE_STEP = macro, never function call
- attractor_table: 42 entries required, none missing

## Known Bugs (do not close without fix)
1. attractor_table incomplete (40 of 42 missing)
2. VOID paradox in attractor #22 (structural)
3. vectra_pulse.S: 4 open AArch64 bugs
4. termux fork: bootstrap paths hardcoded to com.termux

## Constraints
- Low memory (Cortex-A53, Moto E7)
- No heap allocation in hot path
- No libc in assembly modules
- Page size: -Wl,-z,max-page-size=16384 (Android 15)

## Invariants (never violate)
gcd(Δr, R) = 1       — toroidal traversal
|A| = 42             — attractor count
period(BitOmega) = 42 — confirmed, do not break
φ = (1-H)·C          — Lyapunov function

## Agent Behavior
- Read VECTRA_OS.md before touching any .S file
- Never add abstraction layers to assembly
- Falsification condition required for every new theorem
- Attractor #22: flag VOID paradox, do not silently patch
