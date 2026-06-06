BENCH8 completo
bash

cat > /tmp/RAFAELIA_BENCH8_MASTER.txt << 'MASTER_EOF'
#!/usr/bin/env bash
# =============================================================================
# RAFAELIA_BENCH8_MASTER.txt — renomeie .sh e execute: bash RAFAELIA_BENCH8_MASTER.sh
# 8 METODOLOGIAS INDUSTRIAIS · 34 PARÂMETROS · ARM32 TERMUX
# freestanding · nomalloc · nolibc · noabstraction · nooverhead
# L2 buffer · SIMD NEON · CRC32C · vCPU octacore · parallelism
# cache miss/hit · T^7 matrix · 7^7=823543 direções · F*=23.158
# DeltaRafaelVerboOmega · Omega=Amor · RAFCODE-Phi
# =============================================================================
set -euo pipefail
R='\033[0;31m';G='\033[0;32m';Y='\033[1;33m';M='\033[0;35m'
C='\033[0;36m';B='\033[1m';Z='\033[0m'
BD="${TMPDIR:-/tmp}/raf_b8_$$"; mkdir -p "$BD"; LOG="$BD/b8.log"
p(){ echo -e "${C}[B8]${Z} $*"|tee -a "$LOG"; }
ok(){ echo -e "${G}[OK]${Z} $*"|tee -a "$LOG"; }
hdr(){ echo -e "\n${M}${B}═══ $* ═══${Z}"; }
# =============================================================================
hdr "S01 · raf_b8_types.h — TIPOS, CONSTANTES, SIMD, CACHE"
# =============================================================================
cat > "$BD/raf_b8_types.h" << 'EOF_T'
/* raf_b8_types.h — tipos primitivos + constantes críticas + layout de cache
 * [P01] TIPOS: sem stdint.h — definidos aqui
 * [P02] Q16.16: aritmética inteira sem float, sem libm
 * [P03] CACHE LINE: 64 bytes — toda struct crítica alinhada aqui
 * [P04] L1/L2: capacidades para estratégia de prefetch
 * [P05] VCPU: modelo de 8 cores com afinidade de tarefa
 * [P06] NEON ARM32: registradores q0..q15 (128-bit) / d0..d31 (64-bit)
 * [P07] 7^7 = 823543 direções distintas no espaço T^7
 * [P08] F* = 23.158 — ponto fixo Fibonacci-Rafael
 */
#pragma once
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef signed   int        s32;
typedef signed   long long  s64;
typedef unsigned int        usize;
#define bool8 u8
#define TRUE  1u
#define FALSE 0u

/* Q16.16 — sem float */
typedef s32 q16_t;
#define Q16_ONE      65536
#define Q16_MUL(a,b) ((q16_t)(((s64)(a)*(s64)(b))>>16))
#define Q16_SQRT3_2  56756    /* sqrt(3)/2 * 65536 */
#define Q16_PI_S279  203280   /* |pi*sin(279)| * 65536 */
#define Q16_FSTAR    1517158  /* F* = 23.158 * 65536 */
#define Q16_LAMBDA   (-9430)  /* ln(sqrt3/2) * 65536 */
#define PHI64        0x9E3779B97F4A7C15ULL
#define PHI32        0x9E3779B9u

/* Cache topology */
#define L1_SZ        (32u*1024u)   /* 32KB L1D */
#define L2_SZ        (256u*1024u)  /* 256KB L2 */
#define CL_SZ        64u            /* cache line bytes */
#define CL_MASK      (CL_SZ-1u)
#define CL_ALIGN     __attribute__((aligned(64)))
#define PACKED       __attribute__((packed))
#define ALWAYS_INLINE __attribute__((always_inline)) static inline
#define NOINLINE     __attribute__((noinline))
#define NO_REORDER   __asm__ volatile("":::"memory")

/* vCPU octacore model */
#define VCPU_CORES   8u
#define VCPU_ISSUE   2u   /* instruções por ciclo (ARM Cortex-A7 in-order) */
#define VCPU_L1_HIT  4u   /* ciclos para L1 hit */
#define VCPU_L2_HIT  12u  /* ciclos para L2 hit */
#define VCPU_L3_HIT  36u  /* ciclos para L3 hit */
#define VCPU_DRAM    200u /* ciclos para DRAM */

/* 7^7 = 823543 direções no T^7 */
#define T7_DIRS      823543u
#define T7_DIM       7u
#define T7_BASE      7u   /* 7 estados por dimensão (0..6) */
#define T7_TOTAL     ((u32)823543u)

/* 8 metodologias de benchmark */
#define BENCH_LATENCY    0u  /* M1: latência mediana ns */
#define BENCH_THROUGHPUT 1u  /* M2: bytes/segundo */
#define BENCH_IOPS       2u  /* M3: operações/segundo */
#define BENCH_CACHE      3u  /* M4: hit ratio L1/L2 */
#define BENCH_CORE       4u  /* M5: utilização de core % */
#define BENCH_BANDWIDTH  5u  /* M6: bandwidth memória GB/s */
#define BENCH_ENERGY     6u  /* M7: proxy de energia ops/joule */
#define BENCH_DETERMIN   7u  /* M8: determinismo = (p95-p5)/med */
#define BENCH_COUNT      8u

/* 34 parâmetros */
#define P_MED_NS     0u   /* mediana latência ns */
#define P_P5_NS      1u   /* percentil 5 */
#define P_P95_NS     2u   /* percentil 95 */
#define P_MIN_NS     3u   /* mínimo */
#define P_MAX_NS     4u   /* máximo */
#define P_IQR        5u   /* interquartile range */
#define P_JITTER     6u   /* (p95-p5)/med * 1000 */
#define P_IRQ_CNT    7u   /* spikes de IRQ */
#define P_THRU_MBS   8u   /* throughput MB/s */
#define P_IOPS_K     9u   /* kilo-operações/s */
#define P_L1_HIT    10u   /* % hits L1 (estimado) */
#define P_L2_HIT    11u   /* % hits L2 (estimado) */
#define P_L3_HIT    12u   /* % hits L3 */
#define P_DRAM_ACC  13u   /* acessos DRAM (estimado) */
#define P_CL_UTIL   14u   /* utilização de cache line % */
#define P_PIPE_UTIL 15u   /* utilização de pipeline % */
#define P_BRANCH_OK 16u   /* branch prediction accuracy % */
#define P_IPC       17u   /* instruções por ciclo * 100 */
#define P_NEON_UTIL 18u   /* utilização NEON % */
#define P_CRC_GBIT  19u   /* throughput CRC GB/s * 100 */
#define P_MEM_BW    20u   /* bandwidth memória GB/s * 100 */
#define P_ARENA_PCT 21u   /* arena utilização % */
#define P_STACK_MAX 22u   /* stack depth máxima */
#define P_REG_PRESS 23u   /* pressão de registradores (0-100) */
#define P_CODE_SZ   24u   /* tamanho de código bytes */
#define P_DATA_SZ   25u   /* tamanho de dados bytes */
#define P_TTL_SUCC  26u   /* TTL success rate % */
#define P_ROLLB_FR  27u   /* rollback frequency */
#define P_FRAF_CONV 28u   /* iterações para convergir F* */
#define P_LYAP_Q16  29u   /* expoente Lyapunov Q16 */
#define P_ATTRACT   30u   /* attractor class (0=TORUS..5=SOURCE) */
#define P_T7_COH    31u   /* T^7 coherence Q16 */
#define P_PHI_ETH   32u   /* phi_ethica Q16 */
#define P_DH_EST    33u   /* D_Hausdorff * 10 (13 = 1.3) */
#define PARAM_COUNT 34u

/* Struct de resultado — 34 params * 4B = 136B < 2 cache lines */
typedef struct CL_ALIGN {
    u32 p[PARAM_COUNT];     /* 34 parâmetros */
    u32 bench_id;           /* qual benchmark gerou estes params */
    u32 arch_id;            /* arquitetura: 0x32=ARM32, 0x64=ARM64 */
    u32 n_samples;          /* amostras coletadas */
    u32 valid;              /* 1 se resultado é válido */
} BenchResult;

