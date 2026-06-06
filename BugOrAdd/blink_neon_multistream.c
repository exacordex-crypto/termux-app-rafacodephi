#include <stdio.h>
#include <stdint.h>
#include <arm_neon.h>
#include <time.h>

#define ITER 25000000

static inline uint32x4_t operador_vec(uint32x4_t s) {

    uint32x4_t a =
        veorq_u32(s, vshlq_n_u32(s, 5));

    uint32x4_t phi =
        vdupq_n_u32(0x9e3779b9);

    uint32x4_t b =
        vaddq_u32(a, phi);

    uint32x4_t c =
        veorq_u32(b, vshrq_n_u32(b, 7));

    return c;
}

int main() {

    uint32_t seed[4] = {
        0xA5A5A5A5,
        0x5A5A5A5A,
        0x3C3C3C3C,
        0xC3C3C3C3
    };

    uint32x4_t estado =
        vld1q_u32(seed);

    uint32x4_t eco =
        vdupq_n_u32(0x12345678);

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (uint64_t i = 0; i < ITER; i++) {

        uint32x4_t prev = estado;

        estado = operador_vec(estado);

        eco = veorq_u32(eco, prev);

        estado = veorq_u32(estado, eco);

    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    uint32_t out[4];
    vst1q_u32(out, estado);

    double tempo =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("\n--- NEON MULTISTREAM ---\n");

    printf("Estado[0] : 0x%08X\n", out[0]);
    printf("Estado[1] : 0x%08X\n", out[1]);

    printf("Tempo(s)  : %.4f\n", tempo);

    printf("Ops/s     : %.2f\n",
           (ITER * 4.0) / tempo);

    return 0;
}
