/* ========================================================================
   vectra_bench_industrial.c
   =========================================================================
   Benchmark industrial para sistemas dinâmicos (Rafael / Vectra).
   Coleta N amostras de um invariante (ex: CRC32), calcula:
     - Estatísticas clássicas: mediana, média, desvio padrão, min, max
     - Matriz triangular superior de diferenças absolutas
         → métricas de previsibilidade (mediana das diferenças, maior diferença, etc.)
     - Regressão quadrática (Bhaskara): ajusta uma parábola aos dados,
       calcula discriminante, raízes e distância entre raízes ("catetos")
     - Curvatura local (derivada segunda) e tangente média
     - Comprimento geodésico (distância acumulada normalizada)
   Compilar: clang -O2 -lm -o vectra_bench_industrial vectra_bench_industrial.c
   Uso: ./vectra_bench_industrial [--auto] [--seed S]
   ======================================================================== */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

/* ------------------------- configurações ------------------------- */
#define N_SAMPLES       56      /* número de amostras (56 = 7*8) */
#define N_ITER          1000    /* iterações por amostra (ajustável) */
#define CRC32_POLY      0xEDB88320

/* ------------------------- tipos e utils ------------------------- */
typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;

/* RNG simples (xorshift32) */
static u32 rng_state = 0xDEADBEEF;
static u32 xorshift32(void) {
    u32 x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}
static f64 rand01(void) { return (f64)(xorshift32()) / (f64)0xFFFFFFFF; }

/* ---------- invariante kernel (CRC32 software + mix) ---------- */
static u32 crc32_sw(const u8 *buf, size_t len) {
    u32 crc = ~0U;
    for (size_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ ((crc & 1) ? CRC32_POLY : 0);
    }
    return ~crc;
}

static void mix_sw(u8 *buf, size_t len) {
    for (size_t i = 0; i + 64 <= len; i += 64) {
        u8 tmp[64];
        memcpy(tmp, buf + i, 64);
        for (int j = 0; j < 64; j++) {
            u8 a = tmp[(j + 7) & 63];
            u8 b = tmp[(j + 11) & 63];
            buf[i + j] ^= a ^ b;
        }
    }
}

/* Coleta N_SAMPLES invariantes (CRC do buffer após cada mix) */
static void collect_samples(u32 *samples, u32 seed) {
    rng_state = seed ^ 0x5A5A5A5A;
    u8 buf[4096] __attribute__((aligned(64)));

    /* inicialização determinística */
    u64 s = 0x9E3779B97F4A7C15ULL;
    for (int i = 0; i < 4096; i += 8) {
        *(u64*)(buf + i) = s;
        s += 0x9E3779B9;
    }

    for (int i = 0; i < N_SAMPLES; i++) {
        samples[i] = crc32_sw(buf, 4096);
        mix_sw(buf, 4096);
    }
}

/* ---------- estatísticas básicas ---------- */
static int cmp_u32(const void *a, const void *b) {
    u32 va = *(u32*)a, vb = *(u32*)b;
    return (va > vb) - (va < vb);
}

static f64 median_u32(u32 *arr, int n) {
    u32 *tmp = malloc(n * sizeof(u32));
    memcpy(tmp, arr, n * sizeof(u32));
    qsort(tmp, n, sizeof(u32), cmp_u32);
    f64 med = (n % 2) ? tmp[n/2] : (tmp[n/2-1] + tmp[n/2]) / 2.0;
    free(tmp);
    return med;
}

static void basic_stats(u32 *arr, int n, f64 *mean, f64 *stddev, u32 *min, u32 *max) {
    *min = arr[0]; *max = arr[0];
    f64 sum = 0.0;
    for (int i = 0; i < n; i++) {
        if (arr[i] < *min) *min = arr[i];
        if (arr[i] > *max) *max = arr[i];
        sum += arr[i];
    }
    *mean = sum / n;
    f64 var = 0.0;
    for (int i = 0; i < n; i++) {
        f64 d = arr[i] - *mean;
        var += d * d;
    }
    *stddev = sqrt(var / n);
}