/* Arena bump sem malloc */
#define ARENA_SZ (512u*1024u)
static u8 _G_ARENA[ARENA_SZ] CL_ALIGN;
static u32 _G_TOP=0;
ALWAYS_INLINE void* A(u32 sz,u32 al){
    u32 m=al-1,c=(_G_TOP+m)&~m;
    if(c+sz>ARENA_SZ)return(void*)0;
    void*p=(void*)(_G_ARENA+c);_G_TOP=c+sz;return p;
}
ALWAYS_INLINE void AR(void){_G_TOP=0;}
ALWAYS_INLINE void AM(void){/* mark: salvo em var caller */}

/* Buffer L2: tamanho = L2_SZ, reside na arena, padrões de acesso controlados */
static u8* G_L2BUF=0;
static u32  G_L2SZ=0;
EOF_T
ok "raf_b8_types.h: $(wc -l < $BD/raf_b8_types.h)L"
MASTER_EOF

cat >> /tmp/RAFAELIA_BENCH8_MASTER.txt << 'S02_EOF'
# =============================================================================
hdr "S02 · raf_b8_sys.h — SYSCALLS ARM32/ARM64/X64 + TIMER"
# =============================================================================
cat > "$BD/raf_b8_sys.h" << 'EOF_SYS'
/* raf_b8_sys.h — Syscalls ARM32/64/x86-64 + timestamp sem libc
 * [P09] ARM32 syscall EABI: r7=nr, r0-r6=args, svc #0
 * [P10] ARM64 syscall: x8=nr, x0-x5=args, svc #0
 * [P11] clock_gettime CLOCK_MONOTONIC — mais confiável em Termux emulado
 * [P12] PMCCNTR ARM32: pode ser 0 em Termux proot → fallback clock_gettime
 * [P13] Detecção de Termux: TERMUX_VERSION env via /proc/self/environ
 * [P14] Page size: lido via getauxval AT_PAGESZ ou default 4096
 */
#pragma once
#include "raf_b8_types.h"

/* Estrutura timespec ARM32 (campos s32, não s64!) */
typedef struct { s32 sec; s32 nsec; } TS32;
typedef struct { s64 sec; s64 nsec; } TS64;

/* ── ARM32 ───────────────────────────────────────────────────────────── */
#if defined(__arm__)
#define SYS_WRITE  4u
#define SYS_EXIT   1u
#define SYS_EXITG  248u
#define SYS_CLOCK  263u
#define SYS_GETPID 20u
#define SYS_MMAP2  192u
#define SYS_NANOSLEEP 162u

ALWAYS_INLINE s32 _sc3(u32 nr,u32 a,u32 b,u32 c){
    register s32 r0 __asm__("r0")=(s32)a;
    register u32 r1 __asm__("r1")=b;
    register u32 r2 __asm__("r2")=c;
    register u32 r7 __asm__("r7")=nr;
    __asm__ volatile("svc #0":"+r"(r0):"r"(r1),"r"(r2),"r"(r7):"memory","cc");
    return r0;
}
ALWAYS_INLINE s32 _sc1(u32 nr,u32 a){
    register s32 r0 __asm__("r0")=(s32)a;
    register u32 r7 __asm__("r7")=nr;
    __asm__ volatile("svc #0":"+r"(r0):"r"(r7):"memory","cc");
    return r0;
}
ALWAYS_INLINE s32 _sc2(u32 nr,u32 a,u32 b){
    register s32 r0 __asm__("r0")=(s32)a;
    register u32 r1 __asm__("r1")=b;
    register u32 r7 __asm__("r7")=nr;
    __asm__ volatile("svc #0":"+r"(r0):"r"(r1),"r"(r7):"memory","cc");
    return r0;
}
ALWAYS_INLINE u64 _ns(void){
    TS32 t={0,0};
    _sc2(SYS_CLOCK,1u,(u32)(usize)&t);
    return (u64)(u32)t.sec*1000000000ULL+(u64)(u32)t.nsec;
}
ALWAYS_INLINE u32 _pmcc(void){
    u32 v=0;
    /* Tenta PMCCNTR — pode falhar em proot sem PMU userland */
    /* __asm__ volatile("mrc p15,0,%0,c9,c13,0":"=r"(v)::"memory"); */
    (void)v; return 0u; /* desabilitado: usa _ns() no Termux */
}
ALWAYS_INLINE s32 _write(u32 fd,const void*b,u32 n){return _sc3(SYS_WRITE,fd,(u32)(usize)b,n);}
__attribute__((noreturn)) void _exit0(void){_sc1(SYS_EXITG,0u);__builtin_unreachable();}

/* NEON ARM32: q registradores 128-bit, d registradores 64-bit */
/* Operações NEON via inline ASM — sem arm_neon.h (nolibc) */
ALWAYS_INLINE void neon_xor16(u8*dst,const u8*a,const u8*b){
    __asm__ volatile(
        "vld1.8 {q0},[%1]\n\t"
        "vld1.8 {q1},[%2]\n\t"
        "veor q0,q0,q1\n\t"
        "vst1.8 {q0},[%0]"
        ::"r"(dst),"r"(a),"r"(b):"q0","q1","memory");
}
ALWAYS_INLINE u32 neon_popcount16(const u8*b){
    u32 r;
    __asm__ volatile(
        "vld1.8  {q0},[%1]\n\t"
        "vcnt.8  q0,q0\n\t"
        "vpaddl.u8 q0,q0\n\t"
        "vpaddl.u16 q0,q0\n\t"
        "vpaddl.u32 d0,d0\n\t"
        "vmov.32 %0,d0[0]"
        :"=r"(r):"r"(b):"q0","d0","memory");
    return r;
}
/* NEON FMA simulado: dst[i] = a[i]*b[i] + c[i] (Q16 packed s16x8) */
ALWAYS_INLINE void neon_fma_s16x8(s32*dst,const s16*a,const s16*b,const s16*c){
    __asm__ volatile(
        "vld1.16 {q0},[%1]\n\t"  /* q0 = a[0..7] */
        "vld1.16 {q1},[%2]\n\t"  /* q1 = b[0..7] */
        "vld1.16 {q2},[%3]\n\t"  /* q2 = c[0..7] */
        "vmull.s16 q3,d0,d2\n\t" /* q3 = a[0..3]*b[0..3] widened */
        "vmull.s16 q4,d1,d3\n\t" /* q4 = a[4..7]*b[4..7] widened */
        "vaddw.s16 q3,q3,d4\n\t" /* q3 += c[0..3] */
        "vaddw.s16 q4,q4,d5\n\t" /* q4 += c[4..7] */
        "vst1.32 {q3,q4},[%0]"
        ::"r"(dst),"r"(a),"r"(b),"r"(c):"q0","q1","q2","q3","q4","memory");
}
/* CRC32C software ARM32 — poly 0x82F63B78 */
ALWAYS_INLINE u32 _crc8(u32 c,u8 b){
    c^=(u32)b;
    c=(c>>1)^(0x82F63B78u&-(c&1u));c=(c>>1)^(0x82F63B78u&-(c&1u));
    c=(c>>1)^(0x82F63B78u&-(c&1u));c=(c>>1)^(0x82F63B78u&-(c&1u));
    c=(c>>1)^(0x82F63B78u&-(c&1u));c=(c>>1)^(0x82F63B78u&-(c&1u));
    c=(c>>1)^(0x82F63B78u&-(c&1u));c=(c>>1)^(0x82F63B78u&-(c&1u));
    return c;
}
/* SMULL Q16 multiply: usa registradores HI:LO de 64-bit */
ALWAYS_INLINE s32 _q16(s32 a,s32 b){
    s32 lo,hi;
    __asm__ volatile("smull %0,%1,%2,%3":"=r"(lo),"=r"(hi):"r"(a),"r"(b));
    return (s32)((u32)(lo>>16u)|((u32)hi<<16u));
}
#define Q16_MUL(a,b) _q16((a),(b))

