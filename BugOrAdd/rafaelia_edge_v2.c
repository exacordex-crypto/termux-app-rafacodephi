#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BUF_SZ   (256 * 1024)
#define ROUNDS   48
#define WARMUP   8
#define ALPHA_Q16 16384u
#define ONE_Q16 65536u

static uint8_t buf[BUF_SZ];
static uint32_t crc_table[256];
static uint64_t samples[ROUNDS];

static uint64_t now_ns(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

static void crc32_init(void){
    for(uint32_t i=0;i<256;i++){
        uint32_t c = i;
        for(int j=0;j<8;j++)
            c = (c >> 1) ^ (0xEDB88320u & -(c & 1));
        crc_table[i] = c;
    }
}

static uint32_t crc32_fast(const uint8_t *p, size_t n){
    uint32_t crc = 0xFFFFFFFFu;

    while(n--){
        crc = crc_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    }

    return ~crc;
}

static uint64_t fnv1a64(const uint8_t *p, size_t n){
    uint64_t h = 1469598103934665603ull;

    while(n--){
        h ^= *p++;
        h *= 1099511628211ull;
    }

    return h;
}

static uint32_t qema(uint32_t old, uint32_t in){
    return old + (((int32_t)in - (int32_t)old) * (int32_t)ALPHA_Q16 >> 16);
}

static int cmp_u64(const void *a, const void *b){
    uint64_t ua = *(const uint64_t*)a;
    uint64_t ub = *(const uint64_t*)b;
    return (ua > ub) - (ua < ub);
}

int main(void){

    crc32_init();

    for(size_t i=0;i<BUF_SZ;i++){
        buf[i] = (uint8_t)((i * 1315423911u + 42u) ^ (i >> 5));
    }

    uint32_t ema_coherence = ONE_Q16;
    uint32_t rollback = 0;
    uint32_t stuck = 0;
    uint32_t last_crc = 0;

    uint64_t best = ~0ull;
    uint64_t worst = 0;
    uint64_t sum = 0;

    printf("RAFAELIA EDGE V2\\n");
    printf("ARM32 | Android | low-memory | cache-aware\\n");
    printf("buffer=%d KB rounds=%d warmup=%d\\n\\n",
        BUF_SZ / 1024,
        ROUNDS,
        WARMUP);

    for(int r=0;r<ROUNDS + WARMUP;r++){

        uint64_t t0 = now_ns();

        uint32_t crc = crc32_fast(buf, BUF_SZ);
        uint64_t fnv = fnv1a64(buf, BUF_SZ);

        uint64_t mix = ((uint64_t)crc << 32) ^ fnv;

        size_t idx = (mix * 2654435761u) & (BUF_SZ - 1);

        buf[idx] ^= (uint8_t)(mix >> 56);

        uint64_t dt = now_ns() - t0;

        if(r >= WARMUP){

            int i = r - WARMUP;

            samples[i] = dt;

            if(dt < best) best = dt;
            if(dt > worst) worst = dt;

            sum += dt;

            uint32_t coherence =
                (crc ^ last_crc) ? ONE_Q16 : (ONE_Q16 / 4);

            ema_coherence = qema(ema_coherence, coherence);

            if(crc == last_crc)
                stuck++;

            if(ema_coherence < (ONE_Q16 / 2)){
                rollback++;
                buf[idx] ^= (uint8_t)(mix >> 56);
            }

            double mbps =
                ((double)BUF_SZ * 2.0) /
                ((double)dt / 1e9) /
                (1024.0 * 1024.0);

            printf(
                "round=%02d ns=%llu MB/s=%.2f coherence=%.3f rollback=%u stuck=%u\n",
                i,
                (unsigned long long)dt,
                mbps,
                (double)ema_coherence / 65536.0,
                rollback,
                stuck
            );
        }

        last_crc = crc;
    }

    qsort(samples, ROUNDS, sizeof(uint64_t), cmp_u64);

    uint64_t median = samples[ROUNDS / 2];

    double avg =
        (double)sum / (double)ROUNDS;

    double avg_mbps =
        ((double)BUF_SZ * 2.0) /
        (avg / 1e9) /
        (1024.0 * 1024.0);

    double median_mbps =
        ((double)BUF_SZ * 2.0) /
        ((double)median / 1e9) /
        (1024.0 * 1024.0);

    double jitter =
        (double)(worst - best) / avg;

    double mcr =
        ((double)(ROUNDS - rollback + 1) *
        ((double)ema_coherence / 65536.0)) /
        (1.0 + jitter + stuck);

    printf("\n=== RESULTADO EDGE V2 ===\n");

    printf("avg_ns=%.0f\n", avg);
    printf("median_ns=%llu\n",
        (unsigned long long)median);

    printf("best_ns=%llu\n",
        (unsigned long long)best);

    printf("worst_ns=%llu\n",
        (unsigned long long)worst);

    printf("throughput_avg_MBps=%.2f\n",
        avg_mbps);

    printf("throughput_median_MBps=%.2f\n",
        median_mbps);

    printf("jitter_ratio=%.6f\n", jitter);

    printf("rollback=%u stuck=%u\n",
        rollback,
        stuck);

    printf("MCR_proxy=%.6f\n", mcr);

    if(mcr > 20.0)
        puts("REGIME: ESTAVEL");
    else if(mcr > 5.0)
        puts("REGIME: TRANSICAO");
    else
        puts("REGIME: INSTAVEL");

    return 0;
}
