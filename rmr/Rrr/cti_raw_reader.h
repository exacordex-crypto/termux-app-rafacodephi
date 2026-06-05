#ifndef CTI_RAW_READER_H
#define CTI_RAW_READER_H
/**
 * cti_raw_reader.h — CTI BITSTACK: deterministic multi-mode raw file scanner
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Reads any file (RAW, JPEG, GIF, PNG, ZIP…) as a raw byte stream,
 * partitions it into fixed-size blocks and builds a deterministic index:
 *
 *   idx | size | ts | fid_crc32 | entropy | flags | xbad | miss_score
 *
 * Scan modes (from image: READERS = SEQ, SPIRAL, TOROID, RANDOM_PERM):
 *   CTI_SEQ         — sequential 0, 1, 2, …
 *   CTI_SPIRAL      — 2-D spiral traversal from centre of block grid
 *   CTI_TOROID      — toroidal stride, stride = gcd-coprime(n_blocks)
 *   CTI_RANDOM_PERM — xorshift64 deterministic permutation keyed by seed
 *   CTI_DELTA_MISS  — SEQ + live miss-score vs chain-CRC expectation
 *
 * Zero malloc. Static arrays. write(1,…) output. CRC32C inline.
 * ARM32 / ARM64 / x86_64 portable C11.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

/* ── Format detection flags ────────────────────────────────────────────── */
#define CTI_FMT_RAW   0x00u
#define CTI_FMT_JPEG  0x01u   /* SOI = FF D8 */
#define CTI_FMT_GIF   0x02u   /* GIF8x */
#define CTI_FMT_PNG   0x04u   /* 89 50 4E 47 */
#define CTI_FMT_ZIP   0x08u   /* PK 03 04 */

/* ── Scan modes ─────────────────────────────────────────────────────────── */
typedef enum {
    CTI_SEQ         = 0,
    CTI_SPIRAL      = 1,
    CTI_TOROID      = 2,
    CTI_RANDOM_PERM = 3,
    CTI_DELTA_MISS  = 4,
    CTI_MODE_COUNT  = 5
} CtiMode;

/* ── Per-block index entry (26 bytes packed) ────────────────────────────── */
typedef struct __attribute__((packed)) {
    uint32_t idx;        /* physical block index */
    uint32_t size;       /* bytes read from this block */
    uint64_t ts;         /* logical scan counter (monotonic) */
    uint32_t fid_crc32;  /* CRC32C of block bytes */
    uint32_t entropy;    /* Shannon entropy × 1000  (0 = uniform; 8000 = max) */
    uint8_t  flags;      /* CTI_FMT_* of detected file format */
    uint8_t  xbad;       /* saturated count of bad-byte run events */
    int16_t  miss_score; /* DELTA_MISS: signed deviation from expected chain */
} CtiEntry;              /* 26 bytes */

/* ── Scanner context ────────────────────────────────────────────────────── */
#define CTI_MAX_ENTRIES  1024u
#define CTI_BLOCK_SIZE   4096u

typedef struct {
    CtiMode   mode;
    uint32_t  seed;              /* RANDOM_PERM seed */
    uint32_t  block_size;        /* bytes per block */
    uint64_t  file_size;         /* total file bytes */
    uint32_t  n_blocks;          /* ceil(file_size / block_size) */
    uint32_t  n_entries;         /* filled entries */
    uint32_t  total_bad;         /* sum of xbad across all blocks */
    uint32_t  chain_crc;         /* CRC32C chained across all scanned blocks */
    uint8_t   file_fmt;          /* CTI_FMT_* detected at byte 0 */
    uint8_t   _pad[3];
    CtiEntry  entries[CTI_MAX_ENTRIES];
} CtiScanner;

/* ── Public API ─────────────────────────────────────────────────────────── */

/* Scan open file descriptor fd with given mode and seed.
 * Fills sc->entries[]. Returns 0 on success, -1 on error. */
int     cti_scan_fd(CtiScanner *sc, int fd, CtiMode mode, uint32_t seed);

/* Print human-readable report to stdout (write() only). */
void    cti_print_report(const CtiScanner *sc);

/* Detect file format from first hdr_len bytes (≥ 4 needed). */
uint8_t cti_detect_fmt(const uint8_t *hdr, uint32_t hdr_len);

/* Estimate Shannon entropy of buf[0..len-1], scaled × 1000. */
uint32_t cti_entropy(const uint8_t *buf, uint32_t len);

#endif /* CTI_RAW_READER_H */
