#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define BUF_SZ   (256 * 1024)
#define ROUNDS   48
#define WARMUP   8
#define ALPHA_Q16 16384u
#define ONE_Q16 65536u

static uint8_t buf[BUF_SZ];
static uint32_t crc_table[256];
static uint64_t samples_crc[ROUNDS];
static uint64_t samples_fnv[ROUNDS];
static uint64_t samples_total[ROUNDS];

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
    while(n--)
        crc = crc_table[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
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
    uint64_t x = *(const uint64_t*)a;
    uint64_t y = *(const uint64_t*)b;
    return (x > y) - (x < y);
}

static double mbps_from_ns(uint64_t ns){
    return ((double)BUF_SZ) / ((double)ns / 1e9) / (1024.0 * 1024.0);
}

int main(void){
    crc32_init();

    for(size_t i=0;i<BUF_SZ;i++)
        buf[i] = (uint8_t)((i * 1315423911u + 42u) ^ (i >> 5));

    uint32_t ema_coherence = ONE_Q16;
    uint32_t rollback = 0;
    uint32_t stuck = 0;
    uint32_t last_crc = 0;

    uint64_t best = ~0ull, worst = 0, sum_total = 0;

    printf("RAFAELIA EDGE V3\n");
    printf("ARM32 | Android | CRC-only | FNV-only | total | low-memory\n");
    printf("buffer=%d KB rounds=%d warmup=%d\n\n", BUF_SZ/1024, ROUNDS, WARMUP);

    for(int r=0;r<ROUNDS + WARMUP;r++){
        uint64_t t0 = now_ns();
        uint32_t crc = crc32_fast(buf, BUF_SZ);
        uint64_t t1 = now_ns();
        uint64_t fnv = fnv1a64(buf, BUF_SZ);
        uint64_t t2 = now_ns();

        uint64_t mix = ((uint64_t)crc << 32) ^ fnv ^ (uint64_t)(r + 1) * 0x9E3779B97F4A7C15ull;
        size_t idx = (mix * 2654435761u) & (BUF_SZ - 1);
        buf[idx] ^= (uint8_t)(mix >> 56);

        uint64_t dt_crc = t1 - t0;
        uint64_t dt_fnv = t2 - t1;
        uint64_t dt_total = t2 - t0;

        if(r >= WARMUP){
            int i = r - WARMUP;

            samples_crc[i] = dt_crc;
            samples_fnv[i] = dt_fnv;
            samples_total[i] = dt_total;

            if(dt_total < best) best = dt_total;
            if(dt_total > worst) worst = dt_total;
            sum_total += dt_total;

            uint32_t coherence = (crc ^ last_crc) ? ONE_Q16 : ONE_Q16/4;
            ema_coherence = qema(ema_coherence, coherence);

            if(crc == last_crc) stuck++;

            if(ema_coherence < ONE_Q16/2){
                rollback++;
                buf[idx] ^= (uint8_t)(mix >> 56);
            }

            printf("round=%02d crc_ns=%llu fnv_ns=%llu total_ns=%llu total_MB/s=%.2f coherence=%.3f rollback=%u stuck=%u\n",
                i,
                (unsigned long long)dt_crc,
                (unsigned long long)dt_fnv,
                (unsigned long long)dt_total,
                mbps_from_ns(dt_total),
                (double)ema_coherence / 65536.0,
                rollback,
                stuck);
        }

        last_crc = crc;
    }

    qsort(samples_crc, ROUNDS, sizeof(uint64_t), cmp_u64);
    qsort(samples_fnv, ROUNDS, sizeof(uint64_t), cmp_u64);
    qsort(samples_total, ROUNDS, sizeof(uint64_t), cmp_u64);

    uint64_t med_crc = samples_crc[ROUNDS/2];
    uint64_t med_fnv = samples_fnv[ROUNDS/2];
    uint64_t med_total = samples_total[ROUNDS/2];

    double avg_total = (double)sum_total / (double)ROUNDS;
    double jitter = (double)(worst - best) / avg_total;

    double mcr =
        ((double)(ROUNDS - rollback + 1) *
        ((double)ema_coherence / 65536.0)) /
        (1.0 + jitter + stuck);

    printf("\n=== RESULTADO EDGE V3 ===\n");
    printf("median_crc_ns=%llu crc_MBps=%.2f\n", (unsigned long long)med_crc, mbps_from_ns(med_crc));
    printf("median_fnv_ns=%llu fnv_MBps=%.2f\n", (unsigned long long)med_fnv, mbps_from_ns(med_fnv));
    printf("median_total_ns=%llu total_MBps=%.2f\n", (unsigned long long)med_total, mbps_from_ns(med_total));
    printf("avg_total_ns=%.0f best_total_ns=%llu worst_total_ns=%llu\n", avg_total, (unsigned long long)best, (unsigned long long)worst);
    printf("jitter_ratio=%.6f\n", jitter);
    printf("rollback=%u stuck=%u\n", rollback, stuck);
    printf("MCR_proxy=%.6f\n", mcr);

    if(mcr > 20.0) puts("REGIME: ESTAVEL / OPERADOR EFICIENTE");
    else if(mcr > 5.0) puts("REGIME: TRANSICAO / OBSERVAR JITTER");
    else puts("REGIME: INSTAVEL / rollback-stuck dominando");

    return 0;
}
