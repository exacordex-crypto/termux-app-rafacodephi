/*
 * RAFAELIA_CORE_ARM32_NEON.c
 * 
 * Bloco único C/asm low-level para Termux (ARM32, NEON, SIMD)
 * - Círculo de 42 pontos com deformação Fibonacci
 * - Conjunto de Mandelbrot com aceleração NEON (4 floats por vez)
 * - 7 threads paralelas (equalizador estrutural)
 * - Verificação CRC32 da imagem final
 * - Sem dependências externas (apenas libc, pthread, math)
 * 
 * Compilar:
 *   gcc -O3 -march=armv7-a -mfpu=neon -mfloat-abi=hard -pthread -lm -o rafaelia rafaelia.c
 * Executar:
 *   ./rafaelia
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <arm_neon.h>   // NEON intrinsics

// ============================================================================
// CONFIGURAÇÕES GLOBAIS
// ============================================================================
#define WIDTH       640
#define HEIGHT      480
#define MAX_ITER    256
#define N_PONTOS    42          // 42 pontos no círculo
#define N_THREADS   7           // 7 vias paralelas (igualizador)
#define CRC32_POLY  0xEDB88320

// Estrutura para passar dados para as threads
typedef struct {
    int id;
    int start_row;
    int end_row;
    unsigned char *output;      // imagem RGB (3 bytes por pixel)
    double *mandel_data;        // opcional: valores de iteração
} ThreadData;

// ============================================================================
// 1. FUNÇÕES GEOMÉTRICAS: CÍRCULO DE 42 PONTOS + DEFORMAÇÃO FIBONACCI
// ============================================================================
void fibonacci_deform(double *raios, int n) {
    // Gera sequência de Fibonacci normalizada para n pontos
    double fib[128];
    fib[0] = 1.0; fib[1] = 1.0;
    for (int i = 2; i < n; i++) {
        fib[i] = fib[i-1] + fib[i-2];
    }
    double max_fib = fib[n-1];
    for (int i = 0; i < n; i++) {
        fib[i] /= max_fib;   // normaliza [0,1]
    }
    // Ângulos: 0 a 2π, divididos em n pontos
    for (int i = 0; i < n; i++) {
        double theta = 2.0 * M_PI * i / n;
        // Deformação: 1 + 0.4 * fib[i] * |sin(theta)|
        double deform = 1.0 + 0.4 * fib[i] * fabs(sin(theta));
        raios[i] = deform;   // raio base = 1, deformado
    }
}

void gerar_pontos(double *x, double *y, double *raios, int n) {
    for (int i = 0; i < n; i++) {
        double theta = 2.0 * M_PI * i / n;
        x[i] = raios[i] * cos(theta);
        y[i] = raios[i] * sin(theta);
    }
}

// ============================================================================
// 2. MANDELBROT COM ACELERAÇÃO NEON (processa 4 pixels em paralelo)
// ============================================================================
// Calcula o número de iterações para um ponto c = x + i*y
int mandelbrot_single(double cx, double cy) {
    double zx = 0.0, zy = 0.0;
    int iter = 0;
    while (zx*zx + zy*zy < 4.0 && iter < MAX_ITER) {
        double new_zx = zx*zx - zy*zy + cx;
        double new_zy = 2.0*zx*zy + cy;
        zx = new_zx;
        zy = new_zy;
        iter++;
    }
    return iter;
}

// Versão NEON: processa 4 pontos com mesmo cy (linha horizontal)
void mandelbrot_neon_4(double cx0, double cx1, double cx2, double cx3, double cy, int *iters) {
    // Inicializa vetores com zeros
    float32x4_t zx = vdupq_n_f32(0.0f);
    float32x4_t zy = vdupq_n_f32(0.0f);
    float32x4_t cx = { (float)cx0, (float)cx1, (float)cx2, (float)cx3 };
    float32x4_t cy_vec = vdupq_n_f32((float)cy);
    float32x4_t four = vdupq_n_f32(4.0f);
    int32x4_t iter = vdupq_n_s32(0);
    int32x4_t max_iter = vdupq_n_s32(MAX_ITER);
    
    for (int n = 0; n < MAX_ITER; n++) {
        // zx^2, zy^2
        float32x4_t zx2 = vmulq_f32(zx, zx);
        float32x4_t zy2 = vmulq_f32(zy, zy);
        float32x4_t zx2_plus_zy2 = vaddq_f32(zx2, zy2);
        // compara com 4.0
        uint32x4_t cmp = vcltq_f32(zx2_plus_zy2, four);
        // se todos os 4 já ultrapassaram, sai do loop
        if (vminvq_u32(cmp) == 0) break;
        
        // new_zx = zx^2 - zy^2 + cx
        float32x4_t new_zx = vsubq_f32(zx2, zy2);
        new_zx = vaddq_f32(new_zx, cx);
        // new_zy = 2*zx*zy + cy
        float32x4_t new_zy = vmulq_f32(zx, zy);
        new_zy = vaddq_f32(new_zy, new_zy);
        new_zy = vaddq_f32(new_zy, cy_vec);
        
        zx = new_zx;
        zy = new_zy;
        
        // incrementa iter onde ainda não estourou
        int32x4_t inc = vreinterpretq_s32_u32(cmp);
        iter = vaddq_s32(iter, inc);
    }
    // Armazena os 4 resultados
    vst1q_s32(iters, iter);
}

// ============================================================================
// 3. THREADS: CADA UMA PROCESSA UMA FAIXA DE LINHAS
// ============================================================================
void *processar_faixa(void *arg) {
    ThreadData *data = (ThreadData*)arg;
    int start = data->start_row;
    int end = data->end_row;
    unsigned char *img = data->output;
    
    // Região do plano complexo: x ∈ [-2.0, 1.0], y ∈ [-1.2, 1.2]
    double xmin = -2.0, xmax = 1.0;
    double ymin = -1.2, ymax = 1.2;
    double step_x = (xmax - xmin) / WIDTH;
    double step_y = (ymax - ymin) / HEIGHT;
    
    for (int y = start; y < end; y++) {
        double cy = ymin + y * step_y;
        // Processa grupos de 4 pixels por vez (NEON)
        int x = 0;
        for (; x <= WIDTH - 4; x += 4) {
            double cx0 = xmin + (x+0) * step_x;
            double cx1 = xmin + (x+1) * step_x;
            double cx2 = xmin + (x+2) * step_x;
            double cx3 = xmin + (x+3) * step_x;
            int iters[4];
            mandelbrot_neon_4(cx0, cx1, cx2, cx3, cy, iters);
            for (int k = 0; k < 4; k++) {
                int iter = iters[k];
                // Mapeia iteração para cor (escala de cinza / hot)
                unsigned char val = (unsigned char)((iter * 255) / MAX_ITER);
                int idx = (y * WIDTH + (x+k)) * 3;
                img[idx]   = val;       // R
                img[idx+1] = val/2;     // G
                img[idx+2] = val/4;     // B
            }
        }
        // Restante (menos de 4 pixels) processa scalar
        for (; x < WIDTH; x++) {
            double cx = xmin + x * step_x;
            int iter = mandelbrot_single(cx, cy);
            unsigned char val = (unsigned char)((iter * 255) / MAX_ITER);
            int idx = (y * WIDTH + x) * 3;
            img[idx]   = val;
            img[idx+1] = val/2;
            img[idx+2] = val/4;
        }
    }
    return NULL;
}

// ============================================================================
// 4. CÁLCULO DE CRC32 (verificação de integridade)
// ============================================================================
unsigned int crc32(const unsigned char *data, size_t len) {
    unsigned int crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1) ? CRC32_POLY : 0);
        }
    }
    return ~crc;
}

// ============================================================================
// 5. FUNÇÃO PRINCIPAL (BLOCO ÚNICO)
// ============================================================================
int main() {
    printf("RAFAELIA_CORE_ARM32_NEON\n");
    printf("=======================\n");
    
    // --- 1. Círculo de 42 pontos com deformação Fibonacci ---
    double raios[N_PONTOS];
    double x_pts[N_PONTOS], y_pts[N_PONTOS];
    fibonacci_deform(raios, N_PONTOS);
    gerar_pontos(x_pts, y_pts, raios, N_PONTOS);
    
    printf("Pontos deformados (primeiros 5):\n");
    for (int i = 0; i < 5; i++) {
        printf("  P%d: (%.4f, %.4f) raio=%.4f\n", i, x_pts[i], y_pts[i], raios[i]);
    }
    
    // --- 2. Alocar imagem RGB ---
    size_t img_size = WIDTH * HEIGHT * 3;
    unsigned char *image = (unsigned char*)malloc(img_size);
    if (!image) {
        fprintf(stderr, "Erro de alocação de memória.\n");
        return 1;
    }
    memset(image, 0, img_size);
    
    // --- 3. Criar threads (7 vias paralelas) ---
    pthread_t threads[N_THREADS];
    ThreadData thread_data[N_THREADS];
    int rows_per_thread = HEIGHT / N_THREADS;
    int remaining = HEIGHT % N_THREADS;
    int start_row = 0;
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    for (int i = 0; i < N_THREADS; i++) {
        int end_row = start_row + rows_per_thread + (i < remaining ? 1 : 0);
        thread_data[i].id = i;
        thread_data[i].start_row = start_row;
        thread_data[i].end_row = end_row;
        thread_data[i].output = image;
        pthread_create(&threads[i], NULL, processar_faixa, &thread_data[i]);
        start_row = end_row;
    }
    
    for (int i = 0; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    // --- 4. Calcular CRC32 da imagem ---
    unsigned int crc = crc32(image, img_size);
    
    // --- 5. Exibir resultados e métricas ---
    printf("\n=== RESULTADOS ===\n");
    printf("Mandelbrot %dx%d gerado com %d threads (NEON SIMD).\n", WIDTH, HEIGHT, N_THREADS);
    printf("Tempo de processamento: %.3f segundos\n", elapsed);
    printf("CRC32 da imagem: 0x%08X\n", crc);
    printf("Largura de banda estimada: %.2f MB/s\n",
           (img_size / 1e6) / elapsed);
    
    // --- 6. Salvar imagem como PPM (opcional, para verificação) ---
    FILE *f = fopen("mandelbrot_neon.ppm", "wb");
    if (f) {
        fprintf(f, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
        fwrite(image, 1, img_size, f);
        fclose(f);
        printf("Imagem salva em 'mandelbrot_neon.ppm'\n");
    }
    
    // --- 7. Simular "retroalimentação" e "hitmisscache" (laço vazio para demonstrar) ---
    volatile int dummy = 0;
    for (int ciclo = 0; ciclo < 1000000; ciclo++) {
        dummy += (ciclo & 0xFF);
        // Toque de cache: acessa posições diferentes para gerar hits/misses
        if ((ciclo & 0xFF) == 0) {
            dummy += image[ciclo % img_size];
        }
    }
    printf("Ciclo de retroalimentação concluído.\n");
    
    free(image);
    return 0;
}
