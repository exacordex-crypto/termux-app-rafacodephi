/**
 * poly_opt.c — Otimizador de latência, montador final e execução
 * Corrigido: tabela de latência completa, bounds checks, mprotect, cache flush
 */
#include "statecomp.h"
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX_OPCODES 4096
#define MEM_EXEC_BASE 0x1000

/* ── Tabela de latência completamente inicializada (Cortex-A53) ────── */
static u32 latency_table[256] = {
    [FLAG_LOAD] = 2, [FLAG_STORE] = 2, [FLAG_ADD] = 2, [FLAG_SUB] = 2,
    [FLAG_MUL] = 6, [FLAG_DIV] = 8, [FLAG_AND] = 2, [FLAG_OR] = 2,
    [FLAG_XOR] = 2, [FLAG_NOT] = 2, [FLAG_SHL] = 2, [FLAG_SHR] = 2,
    [FLAG_CMP] = 2, [FLAG_IF] = 2, [FLAG_ELSE] = 1, [FLAG_ENDIF] = 1,
    [FLAG_WHILE] = 2, [FLAG_ENDW] = 1, [FLAG_FOR] = 3, [FLAG_ENDF] = 1,
    [FLAG_CALL] = 1, [FLAG_RET] = 2, [FLAG_EMA] = 4, [FLAG_CRC] = 6,
    [FLAG_SIN] = 24, [FLAG_TORO] = 8, [FLAG_GATE] = 18, [FLAG_NEON] = 4,
    [FLAG_HEX] = 0, [FLAG_ASM] = 1, [FLAG_LABEL] = 0, [FLAG_BRANCH] = 2,
    [FLAG_NOP] = 1,
};

static u32 compute_cycle_estimate(const u8 *bytecode, u32 len) {
    u32 cycles = 0;
    for (u32 i = 0; i + 3 < len; i += 4) {
        u32 instr = (bytecode[i] << 24) | (bytecode[i+1] << 16) | (bytecode[i+2] << 8) | bytecode[i+3];
        u32 prime = (instr >> 24) & 0xFF;
        if (prime < 256) cycles += latency_table[prime];
        else cycles += 4; /* fallback */
    }
    return cycles;
}

/* ── Eliminação de NOPs (não-destrutiva) ──────────────────────────────── */
static u32 optimize_nops(const u8 *in, u32 in_len, u8 *out, u32 out_cap) {
    u32 wr = 0;
    for (u32 i = 0; i + 3 < in_len; i += 4) {
        u32 instr = (in[i] << 24) | (in[i+1] << 16) | (in[i+2] << 8) | in[i+3];
        u32 prime = (instr >> 24) & 0xFF;
        if (prime != FLAG_NOP) {
            if (wr + 4 > out_cap) return 0;
            out[wr] = in[i]; out[wr+1] = in[i+1]; out[wr+2] = in[i+2]; out[wr+3] = in[i+3];
            wr += 4;
        }
    }
    return wr;
}

/* ── Montagem final: buffer executável ────────────────────────────────── */
static u8 exec_buffer[4096] __attribute__((aligned(64)));

static i32 assemble_final(const u8 *bytecode, u32 len, u32 cycle_count) {
    u32 off = 0;
    if (off + 2 > sizeof(exec_buffer)) return -1;
    exec_buffer[off++] = 0xB5; exec_buffer[off++] = 0xF0; /* push {r4-r7,lr} */
    for (u32 i = 0; i + 3 < len && off + 4 <= sizeof(exec_buffer); i += 4) {
        exec_buffer[off++] = bytecode[i];
        exec_buffer[off++] = bytecode[i+1];
        exec_buffer[off++] = bytecode[i+2];
        exec_buffer[off++] = bytecode[i+3];
    }
    if (off + 2 > sizeof(exec_buffer)) return -1;
    exec_buffer[off++] = 0xBD; exec_buffer[off++] = 0x70; /* pop {r4-r7,pc} */
    return off;
}

/* ── Execução segura com mprotect e cache flush ────────────────────── */
static void run_secure_execution(u8 *buf, u32 len) {
    long page_size = sysconf(_SC_PAGESIZE);
    uptr start = (uptr)buf;
    uptr end = (uptr)buf + len;
    uptr aligned_start = start & ~(page_size - 1);
    uptr aligned_end = (end + page_size - 1) & ~(page_size - 1);
    u32 aligned_len = (u32)(aligned_end - aligned_start);

    if (mprotect((void*)aligned_start, aligned_len, PROT_READ | PROT_EXEC) == -1) {
        perror("mprotect");
        return;
    }
    __builtin___clear_cache((char*)aligned_start, (char*)aligned_end);
    void (*fn)(void) = (void (*)(void))(void*)buf;
    fn();
}

/* ── Ponto de entrada principal (teste) ──────────────────────────────────── */
int main(void) {
    /* Simula bytecode: LOAD r0,42; ADD r0,r0,1; RET */
    u8 bytecode[] = {
        0x02, 0x00, 0x2A, 0x00,   /* LOAD r0, #42 */
        0x05, 0x00, 0x00, 0x00,   /* ADD r0, r0, #1 (simplificado) */
        0x43, 0x00, 0x00, 0x00,   /* RET */
    };
    u32 len = sizeof(bytecode);

    /* Otimiza (remove NOPs) */
    u8 opt_buf[4096];
    u32 opt_len = optimize_nops(bytecode, len, opt_buf, sizeof(opt_buf));
    if (opt_len == 0) {
        fprintf(stderr, "Falha na otimização\n");
        return 1;
    }

    /* Monta */
    i32 final_len = assemble_final(opt_buf, opt_len, 42);
    if (final_len < 0) {
        fprintf(stderr, "Falha na montagem\n");
        return 1;
    }

    /* Executa com segurança */
    run_secure_execution(exec_buffer, final_len);
    return 0;
}
