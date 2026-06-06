#!/usr/bin/env python3
"""Validate VECTRA/RAFAELIA invariant evidence from repository files.

This script is intentionally static: it does not infer truth from prose. Each row in
reports/vectra_invariant_matrix.csv names a reference file, a pattern or artifact,
and a deterministic check. Failed invariant evidence returns a non-zero exit code.
"""
from __future__ import annotations

import csv
import json
import re
import subprocess
import sys
from dataclasses import asdict, dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "reports" / "vectra_invariant_matrix.csv"
REPORT_MD = ROOT / "reports" / "vectra_invariant_results.md"
REPORT_JSON = ROOT / "reports" / "vectra_invariant_results.json"


@dataclass(frozen=True)
class Result:
    invariant_id: str
    invariant: str
    check_kind: str
    check_target: str
    expected: str
    outcome: str
    detail: str
    mitigation: str


def rel(path: Path) -> str:
    return str(path.relative_to(ROOT))


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def csv_rows(path: Path) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def split_targets(value: str) -> list[Path]:
    return [ROOT / part for part in value.split(";")]


def check_regex_exists(row: dict[str, str]) -> tuple[bool, str]:
    path = ROOT / row["check_target"]
    pattern = row["expected"]
    if not path.exists():
        return False, f"missing file {rel(path)}"
    text = read_text(path)
    if re.search(pattern, text):
        return True, f"pattern found in {rel(path)}: {pattern}"
    return False, f"pattern not found in {rel(path)}: {pattern}"


def check_csv_row_count(row: dict[str, str]) -> tuple[bool, str]:
    path = ROOT / row["check_target"]
    expected = int(row["expected"])
    if not path.exists():
        return False, f"missing file {rel(path)}"
    count = len(csv_rows(path))
    return count == expected, f"{rel(path)} rows={count}, expected={expected}"


def check_json_metrics_count(row: dict[str, str]) -> tuple[bool, str]:
    path = ROOT / row["check_target"]
    expected = int(row["expected"])
    if not path.exists():
        return False, f"missing file {rel(path)}"
    data = json.loads(read_text(path))
    metrics = data.get("metrics", [])
    count = len(metrics)
    return count == expected, f"{rel(path)} metrics={count}, expected={expected}"


def check_top42_names_match(row: dict[str, str]) -> tuple[bool, str]:
    csv_path, json_path = split_targets(row["check_target"])
    for path in (csv_path, json_path):
        if not path.exists():
            return False, f"missing file {rel(path)}"
    names_csv = [item["name"] for item in csv_rows(csv_path)]
    data = json.loads(read_text(json_path))
    names_json = [item.get("name", "") for item in data.get("metrics", [])]
    if names_csv == names_json:
        return True, f"metric names match for {len(names_csv)} entries"
    for index, pair in enumerate(zip(names_csv, names_json), start=1):
        left, right = pair
        if left != right:
            return False, f"first mismatch at {index}: csv={left!r}, json={right!r}"
    return False, f"metric list length mismatch: csv={len(names_csv)}, json={len(names_json)}"


def check_command_exit_code(row: dict[str, str]) -> tuple[bool, str]:
    expected = int(row["expected"])
    command = row["check_target"]
    proc = subprocess.run(
        command,
        cwd=ROOT,
        shell=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=30,
    )
    compact_out = " ".join(proc.stdout.split())
    compact_err = " ".join(proc.stderr.split())
    detail = f"exit={proc.returncode}, expected={expected}"
    if compact_out:
        detail += f", stdout={compact_out[:220]}"
    if compact_err:
        detail += f", stderr={compact_err[:220]}"
    return proc.returncode == expected, detail


def check_csv_code_backed_has_check(row: dict[str, str]) -> tuple[bool, str]:
    path = ROOT / row["check_target"]
    if not path.exists():
        return False, f"missing file {rel(path)}"
    missing: list[str] = []
    for item in csv_rows(path):
        if item.get("status") == "CODE_BACKED" and not item.get("executable_check", "").strip():
            missing.append(item.get("claim_id", "<unknown>"))
    if missing:
        return False, "CODE_BACKED rows missing executable checks: " + ", ".join(missing)
    return True, "all CODE_BACKED rows have executable checks"


CHECKS = {
    "regex_exists": check_regex_exists,
    "csv_row_count": check_csv_row_count,
    "json_metrics_count": check_json_metrics_count,
    "top42_names_match": check_top42_names_match,
    "csv_code_backed_has_check": check_csv_code_backed_has_check,
    "command_exit_code": check_command_exit_code,
}


def run_row(row: dict[str, str]) -> Result:
    check_kind = row["check_kind"]
    if check_kind not in CHECKS:
        ok = False
        detail = f"unknown check kind: {check_kind}"
    else:
        ok, detail = CHECKS[check_kind](row)
    return Result(
        invariant_id=row["invariant_id"],
        invariant=row["invariant"],
        check_kind=check_kind,
        check_target=row["check_target"],
        expected=row["expected"],
        outcome="PASS" if ok else "FAIL",
        detail=detail,
        mitigation=row["mitigation"],
    )


def write_reports(results: list[Result]) -> None:
    REPORT_JSON.write_text(json.dumps([asdict(item) for item in results], ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    pass_count = sum(1 for item in results if item.outcome == "PASS")
    fail_count = sum(1 for item in results if item.outcome == "FAIL")
    lines = [
        "# VECTRA Invariant Execution Results",
        "",
        "This report is generated by `python3 scripts/validate_vectra_invariants.py`.",
        "It checks repository artifacts named in `reports/vectra_invariant_matrix.csv`; it does not infer claims from metaphorical text.",
        "",
        "## Summary",
        "",
        f"- PASS: {pass_count}",
        f"- FAIL: {fail_count}",
        "",
        "## Results",
        "",
        "| Invariant | Outcome | Check | Expected | Detail | Mitigation |",
        "|---|---|---|---|---|---|",
    ]
    for item in results:
        lines.append(
            f"| {item.invariant_id} | {item.outcome} | `{item.check_kind}` on `{item.check_target}` | `{item.expected}` | {item.detail} | {item.mitigation} |"
        )
    REPORT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    if not MATRIX.exists():
        raise SystemExit(f"missing matrix: {rel(MATRIX)}")
    rows = csv_rows(MATRIX)
    results = [run_row(row) for row in rows]
    write_reports(results)
    failures = [item for item in results if item.outcome == "FAIL"]
    print(f"VECTRA invariant checks executed: {len(results)}")
    print(f"PASS: {len(results) - len(failures)}")
    print(f"FAIL: {len(failures)}")
    for item in failures:
        print(f"FAIL {item.invariant_id}: {item.detail}")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
