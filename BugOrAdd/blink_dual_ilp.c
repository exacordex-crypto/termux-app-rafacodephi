#include <stdio.h>
#include <stdint.h>
#include <time.h>

static inline uint32_t mix_crc32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

static inline uint32_t operador(uint32_t s) {
    uint32_t a = s ^ (s << 5);
    uint32_t b = a + 0x9e3779b9;
    uint32_t c = b ^ (b >> 7);
    return c;
}

int main() {

    const uint64_t ITER = 100000000;

    uint32_t s0 = 0xA5A5A5A5;
    uint32_t s1 = 0x5A5A5A5A;

    uint32_t eco = 0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (uint64_t i = 0; i < ITER; i++) {

        uint32_t p0 = s0;
        uint32_t p1 = s1;

        s0 = operador(s0);
        s1 = operador(s1);

        eco ^= mix_crc32((s0 ^ p0) ^ (s1 ^ p1));

        s0 ^= eco;
        s1 ^= eco;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double tempo =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("\n--- DUAL STATE RESULTADO ---\n");
    printf("Estado0 : 0x%08X\n", s0);
    printf("Estado1 : 0x%08X\n", s1);
    printf("Eco      : 0x%08X\n", eco);
    printf("Tempo(s) : %.4f\n", tempo);
    printf("Ciclos/s : %.2f\n", ITER / tempo);

    return 0;
}