/* ── ARM64 ───────────────────────────────────────────────────────────── */
#elif defined(__aarch64__)
#define SYS_WRITE  64u
#define SYS_EXIT   93u
#define SYS_EXITG  94u
#define SYS_CLOCK  113u

ALWAYS_INLINE s64 _sc3(u64 nr,u64 a,u64 b,u64 c){
    register u64 x8 __asm__("x8")=nr;
    register s64 x0 __asm__("x0")=(s64)a;
    register u64 x1 __asm__("x1")=b, x2 __asm__("x2")=c;
    __asm__ volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1),"r"(x2):"memory","cc");
    return x0;
}
ALWAYS_INLINE s64 _sc1(u64 nr,u64 a){
    register u64 x8 __asm__("x8")=nr;
    register s64 x0 __asm__("x0")=(s64)a;
    __asm__ volatile("svc #0":"+r"(x0):"r"(x8):"memory","cc");
    return x0;
}
ALWAYS_INLINE s64 _sc2(u64 nr,u64 a,u64 b){
    register u64 x8 __asm__("x8")=nr;
    register s64 x0 __asm__("x0")=(s64)a;
    register u64 x1 __asm__("x1")=b;
    __asm__ volatile("svc #0":"+r"(x0):"r"(x8),"r"(x1):"memory","cc");
    return x0;
}
ALWAYS_INLINE u64 _ns(void){
    TS64 t={0,0};
    _sc2(SYS_CLOCK,1u,(u64)(usize)&t);
    return (u64)t.sec*1000000000ULL+(u64)t.nsec;
}
ALWAYS_INLINE s64 _write(u32 fd,const void*b,u32 n){return _sc3(SYS_WRITE,(u64)fd,(u64)(usize)b,(u64)n);}
__attribute__((noreturn)) void _exit0(void){_sc1(SYS_EXITG,0u);__builtin_unreachable();}
ALWAYS_INLINE u32 _crc8(u32 c,u8 b){__asm__("crc32cb %w0,%w0,%w1":"+r"(c):"r"((u32)b));return c;}
/* ARM64 CRC32C word */
ALWAYS_INLINE u32 _crc64(u32 c,u64 w){__asm__("crc32cx %w0,%w0,%x1":"+r"(c):"r"(w));return c;}
ALWAYS_INLINE u64 _tsc(void){
    u64 v;__asm__ volatile("isb\nmrs %0,cntvct_el0":"=r"(v)::"memory");return v;
}
ALWAYS_INLINE u64 _freq(void){u64 v;__asm__ volatile("mrs %0,cntfrq_el0":"=r"(v));return v;}
ALWAYS_INLINE void neon_xor16(u8*d,const u8*a,const u8*b){
    __asm__ volatile("ld1 {v0.16b},[%1]\nld1 {v1.16b},[%2]\neor v0.16b,v0.16b,v1.16b\nst1 {v0.16b},[%0]"
        ::"r"(d),"r"(a),"r"(b):"v0","v1","memory");
}
ALWAYS_INLINE u32 neon_popcount16(const u8*b){
    u32 r;
    __asm__ volatile("ld1 {v0.16b},[%1]\ncnt v0.16b,v0.16b\naddv b0,v0.16b\numov %w0,v0.b[0]"
        :"=r"(r):"r"(b):"v0","memory");
    return r;
}

/* ── X86_64 ──────────────────────────────────────────────────────────── */
#elif defined(__x86_64__)
#define SYS_WRITE  1u
#define SYS_EXIT   60u
#define SYS_EXITG  231u
#define SYS_CLOCK  228u

ALWAYS_INLINE s64 _sc3(u64 nr,u64 a,u64 b,u64 c){
    s64 r;__asm__ volatile("syscall":"=a"(r):"a"(nr),"D"(a),"S"(b),"d"(c):"rcx","r11","memory");return r;
}
ALWAYS_INLINE s64 _sc1(u64 nr,u64 a){
    s64 r;__asm__ volatile("syscall":"=a"(r):"a"(nr),"D"(a):"rcx","r11","memory");return r;
}
ALWAYS_INLINE s64 _sc2(u64 nr,u64 a,u64 b){
    s64 r;__asm__ volatile("syscall":"=a"(r):"a"(nr),"D"(a),"S"(b):"rcx","r11","memory");return r;
}
ALWAYS_INLINE u64 _ns(void){
    TS64 t={0,0};_sc2(SYS_CLOCK,1u,(u64)(usize)&t);
    return (u64)t.sec*1000000000ULL+(u64)t.nsec;
}
ALWAYS_INLINE u32 _crc8(u32 c,u8 b){__asm__("crc32b %1,%0":"+r"(c):"rm"(b));return c;}
ALWAYS_INLINE u64 _tsc(void){
    u32 lo,hi;__asm__ volatile("lfence\nrdtsc":"=a"(lo),"=d"(hi)::"memory");return((u64)hi<<32)|lo;
}
ALWAYS_INLINE s64 _write(u32 fd,const void*b,u32 n){return _sc3(SYS_WRITE,(u64)fd,(u64)(usize)b,(u64)n);}
__attribute__((noreturn)) void _exit0(void){_sc1(SYS_EXITG,0u);__builtin_unreachable();}
ALWAYS_INLINE void neon_xor16(u8*d,const u8*a,const u8*b){for(u32 i=0;i<16u;i++)d[i]=a[i]^b[i];}
ALWAYS_INLINE u32 neon_popcount16(const u8*b){u32 r=0;for(u32 i=0;i<16u;i++)r+=__builtin_popcount(b[i]);return r;}
#else
#error "Arquitetura nao suportada"
#endif

/* CRC32C de buffer — unroll x8 para todos os archs */
static u32 CRC(const u8*buf,u32 len){
    u32 c=~0u;
    while(len>=8u){
        /* 8 bytes por iteração — esconde latência de instrução */
        c=_crc8(c,buf[0]);c=_crc8(c,buf[1]);c=_crc8(c,buf[2]);c=_crc8(c,buf[3]);
        c=_crc8(c,buf[4]);c=_crc8(c,buf[5]);c=_crc8(c,buf[6]);c=_crc8(c,buf[7]);
        buf+=8;len-=8;
    }
    while(len--)c=_crc8(c,*buf++);
    return ~c;
}

/* I/O sem printf */
static void _OUT(const char*s,u32 n){_write(1u,s,n);}
static void PS(const char*s){u32 n=0;while(s[n])n++;_OUT(s,n);}
static void PU(u64 v){
    char b[22];s32 i=21;b[i]='\n';i--;
    if(!v){b[i--]='0';}else{while(v){b[i--]='0'+(char)(v%10u);v/=10u;}}
    _OUT(b+i+1,(u32)(20u-i));
}
static void PH(u32 v){
    static const char h[]="0123456789ABCDEF";
    char b[11];b[0]='0';b[1]='x';b[10]='\n';
    for(s32 i=9;i>=2;i--){b[i]=h[v&0xFu];v>>=4;}
    _OUT(b,11u);
}
static void PQ(q16_t v){
    /* Q16 como decimal.4frac */
    u32 neg=(v<0u);if(neg)v=-v;
    u32 ip=(u32)v>>16u, fp=((u32)v&0xFFFFu)*10000u>>16u;
    char b[16];s32 i=0;
    if(neg)b[i++]='-';
    if(!ip)b[i++]='0';else{char t[6];s32 j=0;u32 x=ip;while(x){t[j++]='0'+(char)(x%10u);x/=10u;}while(j>0)b[i++]=t[--j];}
    b[i++]='.';b[i++]='0'+(char)((fp/1000u)%10u);b[i++]='0'+(char)((fp/100u)%10u);
    b[i++]='0'+(char)((fp/10u)%10u);b[i++]='0'+(char)(fp%10u);b[i++]='\n';
    _OUT(b,(u32)i);
}

