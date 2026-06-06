#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define BUF_SZ (256 * 1024)
#define ROUNDS 64
#define DISCARD_LOW 8
#define DISCARD_HIGH 8
#define ONE_Q16 65536u
#define ALPHA_Q16 16384u

static uint8_t buf[BUF_SZ];
static uint32_t crc_table[256];
static uint64_t s4[ROUNDS], s8[ROUNDS];

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
    uint64_t x=*(const uint64_t*)a,y=*(const uint64_t*)b;
    return (x>y)-(x<y);
}

static inline double mbps(uint64_t ns){
    return ((double)BUF_SZ*2.0)/((double)ns/1e9)/(1024.0*1024.0);
}

static inline uint32_t qema(uint32_t old,uint32_t in){
    return old + (((int32_t)in-(int32_t)old)*(int32_t)ALPHA_Q16 >> 16);
}

static uint64_t bench_unroll4(uint32_t *out_crc, uint64_t *out_fnv){
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

static uint64_t bench_unroll8(uint32_t *out_crc, uint64_t *out_fnv){
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

static void report(const char *name, uint64_t *s, uint32_t rollback, uint32_t stuck, uint32_t coh){
    qsort(s, ROUNDS, sizeof(uint64_t), cmp_u64);

    uint64_t median=s[ROUNDS/2];
    double trimmed=0.0;
    int n=0;

    for(int i=DISCARD_LOW;i<ROUNDS-DISCARD_HIGH;i++){
        trimmed+=(double)s[i];
        n++;
    }

    trimmed/=n;

    printf("\n=== %s ===\n", name);
    printf("median_ns=%llu\n", (unsigned long long)median);
    printf("median_MBps=%.2f\n", mbps(median));
    printf("trimmed_avg_ns=%.0f\n", trimmed);
    printf("trimmed_avg_MBps=%.2f\n", ((double)BUF_SZ*2.0)/(trimmed/1e9)/(1024.0*1024.0));
    printf("rollback=%u stuck=%u coherence=%.3f\n", rollback, stuck, (double)coh/65536.0);
}

int main(void){
    crc32_init();

    for(size_t i=0;i<BUF_SZ;i++)
        buf[i]=(uint8_t)((i*1315423911u+42u)^(i>>5));

    printf("RAFAELIA EDGE V6 UNROLL 4x/8x\n");
    printf("ARM32 | fused CRC+FNV | manual unroll | low-memory\n");
    printf("buffer=%d KB rounds=%d\n\n", BUF_SZ/1024, ROUNDS);

    uint32_t coh4=ONE_Q16, coh8=ONE_Q16;
    uint32_t rb4=0, rb8=0, st4=0, st8=0;
    uint32_t last4=0, last8=0;

    for(int r=0;r<ROUNDS;r++){
        uint32_t crc4, crc8;
        uint64_t fnv4, fnv8;

        uint64_t dt4=bench_unroll4(&crc4,&fnv4);
        uint64_t mix4=((uint64_t)crc4<<32)^fnv4^((uint64_t)(r+1)*0x9E3779B97F4A7C15ull);
        buf[(mix4*2654435761u)&(BUF_SZ-1)]^=(uint8_t)(mix4>>56);

        uint64_t dt8=bench_unroll8(&crc8,&fnv8);
        uint64_t mix8=((uint64_t)crc8<<32)^fnv8^((uint64_t)(r+9)*0x9E3779B97F4A7C15ull);
        buf[(mix8*2654435761u)&(BUF_SZ-1)]^=(uint8_t)(mix8>>56);

        s4[r]=dt4;
        s8[r]=dt8;

        coh4=qema(coh4,(crc4^last4)?ONE_Q16:ONE_Q16/4);
        coh8=qema(coh8,(crc8^last8)?ONE_Q16:ONE_Q16/4);

        if(crc4==last4) st4++;
        if(crc8==last8) st8++;

        if(coh4<ONE_Q16/2) rb4++;
        if(coh8<ONE_Q16/2) rb8++;

        printf("round=%02d U4_ns=%llu U4_MB/s=%.2f | U8_ns=%llu U8_MB/s=%.2f\n",
            r,
            (unsigned long long)dt4, mbps(dt4),
            (unsigned long long)dt8, mbps(dt8));

        last4=crc4;
        last8=crc8;
    }

    report("UNROLL 4x", s4, rb4, st4, coh4);
    report("UNROLL 8x", s8, rb8, st8, coh8);

    return 0;
}