/* ---------- matriz triangular superior de diferenças ---------- */
static int _cmp_f64(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

typedef struct {
    f64 mean_diff;      /* média das diferenças absolutas (triangular superior) */
    f64 median_diff;
    f64 max_diff;
    f64 min_diff;
} TriDiffStats;

static TriDiffStats triangular_diff_stats(u32 *arr, int n) {
    TriDiffStats ts = {0};
    int total = n * (n-1) / 2;
    if (total == 0) return ts;
    f64 *diffs = malloc(total * sizeof(f64));
    int idx = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i+1; j < n; j++) {
            diffs[idx++] = fabs((f64)arr[i] - (f64)arr[j]);
        }
    }
    /* média */
    f64 sum = 0.0;
    for (int i = 0; i < total; i++) sum += diffs[i];
    ts.mean_diff = sum / total;
    /* mediana */
    qsort(diffs, total, sizeof(f64), _cmp_f64);
    ts.median_diff = (total % 2) ? diffs[total/2] : (diffs[total/2-1] + diffs[total/2]) / 2.0;
    ts.max_diff = diffs[total-1];
    ts.min_diff = diffs[0];
    free(diffs);
    return ts;
}

/* ---------- regressão quadrática (Bhaskara) ---------- */
/* y = a*x² + b*x + c, ajuste por mínimos quadrados */
typedef struct {
    f64 a, b, c;
    f64 delta;         /* discriminante b² - 4ac */
    f64 root1, root2;  /* raízes reais (se delta>=0) */
    f64 dist_roots;    /* |root1 - root2| (catetos?) */
} QuadraticFit;

static QuadraticFit fit_quadratic(u32 *arr, int n) {
    QuadraticFit qf = {0};
    f64 Sx=0, Sx2=0, Sx3=0, Sx4=0, Sy=0, Sxy=0, Sx2y=0;
    for (int i = 0; i < n; i++) {
        f64 x = i;            /* índice como variável independente */
        f64 y = arr[i];
        f64 x2 = x*x, x3 = x2*x, x4 = x3*x;
        Sx += x; Sx2 += x2; Sx3 += x3; Sx4 += x4;
        Sy += y; Sxy += x*y; Sx2y += x2*y;
    }
    /* sistema normal:
       | n    Sx   Sx2 | |c| = |Sy|
       | Sx  Sx2  Sx3 | |b| = |Sxy|
       | Sx2 Sx3  Sx4 | |a| = |Sx2y|
    */
    f64 det = n * (Sx2*Sx4 - Sx3*Sx3) - Sx * (Sx*Sx4 - Sx3*Sx2) + Sx2 * (Sx*Sx3 - Sx2*Sx2);
    if (fabs(det) < 1e-12) {
        /* degenerado, usar média */
        qf.a = qf.b = 0; qf.c = Sy / n;
        qf.delta = -1; qf.dist_roots = 0;
        return qf;
    }
    f64 inv_det = 1.0 / det;
    qf.c = ( (Sx2*Sx4 - Sx3*Sx3)*Sy + (Sx3*Sx2 - Sx*Sx4)*Sxy + (Sx*Sx3 - Sx2*Sx2)*Sx2y ) * inv_det;
    qf.b = ( (Sx3*Sx2 - Sx*Sx4)*Sy + (n*Sx4 - Sx2*Sx2)*Sxy + (Sx2*Sx - n*Sx3)*Sx2y ) * inv_det;
    qf.a = ( (Sx*Sx3 - Sx2*Sx2)*Sy + (Sx2*Sx - n*Sx3)*Sxy + (n*Sx2 - Sx*Sx)*Sx2y ) * inv_det;

    qf.delta = qf.b*qf.b - 4.0*qf.a*qf.c;
    if (qf.delta >= 0 && fabs(qf.a) > 1e-12) {
        f64 sqrt_delta = sqrt(qf.delta);
        qf.root1 = (-qf.b - sqrt_delta) / (2.0*qf.a);
        qf.root2 = (-qf.b + sqrt_delta) / (2.0*qf.a);
        qf.dist_roots = fabs(qf.root1 - qf.root2);
    } else {
        qf.dist_roots = 0; /* raízes complexas, mas a distância seria sqrt(-delta)/|a| */
    }
    return qf;
}

