#!/usr/bin/env python3
"""Validate the Vectras-VM-Android knowledge archive structure.

This validator is intentionally small and deterministic. It validates the
machine-readable catalog only; it does not claim that DOC_ONLY concepts are
implemented or benchmarked.
"""

from __future__ import annotations

import csv
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CATALOG = ROOT / "docs" / "knowledge_archives" / "vectras-vm-android" / "catalog.json"
REQUIRED_INSERTION_COLUMNS = {
    "slot_id",
    "record_id",
    "layer",
    "practice",
    "axis",
    "pipeline",
    "flag",
    "fail_mode",
    "rollback",
}
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
    if invariants.get("minimum_insertion_slots") != 30000:
        return fail("minimum_insertion_slots must be exactly 30000")
    required_true_flags = (
        "nomalloc",
        "freestanding",
        "deterministic",
        "q16_16",
        "audit_trail_required",
        "failover_required",
        "rollback_required",
        "failsafe_required",
    )
    for flag in required_true_flags:
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

    insertion_lattice = data.get("insertion_lattice", {})
    lattice_path = ROOT / insertion_lattice.get("path", "")
    minimum_slots = insertion_lattice.get("minimum_slots")
    if minimum_slots != invariants.get("minimum_insertion_slots"):
        return fail("insertion lattice minimum does not match invariant")
    if not lattice_path.is_file():
        return fail(f"missing insertion lattice: {insertion_lattice.get('path')}")

    with lattice_path.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None or REQUIRED_INSERTION_COLUMNS.difference(reader.fieldnames):
            return fail("insertion lattice missing required columns")
        slot_count = 0
        slot_ids = set()
        for row in reader:
            slot_count += 1
            slot_id = row.get("slot_id", "")
            if not slot_id.startswith("INS-"):
                return fail(f"invalid slot_id at row {slot_count}: {slot_id}")
            if slot_id in slot_ids:
                return fail(f"duplicate slot_id: {slot_id}")
            slot_ids.add(slot_id)
            if row.get("record_id") not in seen:
                return fail(f"slot {slot_id} references unknown record_id {row.get('record_id')}")
            for column in REQUIRED_INSERTION_COLUMNS:
                if not row.get(column):
                    return fail(f"slot {slot_id} has empty {column}")
        if slot_count < minimum_slots:
            return fail(f"insertion lattice has {slot_count} slots, expected at least {minimum_slots}")

    navigation = data.get("navigation")
    if not isinstance(navigation, list) or len(navigation) < 3:
        return fail("navigation must contain at least 3 levels")
    for item in navigation:
        path = ROOT / item.get("path", "")
        if not path.is_file():
            return fail(f"navigation path does not exist: {item.get('path')}")

    print(f"PASS: {CATALOG.relative_to(ROOT)} records={len(records)} navigation={len(navigation)} insertion_slots={slot_count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