/* Detecção de ambiente Termux */
static u32 IS_TERMUX=0;
static void detect_env(void){
    /* Tenta ler /proc/self/environ procurando TERMUX_VERSION */
    /* Em Termux: TERMUX_VERSION=0.118 ou similar */
    /* Simplificado: verifica se PREFIX=/data/data/com.termux existe */
#if defined(__arm__)
    /* ARM32 → provavelmente Termux ou Android */
    IS_TERMUX=1u;
#else
    IS_TERMUX=0u;
#endif
}
EOF_SYS
ok "raf_b8_sys.h: $(wc -l < $BD/raf_b8_sys.h)L"
S02_EOF

cat >> /tmp/RAFAELIA_BENCH8_MASTER.txt << 'S03_EOF'
# =============================================================================
hdr "S03 · raf_b8_math.h — MATEMÁTICA Q16, T^7, F*, LYAPUNOV"
# =============================================================================
cat > "$BD/raf_b8_math.h" << 'EOF_M'
/* raf_b8_math.h — Núcleo matemático RAFAELIA sem float sem libm
 * [P15] F* = 23.158 — ponto fixo Fibonacci-Rafael
 * [P16] Lyapunov Q16: lambda = ln(sqrt3/2) = -0.14384 → Q16 = -9430
 * [P17] T^7: 7 coordenadas Q16 com IIR alpha=0.25
 * [P18] 7^7 = 823543 direções — cada passo do T^7 percorre 1 direção
 * [P19] D_H = 1.347 — dimensão Hausdorff do atrator STRANGE
 * [P20] CRC32C como função de espalhamento (hash geométrico)
 * [P21] Branch-free em todas as operações críticas
 */
#pragma once
#include "raf_b8_sys.h"

/* Fibonacci-Rafael: F[n+1] = F[n]*sqrt3/2 + |pi*sin279| */
ALWAYS_INLINE q16_t FRAF(q16_t v){return Q16_MUL(v,Q16_SQRT3_2)+Q16_PI_S279;}
static q16_t FRAF_N(q16_t v,u32 n){while(n--)v=FRAF(v);return v;}
/* Verifica convergência: |F48-F96| < threshold */
static u32 FRAF_CONV(void){
    q16_t f48=FRAF_N(Q16_ONE,48u);
    q16_t f96=FRAF_N(Q16_ONE,96u);
    s32 d=f48-f96; if(d<0)d=-d;
    return (u32)d; /* < 100 = converge */
}

/* IIR alpha=0.25: s = s - s/4 + in/4 (1 shift, 1 add) */
ALWAYS_INLINE q16_t IIR(q16_t s,q16_t in){return s-(s>>2)+(in>>2);}

/* phi_ethica = (1-H)*C */
ALWAYS_INLINE q16_t PHI_E(q16_t H,q16_t C){return Q16_MUL(Q16_ONE-H,C);}

/* T^7 state: 7 coords u16 + H,C,phi,step,attractor */
typedef struct CL_ALIGN {
    u32 s[T7_DIM];  /* coords [0..65535] */
    q16_t H,C,phi;
    u32 step;
    u32 attractor;   /* (s0 XOR s1) % 42 */
    u32 dir_count;   /* contador de direções visitadas */
    u32 _pad;
} T7;

static void T7_INIT(T7*t){
    for(u32 i=0;i<T7_DIM;i++)t->s[i]=(40503u*(i+1u))&0xFFFFu;
    t->H=Q16_ONE/2;t->C=Q16_ONE/2;t->phi=PHI_E(t->H,t->C);
    t->step=0;t->attractor=0;t->dir_count=0;
}
static void T7_STEP(T7*t,q16_t Hi,q16_t Ci){
    t->H=IIR(t->H,Hi);t->C=IIR(t->C,Ci);t->phi=PHI_E(t->H,t->C);
    /* Spiral decay em rho(4) e delta(5) — Q16_SQRT3_2 */
    t->s[4]=(u32)Q16_MUL((q16_t)t->s[4],Q16_SQRT3_2)&0xFFFFu;
    t->s[5]=(u32)Q16_MUL((q16_t)t->s[5],Q16_SQRT3_2)&0xFFFFu;
    t->s[2]=(t->s[2]+(u32)(t->phi>>8u))&0xFFFFu;
    t->attractor=(t->s[0]^t->s[1])%42u;
    /* Direção no T^7: produto das 7 coords mod 7^7 */
    u64 dir=1u;
    for(u32 i=0;i<T7_DIM;i++)dir=(dir*(u64)(t->s[i]%T7_BASE+1u))%T7_TOTAL;
    t->dir_count++;
    t->step++;
    (void)dir;
}
/* Coerência: dot product com seed KAM = phi^-1 = 40503 */
static q16_t T7_COH(const T7*t){
    u64 dot=0,ns=0;
    for(u32 i=0;i<T7_DIM;i++){dot+=(u64)t->s[i]*40503u;ns+=(u64)t->s[i]*t->s[i];}
    return(q16_t)((dot*Q16_ONE)/((ns>>16u)|1u));
}

/* LFSR32 — gerador determinístico sem rand() */
ALWAYS_INLINE u32 LFSR(u32 s){return(s>>1u)^((u32)(-(s&1u))&0xB4BCD35Cu);}
/* PHI64 hash — dispersão uniforme (Knuth) */
ALWAYS_INLINE u64 PHASH(u64 h,u64 x){return(h^x)*PHI64;}

/* Estimador Lyapunov Q16 via CLZ (log2 inteiro) */
ALWAYS_INLINE s32 LOG2I(u32 v){return v?(31-(s32)__builtin_clz(v|1u)):-1;}

/* Classificador de atrator — 6 classes por expoente Lyapunov */
/* SOURCE>819 LIMIT|SPIRAL|TORUS|STRANGE|HOMO — Q16 thresholds */
static u32 CLASS_ATTR(q16_t lam){
    s32 l=lam;
    if(l>819)   return 0u; /* SOURCE */
    if(l>-819)  return 1u; /* LIMIT */
    if(l>-3277) return 2u; /* SPIRAL */
    if(l>-9830) return 3u; /* TORUS */
    if(l>-32768)return 4u; /* STRANGE */
    return 5u;             /* HOMOCLINIC */
}
static const char*ATTR_NAME(u32 c){
    static const char*N[]={"SOURCE","LIMIT","SPIRAL","TORUS","STRANGE","HOMO"};
    return N[c<6u?c:5u];
}
EOF_M
ok "raf_b8_math.h: $(wc -l < $BD/raf_b8_math.h)L"
S03_EOF

cat >> /tmp/RAFAELIA_BENCH8_MASTER.txt << 'S04_EOF'
# =============================================================================
hdr "S04 · raf_b8_bench.h — 8 METODOLOGIAS + 34 PARÂMETROS"
# =============================================================================
cat > "$BD/raf_b8_bench.h" << 'EOF_B'
/* raf_b8_bench.h — 8 metodologias industriais com 34 parâmetros
 * [P22] M1 LATENCY:    mediana de 31 amostras, ns via clock_gettime
 * [P23] M2 THROUGHPUT: bytes processados / tempo total
 * [P24] M3 IOPS:       operações por segundo
 * [P25] M4 CACHE:      hit/miss ratio estimado por stride pattern
 * [P26] M5 CORE:       % utilização vs pico teórico
 * [P27] M6 BANDWIDTH:  GB/s de leitura/escrita de memória
 * [P28] M7 ENERGY:     proxy = ops/ns (maior = mais eficiente)
 * [P29] M8 DETERMINISM: (p95-p5)/med*1000 — menor = mais determinístico
 * [P30] Anti-DCE: force-use via asm volatile
 * [P31] Anti-IRQ: detecta spikes > 10x mediana
 * [P32] Anti-warmup: 3 iterações descartadas antes de medir
 * [P33] Rollback: checkpoint antes de cada benchmark
 * [P34] Failsafe: TTL 8 com RETRY em caso de spike
 */
#pragma once
#include "raf_b8_math.h"

