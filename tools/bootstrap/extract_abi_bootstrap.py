#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / 'asm.sh'
OUT = ROOT / 'app' / 'src' / 'main' / 'cpp' / 'lowlevel' / 'bootstrap_ref'

text = SRC.read_text(encoding='utf-8')

# Extract heredoc content only
m = re.search(r"<< 'TERMINUS'\n(.*)\nTERMINUS\s*$", text, re.S)
if not m:
    raise SystemExit('Could not locate TERMINUS heredoc in asm.sh')
content = m.group(1)

arm64_start = content.find('SEÇÃO A — ARM64')
arm32_start = content.find('SEÇÃO B — ARM32')
if arm64_start < 0 or arm32_start < 0 or arm32_start <= arm64_start:
    raise SystemExit('Could not locate ARM64/ARM32 sections in reference text')

header = content[:arm64_start].strip() + '\n'
arm64 = content[arm64_start:arm32_start].strip() + '\n'
arm32 = content[arm32_start:].strip() + '\n'

OUT.mkdir(parents=True, exist_ok=True)
(OUT / 'README.txt').write_text(
    'Generated from asm.sh reference text. Do not edit manually; run tools/bootstrap/extract_abi_bootstrap.py\n',
    encoding='utf-8'
)
(OUT / 'rafaelia_abi_header.txt').write_text(header, encoding='utf-8')
(OUT / 'bootstrap_arm64_adaptive.s.txt').write_text(arm64, encoding='utf-8')
(OUT / 'bootstrap_arm32_adaptive.s.txt').write_text(arm32, encoding='utf-8')
print('Generated bootstrap reference files in', OUT)
