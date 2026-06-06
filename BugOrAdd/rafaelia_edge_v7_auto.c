#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define BUF_SZ (256 * 1024)
#define ROUNDS 72
#define DISCARD_LOW 8
#define DISCARD_HIGH 8
#define ONE_Q16 65536u
#define ALPHA_Q16 16384u

static uint8_t buf[BUF_SZ];
static uint32_t crc_table[256];

static uint64_t s_base[ROUNDS];
static uint64_t s_u4[ROUNDS];
static uint64_t s_u8[ROUNDS];

static uint64_t now_ns(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

static void crc32_init(void){
    for(uint32_t i=0;i<256;i++){
        uint32_t c=i;
        for(int j=0;j<8;j++)
            c=(c>>1)^(0xEDB88320u & -(c&1));
        crc_table[i]=c;
    }
}

static int cmp_u64(const void *a,const void *b){
    uint64_t x=*(const uint64_t*)a;
    uint64_t y=*(const uint64_t*)b;
    return (x>y)-(x<y);
}

static inline double mbps(uint64_t ns){
    return ((double)BUF_SZ*2.0)/((double)ns/1e9)/(1024.0*1024.0);
}

static inline uint32_t qema(uint32_t old,uint32_t in){
    return old + (((int32_t)in-(int32_t)old)*(int32_t)ALPHA_Q16 >> 16);
}

static uint64_t bench_base(uint32_t *out_crc, uint64_t *out_fnv){
    uint32_t crc=0xFFFFFFFFu;
    uint64_t fnv=1469598103934665603ull;
    uint64_t t0=now_ns();

    for(size_t i=0;i<BUF_SZ;i++){
        uint8_t v=buf[i];
        crc=crc_table[(crc^v)&255]^(crc>>8);
        fnv^=v;
        fnv*=1099511628211ull;
    }

    *out_crc=~crc;
    *out_fnv=fnv;
    return now_ns()-t0;
}

static uint64_t bench_u4(uint32_t *out_crc, uint64_t *out_fnv){
    uint32_t crc=0xFFFFFFFFu;
    uint64_t fnv=1469598103934665603ull;
    uint64_t t0=now_ns();

    for(size_t i=0;i<BUF_SZ;i+=4){
        uint8_t a=buf[i+0], b=buf[i+1], c=buf[i+2], d=buf[i+3];

        crc=crc_table[(crc^a)&255]^(crc>>8); fnv^=a; fnv*=1099511628211ull;
        crc=crc_table[(crc^b)&255]^(crc>>8); fnv^=b; fnv*=1099511628211ull;
        crc=crc_table[(crc^c)&255]^(crc>>8); fnv^=c; fnv*=1099511628211ull;
        crc=crc_table[(crc^d)&255]^(crc>>8); fnv^=d; fnv*=1099511628211ull;
    }

    *out_crc=~crc;
    *out_fnv=fnv;
    return now_ns()-t0;
}

static uint64_t bench_u8(uint32_t *out_crc, uint64_t *out_fnv){
    uint32_t crc=0xFFFFFFFFu;
    uint64_t fnv=1469598103934665603ull;
    uint64_t t0=now_ns();

    for(size_t i=0;i<BUF_SZ;i+=8){
        uint8_t a0=buf[i+0], a1=buf[i+1], a2=buf[i+2], a3=buf[i+3];
        uint8_t a4=buf[i+4], a5=buf[i+5], a6=buf[i+6], a7=buf[i+7];

        crc=crc_table[(crc^a0)&255]^(crc>>8); fnv^=a0; fnv*=1099511628211ull;
        crc=crc_table[(crc^a1)&255]^(crc>>8); fnv^=a1; fnv*=1099511628211ull;
        crc=crc_table[(crc^a2)&255]^(crc>>8); fnv^=a2; fnv*=1099511628211ull;
        crc=crc_table[(crc^a3)&255]^(crc>>8); fnv^=a3; fnv*=1099511628211ull;
        crc=crc_table[(crc^a4)&255]^(crc>>8); fnv^=a4; fnv*=1099511628211ull;
        crc=crc_table[(crc^a5)&255]^(crc>>8); fnv^=a5; fnv*=1099511628211ull;
        crc=crc_table[(crc^a6)&255]^(crc>>8); fnv^=a6; fnv*=1099511628211ull;
        crc=crc_table[(crc^a7)&255]^(crc>>8); fnv^=a7; fnv*=1099511628211ull;
    }

    *out_crc=~crc;
    *out_fnv=fnv;
    return now_ns()-t0;
}

static void mutate(uint32_t crc, uint64_t fnv, int r, uint64_t salt){
    uint64_t mix=((uint64_t)crc<<32)^fnv^((uint64_t)(r+1)*salt);
    size_t idx=(mix*2654435761u)&(BUF_SZ-1);
    buf[idx]^=(uint8_t)(mix>>56);
}

static double trimmed_avg_ns(uint64_t *s){
    uint64_t tmp[ROUNDS];

    for(int i=0;i<ROUNDS;i++)
        tmp[i]=s[i];

    qsort(tmp, ROUNDS, sizeof(uint64_t), cmp_u64);

    double acc=0.0;
    int n=0;

    for(int i=DISCARD_LOW;i<ROUNDS-DISCARD_HIGH;i++){
        acc+=(double)tmp[i];
        n++;
    }

    return acc/(double)n;
}

static uint64_t median_ns(uint64_t *s){
    uint64_t tmp[ROUNDS];

    for(int i=0;i<ROUNDS;i++)
        tmp[i]=s[i];

    qsort(tmp, ROUNDS, sizeof(uint64_t), cmp_u64);

    return tmp[ROUNDS/2];
}

static void report_one(const char *name, uint64_t *s){
    uint64_t med=median_ns(s);
    double trim=trimmed_avg_ns(s);

    printf("\n[%s]\n", name);
    printf("median_ns=%llu median_MBps=%.2f\n",
        (unsigned long long)med, mbps(med));
    printf("trimmed_avg_ns=%.0f trimmed_avg_MBps=%.2f\n",
        trim,
        ((double)BUF_SZ*2.0)/(trim/1e9)/(1024.0*1024.0));
}

int main(void){
    crc32_init();

    for(size_t i=0;i<BUF_SZ;i++)
        buf[i]=(uint8_t)((i*1315423911u+42u)^(i>>5));

    printf("RAFAELIA EDGE V7 AUTO-SELECTOR\n");
    printf("ARM32 | base vs unroll4 vs unroll8 | median adaptive\n");
    printf("buffer=%d KB rounds=%d discard_low=%d discard_high=%d\n\n",
        BUF_SZ/1024, ROUNDS, DISCARD_LOW, DISCARD_HIGH);

    uint32_t coh=ONE_Q16;
    uint32_t rollback=0, stuck=0, last_crc=0;

    for(int r=0;r<ROUNDS;r++){
        uint32_t c0,c4,c8;
        uint64_t f0,f4,f8;

        uint64_t d0=bench_base(&c0,&f0);
        mutate(c0,f0,r,0x9E3779B97F4A7C15ull);

        uint64_t d4=bench_u4(&c4,&f4);
        mutate(c4,f4,r,0xBF58476D1CE4E5B9ull);

        uint64_t d8=bench_u8(&c8,&f8);
        mutate(c8,f8,r,0x94D049BB133111EBull);

        s_base[r]=d0;
        s_u4[r]=d4;
        s_u8[r]=d8;

        uint32_t crc_mix=c0^c4^c8;
        coh=qema(coh,(crc_mix^last_crc)?ONE_Q16:ONE_Q16/4);

        if(crc_mix==last_crc)
            stuck++;

        if(coh<ONE_Q16/2)
            rollback++;

        printf("round=%02d BASE=%llu %.2f | U4=%llu %.2f | U8=%llu %.2f | coh=%.3f rb=%u st=%u\n",
            r,
            (unsigned long long)d0, mbps(d0),
            (unsigned long long)d4, mbps(d4),
            (unsigned long long)d8, mbps(d8),
            (double)coh/65536.0,
            rollback,
            stuck);

        last_crc=crc_mix;
    }

    report_one("BASE", s_base);
    report_one("UNROLL4", s_u4);
    report_one("UNROLL8", s_u8);

    double t_base=trimmed_avg_ns(s_base);
    double t_u4=trimmed_avg_ns(s_u4);
    double t_u8=trimmed_avg_ns(s_u8);

    const char *winner="BASE";
    double best=t_base;

    if(t_u4<best){
        best=t_u4;
        winner="UNROLL4";
    }

    if(t_u8<best){
        best=t_u8;
        winner="UNROLL8";
    }

    double best_mbps =
        ((double)BUF_SZ*2.0)/(best/1e9)/(1024.0*1024.0);

    double penalty_u4 = (t_u4 / t_base) - 1.0;
    double penalty_u8 = (t_u8 / t_base) - 1.0;

    printf("\n=== RESULTADO EDGE V7 ===\n");
    printf("winner=%s\n", winner);
    printf("winner_trimmed_avg_ns=%.0f\n", best);
    printf("winner_trimmed_avg_MBps=%.2f\n", best_mbps);
    printf("base_trimmed_avg_ns=%.0f\n", t_base);
    printf("u4_trimmed_avg_ns=%.0f penalty_vs_base=%.3f\n", t_u4, penalty_u4);
    printf("u8_trimmed_avg_ns=%.0f penalty_vs_base=%.3f\n", t_u8, penalty_u8);
    printf("rollback=%u stuck=%u coherence=%.3f\n",
        rollback, stuck, (double)coh/65536.0);

    if(rollback==0 && stuck==0 && coh>ONE_Q16*3/4)
        puts("REGIME: ESTAVEL / SELETOR CONFIAVEL");
    else
        puts("REGIME: OBSERVAR / RUIDO DOMINANDO");

    return 0;
}
