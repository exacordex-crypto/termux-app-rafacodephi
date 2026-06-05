#ifndef ZIPRAF_INDEX_H
#define ZIPRAF_INDEX_H
/**
 * zipraf_index.h — ZIPRAF Deterministic Manifesto Matrix Index
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Implements the "Manifesto Matrix Index" from the RAFAELIA architecture:
 *
 *   mode | mod_id | k | offset | len | policy
 *
 * Core principle (from image: "NOT compression"):
 *   Physical bytes in the ZIP container are NEVER modified.
 *   The manifesto adds LOGICAL structure — 8 reading modes × 33 density
 *   levels — over the same physical data.  Like DNA codons over atoms:
 *   the atoms (bytes) are identical; the codon table (manifesto) gives
 *   them meaning at different resolutions.
 *
 *   Logical capacity = physical_size × 8 × 33 = physical_size × 264.
 *   For a 1 GB ZIP → 264 GB of addressable logical space.
 *   For a 1 TB ZIP → 264 TB  (→ "1000 TB+" design target from image).
 *
 * Geometric Coherence Theorem (image: "Teorema da Coerência Geométrica"):
 *   ∀ k ∈ n, G / Sk is reconstructible.
 *   Implemented via ZR_FLAG_REDUNDANT + ZR_MODE_DIRECT fallback.
 *
 * Zero malloc. Static array of entries. write() I/O. CRC32C inline.
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

/* ── 8 Reading Modes (image: "8 MODES OF READING") ─────────────────────── */
typedef enum {
    ZR_MODE_DIRECT   = 0, /* direta: 1:1 offset → data */
    ZR_MODE_MEMORIA  = 1, /* in-memory blob overlay */
    ZR_MODE_LEETRA   = 2, /* character / symbol keyed */
    ZR_MODE_ORBITAL  = 3, /* harmonic / frequency indexed */
    ZR_MODE_TOROIDAL = 4, /* toroidal wrap addressing */
    ZR_MODE_SIGIL    = 5, /* sigil-keyed (IA_SIGILS control plane) */
    ZR_MODE_FRACTAL  = 6, /* fractal subdivision */
    ZR_MODE_ENTROPIC = 7, /* entropy-ordered blocks */
    ZR_MODES         = 8
} ZrMode;

/* ── 33 Density Levels: Size ↔ Granularity (from image) ────────────────── */
/* level  1 = coarsest: one entry covers the whole ZIP */
/* level 33 = finest:   one entry ≈ one 4 KB block */
#define ZR_DENSITY_MIN   1u
#define ZR_DENSITY_MAX   33u

/* ── Access Policy ──────────────────────────────────────────────────────── */
typedef enum {
    ZR_POL_DIRETA    = 0, /* direct read — default */
    ZR_POL_READONLY  = 1, /* immutable region */
    ZR_POL_OVERLAY   = 2, /* redundant overlay (coherence guarantee) */
    ZR_POL_SIGIL_KEY = 3, /* requires sigil intent unlock */
    ZR_POL_COUNT     = 4
} ZrPolicy;

/* ── Entry flags ────────────────────────────────────────────────────────── */
#define ZR_FLAG_VALID      0x01u  /* entry is live */
#define ZR_FLAG_BAD        0x02u  /* CTI bad-event detected in region */
#define ZR_FLAG_REDUNDANT  0x04u  /* redundant overlay copy exists */
#define ZR_FLAG_BOOT       0x08u  /* part of boot / policy table */

/* ── Manifesto Entry — 28 bytes, 4-byte aligned ─────────────────────────── */
typedef struct __attribute__((packed, aligned(4))) {
    uint8_t  mode;      /* ZrMode 0-7 */
    uint8_t  density;   /* density level 1-33 */
    uint16_t mod_id;    /* module id (from image: 00987) */
    uint32_t k;         /* dimension key (from image: 23) */
    uint64_t offset;    /* physical byte offset inside ZIP */
    uint32_t len;       /* logical data length in bytes */
    uint8_t  policy;    /* ZrPolicy */
    uint8_t  flags;     /* ZR_FLAG_* */
    uint16_t ext;       /* extension: sigil id, orbital freq, etc. */
    uint32_t crc32;     /* CRC32C of this entry (excl. crc32 field) */
} ZrEntry;              /* 28 bytes */

/* ── Manifesto — static capacity ────────────────────────────────────────── */
/* 8 modes × 33 densities × 8 entries/cell = 2112 max entries */
#define ZR_MAX_ENTRIES  (ZR_MODES * ZR_DENSITY_MAX * 8u)

typedef struct {
    ZrEntry  entries[ZR_MAX_ENTRIES];
    uint32_t n_entries;
    uint32_t manifest_crc;   /* CRC32C of all valid entries */
    uint8_t  n_modes;        /* always ZR_MODES = 8 */
    uint8_t  n_densities;    /* always ZR_DENSITY_MAX = 33 */
    uint16_t version;
    uint64_t zip_size;       /* physical size of ZIP container */
    char     zip_path[64];   /* null-terminated path */
} ZrManifest;

/* ── Public API ─────────────────────────────────────────────────────────── */

/* Initialise manifest with ZIP path and physical file size. */
void     zr_init(ZrManifest *m, const char *zip_path, uint64_t zip_size);

/* Add a single entry.  Returns 0 on success, -1 if table is full or args
 * are out of range. */
int      zr_add(ZrManifest *m, ZrMode mode, uint8_t density,
                uint16_t mod_id, uint32_t k,
                uint64_t offset, uint32_t len, ZrPolicy policy);

/* Exact lookup by (mode, density, mod_id, k). Returns NULL if not found. */
ZrEntry *zr_lookup(ZrManifest *m, ZrMode mode, uint8_t density,
                   uint16_t mod_id, uint32_t k);

/* Lookup by physical (offset, len) — useful for reverse mapping. */
ZrEntry *zr_lookup_by_offset(ZrManifest *m, uint64_t offset, uint32_t len);

/* Verify CRC of every entry and the manifest itself.
 * Returns 1 if intact, 0 if corrupted. */
int      zr_verify(ZrManifest *m);

/* Auto-generate a full manifesto for a ZIP of given size and base block
 * size.  Fills all 8 modes × 33 density levels. */
void     zr_auto_index(ZrManifest *m, uint64_t zip_size, uint32_t block_size);

/* Print manifesto to stdout (write() only). */
void     zr_print(const ZrManifest *m);

#endif /* ZIPRAF_INDEX_H */
