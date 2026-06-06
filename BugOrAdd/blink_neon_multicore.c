#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <arm_neon.h>
#include <time.h>

#define CORES 8
#define ITER  10000000

typedef struct {
    uint32_t seed[4];
} thread_data;

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

void* worker(void* arg) {

    thread_data* td =
        (thread_data*)arg;

    uint32x4_t estado =
        vld1q_u32(td->seed);

    uint32x4_t eco =
        vdupq_n_u32(0x12345678);

    for (uint64_t i = 0; i < ITER; i++) {

        uint32x4_t prev = estado;

        estado = operador_vec(estado);

        eco = veorq_u32(eco, prev);

        estado = veorq_u32(estado, eco);
    }

    vst1q_u32(td->seed, estado);

    return NULL;
}

int main() {

    pthread_t threads[CORES];
    thread_data data[CORES];

    struct timespec start, end;

    for (int i = 0; i < CORES; i++) {

        data[i].seed[0] = 0xA5A5A5A5 ^ i;
        data[i].seed[1] = 0x5A5A5A5A ^ i;
        data[i].seed[2] = 0x3C3C3C3C ^ i;
        data[i].seed[3] = 0xC3C3C3C3 ^ i;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < CORES; i++) {

        pthread_create(
            &threads[i],
            NULL,
            worker,
            &data[i]
        );
    }

    for (int i = 0; i < CORES; i++) {

        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double tempo =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    uint64_t total_ops =
        (uint64_t)CORES *
        ITER * 4;

    printf("\n--- NEON MULTICORE ---\n");

    printf("Threads  : %d\n", CORES);

    printf("Tempo(s) : %.4f\n", tempo);

    printf("Ops/s    : %.2f\n",
           total_ops / tempo);

    return 0;
}
