#include <stdio.h>
#include <stdint.h>

#if defined(__aarch64__)
#define ARCH64 1
#else
#define ARCH64 0
#endif

// ================= TIMER LOW LEVEL =================
static inline uint64_t now() {
#if ARCH64
    uint64_t v;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(v));
    return v;
#else
    uint32_t lo, hi;
    __asm__ volatile("mrrc p15, 0, %0, %1, c14" : "=r"(lo), "=r"(hi));
    return ((uint64_t)hi << 32) | lo;
#endif
}

// ================= CORE BENCH (PONTO C STYLE) =================
static inline uint64_t core_op(uint64_t x) {
    x ^= (x << 13);
    x ^= (x >> 7);
    x ^= (x << 17);
    return x;
}

int main() {
    // warmup (anti cold cache jitter)
    uint64_t x = 123456789;
    for (int i = 0; i < 100000; i++) x = core_op(x);

    uint64_t t1 = now();

    // bench loop (determinístico)
    for (int i = 0; i < 1000000; i++) {
        x = core_op(x);
    }

    uint64_t t2 = now();

    printf("RESULT=%llu\n", (unsigned long long)x);
    printf("DELTA=%llu cycles\n", (unsigned long long)(t2 - t1));

    return 0;
}