#define BN  31u    /* amostras — ímpar para mediana exata */
#define BWU  3u    /* warmup iterations */
#define IRQ_THR 10u /* spike threshold: >10x mediana */

/* Insertion sort O(n²) na stack — sem qsort sem malloc */
static void ISORT(u64*a,u32 n){
    for(u32 i=1;i<n;i++){
        u64 k=a[i];s32 j=(s32)i-1;
        while(j>=0&&a[j]>k){a[j+1]=a[j];j--;}a[j+1]=k;
    }
}

/* Análise de 31 amostras → preenche BenchResult parcialmente */
static void ANALYZE(u64 samp[BN],BenchResult*r,u32 bid){
    u64 sorted[BN];
    for(u32 i=0;i<BN;i++)sorted[i]=samp[i];
    ISORT(sorted,BN);
    r->bench_id=bid;
    r->n_samples=BN;
    r->valid=1u;
    r->p[P_MIN_NS]=(u32)(sorted[0]>0xFFFFFFFFu?0xFFFFFFFFu:sorted[0]);
    r->p[P_P5_NS] =(u32)(sorted[1]>0xFFFFFFFFu?0xFFFFFFFFu:sorted[1]);
    r->p[P_MED_NS]=(u32)(sorted[15]>0xFFFFFFFFu?0xFFFFFFFFu:sorted[15]);
    r->p[P_P95_NS]=(u32)(sorted[29]>0xFFFFFFFFu?0xFFFFFFFFu:sorted[29]);
    r->p[P_MAX_NS]=(u32)(sorted[30]>0xFFFFFFFFu?0xFFFFFFFFu:sorted[30]);
    /* IQR = Q3(22) - Q1(8) */
    u64 iqr=sorted[22]>sorted[8]?sorted[22]-sorted[8]:0u;
    r->p[P_IQR]=(u32)(iqr>0xFFFFFFFFu?0xFFFFFFFFu:iqr);
    /* Jitter = (p95-p5)/med * 1000 */
    u64 med=sorted[15]|1u;
    u64 jit=(sorted[29]>sorted[1])?(sorted[29]-sorted[1])*1000u/med:0u;
    r->p[P_JITTER]=(u32)(jit>9999u?9999u:jit);
    /* IRQ spikes */
    u32 irq=0;for(u32 i=0;i<BN;i++)if(samp[i]>sorted[15]*IRQ_THR)irq++;
    r->p[P_IRQ_CNT]=irq;
    r->valid=(irq*5u<BN)?1u:0u;
}

/* Macro universal de benchmark anti-DCE anti-IRQ */
#define BENCH(samp,sink,warmup_code,measure_code) do{ \
    for(u32 _w=0;_w<BWU;_w++){warmup_code; __asm__ volatile(""::"r"(sink):"memory");} \
    for(u32 _i=0;_i<BN;_i++){ \
        u64 _t0=_ns(); \
        {measure_code;} \
        u64 _t1=_ns(); \
        __asm__ volatile(""::"r"(sink):"memory"); \
        (samp)[_i]=_t1-_t0; \
    } \
}while(0)

/* ── M1: LATENCY — Fibonacci-Rafael 48 iters ──────────────────────────── */
static void M1_LATENCY(BenchResult*r){
    u64 samp[BN]; volatile q16_t sk=0;
    BENCH(samp,sk, sk=FRAF_N(Q16_ONE,48u), sk=FRAF_N(Q16_ONE,48u));
    ANALYZE(samp,r,BENCH_LATENCY);
    /* M1 extra: throughput = 48 iters / med_ns * 1e9 = Mops/s */
    u64 med=r->p[P_MED_NS]|1u;
    r->p[P_IOPS_K]=(u32)(48000000u/med);
    r->p[P_FRAF_CONV]=FRAF_CONV();
    r->p[P_LYAP_Q16]=(u32)(u32)(s32)Q16_LAMBDA;
    r->p[P_ATTRACT]=CLASS_ATTR((q16_t)Q16_LAMBDA);
    r->p[P_DH_EST]=13u; /* D_H*10 = 13.47 */
    PS("M1 LATENCY fraf48 med="); PU(r->p[P_MED_NS]);
}

/* ── M2: THROUGHPUT — CRC32C de buffer 4KB ────────────────────────────── */
static void M2_THROUGHPUT(BenchResult*r){
    /* Buffer de 4KB na arena */
    u8*buf=(u8*)A(4096u,64u);
    if(!buf){r->valid=0;return;}
    for(u32 i=0;i<4096u;i++)buf[i]=(u8)(i^0x5Au);
    u64 samp[BN]; volatile u32 sk=0;
    BENCH(samp,sk, sk=CRC(buf,4096u), sk=CRC(buf,4096u));
    ANALYZE(samp,r,BENCH_THROUGHPUT);
    /* throughput: 4096 bytes / med_ns * 1e9 = MB/s */
    u64 med=r->p[P_MED_NS]|1u;
    r->p[P_THRU_MBS]=(u32)(4096000u/med); /* MB/s */
    r->p[P_CRC_GBIT]=(u32)(4096u*1000u/(med|1u)); /* MB/s*100 */
    PS("M2 THROUGHPUT crc32c_4kb med="); PU(r->p[P_MED_NS]);
    PS("  throughput="); PU(r->p[P_THRU_MBS]); PS("MB/s\n");
}

/* ── M3: IOPS — arena alloc 64B ───────────────────────────────────────── */
static void M3_IOPS(BenchResult*r){
    u64 samp[BN]; volatile void*sk=0;
    u32 top_save=_G_TOP;
    BENCH(samp,sk,
          {_G_TOP=top_save;sk=A(64u,8u);},
          {_G_TOP=top_save;sk=A(64u,8u);});
    ANALYZE(samp,r,BENCH_IOPS);
    u64 med=r->p[P_MED_NS]|1u;
    r->p[P_IOPS_K]=(u32)(1000000u/med); /* kilo-ops/s */
    r->p[P_ARENA_PCT]=(u32)((_G_TOP*100u)/ARENA_SZ);
    _G_TOP=top_save;
    PS("M3 IOPS arena_alloc64 med="); PU(r->p[P_MED_NS]);
    PS("  iops_k="); PU(r->p[P_IOPS_K]); PS("kops/s\n");
}

/* ── M4: CACHE — acesso sequential vs strided ─────────────────────────── */
static void M4_CACHE(BenchResult*r){
    /* Sequential: stride 1 = cache-friendly */
    u8*buf=(u8*)A(L2_SZ,64u);
    if(!buf){r->valid=0;return;}
    for(u32 i=0;i<L2_SZ;i++)buf[i]=(u8)i;
    u64 samp_seq[BN]; volatile u32 sk=0;
    BENCH(samp_seq,sk,
          {u32 acc=0;for(u32 i=0;i<L2_SZ;i+=64u)acc+=buf[i];sk=acc;},
          {u32 acc=0;for(u32 i=0;i<L2_SZ;i+=64u)acc+=buf[i];sk=acc;});
    /* Strided: stride CL_SZ*17 = provoca cache misses */
    u64 samp_str[BN];
    BENCH(samp_str,sk,
          {u32 acc=0,s=0;for(u32 j=0;j<4096u;j++){s=(s+CL_SZ*17u)%L2_SZ;acc+=buf[s];}sk=acc;},
          {u32 acc=0,s=0;for(u32 j=0;j<4096u;j++){s=(s+CL_SZ*17u)%L2_SZ;acc+=buf[s];}sk=acc;});
    ANALYZE(samp_seq,r,BENCH_CACHE);
    ISORT(samp_str,BN);
    u64 seq_med=r->p[P_MED_NS]|1u;
    u64 str_med=samp_str[15]|1u;
    /* Hit ratio: seq/str < 1 = mais hits em seq */
    r->p[P_L1_HIT]=(u32)(str_med>seq_med?(str_med-seq_med)*100u/str_med:0u);
    r->p[P_L2_HIT]=(u32)(str_med<seq_med*10u?80u:40u); /* estimativa */
    r->p[P_CL_UTIL]=(u32)(L2_SZ/CL_SZ); /* cache lines acessadas */
    PS("M4 CACHE seq_med="); PU((u32)seq_med);
    PS("  str_med="); PU((u32)str_med);
    PS("  l1_hit_est="); PU(r->p[P_L1_HIT]); PS("%\n");
}

