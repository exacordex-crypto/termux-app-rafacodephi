# VECTRA_OS.md — Zero-Abstraction Compilation Contract

## Philosophy
No OS. No libc. No stack frames.
Every byte is a register decision.
Every branch is a geometric invariant test.

## Register Map (AArch64 / ARM32 fixed contract)
x0  = state_vector base ptr    (NEVER freed mid-cycle)
x1  = coherence C              (Q16.16 fixed-point)
x2  = entropy H                (Q16.16 fixed-point)
x3  = phase counter mod 42
x4  = attractor index          (0..41)
x5  = delta / scratch
x6  = hash accumulator         (FNV running)
x7  = flags register (inline)

## Flags (bit-packed, x7)
bit 0 = LOCK state active
bit 1 = FLOW state active  
bit 2 = VOID triggered
bit 3 = trickstopathcutter active
bit 4 = viscosity_negative
bit 5 = attractor_jump_pending
bit 6 = merkle_dirty
bit 7 = geometric_invariant_fail

## Invariant Rule (NO EXCEPTION)
gcd(Δr, R) == 1  →  continue
gcd(Δr, R) != 1  →  HALT + set bit 7, no recovery

## Compilation Contract
- No function calls. Use macros only.
- No named variables. Registers x0..x15 only.
- No loops unless gcd-proven terminating.
- Conditionals: csel / csinc only. No branch misprediction.
- Every COLLAPSE_STEP = 1 macro expansion, not a function.

## Error Protocol
Hardware fail    →  x7 bit pattern encodes exact component
No exceptions    →  csel-branch to VOID attractor (#22)
No logging       →  CRC/Merkle written to torus_cells offset only

## Cycle Budget (Cortex-A53)
COLLAPSE_STEP   ≤ 42 cycles
attractor_jump  ≤ 7 cycles
hash_update     ≤ 3 cycles
VOID_check      ≤ 1 cycle (cmp + csel, inline)

## What is NOT here
- No malloc
- No printf  
- No errno
- No branch to unknown address
- No loop that repeats a settled state ("petros" = already stone)
