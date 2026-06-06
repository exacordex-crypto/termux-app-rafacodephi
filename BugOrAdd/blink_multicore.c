#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define CORES 8
#define ITER  50000000

typedef struct {
    int id;
    uint32_t estado;
} thread_data;

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

void* worker(void* arg) {

    thread_data* td = (thread_data*)arg;

    uint32_t estado = td->estado;
    uint32_t eco = 0;

    for (uint64_t i = 0; i < ITER; i++) {

        uint32_t prev = estado;

        estado = operador(estado);

        eco = mix_crc32(estado ^ prev);

        estado ^= eco;
    }

    td->estado = estado;

    return NULL;
}

int main() {

    pthread_t threads[CORES];
    thread_data data[CORES];

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < CORES; i++) {

        data[i].id = i;
        data[i].estado = 0xA5A5A5A5 ^ i;

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

    uint64_t total_iter =
        (uint64_t)CORES * ITER;

    printf("\n--- MULTICORE RESULTADO ---\n");
    printf("Threads   : %d\n", CORES);
    printf("Total Ops : %llu\n", total_iter);
    printf("Tempo(s)  : %.4f\n", tempo);
    printf("Ops/s     : %.2f\n",
           total_iter / tempo);

    return 0;
}
