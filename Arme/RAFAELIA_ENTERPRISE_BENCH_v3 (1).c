// =========================================================================
// RAFAELIA_ENTERPRISE_BENCH_v3.c
// v3 DELTA: B5 register linearity (detect-first, disjoint reg blocks per path)
//           B6 _write_u32 '\n' overwrite (digit loop clobbers newline slot)
//
// Compilar ARM32:
//   clang -O2 -march=armv7-a -mfpu=neon-vfpv4 -mfloat-abi=softfp \
//         -std=c11 -ffreestanding -nostdlib -fno-builtin \
//         -o raf_bench RAFAELIA_ENTERPRISE_BENCH_v3.c
//
// Compilar AArch64:
//   clang -O2 -march=armv8-a+crc -std=c11 -ffreestanding -nostdlib -fno-builtin \
//         -o raf_bench RAFAELIA_ENTERPRISE_BENCH_v3.c
// =========================================================================

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  i32;
typedef int64_t  i64;

// ─── Q16.16 ──────────────────────────────────────────────────────────────────
#define Q16_HALF   32768u
#define Q16_2PI    411774u
#define Q16_INV6   10923u
#define Q16_INV120 546u

static inline u32 qmul(u32 a, u32 b) { return (u32)(((u64)a * b) >> 16); }
static inline u32 qema(u32 old, u32 in) {
    return (u32)(((u64)old * 49152u + (u64)in * 16384u) >> 16);
}
static inline u32 qsin(u32 x) {
    while (x >= Q16_2PI) x -= Q16_2PI;
    int neg = (x >= 205887u);
    if (neg) x -= 205887u;
    u64 x2 = (u64)x  * x  >> 16;
    u64 x3 = (u64)x2 * x  >> 16;
    u64 x5 = (u64)x3 * x2 >> 16;
    i64 r  = (i64)x
           - (i64)((u64)x3 * Q16_INV6   >> 16)
           + (i64)((u64)x5 * Q16_INV120 >> 16);
    if (r < 0)     r = 0;
    if (r > 65535) r = 65535;
    return neg ? 65535u - (u32)r : (u32)r;
}

// ─── CRC32C software (Castagnoli 0x82F63B78) ─────────────────────────────────
static u32  crc32c_table[256];
static int  crc32c_ready = 0;

static void crc32c_build_table(void) {
    for (u32 i = 0; i < 256; i++) {
        u32 c = i;
        for (int k = 0; k < 8; k++)
            c = (c >> 1) ^ (0x82F63B78u & -(c & 1u));
        crc32c_table[i] = c;
    }
    crc32c_ready = 1;
}
static u32 crc32c(const void *data, u32 len) {
    const u8 *p = (const u8 *)data;
    u32 crc = 0xFFFFFFFFu;
    for (u32 i = 0; i < len; i++)
        crc = (crc >> 8) ^ crc32c_table[(crc ^ p[i]) & 0xFFu];
    return crc ^ 0xFFFFFFFFu;
}

// ═══════════════════════════════════════════════════════════════════════════════
// TIMER — B5 FIX: detect-first, blocos de registradores DISJUNTOS e LINEARES
//
// PRINCÍPIO: registrador comprometido = slot fechado. Não reabre, não intercala.
//   CAMINHO HW  → x9 (cntfrq), x10 (cntvct), x11-x13 (aritmética)
//   CAMINHO SW  → x0, x1, x8 (syscall ABI Linux)
//   DISPATCH    → decide em g_timer_mode ANTES de tocar qualquer registrador
//
// ARM32: PMCCNTR requer PMUSERENR.EN=1 (raro no Android) → sempre SW path
// ═══════════════════════════════════════════════════════════════════════════════

#if defined(__arm__)

#  define SYS_WRITE     4
#  define SYS_EXIT      1
#  define SYS_CLOCK_GET 263

static inline long _sys3(long nr, long a, long b, long c) {
    // Registradores em ordem linear ascendente: r0→r1→r2→r7
    // r3-r6 não são tocados nem lidos neste ponto → sem gap ativo neste escopo
    register long r0 __asm__("r0") = a;
    register long r1 __asm__("r1") = b;
    register long r2 __asm__("r2") = c;
    register long r7 __asm__("r7") = nr;
    __asm__ volatile(
        "svc #0"
        : "+r"(r0)
        : "r"(r1), "r"(r2), "r"(r7)
        : "memory", "cc", "r3", "r4", "r5", "r6"  // declara sujos explicitamente
    );
    return r0;
}

static void   timer_detect(void) { /* ARM32: só SW path disponível */ }
static inline u64 now_ns(void) {
    // SW path: r0→r1→r7 lineares; r2=0 explícito (sem valor sujo implícito)
    struct { long sec; long nsec; } ts;
    _sys3(SYS_CLOCK_GET, 1, (long)&ts, 0);
    return (u64)ts.sec * 1000000000ULL + (u64)(u32)ts.nsec;
}

