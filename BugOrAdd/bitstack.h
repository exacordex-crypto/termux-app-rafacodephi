#ifndef BITSTACK_H
#define BITSTACK_H

#include <stdint.h>
#include <stddef.h>

/* Static pool — 64 KB of uint64_t words = 524 288 bits.
 * Increase BS_POOL_WORDS if larger grids are needed. */
#define BS_POOL_WORDS  8192u

typedef struct {
    int      X, Y, D;
    int      bits_per_stack;
    int      blocks_per_stack; /* ceil(bits_per_stack / 64) */
    uint64_t *blocks;          /* points into static pool */
} BitStacks;

/* Initialise the singleton. Returns NULL if dimensions exceed pool or
 * one instance is already active. */
BitStacks *bitstacks_create(int X, int Y, int D, int bits_per_stack);

/* Release the singleton (marks pool as available for reuse). */
void bitstacks_free(BitStacks *bs);

/* Set / get a single bit inside cell (x,y,d) at position stack_idx. */
void bitstacks_set(BitStacks *bs, int x, int y, int d, int stack_idx, int value);
int  bitstacks_get(BitStacks *bs, int x, int y, int d, int stack_idx);

/* Return flat pool index for debugging / direct access. */
size_t bitstacks_index(const BitStacks *bs, int x, int y, int d, int block_idx);

#endif /* BITSTACK_H */
