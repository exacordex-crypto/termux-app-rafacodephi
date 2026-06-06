/**
 * bitstack.c — 3D (X,Y,D) bit-stack array — zero-malloc static-pool version
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Singleton: only one BitStacks may be active at a time.
 * Pool: BS_POOL_WORDS × 64 bits = 524 288 bits total capacity.
 */
#include "bitstack.h"
#include <string.h>

static uint64_t _bs_pool[BS_POOL_WORDS];
static BitStacks _bs_singleton;
static uint8_t   _bs_active = 0u;

BitStacks *bitstacks_create(int X, int Y, int D, int bits_per_stack) {
    if (_bs_active) return NULL;
    if (X <= 0 || Y <= 0 || D <= 0 || bits_per_stack <= 0) return NULL;

    int    bps      = (bits_per_stack + 63) / 64;
    size_t n_stacks = (size_t)X * (size_t)Y * (size_t)D;
    size_t n_words  = n_stacks * (size_t)bps;
    if (n_words > BS_POOL_WORDS) return NULL;

    _bs_singleton.X                = X;
    _bs_singleton.Y                = Y;
    _bs_singleton.D                = D;
    _bs_singleton.bits_per_stack   = bits_per_stack;
    _bs_singleton.blocks_per_stack = bps;
    _bs_singleton.blocks           = _bs_pool;
    memset(_bs_pool, 0, n_words * sizeof(uint64_t));
    _bs_active = 1u;
    return &_bs_singleton;
}

void bitstacks_free(BitStacks *bs) {
    if (!bs || bs != &_bs_singleton) return;
    _bs_active = 0u;
}

size_t bitstacks_index(const BitStacks *bs, int x, int y, int d, int block_idx) {
    size_t stack = ((size_t)x * (size_t)bs->Y + (size_t)y) * (size_t)bs->D + (size_t)d;
    return stack * (size_t)bs->blocks_per_stack + (size_t)block_idx;
}

void bitstacks_set(BitStacks *bs, int x, int y, int d, int stack_idx, int value) {
    if (!bs || stack_idx < 0 || stack_idx >= bs->bits_per_stack) return;
    if (x < 0 || x >= bs->X || y < 0 || y >= bs->Y || d < 0 || d >= bs->D) return;
    int      blk    = stack_idx / 64;
    int      offset = stack_idx % 64;
    size_t   bi     = bitstacks_index(bs, x, y, d, blk);
    uint64_t mask   = (uint64_t)1u << offset;
    if (value) bs->blocks[bi] |=  mask;
    else       bs->blocks[bi] &= ~mask;
}

int bitstacks_get(BitStacks *bs, int x, int y, int d, int stack_idx) {
    if (!bs || stack_idx < 0 || stack_idx >= bs->bits_per_stack) return 0;
    if (x < 0 || x >= bs->X || y < 0 || y >= bs->Y || d < 0 || d >= bs->D) return 0;
    int    blk    = stack_idx / 64;
    int    offset = stack_idx % 64;
    size_t bi     = bitstacks_index(bs, x, y, d, blk);
    return (int)((bs->blocks[bi] >> offset) & 1u);
}