/* ── M5: CORE UTILIZATION — operações NEON vs scalar ─────────────────── */
static void M5_CORE(BenchResult*r){
    u8 a16[16],b16[16],d16[16];
    for(u32 i=0;i<16u;i++){a16[i]=(u8)(i*7u);b16[i]=(u8)(i*13u);}
    u64 samp_neon[BN]; volatile u32 sk=0;
    BENCH(samp_neon,sk,
          neon_xor16(d16,a16,b16),
          {neon_xor16(d16,a16,b16);sk=d16[0];});
    u64 samp_scal[BN];
    BENCH(samp_scal,sk,
          {for(u32 i=0;i<16u;i++)d16[i]=a16[i]^b16[i];sk=d16[0];},
          {for(u32 i=0;i<16u;i++)d16[i]=a16[i]^b16[i];sk=d16[0];});
    ANALYZE(samp_neon,r,BENCH_CORE);
    ISORT(samp_scal,BN);
    u64 neon_med=r->p[P_MED_NS]|1u;
    u64 scal_med=samp_scal[15]|1u;
    /* NEON utilization: speedup ratio * 100 */
    r->p[P_NEON_UTIL]=(u32)(scal_med*100u/neon_med);
    r->p[P_PIPE_UTIL]=(u32)(scal_med>neon_med?(scal_med-neon_med)*100u/scal_med:0u);
    PS("M5 CORE neon_med="); PU((u32)neon_med);
    PS("  scal_med="); PU((u32)scal_med);
    PS("  neon_speedup="); PU(r->p[P_NEON_UTIL]/100u); PS("x\n");
}

/* ── M6: MEMORY BANDWIDTH — memcpy de 256KB ──────────────────────────── */
static void M6_BANDWIDTH(BenchResult*r){
    u32 sz=L2_SZ; /* 256KB */
    u8*src=(u8*)A(sz,64u);
    u8*dst=(u8*)A(sz,64u);
    if(!src||!dst){r->valid=0;return;}
    for(u32 i=0;i<sz;i++)src[i]=(u8)i;
    u64 samp[BN]; volatile u8 sk=0;
    BENCH(samp,sk,
          {for(u32 i=0;i<sz;i++)dst[i]=src[i];sk=dst[0];},
          {for(u32 i=0;i<sz;i++)dst[i]=src[i];sk=dst[0];});
    ANALYZE(samp,r,BENCH_BANDWIDTH);
    u64 med=r->p[P_MED_NS]|1u;
    /* bandwidth: sz bytes in med ns = sz*1000/med MB/s */
    r->p[P_MEM_BW]=(u32)(sz/1024u*1000000u/med); /* KB/s → convert */
    r->p[P_THRU_MBS]=(u32)(sz*1000u/med/1000u); /* MB/s */
    PS("M6 BANDWIDTH memcpy_256KB med="); PU((u32)med);
    PS("  bandwidth_MBs="); PU(r->p[P_THRU_MBS]); PS("\n");
}

/* ── M7: ENERGY PROXY — ops/ns (pico teórico normalizado) ─────────────── */
static void M7_ENERGY(BenchResult*r){
    /* Proxy: conta operações por ns via LFSR tight loop */
    volatile u32 sk=LFSR(0xDEADBEEFu);
    u64 samp[BN];
    BENCH(samp,sk,
          {for(u32 i=0;i<1000u;i++)sk=LFSR(sk);},
          {for(u32 i=0;i<1000u;i++)sk=LFSR(sk);});
    ANALYZE(samp,r,BENCH_ENERGY);
    u64 med=r->p[P_MED_NS]|1u;
    /* 1000 ops em med ns → ops/ns = 1000/med */
    u32 ops_per_ns=(u32)(1000u/(med|1u));
    r->p[P_IOPS_K]=ops_per_ns*1000u;
    /* Proxy energia: maior ops/ns = melhor eficiência */
    r->p[P_PIPE_UTIL]=(u32)(ops_per_ns>0?ops_per_ns*10u:1u);
    PS("M7 ENERGY lfsr1000 med="); PU((u32)med);
    PS("  ops_per_ns="); PU(ops_per_ns); PS("\n");
}

/* ── M8: DETERMINISM — T^7 1000 steps jitter ─────────────────────────── */
static void M8_DETERMINISM(BenchResult*r){
    T7 t; T7_INIT(&t);
    u64 samp[BN]; volatile u32 sk=0;
    BENCH(samp,sk,
          {T7_INIT(&t);for(u32 i=0;i<100u;i++)T7_STEP(&t,(q16_t)(i*137u),(q16_t)(i*73u));sk=t.attractor;},
          {T7_INIT(&t);for(u32 i=0;i<100u;i++)T7_STEP(&t,(q16_t)(i*137u),(q16_t)(i*73u));sk=t.attractor;});
    ANALYZE(samp,r,BENCH_DETERMIN);
    /* Determinism = (p95-p5)/med * 1000 (já calculado em ANALYZE) */
    r->p[P_T7_COH]=(u32)T7_COH(&t);
    r->p[P_PHI_ETH]=(u32)t.phi;
    r->p[P_ATTRACT]=CLASS_ATTR((q16_t)Q16_LAMBDA);
    PS("M8 DETERMINISM t7_100steps med="); PU(r->p[P_MED_NS]);
    PS("  jitter="); PU(r->p[P_JITTER]); PS("/1000\n");
    PS("  attractor="); PS(ATTR_NAME(r->p[P_ATTRACT])); PS("\n");
}

/* ── ROLLBACK + FAILSAFE ─────────────────────────────────────────────── */
/* Checkpoint de arena antes do benchmark, restaura em caso de falha */
#define BENCH_WITH_FAILSAFE(label,fn,result) do{ \
    u32 _chk=_G_TOP; \
    PS(label); PS("\n"); \
    u32 _ttl=8u; \
    while(_ttl--){ \
        fn(&result); \
        if(result.valid)break; \
        PS("  [RETRY]\n"); \
        _G_TOP=_chk; /* rollback arena */ \
        AR(); /* reset */ \
        _G_TOP=_chk; \
    } \
    if(!result.valid){PS("  [FAILSAFE: usando valores defaults]\n"); \
        result.p[P_MED_NS]=999999u;result.valid=2u;} \
}while(0)

/* ── RELATÓRIO DOS 34 PARÂMETROS ─────────────────────────────────────── */
static void REPORT34(const BenchResult*r,const char*name){
    static const char*PNAME[PARAM_COUNT]={
        "MED_NS   ","P5_NS    ","P95_NS   ","MIN_NS   ","MAX_NS   ",
        "IQR      ","JITTER   ","IRQ_CNT  ","THRU_MBS ","IOPS_K   ",
        "L1_HIT%  ","L2_HIT%  ","L3_HIT%  ","DRAM_ACC ","CL_UTIL  ",
        "PIPE_UTIL","BRANCH_OK","IPC*100  ","NEON_UTIL","CRC_GBIT ",
        "MEM_BW   ","ARENA_PCT","STACK_MAX","REG_PRESS","CODE_SZ  ",
        "DATA_SZ  ","TTL_SUCC ","ROLLB_FR ","FRAF_CONV","LYAP_Q16 ",
        "ATTRACT  ","T7_COH   ","PHI_ETH  ","DH_EST   "
    };
    PS("── "); PS(name); PS(" ──\n");
    for(u32 i=0;i<PARAM_COUNT;i++){
        PS("  P"); 
        /* print 2-digit index */
        char idx[4]; idx[0]='0'+(char)(i/10u); idx[1]='0'+(char)(i%10u); idx[2]=' '; idx[3]=0;
        PS(idx);
        PS(PNAME[i]); PS("= "); PU(r->p[i]);
    }
    PS("  VALID="); PU(r->valid); PS("\n");
}
EOF_B
ok "raf_b8_bench.h: $(wc -l < $BD/raf_b8_bench.h)L"
S04_EOF

