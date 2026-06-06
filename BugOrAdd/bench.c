#include <stdio.h>
#include <time.h>

extern void mk_scalar(int);

double now(){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(){
    double t0 = now();
    mk_scalar(20000000);
    double t1 = now();

    printf("{\"latency\": %.6f}\n", t1 - t0);
    return 0;
}
