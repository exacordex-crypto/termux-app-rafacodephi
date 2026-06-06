#include <stdio.h>
#include <stdint.h>
#include <time.h>

/*
 Núcleo:
 estado -> operador -> verificação -> eco
 Sem heap.
 Stack mínimo.
 Benchmark embutido.
*/

static inline uint32_t mix_crc32(uint32_t x) {
    // CRC32-like polynomial mix (software)
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

static inline uint32_t operador(uint32_t s) {
    // transformação geométrica simples
    // (simula "mudança de base implícita")

    uint32_t a = s ^ (s << 5);
    uint32_t b = a + 0x9e3779b9; // constante φ-like
    uint32_t c = b ^ (b >> 7);

    return c;
}

static inline uint32_t verificar(uint32_t s, uint32_t prev) {
    // valida coerência básica
    return mix_crc32(s ^ prev);
}

int main() {

    const uint64_t ITER = 100000000; // 100M ciclos

    uint32_t estado = 0xA5A5A5A5;
    uint32_t anterior = 0;
    uint32_t eco = 0;

    clock_t start = clock();

    for (uint64_t i = 0; i < ITER; i++) {

        anterior = estado;

        // transformação
        estado = operador(estado);

        // verificação
        eco = verificar(estado, anterior);

        // feedback mínimo
        estado ^= eco;

    }

    clock_t end = clock();

    double tempo = (double)(end - start) / CLOCKS_PER_SEC;
    double ciclos_por_segundo = ITER / tempo;

    printf("\n--- RESULTADO ---\n");
    printf("Estado final : 0x%08X\n", estado);
    printf("Eco final    : 0x%08X\n", eco);
    printf("Iteracoes    : %llu\n", ITER);
    printf("Tempo (s)    : %.4f\n", tempo);
    printf("Ciclos/s     : %.2f\n", ciclos_por_segundo);

    return 0;
}