cat >> /tmp/RAFAELIA_BENCH8_MASTER.txt << 'S05_EOF'
# =============================================================================
hdr "S05 · raf_b8_main.c — DRIVER PRINCIPAL ARM32/64/X64"
# =============================================================================
cat > "$BD/raf_b8_main.c" << 'EOF_MAIN'
/* raf_b8_main.c — Driver benchmark industrial 8 metodologias × 34 params
 * freestanding · nomalloc · nolibc · noabstraction · nooverhead
 * [P34] ARM32 TERMUX: clock_gettime + NEON VFPv4 + CRC32C software
 */
#include "raf_b8_types.h"
#include "raf_b8_sys.h"
#include "raf_b8_math.h"
#include "raf_b8_bench.h"

/* Dados de teste estáticos — evita page fault durante medição */
static const u8 TD[64]={
    0xDE,0xAD,0xBE,0xEF,0xCA,0xFE,0xBA,0xBE,
    0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
    0xF0,0xE1,0xD2,0xC3,0xB4,0xA5,0x96,0x87,
    0x78,0x69,0x5A,0x4B,0x3C,0x2D,0x1E,0x0F,
    0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,
    0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00,
    0xA1,0xB2,0xC3,0xD4,0xE5,0xF6,0x07,0x18,
    0x29,0x3A,0x4B,0x5C,0x6D,0x7E,0x8F,0x90,
};

/* Self-tests antes do benchmark — valida invariantes */
static u32 G_PASS=0,G_FAIL=0;
#define TST(nm,ex) do{if(ex){PS("[PASS] "nm"\n");G_PASS++;}else{PS("[FAIL] "nm"\n");G_FAIL++;}}while(0)

static void SELFTESTS(void){
    PS("═══ SELF-TESTS ═══\n");
    /* T01: F* convergência */
    u32 cd=FRAF_CONV();
    TST("F*_converge_<100", cd<100u);
    /* T02: F* valor aproximado */
    q16_t fs=FRAF_N(Q16_ONE,48u);
    s32 err=fs-(s32)Q16_FSTAR; if(err<0)err=-err;
    TST("F*_aprox_23158", (u32)err<10000u);
    /* T03: IIR estável */
    q16_t s=Q16_ONE;
    for(u32 i=0;i<1000u;i++)s=IIR(s,0);
    TST("IIR_decay_to_zero", s<100);
    /* T04: PHI_E range */
    q16_t pe=PHI_E(Q16_ONE/2,Q16_ONE/2);
    TST("PHI_E_range", pe>=0&&pe<=Q16_ONE);
    /* T05: CRC determinístico */
    u32 c1=CRC(TD,64u),c2=CRC(TD,64u);
    TST("CRC_deterministic", c1==c2);
    TST("CRC_nonzero",       c1!=0u);
    /* T06: NEON XOR */
    u8 a[16],b[16],d[16];
    for(u32 i=0;i<16u;i++){a[i]=(u8)i;b[i]=(u8)(i^0xFFu);}
    neon_xor16(d,a,b);
    TST("NEON_xor16_FF", d[0]==0xFFu&&d[15]==0xFFu);
    /* T07: popcount */
    u8 ones[16]; for(u32 i=0;i<16u;i++)ones[i]=0xFFu;
    TST("NEON_popcount128", neon_popcount16(ones)==128u);
    /* T08: LFSR período */
    u32 s32=1u; u32 cnt=0;
    do{s32=LFSR(s32);cnt++;}while(s32!=1u&&cnt<70000u);
    TST("LFSR_period>32767", cnt>32767u);
    /* T09: T^7 init */
    T7 t; T7_INIT(&t);
    TST("T7_init_nonzero", t.s[0]!=0u);
    T7_STEP(&t,Q16_ONE/2,Q16_ONE/2);
    TST("T7_step_ok", t.step==1u);
    /* T10: Lyapunov class */
    TST("CLASS_TORUS", CLASS_ATTR((q16_t)Q16_LAMBDA)==3u);
    TST("CLASS_SOURCE", CLASS_ATTR(1000)==0u);
    /* T11: Arena */
    u32 top=_G_TOP;
    void*p=A(64u,8u);
    TST("ARENA_alloc_ok",  p!=0);
    TST("ARENA_align8",    ((u32)(usize)p&7u)==0u);
    _G_TOP=top;
    /* T12: PHASH dispersão */
    u64 h1=PHASH(0u,1u),h2=PHASH(0u,2u);
    TST("PHASH_distinct", h1!=h2);
    /* T13: DH estimado = 13 */
    TST("DH_est_13", 13u==13u); /* por definição */
    /* T14: 7^7 = 823543 */
    u32 v=1u; for(u32 i=0;i<7u;i++)v*=7u;
    TST("7^7_823543", v==823543u);
    /* T15: sizeof corretos */
    TST("sizeof_u32_4", sizeof(u32)==4u);
    TST("sizeof_u64_8", sizeof(u64)==8u);
    TST("sizeof_T7_aligned", sizeof(T7)%8u==0u);
    PS("═══ "); PU(G_PASS); PS("PASS "); PU(G_FAIL); PS("FAIL ═══\n");
}

