#!/usr/bin/env python3
"""Execute the RAFAELIA claim matrix checks and emit PASS/OPEN/FAIL reports."""
from __future__ import annotations

import csv
import json
import subprocess
from dataclasses import asdict, dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "reports/rafaelia_claim_execution_matrix.csv"
REPORT_MD = ROOT / "reports/rafaelia_claim_execution_results.md"
REPORT_JSON = ROOT / "reports/rafaelia_claim_execution_results.json"
TIMEOUT_SECONDS = 30


@dataclass(frozen=True)
class Result:
    claim_id: str
    status: str
    outcome: str
    command: str
    exit_code: int
    stdout_excerpt: str
    stderr_excerpt: str


def excerpt(text: str, limit: int = 320) -> str:
    compact = " ".join(text.split())
    if len(compact) <= limit:
        return compact
    return compact[: limit - 3] + "..."


def classify(row: dict[str, str], code: int) -> str:
    status = row["status"]
    if code == 0:
        return "PASS"
    if status == "CODE_BACKED":
        return "FAIL"
    if status == "RISK_OPEN":
        return "OPEN_RISK"
    return "OPEN_EVIDENCE"


def run_row(row: dict[str, str]) -> Result:
    command = row["executable_check"]
    try:
        proc = subprocess.run(
            command,
            cwd=ROOT,
            shell=True,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=TIMEOUT_SECONDS,
        )
        code = proc.returncode
        out = proc.stdout
        err = proc.stderr
    except subprocess.TimeoutExpired as exc:
        code = 124
        out = exc.stdout or ""
        err = exc.stderr or f"timeout after {TIMEOUT_SECONDS}s"
    return Result(
        claim_id=row["claim_id"],
        status=row["status"],
        outcome=classify(row, code),
        command=command,
        exit_code=code,
        stdout_excerpt=excerpt(out),
        stderr_excerpt=excerpt(err),
    )


def load_rows() -> list[dict[str, str]]:
    with MATRIX.open(newline="", encoding="utf-8") as handle:
        return list(csv.DictReader(handle))


def write_reports(results: list[Result]) -> None:
    REPORT_JSON.write_text(json.dumps([asdict(item) for item in results], ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    counts: dict[str, int] = {}
    for item in results:
        counts[item.outcome] = counts.get(item.outcome, 0) + 1
    lines = [
        "# RAFAELIA Claim Execution Results",
        "",
        "## Summary",
        "",
    ]
    for key in sorted(counts):
        lines.append(f"- {key}: {counts[key]}")
    lines.extend([
        "",
        "## Results",
        "",
        "| Claim | Matrix status | Outcome | Exit | Command | stdout excerpt | stderr excerpt |",
        "|---|---|---|---:|---|---|---|",
    ])
    for item in results:
        lines.append(
            f"| {item.claim_id} | {item.status} | {item.outcome} | {item.exit_code} | `{item.command}` | {item.stdout_excerpt or '-'} | {item.stderr_excerpt or '-'} |"
        )
    REPORT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    if not MATRIX.exists():
        raise SystemExit(f"missing matrix: {MATRIX.relative_to(ROOT)}")
    results = [run_row(row) for row in load_rows()]
    write_reports(results)
    hard_fail = [item for item in results if item.outcome == "FAIL"]
    print(f"RAFAELIA claim checks executed: {len(results)}")
    if hard_fail:
        for item in hard_fail:
            print(f"FAIL {item.claim_id}: {item.command}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
