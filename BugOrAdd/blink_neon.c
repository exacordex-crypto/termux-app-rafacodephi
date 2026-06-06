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

    uint32x4_t estado =
        vdupq_n_u32(0xA5A5A5A5);

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (uint64_t i = 0; i < ITER; i++) {

        estado = operador_vec(estado);

    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    uint32_t out[4];
    vst1q_u32(out, estado);

    double tempo =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("\n--- NEON RESULTADO ---\n");
    printf("Estado[0] : 0x%08X\n", out[0]);
    printf("Estado[1] : 0x%08X\n", out[1]);
    printf("Estado[2] : 0x%08X\n", out[2]);
    printf("Estado[3] : 0x%08X\n", out[3]);

    printf("Tempo(s)  : %.4f\n", tempo);
    printf("Ops/s     : %.2f\n",
           (ITER * 4.0) / tempo);

    return 0;
}
