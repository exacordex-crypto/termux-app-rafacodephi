# RAFAELIA Generated Artifacts Checklist

This folder now contains all requested Objective-5 artifacts:

1. `rafaelia_qpu_simulator.py` — classical simulator (T^7, Q16.16, 42 cycles, 7 directions, commit/rollback, CRC/XOR, Mandelbrot).
2. `rafaelia_isa_spec.md` — ISA symbolic definitions + BitRAF 42-bit binary encoding.
3. `rafaelia_whitepaper.tex` — architecture whitepaper with requested title/sections/figures.
4. `rafaelia_ctrl_fpga.sv` — FPGA control firmware sketch for 1008x42 BRAM + stride routing + rollback signal.
5. `rafaelia_auto_calibration.py` — automatic calibration over 7 pumping frequencies for 42-cycle coherence objective.
6. `RAFAELIA_DELIVERY.md` — mapping of mathematical principles to delivered implementation.

## Expected outputs after running tooling
- `rafaelia_metrics.png`
- `calibration_result.txt`
- `rafaelia_whitepaper.pdf` (if TeX toolchain is available)
- `rafaelia_generated_artifacts.tar.gz` (via GitHub Actions workflow)


## Conceptual completion
- `KNOWLEDGE_FLOW_MATRIX.md` — bridge from toroidal equations to multilingual/signal interpretation and falsification conditions.

## Source-of-truth policy
- This directory stores **source artifacts only** (`.md`, `.py`, `.sv`, `.tex`).
- Build/runtime outputs (`.png`, `.txt`, `.pdf`, archives) are generated during local runs or GitHub Actions and must not be committed as source.