/* ---------- curvatura e tangente ---------- */
typedef struct {
    f64 mean_curvature;    /* média da curvatura (derivada segunda) */
    f64 mean_tangent;      /* média da tangente (derivada primeira) */
    f64 geodesic_length;   /* comprimento geodésico (distância acumulada normalizada) */
} CurvatureMetrics;

static CurvatureMetrics compute_curvature(u32 *arr, int n) {
    CurvatureMetrics cm = {0};
    if (n < 3) return cm;

    f64 *x = malloc(n * sizeof(f64));
    for (int i = 0; i < n; i++) x[i] = (f64)arr[i];
    /* normaliza para [0,1] a fim de tratar como ângulos na esfera */
    f64 minv = x[0], maxv = x[0];
    for (int i = 1; i < n; i++) { if (x[i] < minv) minv = x[i]; if (x[i] > maxv) maxv = x[i]; }
    f64 range = maxv - minv;
    if (range < 1e-12) range = 1.0;
    for (int i = 0; i < n; i++) x[i] = (x[i] - minv) / range;  /* agora em [0,1] */

    /* derivada primeira central (tangente) e segunda (curvatura) */
    f64 sum_tan = 0.0, sum_curv = 0.0;
    f64 geo_len = 0.0;
    for (int i = 1; i < n; i++) {
        geo_len += fabs(x[i] - x[i-1]);   /* distância geodésica 1D */
    }
    for (int i = 1; i < n-1; i++) {
        f64 dx1 = x[i] - x[i-1];
        f64 dx2 = x[i+1] - x[i];
        f64 ddx = dx2 - dx1;   /* derivada segunda (diferença das diferenças) */
        f64 tan = dx1;          /* tangente aproximada */
        sum_tan += fabs(tan);
        sum_curv += fabs(ddx);
    }
    cm.mean_tangent = sum_tan / (n-2);
    cm.mean_curvature = sum_curv / (n-2);
    cm.geodesic_length = geo_len;
    free(x);
    return cm;
}

/* ---------- matriz de previsibilidade (triangular) como relatório ---------- */
static void print_triangular_matrix(u32 *arr, int n) {
    /* imprime apenas uma visualização compacta: primeiras 8 linhas e colunas */
    printf("\n=== Matriz Triangular de Diferenças (superior) ===\n");
    printf("   ");
    for (int j = 0; j < (n > 8 ? 8 : n); j++) printf("%8d ", j);
    printf("\n");
    for (int i = 0; i < (n > 8 ? 8 : n); i++) {
        printf("%2d ", i);
        for (int j = 0; j < (n > 8 ? 8 : n); j++) {
            if (j <= i) printf("         ");
            else printf("%8.0f ", fabs((f64)arr[i] - (f64)arr[j]));
        }
        printf("\n");
    }
    if (n > 8) printf("  ... (matriz %d x %d completa)\n", n, n);
}

