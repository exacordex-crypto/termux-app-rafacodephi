#!/usr/bin/env python3
"""Validate the Vectras-VM-Android knowledge archive structure.

This validator is intentionally small and deterministic. It validates the
machine-readable catalog only; it does not claim that DOC_ONLY concepts are
implemented or benchmarked.
"""

from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CATALOG = ROOT / "docs" / "knowledge_archives" / "vectras-vm-android" / "catalog.json"
REQUIRED_RECORD_KEYS = {
    "id",
    "title",
    "domain",
    "status",
    "truth_level",
    "raw_lines",
    "expanded_blocks",
    "falsification_condition",
}
ALLOWED_STATUS = {"DOC_ONLY", "NEEDS_EVIDENCE", "NEEDS_BENCHMARK", "CODE_BACKED", "RISK_OPEN"}
REQUIRED_IDS = {"E20", "E13", "S11"}


def fail(message: str) -> int:
    print(f"FAIL: {message}", file=sys.stderr)
    return 1


def main() -> int:
    if not CATALOG.is_file():
        return fail(f"missing catalog: {CATALOG.relative_to(ROOT)}")

    data = json.loads(CATALOG.read_text(encoding="utf-8"))
    invariants = data.get("invariants", {})
    if invariants.get("attractor_count") != 42:
        return fail("attractor_count must be 42")
    if invariants.get("bitomega_period") != 42:
        return fail("bitomega_period must be 42")
    if invariants.get("lyapunov_phi") != "phi=(1-H)*C":
        return fail("lyapunov_phi contract changed")
    for flag in ("nomalloc", "freestanding", "deterministic", "q16_16", "audit_trail_required"):
        if invariants.get(flag) is not True:
            return fail(f"invariant {flag} must be true")

    records = data.get("records")
    if not isinstance(records, list) or not records:
        return fail("records must be a non-empty list")

    seen = set()
    for record in records:
        missing = REQUIRED_RECORD_KEYS.difference(record)
        if missing:
            return fail(f"record {record.get('id', '<unknown>')} missing keys: {sorted(missing)}")
        record_id = record["id"]
        if record_id in seen:
            return fail(f"duplicate record id: {record_id}")
        seen.add(record_id)
        if record["status"] not in ALLOWED_STATUS:
            return fail(f"record {record_id} has invalid status {record['status']}")
        if not record["raw_lines"] or not all(isinstance(item, str) and item for item in record["raw_lines"]):
            return fail(f"record {record_id} has invalid raw_lines")
        if not record["expanded_blocks"] or not all(isinstance(item, str) and item for item in record["expanded_blocks"]):
            return fail(f"record {record_id} has invalid expanded_blocks")
        if "fal" not in record["falsification_condition"].lower():
            return fail(f"record {record_id} must carry a falsification/failure condition")

    missing_ids = REQUIRED_IDS.difference(seen)
    if missing_ids:
        return fail(f"missing required ids: {sorted(missing_ids)}")

    navigation = data.get("navigation")
    if not isinstance(navigation, list) or len(navigation) < 3:
        return fail("navigation must contain at least 3 levels")
    for item in navigation:
        path = ROOT / item.get("path", "")
        if not path.is_file():
            return fail(f"navigation path does not exist: {item.get('path')}")

    print(f"PASS: {CATALOG.relative_to(ROOT)} records={len(records)} navigation={len(navigation)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
