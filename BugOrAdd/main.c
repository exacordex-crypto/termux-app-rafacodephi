#include <stdio.h>
#include <time.h>
#include <stdint.h>

extern void core_run(int);

static double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(void) {
    int iters = 10000000;   // 10M
    double warm[3];

    for (int i = 0; i < 3; i++) {
        double t0 = now();
        core_run(2000000);
        double t1 = now();
        warm[i] = t1 - t0;
    }

    double t0 = now();
    core_run(iters);
    double t1 = now();

    double dt = t1 - t0;
    double ops = (double)iters * 35.0; // instruções aprox
    double ops_sec = ops / dt;

    printf("\n=== RESULTADOS ===\n");
    printf("Tempo total: %.6f s\n", dt);
    printf("Instrucoes/s: %.2f M\n", ops_sec / 1e6);
    printf("Warmup: %.4f / %.4f / %.4f s\n", warm[0], warm[1], warm[2]);

    return 0;
}
