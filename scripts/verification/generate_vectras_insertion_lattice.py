#!/usr/bin/env python3
"""Generate the deterministic Vectras 30000 insertion lattice.

The lattice is documentation/test-planning data. It does not claim that every
slot is implemented production code; promotion still goes through claim gates.
"""

from __future__ import annotations

import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUTPUT = ROOT / "docs" / "knowledge_archives" / "vectras-vm-android" / "INSERTION_LATTICE_30000.csv"
SLOT_COUNT = 30000
RECORDS = ("E20", "E13", "S11")
LAYERS = (
    "archive",
    "catalog",
    "android",
    "native_c",
    "asm_contract",
    "federated_data",
    "security",
    "observability",
    "enterprise",
    "rollback",
)
PRACTICES = (
    "nomalloc_hot_path",
    "q16_16",
    "branchless_preferred",
    "failover",
    "rollback",
    "watchdog",
    "gcd_termination",
    "audit_trail",
    "doc_only_gate",
    "benchmark_gate",
)
AXES = (
    "x_implementation",
    "y_evidence",
    "z_meaning",
    "time_epoch",
    "entropy",
    "coherence",
    "security",
)
PIPELINES = (
    "generic_safe",
    "arm32_fallback",
    "neon_optional",
    "aarch64_primary",
    "termux_api28",
    "local_node",
    "doc_gate",
)
FLAGS = ("DOC_ONLY", "NEEDS_EVIDENCE", "NEEDS_BENCHMARK", "RISK_OPEN", "CODE_BACKED_PENDING")
FAIL_MODES = (
    "invalid_input",
    "missing_evidence",
    "memory_budget_exceeded",
    "period_break",
    "void_22",
    "raw_data_leak",
    "benchmark_missing",
    "abi_contract_break",
)
ROLLBACKS = (
    "status_downgrade",
    "fallback_c",
    "feature_flag_off",
    "commit_revert",
    "deny_query",
    "restore_bootstrap",
    "doc_only_mode",
    "risk_register",
)
COLUMNS = ("slot_id", "record_id", "layer", "practice", "axis", "pipeline", "flag", "fail_mode", "rollback")


def pick(items: tuple[str, ...], index: int, divisor: int = 1) -> str:
    return items[(index // divisor) % len(items)]


def row(index: int) -> tuple[str, ...]:
    zero = index - 1
    layer_stride = len(LAYERS)
    practice_stride = layer_stride * len(PRACTICES)
    axis_stride = practice_stride * len(AXES)
    return (
        f"INS-{index:05d}",
        pick(RECORDS, zero),
        pick(LAYERS, zero),
        pick(PRACTICES, zero, layer_stride),
        pick(AXES, zero, practice_stride),
        pick(PIPELINES, zero, axis_stride),
        pick(FLAGS, zero, 997),
        pick(FAIL_MODES, zero, 389),
        pick(ROLLBACKS, zero, 211),
    )


def main() -> int:
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    with OUTPUT.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(COLUMNS)
        for index in range(1, SLOT_COUNT + 1):
            writer.writerow(row(index))
    print(f"PASS: wrote {SLOT_COUNT} slots to {OUTPUT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