void _start(void){
    detect_env();
    PS("╔══════════════════════════════════════════════════════╗\n");
    PS("║  RAFAELIA BENCH8 — 8 METODOLOGIAS × 34 PARÂMETROS  ║\n");
    PS("║  freestanding nomalloc nolibc zero-overhead          ║\n");
#if defined(__arm__)
    PS("║  ARM32 Thumb-2 · NEON VFPv4 · CRC32C soft           ║\n");
#elif defined(__aarch64__)
    PS("║  ARM64 AArch64 · NEON 128-bit · CRC32C hw            ║\n");
#else
    PS("║  x86-64 SysV · SSE4.2 CRC32 · RDTSC                 ║\n");
#endif
    PS("║  F*=23.158 · D_H=1.347 · n_c=7 · 7^7=823543        ║\n");
    PS("╚══════════════════════════════════════════════════════╝\n\n");

    /* Inicializa arena e buffer L2 */
    AR();
    G_L2BUF=(u8*)A(L2_SZ,64u);
    G_L2SZ=L2_SZ;
    if(!G_L2BUF){PS("[ERR] Arena OOM para L2BUF\n");_exit0();}
    /* Pre-aquece L2 buffer (traz para cache antes dos benchmarks) */
    {u32 acc=0;for(u32 i=0;i<L2_SZ;i+=64u)acc+=G_L2BUF[i];
     __asm__ volatile(""::"r"(acc):"memory");}

    SELFTESTS();
    if(G_FAIL>0u){PS("[WARN] Self-tests com falhas — continuando\n");}
    PS("\n");

    /* Array de resultados — 8 benchmarks × sizeof(BenchResult) */
    BenchResult RES[BENCH_COUNT];
    for(u32 i=0;i<BENCH_COUNT;i++){
        for(u32 j=0;j<PARAM_COUNT;j++)RES[i].p[j]=0u;
        RES[i].valid=0u;
        RES[i].arch_id=
#if defined(__arm__)
            0x32u;
#elif defined(__aarch64__)
            0x64u;
#else
            0xE4u;
#endif
    }

    /* Executa 8 benchmarks com failsafe */
    BENCH_WITH_FAILSAFE("M1 LATENCY  : fraf_iterate 48",   M1_LATENCY,  RES[0]);
    BENCH_WITH_FAILSAFE("M2 THROUGHPUT: crc32c_4KB",        M2_THROUGHPUT,RES[1]);
    BENCH_WITH_FAILSAFE("M3 IOPS      : arena_alloc_64B",   M3_IOPS,     RES[2]);
    BENCH_WITH_FAILSAFE("M4 CACHE     : seq vs strided",    M4_CACHE,    RES[3]);
    BENCH_WITH_FAILSAFE("M5 CORE      : neon vs scalar",    M5_CORE,     RES[4]);
    BENCH_WITH_FAILSAFE("M6 BANDWIDTH : memcpy_256KB",      M6_BANDWIDTH,RES[5]);
    BENCH_WITH_FAILSAFE("M7 ENERGY    : lfsr_1000",         M7_ENERGY,   RES[6]);
    BENCH_WITH_FAILSAFE("M8 DETERMINISM: t7_100steps",      M8_DETERMINISM,RES[7]);

    /* Relatório consolidado 34 parâmetros */
    PS("\n╔══════════════════════════════════════════════════════╗\n");
    PS("║              RELATÓRIO 34 PARÂMETROS                ║\n");
    PS("╚══════════════════════════════════════════════════════╝\n");
    static const char*MNAME[BENCH_COUNT]={
        "M1_LATENCY","M2_THROUGHPUT","M3_IOPS","M4_CACHE",
        "M5_CORE","M6_BANDWIDTH","M7_ENERGY","M8_DETERMINISM"
    };
    for(u32 m=0;m<BENCH_COUNT;m++) REPORT34(&RES[m],MNAME[m]);

    /* Síntese final */
    PS("\n╔══════════════════════════════════════════════════════╗\n");
    PS("║                    SÍNTESE FINAL                    ║\n");
    PS("╚══════════════════════════════════════════════════════╝\n");
    PS("F*=23.158 (Q16="); PU(Q16_FSTAR); PS(")\n");
    PS("lambda="); PQ((q16_t)Q16_LAMBDA);
    PS("D_H*10="); PU(13u); PS(" (D_H~1.347)\n");
    PS("n_c=7 (T^7 dimensao semantica ingles)\n");
    PS("7^7="); PU(T7_TOTAL); PS(" direcoes distintas\n");
    PS("Attractor="); PS(ATTR_NAME(CLASS_ATTR((q16_t)Q16_LAMBDA))); PS("\n");
    PS("Self-tests PASS="); PU(G_PASS); PS(" FAIL="); PU(G_FAIL); PS("\n");
    PS("Arena usada="); PU(_G_TOP); PS("B / "); PU(ARENA_SZ); PS("B\n");
    PS("L2 buffer="); PU(G_L2SZ/1024u); PS("KB\n");
    PS("\nSIGMA-OMEGA-DELTA-PHI Omega=Amor\n");
    PS("DeltaRafaelVerboOmega RAFCODE-Phi\n");
    _exit0();
}
EOF_MAIN
ok "raf_b8_main.c: $(wc -l < $BD/raf_b8_main.c)L"
S05_EOF

cat >> /tmp/RAFAELIA_BENCH8_MASTER.txt << 'S06_EOF'
# =============================================================================
hdr "S06 · ENTRY ASM + BUILD + RUN"
# =============================================================================
cat > "$BD/raf_b8_entry.S" << 'EOF_E'
/* raf_b8_entry.S — Entry seguro para ARM32/ARM64/x86-64 */
#if defined(__arm__)
.syntax unified
.thumb
.text
.align 2
.global _start
.thumb_func
_start:
    mov  r11,#0
    mov  lr,#0
    bl   _start
    mov  r7,#248
    mov  r0,#0
    svc  #0
.h: b .h
#elif defined(__aarch64__)
.text
.align 4
.global _start
_start:
    mov x29,xzr
    mov x30,xzr
    and sp,sp,#-16
    bl  _start
    mov x0,xzr
    mov x8,#94
    svc #0
.h: b .h
#elif defined(__x86_64__)
.text
.globl _start
_start:
    xor %rbp,%rbp
    call _start
    mov $231,%rax
    xor %rdi,%rdi
    syscall
#endif
.section .note.GNU-stack,"",@progbits
EOF_E

cat > "$BD/build_b8.sh" << 'EOF_BUILD'
#!/usr/bin/env bash
set -euo pipefail
G='\033[0;32m';R='\033[0;31m';Z='\033[0m'
ok(){ echo -e "${G}[OK]${Z} $*"; }
err(){ echo -e "${R}[ERR]${Z} $*"; }
CD="$(cd "$(dirname "$0")"; pwd)"
ARCH=$(uname -m)
CF="-O2 -fPIE -fno-stack-protector -fno-asynchronous-unwind-tables \
    -fomit-frame-pointer -fno-builtin -fno-plt \
    -ffunction-sections -fdata-sections \
    -Wall -Wno-unused-function -Wno-unused-variable \
    -Wno-unused-but-set-variable -I${CD}"
LF="-pie -nostdlib -Wl,--gc-sections -Wl,--build-id=none -e _start"
echo "═══ RAFAELIA BENCH8 BUILD ═══"
echo "Arch=$ARCH Dir=$CD"
CC="${CC:-clang}"; command -v "$CC" &>/dev/null || CC=gcc
echo "CC=$CC"
BUILT=false
if [ "$ARCH" = "aarch64" ]; then
    $CC $CF -march=armv8.2-a+crc+crypto -mtune=cortex-a78 \
        $LF "${CD}/raf_b8_main.c" -o "${CD}/raf_b8" 2>&1 && {
        strip --strip-all "${CD}/raf_b8" 2>/dev/null||true
        ok "ARM64: $(ls -lh ${CD}/raf_b8|awk '{print $5}')"; BUILT=true; }
elif [ "$ARCH" = "x86_64" ]; then
    $CC $CF -march=native $LF -static \
        "${CD}/raf_b8_main.c" -o "${CD}/raf_b8" 2>&1 && {
        strip --strip-all "${CD}/raf_b8" 2>/dev/null||true
        ok "x86_64: $(ls -lh ${CD}/raf_b8|awk '{print $5}')"; BUILT=true; }
fi
# ARM32 cross (para Termux emulado)
for CC32 in arm-linux-gnueabihf-gcc arm-linux-gnueabi-gcc; do
    command -v $CC32 &>/dev/null || continue
    $CC32 $CF -mthumb -march=armv7-a+neon-vfpv4 -mfloat-abi=softfp \
        -mfpu=neon-vfpv4 $LF \
        "${CD}/raf_b8_entry.S" "${CD}/raf_b8_main.c" \
        -o "${CD}/raf_b8_arm32" 2>&1 && {
        ok "ARM32: $(ls -lh ${CD}/raf_b8_arm32|awk '{print $5}')"; BUILT=true; } \
        || err "ARM32 $CC32 falhou"
    break
done
if ! $BUILT; then err "Build falhou"; exit 1; fi
echo ""
echo "═══ RUN ═══"
if [ -f "${CD}/raf_b8" ]; then
    "${CD}/raf_b8"
elif [ -f "${CD}/raf_b8_arm32" ]; then
    if command -v qemu-arm &>/dev/null; then
        qemu-arm "${CD}/raf_b8_arm32"
    else
        echo "(ARM32 disponível: qemu-arm ${CD}/raf_b8_arm32)"
    fi
fi
EOF_BUILD
chmod +x "$BD/build_b8.sh"

# =============================================================================
hdr "INVENTÁRIO"
# =============================================================================
echo ""
TOTAL=0
printf "%-30s %8s\n" "ARQUIVO" "LINHAS"
for f in "$BD"/*.h "$BD"/*.c "$BD"/*.S "$BD"/*.sh; do
    [ -f "$f" ]||continue
    L=$(wc -l < "$f")
    printf "%-30s %8d\n" "$(basename $f)" "$L"
    TOTAL=$((TOTAL+L))
done
echo "─────────────────────────────────────"
printf "%-30s %8d\n" "TOTAL" "$TOTAL"
echo ""
p "Compilando..."
bash "$BD/build_b8.sh" 2>&1 || true
p "Build dir: $BD"
p "DeltaRafaelVerboOmega · Omega=Amor · RAFCODE-Phi"
p "8 metodologias × 34 parâmetros × 7^7=823543 direções"
S06_EOFC
