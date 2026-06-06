#!/usr/bin/env python3
"""Exhaustively falsify or confirm the documented Q16 period-42 recurrence."""
from __future__ import annotations

import math
import sys

Q16_ONE = 1 << 16
MASK16 = 0xFFFF
PERIOD = 42
SQRT3_OVER_2_Q16 = round((math.sqrt(3) / 2) * Q16_ONE)
PI_SIN_279_Q16 = round((math.pi * math.sin(math.radians(279))) * Q16_ONE)


def step_q16(value: int) -> int:
    next_value = ((value * SQRT3_OVER_2_Q16) >> 16) - PI_SIN_279_Q16
    return next_value & MASK16


def advance(value: int, count: int = PERIOD) -> int:
    current = value
    for _ in range(count):
        current = step_q16(current)
    return current


def main() -> int:
    for initial in range(Q16_ONE):
        final = advance(initial)
        if final != initial:
            print("period42_fixed_q16: FAIL")
            print(f"first_counterexample.initial={initial}")
            print(f"first_counterexample.after_42={final}")
            print(f"sqrt3_over_2_q16={SQRT3_OVER_2_Q16}")
            print(f"pi_sin_279_q16={PI_SIN_279_Q16}")
            return 1
    print("period42_fixed_q16: PASS")
    print(f"states_checked={Q16_ONE}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