#elif defined(__aarch64__)

#  define SYS_WRITE     64
#  define SYS_EXIT      93
#  define SYS_CLOCK_GET 113

// ── Bloco SW: x0, x1, x8 — nunca reabertos depois de comprometidos ────────────
static inline long _sys3(long nr, long a, long b, long c) {
    register long x0 __asm__("x0") = a;
    register long x1 __asm__("x1") = b;
    register long x2 __asm__("x2") = c;
    register long x8 __asm__("x8") = nr;
    __asm__ volatile(
        "svc #0"
        : "+r"(x0)
        : "r"(x1), "r"(x2), "r"(x8)
        : "memory", "cc", "x3", "x4", "x5", "x6", "x7"  // sujos explícitos
    );
    return x0;
}

// ── Bloco HW: x9, x10, x11, x12, x13 — disjunto do bloco SW ────────────────
// Lê cntfrq_el0 → x9 (uma vez, slot fechado)
// Lê cntvct_el0 → x10 (uma vez, slot fechado)
// Aritmética cycles→ns em x11, x12, x13 (lineares, sem retorno)
// Nenhum destes registradores aparece no bloco SW acima.

static u64 g_cntfrq   = 0;
static int g_timer_hw = 0;  // 1 = HW autorizado, 0 = fallback SW

static void timer_detect(void) {
    // Lê cntfrq_el0 via x9 (slot HW, não compartilhado com syscall ABI)
    u64 freq;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(freq));
    // Frequência válida: 1 MHz – 1 GHz (fora desse range = não autorizado/inválido)
    if (freq >= 1000000ULL && freq <= 1000000000ULL) {
        g_cntfrq   = freq;
        g_timer_hw = 1;
    }
    // Se freq == 0 ou fora do range: g_timer_hw permanece 0 → SW path
}

// ── HW path: ciclos / cntfrq → ns ────────────────────────────────────────────
static inline u64 _now_ns_hw(void) {
    // x9 = cntfrq (já salvo em g_cntfrq; relê para manter o bloco autossuficiente)
    // x10 = cntvct_el0 (ciclos brutos)
    // Conversão: ns = (cnt/freq)*1e9 + (cnt%freq)*1e9/freq
    // Sem overflow: freq ≤ 1e9, então cnt%freq < 1e9, e *1e9 pode estourar u64
    // Usa divisão em dois estágios para segurança.
    u64 cnt, freq;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(cnt));   // x10 (slot HW)
    freq = g_cntfrq;                                        // x9 lógico
    u64 q = cnt / freq;                                     // x11
    u64 r = cnt % freq;                                     // x12
    return q * 1000000000ULL + r * 1000000000ULL / freq;   // x13
}

// ── Dispatch: decide ANTES de tocar qualquer registrador ────────────────────
// g_timer_hw é lido primeiro; cada ramo toca apenas seu bloco próprio.
static inline u64 now_ns(void) {
    if (g_timer_hw) return _now_ns_hw();               // bloco HW (x9-x13)
    struct { long sec; long nsec; } ts;
    _sys3(SYS_CLOCK_GET, 1, (long)&ts, 0);            // bloco SW (x0,x1,x8)
    return (u64)ts.sec * 1000000000ULL + (u64)(u32)ts.nsec;
}

#else
#  error "Arch não suportada: arm | aarch64"
#endif

// ─── I/O sem libc ────────────────────────────────────────────────────────────
static void _write(const char *s) {
    u32 n = 0; while (s[n]) n++;
    _sys3(SYS_WRITE, 1, (long)s, (long)n);
}

// B6 FIX: buf tem zona de dígitos [1..10] + '\n' fixo em [11] + '\0' em [12]
// O loop de dígitos escreve de trás para frente DENTRO de [1..10].
// '\n' em [11] nunca é tocado pelo loop — não há sobreescrita.
static void _write_u32(u32 v) {
    char buf[13];
    buf[11] = '\n';   // posição fixa — loop não alcança aqui
    buf[12] = '\0';
    int i = 10;       // índice máximo de dígito = 10 (abaixo do '\n')
    if (!v) { buf[10] = '0'; _write(buf + 10); return; }
    while (v && i >= 1) { buf[i--] = '0' + (char)(v % 10); v /= 10; }
    // agora buf[i+1..10] = dígitos, buf[11]='\n', buf[12]='\0'
    _write(buf + i + 1);
}

static void _write_hex(u32 v) {
    static const char hx[] = "0123456789abcdef";
    char buf[12];
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 9; i >= 2; i--) { buf[i] = hx[v & 0xFu]; v >>= 4; }
    buf[10] = '\n';
    buf[11] = '\0';
    _write(buf);
}

