/**
 * bitstack.h — 3D (X,Y,D) bit-stack array — zero-malloc static-pool version
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Provides an X×Y×D grid of bit-stacks.  Each cell (x,y,d) holds
 * `bits_per_stack` bits addressable by index 0…bits_per_stack-1.
 *
 * Zero malloc: backed by a single static uint64_t pool.
 * Only one BitStacks instance may be active at a time (singleton pattern).
 * Maximum capacity: BS_POOL_WORDS × 64 bits total across all cells.
 */
#ifndef BITSTACK_H
#define BITSTACK_H

#include <stdint.h>
#include <stddef.h>

/* Static pool — 64 KB of uint64_t words = 524 288 bits.
 * Covers 10×10×10×64 = 640 000 bits when blocks_per_stack=1 per cell.
 * Increase if larger grids are needed. */
#define BS_POOL_WORDS  8192u

typedef struct {
    int      X, Y, D;
    int      bits_per_stack;
    int      blocks_per_stack;  /* ceil(bits_per_stack / 64) */
    uint64_t *blocks;           /* points into static pool */
} BitStacks;

/* Initialise the singleton.  Returns a pointer on success, NULL if the
 * requested dimensions exceed BS_POOL_WORDS or one is already active. */
BitStacks *bitstacks_create(int X, int Y, int D, int bits_per_stack);

/* Release the singleton (marks pool as available for reuse). */
void bitstacks_free(BitStacks *bs);

/* Set / get a single bit inside cell (x,y,d) at position stack_idx. */
void bitstacks_set(BitStacks *bs, int x, int y, int d, int stack_idx, int value);
int  bitstacks_get(BitStacks *bs, int x, int y, int d, int stack_idx);

/* Return flat pool index for debugging / direct access. */
size_t bitstacks_index(const BitStacks *bs, int x, int y, int d, int block_idx);

#endif /* BITSTACK_H */
