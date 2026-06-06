#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define BUF_SZ      (256 * 1024)
#define ROUNDS      64
#define DISCARD_LOW 8
#define DISCARD_HIGH 8

#define ALPHA_Q16   16384u
#define ONE_Q16     65536u

static uint8_t  buf[BUF_SZ];
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
            c = (c >> 1) ^
                (0xEDB88320u & -(c & 1));

        crc_table[i] = c;
    }
}

static inline uint32_t qema(uint32_t old, uint32_t in){

    return old +
        (((int32_t)in - (int32_t)old) *
        (int32_t)ALPHA_Q16 >> 16);
}

static int cmp_u64(const void *a, const void *b){

    uint64_t x = *(const uint64_t*)a;
    uint64_t y = *(const uint64_t*)b;

    return (x > y) - (x < y);
}

static inline double mbps(uint64_t ns){

    return ((double)BUF_SZ * 2.0) /
        ((double)ns / 1e9) /
        (1024.0 * 1024.0);
}

int main(void){

    crc32_init();

    for(size_t i=0;i<BUF_SZ;i++){

        buf[i] =
            (uint8_t)(
                (i * 1315423911u + 42u) ^
                (i >> 5)
            );
    }

    uint32_t ema_coherence = ONE_Q16;

    uint32_t rollback = 0;
    uint32_t stuck = 0;
    uint32_t last_crc = 0;

    uint64_t best = ~0ull;
    uint64_t worst = 0;
    uint64_t sum = 0;

    printf("RAFAELIA EDGE V5 STABLE-WINDOW\n");
    printf("ARM32 | fused-pass | trimmed-median | adaptive window\n");
    printf("buffer=%d KB rounds=%d discard_low=%d discard_high=%d\n\n",
        BUF_SZ / 1024,
        ROUNDS,
        DISCARD_LOW,
        DISCARD_HIGH);

    for(int r=0;r<ROUNDS;r++){

        uint64_t t0 = now_ns();

        uint32_t crc = 0xFFFFFFFFu;
        uint64_t fnv = 1469598103934665603ull;

        for(size_t i=0;i<BUF_SZ;i++){

            uint8_t v = buf[i];

            crc =
                crc_table[(crc ^ v) & 0xFF] ^
                (crc >> 8);

            fnv ^= v;
            fnv *= 1099511628211ull;
        }

        crc = ~crc;

        uint64_t mix =
            ((uint64_t)crc << 32) ^
            fnv ^
            ((uint64_t)(r + 1) *
            0x9E3779B97F4A7C15ull);

        size_t idx =
            (mix * 2654435761u) &
            (BUF_SZ - 1);

        buf[idx] ^= (uint8_t)(mix >> 56);

        uint64_t dt = now_ns() - t0;

        samples[r] = dt;

        if(dt < best) best = dt;
        if(dt > worst) worst = dt;

        sum += dt;

        uint32_t coherence =
            (crc ^ last_crc)
            ? ONE_Q16
            : (ONE_Q16 / 4);

        ema_coherence =
            qema(ema_coherence, coherence);

        if(crc == last_crc)
            stuck++;

        if(ema_coherence < (ONE_Q16 / 2)){

            rollback++;

            buf[idx] ^=
                (uint8_t)(mix >> 56);
        }

        printf(
            "round=%02d ns=%llu MB/s=%.2f coherence=%.3f rollback=%u stuck=%u\n",
            r,
            (unsigned long long)dt,
            mbps(dt),
            (double)ema_coherence / 65536.0,
            rollback,
            stuck
        );

        last_crc = crc;
    }

    qsort(samples,
          ROUNDS,
          sizeof(uint64_t),
          cmp_u64);

    uint64_t median =
        samples[ROUNDS / 2];

    double trimmed_sum = 0.0;
    int trimmed_n = 0;

    for(int i=DISCARD_LOW;
        i < (ROUNDS - DISCARD_HIGH);
        i++){

        trimmed_sum += (double)samples[i];
        trimmed_n++;
    }

    double trimmed_avg =
        trimmed_sum / (double)trimmed_n;

    double trimmed_mbps =
        ((double)BUF_SZ * 2.0) /
        (trimmed_avg / 1e9) /
        (1024.0 * 1024.0);

    double jitter =
        (double)(worst - best) /
        ((double)sum / (double)ROUNDS);

    double mcr =
        ((double)(ROUNDS - rollback + 1) *
        ((double)ema_coherence / 65536.0)) /
        (1.0 + jitter + stuck);

    printf("\n=== RESULTADO EDGE V5 ===\n");

    printf("median_ns=%llu\n",
        (unsigned long long)median);

    printf("median_MBps=%.2f\n",
        mbps(median));

    printf("trimmed_avg_ns=%.0f\n",
        trimmed_avg);

    printf("trimmed_avg_MBps=%.2f\n",
        trimmed_mbps);

    printf("best_ns=%llu\n",
        (unsigned long long)best);

    printf("worst_ns=%llu\n",
        (unsigned long long)worst);

    printf("jitter_ratio=%.6f\n",
        jitter);

    printf("rollback=%u stuck=%u\n",
        rollback,
        stuck);

    printf("MCR_proxy=%.6f\n",
        mcr);

    if(mcr > 20.0)
        puts("REGIME: ESTAVEL");

    else if(mcr > 5.0)
        puts("REGIME: TRANSICAO");

    else
        puts("REGIME: INSTAVEL");

    return 0;
}