/* ---------- relatório completo ---------- */
static void report_benchmark(u32 *samples, u32 seed) {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║        BENCHMARK INDUSTRIAL VECTRA – MÉTRICAS COMPLETAS       ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("Seed: %u   Amostras: %d\n", seed, N_SAMPLES);

    /* 1. Estatísticas básicas */
    f64 mean, stddev;
    u32 minv, maxv;
    basic_stats(samples, N_SAMPLES, &mean, &stddev, &minv, &maxv);
    f64 median = median_u32(samples, N_SAMPLES);
    printf("\n▶ Estatísticas univariadas:\n");
    printf("  Mediana  = %.0f\n", median);
    printf("  Média    = %.2f\n", mean);
    printf("  Desvio p = %.2f\n", stddev);
    printf("  Mínimo   = %u\n", minv);
    printf("  Máximo   = %u\n", maxv);
    printf("  Range    = %u\n", maxv - minv);

    /* 2. Matriz triangular (previsibilidade) */
    TriDiffStats td = triangular_diff_stats(samples, N_SAMPLES);
    printf("\n▶ Matriz triangular superior (diferenças absolutas):\n");
    printf("  Média das diferenças = %.2f\n", td.mean_diff);
    printf("  Mediana das diferenças = %.2f\n", td.median_diff);
    printf("  Maior diferença (catetos?) = %.0f\n", td.max_diff);
    printf("  Menor diferença = %.0f\n", td.min_diff);
    print_triangular_matrix(samples, N_SAMPLES);

    /* 3. Regressão quadrática (Bhaskara) */
    QuadraticFit qf = fit_quadratic(samples, N_SAMPLES);
    printf("\n▶ Ajuste parabólico (y = a*x² + b*x + c):\n");
    printf("  a = %.6e   b = %.6e   c = %.2f\n", qf.a, qf.b, qf.c);
    printf("  Discriminante Δ = %.6e\n", qf.delta);
    if (qf.delta >= 0) {
        printf("  Raízes: x1 = %.4f , x2 = %.4f\n", qf.root1, qf.root2);
        printf("  Distância entre raízes (catetos) = %.4f\n", qf.dist_roots);
    } else {
        printf("  Raízes complexas. Distância imaginária = %.4f\n", sqrt(-qf.delta)/fabs(qf.a));
    }

    /* 4. Curvatura e geodésica */
    CurvatureMetrics cm = compute_curvature(samples, N_SAMPLES);
    printf("\n▶ Dinâmica toroidal / geodésica:\n");
    printf("  Tangente média (derivada primeira) = %.6f\n", cm.mean_tangent);
    printf("  Curvatura média (derivada segunda) = %.6f\n", cm.mean_curvature);
    printf("  Comprimento geodésico (normalizado) = %.6f\n", cm.geodesic_length);

    /* 5. Critério de atrator (baixa variância + baixa curvatura) */
    f64 attractor_score = stddev * cm.mean_curvature;
    printf("\n▶ Índice de atrator (stddev * curvatura): %.6f\n", attractor_score);
    if (attractor_score < 1e-3) printf("  ⚡ Sistema em atrator (alta previsibilidade)\n");
    else if (attractor_score < 1e-1) printf("  ⚡ Região de transição\n");
    else printf("  ⚡ Regime caótico / não convergente\n");

    /* 6. Classificação final */
    printf("\n┌────────────────────────────────────────────────────────────────┐\n");
    if (td.median_diff < 1000 && cm.mean_curvature < 0.01)
        printf("│  PREVISIBILIDADE ALTA – sistema estável (atrator)              │\n");
    else if (td.median_diff > 50000 && cm.mean_curvature > 0.1)
        printf("│  PREVISIBILIDADE BAIXA – sistema caótico / divergente          │\n");
    else
        printf("│  PREVISIBILIDADE MÉDIA – regime de transição                    │\n");
    printf("└────────────────────────────────────────────────────────────────┘\n");
}

/* ------------------------- main ------------------------- */
int main(int argc, char **argv) {
    u32 seed = (u32)time(NULL);
    int auto_mode = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--auto") == 0) auto_mode = 1;
        else if (strcmp(argv[i], "--seed") == 0 && i+1 < argc) {
            seed = (u32)atoi(argv[++i]);
        }
    }

    printf("\n[VECTRA] Coletando %d amostras do invariante (CRC32 + mix)...\n", N_SAMPLES);
    u32 *samples = malloc(N_SAMPLES * sizeof(u32));
    if (!samples) { fprintf(stderr, "memória insuficiente\n"); return 1; }

    collect_samples(samples, seed);
    report_benchmark(samples, seed);
    free(samples);

    /* modo automático: executa silenciosamente com seed variável? Não necessário */
    if (auto_mode) {
        printf("\n[auto] Modo automático concluído.\n");
    }
    return 0;
}y
