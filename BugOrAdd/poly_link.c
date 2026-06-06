/**
 * poly_link.c — Linker de Resolução de Símbolos
 * Converte bytecode com labels em binário executável com fixups.
 */
#include "statecomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define MAX_SYMBOLS 256
#define EXEC_BUF_SIZE (1024 * 1024)

typedef struct {
    char *name;   /* strdup'd — must be free'd via link_cleanup */
    u32 offset;
    u32 defined;
} Symbol;

typedef struct {
    const u8 *bytecode;
    u32 bytecode_len;
    u8 *out_buf;
    u32 out_len;
    Symbol symbols[MAX_SYMBOLS];
    u32 symbol_count;
} LinkContext;

static void link_init(LinkContext *ctx, const u8 *bytecode, u32 len) {
    ctx->bytecode = bytecode;
    ctx->bytecode_len = len;
    ctx->out_len = 0;
    ctx->symbol_count = 0;
    ctx->out_buf = NULL;
}

static void link_cleanup(LinkContext *ctx) {
    for (u32 i = 0; i < ctx->symbol_count; i++) {
        free(ctx->symbols[i].name);
        ctx->symbols[i].name = NULL;
    }
    ctx->symbol_count = 0;
    free(ctx->out_buf);
    ctx->out_buf = NULL;
}

static u32 link_add_symbol(LinkContext *ctx, const char *name, u32 offset) {
    for (u32 i = 0; i < ctx->symbol_count; i++) {
        if (strcmp(ctx->symbols[i].name, name) == 0) {
            ctx->symbols[i].offset = offset;
            ctx->symbols[i].defined = 1;
            return i;
        }
    }
    if (ctx->symbol_count >= MAX_SYMBOLS) return 0xFFFFFFFF;
    ctx->symbols[ctx->symbol_count].name = strdup(name);
    ctx->symbols[ctx->symbol_count].offset = offset;
    ctx->symbols[ctx->symbol_count].defined = 1;
    return ctx->symbol_count++;
}

static u32 link_find_symbol(LinkContext *ctx, const char *name) {
    for (u32 i = 0; i < ctx->symbol_count; i++) {
        if (ctx->symbols[i].defined && strcmp(ctx->symbols[i].name, name) == 0)
            return ctx->symbols[i].offset;
    }
    return 0xFFFFFFFF;
}

static i32 link_run(LinkContext *ctx) {
    ctx->out_buf = (u8*)malloc(EXEC_BUF_SIZE);
    if (!ctx->out_buf) return -1;
    u8 *bp = ctx->out_buf;

    /* Passo 1: coleta símbolos */
    const u8 *pc = ctx->bytecode;
    while ((u32)(pc - ctx->bytecode) < ctx->bytecode_len) {
        u32 instr = (pc[0] << 24) | (pc[1] << 16) | (pc[2] << 8) | pc[3];
        u32 prime = (instr >> 24) & 0xFF;
        if (prime == FLAG_LABEL) {
            const char *name = (const char*)(pc + 4);
            link_add_symbol(ctx, name, (u32)(bp - ctx->out_buf));
            pc += 4 + strlen(name) + 1;
        } else {
            bp[0] = pc[0]; bp[1] = pc[1]; bp[2] = pc[2]; bp[3] = pc[3];
            bp += 4;
            pc += 4;
        }
    }

    /* Passo 2: resolve referências */
    bp = ctx->out_buf;
    pc = ctx->bytecode;
    while ((u32)(pc - ctx->bytecode) < ctx->bytecode_len) {
        u32 instr = (pc[0] << 24) | (pc[1] << 16) | (pc[2] << 8) | pc[3];
        u32 prime = (instr >> 24) & 0xFF;
        if (prime == FLAG_BRANCH) {
            const char *name = (const char*)(pc + 4);
            u32 target = link_find_symbol(ctx, name);
            if (target == 0xFFFFFFFF) { link_cleanup(ctx); return -1; }
            i32 offset = (i32)(target - ((u32)(bp - ctx->out_buf) + 4));
            /* Corrige os bytes da instrução (branch Thumb) */
            bp[0] = pc[0]; bp[1] = pc[1];
            bp[2] = (u8)((offset >> 8) & 0xFF); bp[3] = (u8)(offset & 0xFF);
            pc += 4 + strlen(name) + 1;
        } else if (prime != FLAG_LABEL) {
            /* Instrução normal */
            bp[0] = pc[0]; bp[1] = pc[1]; bp[2] = pc[2]; bp[3] = pc[3];
            pc += 4;
        } else {
            pc += 4 + strlen((const char*)(pc + 4)) + 1;
        }
        bp += 4;
    }
    ctx->out_len = (u32)(bp - ctx->out_buf);
    return 0;
}

int main(void) {
    /* Exemplo de bytecode com label */
    u8 bytecode[] = {
        0x02, 0x00, 0x2A, 0x00,           /* LOAD r0, #42 */
        0x6D, 'l', 'o', 'o', 'p', 0,      /* LABEL loop */
        0x71, 'l', 'o', 'o', 'p', 0,      /* BRANCH loop */
        0x43, 0x00, 0x00, 0x00,           /* RET */
    };
    LinkContext ctx;
    link_init(&ctx, bytecode, sizeof(bytecode));
    if (link_run(&ctx) == 0) {
        printf("Link OK, out_len=%u\n", ctx.out_len);
        /* Executa com segurança */
        long page_size = sysconf(_SC_PAGESIZE);
        uptr start = (uptr)ctx.out_buf;
        uptr end = start + ctx.out_len;
        uptr aligned_start = start & ~(page_size - 1);
        uptr aligned_end = (end + page_size - 1) & ~(page_size - 1);
        u32 aligned_len = (u32)(aligned_end - aligned_start);
        if (mprotect((void*)aligned_start, aligned_len, PROT_READ | PROT_EXEC) == 0) {
            __builtin___clear_cache((char*)aligned_start, (char*)aligned_end);
            void (*fn)(void) = (void (*)(void))(void*)ctx.out_buf;
            fn();
        } else {
            perror("mprotect");
        }
        link_cleanup(&ctx);
    } else {
        fprintf(stderr, "Link falhou\n");
        link_cleanup(&ctx);
    }
    return 0;
}
