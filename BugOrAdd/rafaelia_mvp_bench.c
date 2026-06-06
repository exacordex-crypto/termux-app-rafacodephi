#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define N (1<<20)
#define ROUNDS 64
#define ALPHA_Q16 16384u   /* 0.25 */
#define ONE_Q16 65536u
#define PHI64 0x9E3779B97F4A7C15ULL

static uint8_t buf[N];

static uint64_t now_ns(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec*1000000000ull + ts.tv_nsec;
}

static uint32_t crc32_soft(const uint8_t *p, size_t n){
    uint32_t crc = 0xFFFFFFFFu;
    for(size_t i=0;i<n;i++){
        crc ^= p[i];
        for(int k=0;k<8;k++)
            crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1));
    }
    return ~crc;
}

static uint64_t fnv1a64(const uint8_t *p, size_t n){
    uint64_t h = 1469598103934665603ull;
    for(size_t i=0;i<n;i++){
        h ^= p[i];
        h *= 1099511628211ull;
    }
    return h;
}

static uint32_t qema(uint32_t old, uint32_t in){
    return old + (((int32_t)in - (int32_t)old) * (int32_t)ALPHA_Q16 >> 16);
}

int main(void){
    for(size_t i=0;i<N;i++)
        buf[i] = (uint8_t)((i * 1315423911u + 42u) ^ (i >> 3));

    uint32_t ema_latency = 0;
    uint32_t ema_coherence = ONE_Q16;
    uint32_t stuck = 0, rollback = 0;
    uint64_t best = ~0ull, worst = 0, sum = 0;
    uint32_t last_crc = 0;

    printf("RAFAELIA MVP BENCH — runtime | ARM/edge | determinismo | CRC/FNV | EMA 0.25\\n");
    printf("buffer=%d bytes rounds=%d\\n\\n", N, ROUNDS);

    for(int r=0;r<ROUNDS;r++){
        uint64_t t0 = now_ns();

        uint32_t crc = crc32_soft(buf, N);
        uint64_t fnv = fnv1a64(buf, N);

        uint64_t mix = ((uint64_t)crc << 32) ^ fnv ^ (PHI64 * (uint64_t)(r+1));
        size_t idx = mix & (N-1);
        buf[idx] ^= (uint8_t)(mix >> 56);

        uint64_t t1 = now_ns();
        uint64_t dt = t1 - t0;

        if(dt < best) best = dt;
        if(dt > worst) worst = dt;
        sum += dt;

        uint32_t lat_q16 = (uint32_t)((dt > 1000000000ull ? 1000000000ull : dt) / 1000u);
        ema_latency = qema(ema_latency, lat_q16);

        uint32_t delta_crc = crc ^ last_crc;
        uint32_t coherence = delta_crc ? ONE_Q16 : ONE_Q16/4;
        ema_coherence = qema(ema_coherence, coherence);

        if(crc == last_crc) stuck++;
        else if(stuck) stuck--;

        if(ema_coherence < ONE_Q16/2){
            rollback++;
            buf[idx] ^= (uint8_t)(mix >> 56);
            ema_coherence = qema(ema_coherence, ONE_Q16);
        }

        last_crc = crc;

        double mbps = ((double)N * 2.0) / ((double)dt / 1e9) / (1024.0*1024.0);
        double coh = (double)ema_coherence / 65536.0;

        printf("round=%02d ns=%llu MB/s=%.2f crc=%08x fnv=%016llx coherence=%.3f stuck=%u rollback=%u\\n",
            r, (unsigned long long)dt, mbps, crc, (unsigned long long)fnv, coh, stuck, rollback);
    }

    double avg = (double)sum / ROUNDS;
    double avg_mbps = ((double)N * 2.0) / (avg / 1e9) / (1024.0*1024.0);
    double jitter = (double)(worst - best) / avg;
    double mcr = ((double)(ROUNDS - rollback + 1) * ((double)ema_coherence/65536.0)) / (1.0 + jitter + stuck);

    printf("\\n=== RESULTADO MVP ===\\n");
    printf("avg_ns=%.0f best_ns=%llu worst_ns=%llu\\n", avg, (unsigned long long)best, (unsigned long long)worst);
    printf("throughput_avg_MBps=%.2f\\n", avg_mbps);
    printf("jitter_ratio=%.6f\\n", jitter);
    printf("rollback=%u stuck=%u\\n", rollback, stuck);
    printf("MCR_proxy=%.6f\\n", mcr);

    if(mcr > 20.0) puts("REGIME: ESTAVEL / OPERADOR EFICIENTE");
    else if(mcr > 5.0) puts("REGIME: TRANSICAO / OBSERVAR JITTER");
    else puts("REGIME: INSTAVEL / rollback-stuck dominando");

    return 0;
}