// ─── configuração ─────────────────────────────────────────────────────────────
#define N_CELLS   1024
#define N_VCPU    8
#define PERIOD    42
#define TORUS_DIM 7

static const u32 HZ_TABLE[N_VCPU] = {
    58000, 58000, 58000, 50296, 43500, 43500, 37709, 26836
};

typedef struct {
    i32 s[TORUS_DIM];
    u16 ttl;
    u8  state;
    u8  vcpu;
} __attribute__((aligned(16))) cell_t;

static cell_t g_cells[N_CELLS] __attribute__((aligned(64)));
static u32 g_C[N_VCPU], g_H[N_VCPU], g_phase[N_VCPU];
static u32 g_crc_chain, g_rollbacks;

static void init(void) {
    crc32c_build_table();
    timer_detect();           // ← detecta HW vs SW antes de qualquer benchmark
    u32 seed = 0xDEADBEEFu;
    for (u32 i = 0; i < N_CELLS; i++) {
        cell_t *c = &g_cells[i];
        u32 h = (seed * 0x9E3779B9u) ^ i;
        for (int d = 0; d < TORUS_DIM; d++) c->s[d] = (i32)((h >> (d*4)) & 0xFFFFu);
        c->ttl   = PERIOD;
        c->state = 0;
        c->vcpu  = (u8)(i % N_VCPU);
    }
    for (u32 v = 0; v < N_VCPU; v++) {
        g_C[v] = Q16_HALF; g_H[v] = Q16_HALF; g_phase[v] = 0;
    }
    g_crc_chain = 0; g_rollbacks = 0;
}

static void cell_step(u32 idx) {
    cell_t *c = &g_cells[idx];
    u32 v     = c->vcpu;
    u32 C     = g_C[v], H = g_H[v];
    u32 sv    = qsin(g_phase[v] * (HZ_TABLE[v] >> 8));
    C = qema(C, sv);
    H = qema(H, 65535u - sv);
    u32 phi = qmul(65535u - H, C);   // φ=(1-H)·C [eq.8]

    i32 new_s[TORUS_DIM];
    for (int d = 0; d < TORUS_DIM; d++)
        new_s[d] = (i32)qema((u32)c->s[d], (u32)c->s[(d+1)%TORUS_DIM]);
    for (int d = 0; d < TORUS_DIM; d++) c->s[d] = new_s[d];

    if (c->ttl > 0) c->ttl--;
    if (c->ttl == 0) {
        u32 h = idx * 0x9E3779B9u;
        for (int d = 0; d < TORUS_DIM; d++) c->s[d] = (i32)((h >> (d*4)) & 0xFFFFu);
        c->ttl = PERIOD; c->state = 0; g_rollbacks++;
    }
    g_C[v] = C; g_H[v] = H;
    g_phase[v] = (g_phase[v] + 1) % PERIOD;

    u32 pair[2] = { phi, g_crc_chain };
    g_crc_chain = crc32c(pair, 8u);
}

static void run_benchmark(void) {
    for (u32 cyc = 0; cyc < PERIOD; cyc++)
        for (u32 i = 0; i < N_CELLS; i++)
            cell_step(i);
}

static u64 bench_median(u64 *samples, u32 n) {
    for (u32 i = 1; i < n; i++) {
        u64 key = samples[i]; u32 j = i;
        while (j > 0 && samples[j-1] > key) { samples[j] = samples[j-1]; j--; }
        samples[j] = key;
    }
    return samples[n / 2];
}

void _start(void) {
    init();

    const u32 SAMPLES = 31;
    u64 times[SAMPLES];
    for (u32 i = 0; i < SAMPLES; i++) {
        u64 t0 = now_ns(); run_benchmark(); u64 t1 = now_ns();
        times[i] = t1 - t0;
    }

    u64 med    = bench_median(times, SAMPLES);
    u32 med_hi = (u32)(med >> 32);
    u32 med_lo = (u32)(med & 0xFFFFFFFFu);

    _write("=== RAFAELIA BENCH v3 ===\n");
#if defined(__aarch64__)
    _write(g_timer_hw ? "timer=HW(cntvct/cntfrq)\n" : "timer=SW(syscall113)\n");
#else
    _write("timer=SW(syscall263)\n");
#endif
    _write("median_ns_hi="); _write_u32(med_hi);
    _write("median_ns_lo="); _write_u32(med_lo);
    _write("crc_final=");    _write_hex(g_crc_chain);
    _write("rollbacks=");    _write_u32(g_rollbacks);
    _write("Ω=Amor FIAT LUX\n");

    _sys3(SYS_EXIT, 0, 0, 0);
    __builtin_unreachable();
}
