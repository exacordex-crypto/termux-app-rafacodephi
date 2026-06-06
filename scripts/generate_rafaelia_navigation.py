#!/usr/bin/env python3
"""Generate RAFAELIA documentation navigation and claim execution artifacts.

The generator is intentionally dependency-free and deterministic. It does not touch
runtime/native code; it only scans Markdown/TXT documentation up to a configurable
maximum depth and emits navigation plus claim/evidence matrices.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
DOC_SUFFIXES = {".md", ".txt"}
EXCLUDED_PARTS = {".git", ".gradle", "build", ".idea"}


@dataclass(frozen=True)
class DocEntry:
    path: str
    depth: int
    domain: str
    status: str


@dataclass(frozen=True)
class ClaimEntry:
    claim_id: str
    seed: str
    source: str
    status: str
    evidence_file: str
    executable_check: str
    falsification: str
    rollback: str


def rel_path(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def has_excluded_part(path: Path) -> bool:
    rel = path.relative_to(ROOT)
    return any(part in EXCLUDED_PARTS for part in rel.parts)


def domain_for(path: str) -> str:
    low = path.lower()
    if "security" in low or "audit" in low or "auditoria" in low:
        return "segurança/auditoria"
    if "android" in low or "termux" in low or "bootstrap" in low:
        return "android/bootstrap"
    if "asm" in low or "lowlevel" in low or "baremetal" in low or "arme" in low or "arm" in low:
        return "low-level/arm"
    if "benchmark" in low or "test" in low or "validation" in low or "validate" in low:
        return "testes/benchmark"
    if "math" in low or "formula" in low or "toro" in low or "vectra" in low:
        return "matemática/vectra"
    if "mercado" in low or "market" in low:
        return "mercado/dados"
    if "readme" in low or "indice" in low or "doc" in low or "summary" in low or "navigation" in low:
        return "documentação/navegação"
    return "conceito/manifesto/outros"


def status_for(path: str) -> str:
    low = path.lower()
    if low.startswith("reports/"):
        return "ARTIFACT"
    if "old/" in low or "solto" in low or "mvp/" in low or "bugoradd/" in low:
        return "LOOSE_OR_EXPERIMENTAL"
    if "audit" in low or "validation" in low or "test" in low or "benchmark" in low:
        return "EVIDENCE_OR_TEST_PLAN"
    return "DOC_INDEXED"


def iter_docs(max_depth: int) -> list[DocEntry]:
    entries: list[DocEntry] = []
    for path in ROOT.rglob("*"):
        if not path.is_file():
            continue
        if has_excluded_part(path):
            continue
        if path.suffix.lower() not in DOC_SUFFIXES:
            continue
        rel = rel_path(path)
        depth = len(Path(rel).parts)
        if depth > max_depth:
            continue
        entries.append(DocEntry(rel, depth, domain_for(rel), status_for(rel)))
    return sorted(entries, key=lambda item: item.path)


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def claim_entries() -> list[ClaimEntry]:
    rows = [
        ("C01", "T^7 toroidal state", "Formulas 1-4,45", "DOC_ONLY", "docs/RAFAELIA_CONCEPT_CARRY_MAP.md", "rg -n 'ToroidalMap|T\\^7|\\[0,1\\)' docs Arme BugOrAdd", "No executable state encoder/decoder or normalization rule exists.", "Keep as DOC_ONLY and do not wire into runtime."),
        ("C02", "EMA coherence/entropy alpha=0.25", "Formulas 5-8", "NEEDS_EVIDENCE", "docs/RAFAELIA_CONCEPT_CARRY_MAP.md", "rg -n 'alpha=0.25|\\(1-H\\).*C|entropy_milli' .", "Measured code uses a different alpha or cannot reproduce phi.", "Gate claim behind docs until deterministic test vector exists."),
        ("C03", "Attractor count |A|=42", "Formulas 9-10,23", "RISK_OPEN", "AGENTS.md", "rg -n 'attractor_table|period-42|BitOmega|\\|A\\| = 42' .", "Table has fewer/more than 42 entries or period test fails.", "Do not close known bug; keep doc-only until table/test fixed."),
        ("C04", "Signal FFT/cardio correlation", "Formulas 11-12,44", "NEEDS_BENCHMARK", "docs/RAFAELIA_HZ_AS_MEMORY.md", "rg -n 'cardio|FFT|S\\(omega\\)|H_cardio|Hz' docs reports scripts", "No signal artifact or correlation baseline is present.", "Label as model; do not assert biological measurement."),
        ("C05", "Layer tensor/integral information", "Formula 13,41,50", "DOC_ONLY", "docs/RAFAELIA_SEMANTIC_LAYERS.md", "rg -n 'semantic|layer|tensor|Phi|mathcal' docs", "No layer composition implementation exists.", "Preserve as navigation semantics only."),
        ("C06", "Entropy approximation", "Formulas 14,43", "NEEDS_EVIDENCE", "docs/RAFAELIA_CONCEPT_CARRY_MAP.md", "rg -n 'entropy_milli|unique.*6000|transitions.*2000|entropy' app scripts docs", "Entropy formula differs from documented test vectors.", "Keep claim out of runtime until vector set passes."),
        ("C07", "XOR/FNV-like hash flow", "Formulas 15,30-32", "NEEDS_EVIDENCE", "docs/RAFAELIA_CONCEPT_CARRY_MAP.md", "rg -n '0x100000001B3|FNV|\\^|xor|hash' app scripts rmr Arme", "Checksum differs across ABI or is described as cryptographic when not.", "Classify as integrity/checksum only."),
        ("C08", "CRC/Merkle integrity", "Formulas 16-17,33", "CODE_BACKED", "docs/RAFAELIA_CONCEPT_CARRY_MAP.md", "./scripts/raf_external_integrity.sh", "Integrity script fails or required digest missing.", "Block promotion and keep previous artifact hashes."),
        ("C09", "sqrt(3)/2 spiral/Fibonacci geometry", "Formulas 18-22,38-39", "NEEDS_EVIDENCE", "docs/RAFAELIA_SQRT3_FIBONACCI_AUDIT.md", "rg -n 'sqrt3|sqrt\\(3\\)|Fibonacci|Spiral|279' docs app scripts Arme", "Q16 conversion or recurrence test exceeds tolerance.", "Demote performance/physics claims to DOC_ONLY."),
        ("C10", "Capacity/log2 geometry", "Formulas 24-25,46,49", "NEEDS_EVIDENCE", "docs/RAFAELIA_MEMORY_MODEL.md", "rg -n 'log2|capacity|M.*N|C_geom|bits_geom' docs app scripts", "Buffer dimensions or bounds do not match capacity rule.", "Fallback to explicit bounds documentation."),
        ("C11", "VOID paradox / Pi max", "Formulas 26-27", "RISK_OPEN", "AGENTS.md", "rg -n 'VOID|Pi|max|paradox|attractor #22|#22' .", "VOID is silently patched or attractor #22 loses flag.", "Keep #22 flagged; no silent patch."),
        ("C12", "Coprime toroidal traversal", "Formulas 28-29", "NEEDS_EVIDENCE", "docs/RAFAELIA_CONCEPT_CARRY_MAP.md", "rg -n 'gcd|coprime|toroidal|stride|modulo' docs app scripts Arme", "gcd(delta, dimension) != 1 in traversal config.", "Reject traversal table and keep previous route."),
        ("C13", "VFC key stream XOR", "Formulas 34-35", "DOC_ONLY", "docs/RAFAELIA_CONCEPT_CARRY_MAP.md", "rg -n 'VFC|keystream|k\\(t\\)|xor' docs app scripts", "Claim is presented as secure encryption without threat model.", "State experimental only; require security review."),
        ("C14", "Physics/Maxwell/Hamiltonian metaphors", "Formulas 36-37,47-48", "DOC_ONLY", "docs/RAFAELIA_SESSION_TRUTH_NAVIGATION.md", "rg -n 'Maxwell|Hamilton|quant|sin\\(Delta|E_link' docs", "Metaphor is promoted to measured physical claim.", "Demote to parable/model until instrumented evidence exists."),
        ("C15", "Multilingual sound/Hz semantic layer", "Prompt linguistic/audio seed", "NEEDS_BENCHMARK", "docs/RAFAELIA_SESSION_TRUTH_NAVIGATION.md", "rg -n 'som|Hz|fonet|Unicode|multil|direção|direction|audio' docs", "No phonetic/acoustic dataset or reproducible transform exists.", "Keep as semantic metadata and avoid runtime claim."),
        ("C16", "Market/data feature matrix", "Prompt market variables", "NEEDS_EVIDENCE", "ANALISE_MERCADO.md", "rg -n 'Mercado|SWOT|Projeções|Métricas|mobile|Termux' ANALISE_MERCADO.md", "No dataset split, leakage control, or metric artifact exists.", "Document as schema; no financial recommendation."),
        ("C17", "No-heap/no-GC hot path", "User architecture constraint", "NEEDS_EVIDENCE", "AGENTS.md", "rg -n 'malloc|calloc|realloc|new |Garbage|heap' app rmr Arme scripts", "Hot path allocates heap or JNI path hides allocation.", "Move allocation to cold path or keep module out of hot path."),
        ("C18", "Branchless/low-friction path", "User architecture constraint", "NEEDS_EVIDENCE", "docs/RAFAELIA_BRANCHLESS_MAP.md", "rg -n 'branchless|csel|csinc|if \\(|for \\(' docs app rmr Arme", "Branch-heavy path is claimed branchless without asm/C evidence.", "Demote claim and add measured microbenchmark before restore."),
        ("C19", "Failsafe/failover/rollback/watchdog", "User operational constraint", "NEEDS_EVIDENCE", "docs/ENGINEERING_SYSTEM_RUNBOOK.md", "rg -n 'failsafe|failover|rollback|watchdog|smoke_release_gate' docs scripts reports", "No command or artifact demonstrates rollback/guard.", "Keep as runbook requirement; do not call enterprise-ready."),
        ("C20", "Documentation 5-level navigation", "User catalog request", "CODE_BACKED", "docs/RAFAELIA_5_LEVEL_DOCUMENTATION_NAVIGATION.md", "python3 scripts/generate_rafaelia_navigation.py --max-depth 5 --check", "Generated docs differ from checked-in docs.", "Regenerate and recommit deterministic artifacts."),
    ]
    return [ClaimEntry(*row) for row in rows]


def write_inventory_csv(entries: list[DocEntry], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(["path", "depth", "domain", "status"])
        for entry in entries:
            writer.writerow([entry.path, entry.depth, entry.domain, entry.status])


def write_claim_csv(entries: list[ClaimEntry], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle, lineterminator="\n")
        writer.writerow(["claim_id", "seed", "source", "status", "evidence_file", "executable_check", "falsification", "rollback"])
        for entry in entries:
            writer.writerow([entry.claim_id, entry.seed, entry.source, entry.status, entry.evidence_file, entry.executable_check, entry.falsification, entry.rollback])


def write_json(entries: list[DocEntry], claims: list[ClaimEntry], path: Path, max_depth: int) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    data = {
        "max_depth": max_depth,
        "document_count": len(entries),
        "claim_count": len(claims),
        "documents_sha256": sha256_file(ROOT / "reports/rafaelia_docs_inventory.csv"),
        "claims_sha256": sha256_file(ROOT / "reports/rafaelia_claim_execution_matrix.csv"),
        "documents": [entry.__dict__ for entry in entries],
        "claims": [entry.__dict__ for entry in claims],
    }
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def write_inventory_md(entries: list[DocEntry], path: Path, max_depth: int) -> None:
    by_depth: dict[int, list[DocEntry]] = {}
    by_domain: dict[str, list[DocEntry]] = {}
    for entry in entries:
        by_depth.setdefault(entry.depth, []).append(entry)
        by_domain.setdefault(entry.domain, []).append(entry)

    lines: list[str] = [
        "# RAFAELIA 5-Level Documentation Navigation",
        "",
        "## Propósito",
        "",
        f"Este mapa é gerado por `scripts/generate_rafaelia_navigation.py --max-depth {max_depth}` e cataloga arquivos Markdown/TXT encontrados até {max_depth} níveis de profundidade, excluindo `.git`, `.gradle`, `build` e `.idea`.",
        "",
        "Use este arquivo junto com `docs/RAFAELIA_SESSION_TRUTH_NAVIGATION.md`: este mapa aponta onde estão os textos; a navegação de sessão define como separar fato, hipótese, metáfora, risco e evidência.",
        "",
        "## Resumo quantitativo",
        "",
        f"- Total de arquivos Markdown/TXT até {max_depth} níveis: **{len(entries)}**.",
    ]
    for depth in sorted(by_depth):
        lines.append(f"- Nível de profundidade {depth}: **{len(by_depth[depth])}** arquivos.")
    lines.extend(["", "## Domínios", "", "| Domínio | Quantidade | Primeiras entradas |", "|---|---:|---|"])
    for domain in sorted(by_domain):
        sample = ", ".join(f"`{entry.path}`" for entry in by_domain[domain][:4])
        if len(by_domain[domain]) > 4:
            sample += ", ..."
        lines.append(f"| {domain} | {len(by_domain[domain])} | {sample} |")
    lines.extend(["", "## Navegação por profundidade"])
    for depth in sorted(by_depth):
        lines.extend(["", f"### Nível {depth}"])
        for entry in by_depth[depth]:
            lines.append(f"- `{entry.path}` — domínio: {entry.domain}; status: {entry.status}")
    lines.extend([
        "",
        "## Regras de uso",
        "",
        "1. Não promover arquivo solto a build oficial sem teste e rollback.",
        "2. Não apagar documento histórico sem preservar rastreabilidade.",
        "3. Para `.S`, cumprir o contrato AGENTS e ler `VECTRA_OS.md` antes de tocar, se o arquivo existir no repositório.",
        "4. Para claims de performance, exigir benchmark reproduzível antes de afirmar ganho.",
        "5. Para metáforas e parábolas, manter valor didático sem converter em evidência científica.",
    ])
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_claim_md(entries: list[ClaimEntry], path: Path) -> None:
    lines = [
        "# RAFAELIA Claim Execution Matrix",
        "",
        "## Propósito",
        "",
        "Esta matriz executa o próximo passo recomendado: ligar cada semente/claim a arquivo, status, comando verificável, condição de falsificação e rollback. Ela não transforma metáfora em prova; ela define o caminho para provar, negar ou manter `DOC_ONLY`.",
        "",
        "## Estados",
        "",
        "- `DOC_ONLY`: útil como semântica, parábola ou requisito, mas sem promoção para runtime/build.",
        "- `NEEDS_EVIDENCE`: precisa de teste, vetor, dataset ou inspeção antes de claim forte.",
        "- `NEEDS_BENCHMARK`: precisa de benchmark reproduzível antes de claim de desempenho/sinal.",
        "- `CODE_BACKED`: há comando/caminho de código ou artefato para verificação.",
        "- `RISK_OPEN`: risco conhecido permanece aberto; não fechar sem correção real.",
        "",
        "## Matriz",
        "",
        "| ID | Semente/claim | Origem | Status | Evidência | Check executável | Falsificação | Rollback/mitigação |",
        "|---|---|---|---|---|---|---|---|",
    ]
    for entry in entries:
        lines.append(
            f"| {entry.claim_id} | {entry.seed} | {entry.source} | {entry.status} | `{entry.evidence_file}` | `{entry.executable_check}` | {entry.falsification} | {entry.rollback} |"
        )
    lines.extend([
        "",
        "## Gate enterprise/fullstack seguro",
        "",
        "Uma entrega só pode ser chamada de funcional/enterprise neste repositório quando os itens `CODE_BACKED` passam, os itens `RISK_OPEN` estão explicitamente aceitos ou corrigidos, e os itens `NEEDS_EVIDENCE`/`NEEDS_BENCHMARK` não são apresentados como fatos medidos.",
    ])
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def outputs(max_depth: int) -> dict[str, Path]:
    return {
        "inventory_md": ROOT / "docs/RAFAELIA_5_LEVEL_DOCUMENTATION_NAVIGATION.md",
        "claim_md": ROOT / "docs/RAFAELIA_CLAIM_EXECUTION_MATRIX.md",
        "inventory_csv": ROOT / "reports/rafaelia_docs_inventory.csv",
        "claim_csv": ROOT / "reports/rafaelia_claim_execution_matrix.csv",
        "json": ROOT / "reports/rafaelia_navigation_summary.json",
    }


def generate(max_depth: int) -> None:
    claims = claim_entries()
    out = outputs(max_depth)
    # Emit claim artifacts first so the documentation scan is stable even when
    # the claim Markdown file is newly created in the same run.
    write_claim_csv(claims, out["claim_csv"])
    write_claim_md(claims, out["claim_md"])
    entries = iter_docs(max_depth)
    write_inventory_csv(entries, out["inventory_csv"])
    write_inventory_md(entries, out["inventory_md"], max_depth)
    write_json(entries, claims, out["json"], max_depth)


def snapshot(paths: Iterable[Path]) -> dict[str, bytes | None]:
    snap: dict[str, bytes | None] = {}
    for path in paths:
        snap[path.as_posix()] = path.read_bytes() if path.exists() else None
    return snap


def changed(before: dict[str, bytes | None]) -> list[str]:
    delta: list[str] = []
    for name, content in before.items():
        path = Path(name)
        after = path.read_bytes() if path.exists() else None
        if after != content:
            delta.append(rel_path(path))
    return delta


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate RAFAELIA navigation and claim matrices.")
    parser.add_argument("--max-depth", type=int, default=5, help="maximum file depth to scan")
    parser.add_argument("--check", action="store_true", help="fail if generated artifacts are not up to date")
    args = parser.parse_args()
    if args.max_depth < 1:
        parser.error("--max-depth must be >= 1")
    out = outputs(args.max_depth)
    before = snapshot(out.values()) if args.check else {}
    generate(args.max_depth)
    if args.check:
        delta = changed(before)
        if delta:
            print("Generated artifacts are stale:")
            for item in delta:
                print(f"- {item}")
            return 1
    print("RAFAELIA navigation artifacts generated")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
