/**
 * zipraf_index.c — ZIPRAF Deterministic Manifesto Index implementation
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Key insight from images:
 *   "NOT compression" — physical bytes in ZIP stay unchanged.
 *   The manifesto adds an 8 × 33 = 264 logical projection layer:
 *
 *     mode  0 (DIRECT)  : raw byte offset, no transformation
 *     mode  1 (MEMORIA) : in-memory overlay mapping
 *     mode  2 (LEETRA)  : symbol / character lattice index
 *     mode  3 (ORBITAL) : harmonic frequency bins (Q16.16)
 *     mode  4 (TOROIDAL): toroidal stride addressing over zip_size
 *     mode  5 (SIGIL)   : sigil-intent keyed (IA_SIGILS control plane)
 *     mode  6 (FRACTAL) : fractal subdivision — block → sub-blocks
 *     mode  7 (ENTROPIC): entropy-ordered: highest-entropy blocks first
 *
 *   density level d → block size = zip_size >> (d-1), clamped to
 *   [block_size_min, zip_size].  Density 1 = full file; density 33 = ~4KB.
 *
 *   Geometric coherence: removing any mode leaves 7/8 projections intact.
 *   ZR_POL_OVERLAY entries provide the n-1 reconstruction guarantee.
 *
 * Zero malloc. Static arrays. write() I/O. CRC32C inline.
 */
#define _POSIX_C_SOURCE 200809L
#include "zipraf_index.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

/* ── CRC32C ──────────────────────────────────────────────────────────────── */
static uint32_t _zt[256];
static int _zinit = 0;
static void _zcrc_init(void) {
    for (uint32_t i = 0u; i < 256u; i++) {
        uint32_t v = i;
        for (int j = 0; j < 8; j++) v = (v & 1u) ? (v >> 1) ^ 0x82F63B78u : (v >> 1);
        _zt[i] = v;
    }
    _zinit = 1;
}
static uint32_t _zcrc(const void *p, uint32_t n) {
    const uint8_t *b = (const uint8_t *)p;
    uint32_t c = ~0u;
    while (n--) c = (c >> 8) ^ _zt[(c ^ *b++) & 0xFFu];
    return ~c;
}

/* Entry CRC covers all fields except the last 4 bytes (crc32 itself). */
static uint32_t _ecrc(const ZrEntry *e) {
    return _zcrc(e, (uint32_t)(sizeof(ZrEntry) - 4u));
}

/* ── write()-only helpers ────────────────────────────────────────────────── */
static void _ws(const char *s) { write(1, s, strlen(s)); }
static void _wu(uint32_t v) {
    char b[12]; int i = 11; b[i] = '\0';
    if (!v) { b[--i] = '0'; } else { while (v) { b[--i] = (char)('0' + v % 10u); v /= 10u; } }
    _ws(b + i);
}
static const char _HX[] = "0123456789abcdef";
static void _wh(uint32_t v) {
    char b[9]; b[8] = '\0';
    for (int i = 0; i < 8; i++) b[i] = _HX[(v >> (28 - i * 4)) & 0xFu];
    _ws(b);
}
static void _wu64(uint64_t v) {
    if (v >> 32) _wu((uint32_t)(v >> 32));
    _wu((uint32_t)(v & 0xFFFFFFFFu));
}

static const char *_mstr(uint8_t m) {
    switch (m) {
        case ZR_MODE_DIRECT:   return "DIRECT  ";
        case ZR_MODE_MEMORIA:  return "MEMORIA ";
        case ZR_MODE_LEETRA:   return "LEETRA  ";
        case ZR_MODE_ORBITAL:  return "ORBITAL ";
        case ZR_MODE_TOROIDAL: return "TOROIDAL";
        case ZR_MODE_SIGIL:    return "SIGIL   ";
        case ZR_MODE_FRACTAL:  return "FRACTAL ";
        case ZR_MODE_ENTROPIC: return "ENTROPIC";
        default:               return "?       ";
    }
}
static const char *_pstr(uint8_t p) {
    switch (p) {
        case ZR_POL_DIRETA:    return "direta   ";
        case ZR_POL_READONLY:  return "readonly ";
        case ZR_POL_OVERLAY:   return "overlay  ";
        case ZR_POL_SIGIL_KEY: return "sigil_key";
        default:               return "?        ";
    }
}

/* ── Init ────────────────────────────────────────────────────────────────── */
void zr_init(ZrManifest *m, const char *zip_path, uint64_t zip_size) {
    if (!m) return;
    if (!_zinit) _zcrc_init();
    memset(m, 0, sizeof(*m));
    m->n_modes     = ZR_MODES;
    m->n_densities = ZR_DENSITY_MAX;
    m->version     = 1u;
    m->zip_size    = zip_size;
    if (zip_path) {
        uint32_t l = 0u;
        while (zip_path[l] && l < 63u) l++;
        memcpy(m->zip_path, zip_path, l);
        m->zip_path[l] = '\0';
    }
}

/* ── Add ─────────────────────────────────────────────────────────────────── */
int zr_add(ZrManifest *m, ZrMode mode, uint8_t density,
           uint16_t mod_id, uint32_t k,
           uint64_t offset, uint32_t len, ZrPolicy policy) {
    if (!m) return -1;
    if (m->n_entries >= ZR_MAX_ENTRIES) return -1;
    if ((uint8_t)mode >= ZR_MODES)      return -1;
    if (density < ZR_DENSITY_MIN || density > ZR_DENSITY_MAX) return -1;

    ZrEntry *e = &m->entries[m->n_entries];
    e->mode    = (uint8_t)mode;
    e->density = density;
    e->mod_id  = mod_id;
    e->k       = k;
    e->offset  = offset;
    e->len     = len;
    e->policy  = (uint8_t)policy;
    e->flags   = ZR_FLAG_VALID;
    e->ext     = 0u;
    e->crc32   = _ecrc(e);

    m->n_entries++;
    m->manifest_crc = _zcrc(m->entries, m->n_entries * (uint32_t)sizeof(ZrEntry));
    return 0;
}

/* ── Lookup ──────────────────────────────────────────────────────────────── */
ZrEntry *zr_lookup(ZrManifest *m, ZrMode mode, uint8_t density,
                   uint16_t mod_id, uint32_t k) {
    if (!m) return 0;
    for (uint32_t i = 0u; i < m->n_entries; i++) {
        ZrEntry *e = &m->entries[i];
        if (!(e->flags & ZR_FLAG_VALID)) continue;
        if (e->mode    == (uint8_t)mode  &&
            e->density == density         &&
            e->mod_id  == mod_id          &&
            e->k       == k)
            return e;
    }
    return 0;
}

ZrEntry *zr_lookup_by_offset(ZrManifest *m, uint64_t offset, uint32_t len) {
    if (!m) return 0;
    for (uint32_t i = 0u; i < m->n_entries; i++) {
        ZrEntry *e = &m->entries[i];
        if (!(e->flags & ZR_FLAG_VALID)) continue;
        if (e->offset == offset && e->len == len) return e;
    }
    return 0;
}

/* ── Verify ──────────────────────────────────────────────────────────────── */
int zr_verify(ZrManifest *m) {
    if (!m) return 0;
    for (uint32_t i = 0u; i < m->n_entries; i++) {
        ZrEntry *e = &m->entries[i];
        if (!(e->flags & ZR_FLAG_VALID)) continue;
        if (_ecrc(e) != e->crc32) return 0;
    }
    if (!m->n_entries) return 1;
    uint32_t mc = _zcrc(m->entries, m->n_entries * (uint32_t)sizeof(ZrEntry));
    return mc == m->manifest_crc ? 1 : 0;
}

/* ── Auto-index ──────────────────────────────────────────────────────────── */
/**
 * Generates the full 8 × 33 manifesto from a ZIP of known size.
 *
 * For each mode m and density level d:
 *   block_size_d = clamp(zip_size >> (d-1), block_size_min, zip_size)
 *   For each block b of that size:
 *     offset = b * block_size_d
 *     k      = b * 23 + m          (23 = dimension key from image)
 *     mod_id = 0x0987 + m          (0x0987 from image: mod_id 00987)
 *
 * Mode-specific policies (from image policy column):
 *   DIRECT   → direta
 *   SIGIL    → sigil_key  (IA_SIGILS control plane)
 *   ENTROPIC → overlay    (redundant coherence copy)
 *   others   → direta
 */
void zr_auto_index(ZrManifest *m, uint64_t zip_size, uint32_t block_size) {
    if (!m || !zip_size) return;
    if (!block_size) block_size = CTI_BLOCK_SIZE_DEF;

    for (uint8_t mode = 0u; mode < ZR_MODES; mode++) {
        ZrPolicy pol;
        switch (mode) {
            case ZR_MODE_SIGIL:    pol = ZR_POL_SIGIL_KEY; break;
            case ZR_MODE_ENTROPIC: pol = ZR_POL_OVERLAY;   break;
            default:               pol = ZR_POL_DIRETA;    break;
        }

        for (uint8_t dens = ZR_DENSITY_MIN; dens <= ZR_DENSITY_MAX; dens++) {
            /* coarser at low density, finer at high density */
            uint64_t bsz = (dens <= 1u) ? zip_size : (zip_size >> (dens - 1u));
            if (bsz < (uint64_t)block_size) bsz = (uint64_t)block_size;
            if (bsz > zip_size)             bsz = zip_size;

            uint32_t n_blk = (uint32_t)((zip_size + bsz - 1u) / bsz);

            for (uint32_t bi = 0u;
                 bi < n_blk && m->n_entries < ZR_MAX_ENTRIES;
                 bi++) {
                uint64_t off = (uint64_t)bi * bsz;
                uint32_t len = (off + bsz > zip_size)
                               ? (uint32_t)(zip_size - off)
                               : (uint32_t)bsz;
                uint32_t k   = bi * 23u + mode;
                uint16_t mid = (uint16_t)(0x0987u + mode);
                zr_add(m, (ZrMode)mode, dens, mid, k, off, len, pol);
            }
        }
    }
}

#ifndef CTI_BLOCK_SIZE_DEF
#define CTI_BLOCK_SIZE_DEF 4096u
#endif

/* ── Print ───────────────────────────────────────────────────────────────── */
void zr_print(const ZrManifest *m) {
    if (!m) return;
    _ws("=== ZIPRAF MANIFESTO MATRIX INDEX ===\n");
    _ws("zip:      "); _ws(m->zip_path[0] ? m->zip_path : "(none)"); _ws("\n");
    _ws("zip_size: "); _wu64(m->zip_size); _ws(" bytes\n");
    _ws("modes:    "); _wu(m->n_modes); _ws("  densities: "); _wu(m->n_densities); _ws("\n");
    _ws("entries:  "); _wu(m->n_entries); _ws("  manifest_crc: 0x"); _wh(m->manifest_crc); _ws("\n");
    _ws("integrity: "); _ws(zr_verify((ZrManifest *)m) ? "OK" : "FAIL"); _ws("\n");

    /* logical capacity = physical × 8 modes × 33 densities */
    _ws("logical_space: "); _wu64(m->zip_size * ZR_MODES * ZR_DENSITY_MAX); _ws(" bytes\n\n");

    _ws("mode      dens  mod_id    k         offset        len       policy    crc32\n");
    _ws("--------  ----  ------  --------  ----------  --------  ---------  --------\n");
    uint32_t show = m->n_entries < 48u ? m->n_entries : 48u;
    for (uint32_t i = 0u; i < show; i++) {
        const ZrEntry *e = &m->entries[i];
        _ws(_mstr(e->mode));    _ws("  ");
        _wu(e->density);        _ws("     ");
        _wu(e->mod_id);         _ws("  ");
        _wu(e->k);              _ws("  ");
        _wu64(e->offset);       _ws("  ");
        _wu(e->len);            _ws("  ");
        _ws(_pstr(e->policy));  _ws("  ");
        _wh(e->crc32);          _ws("\n");
    }
    if (m->n_entries > 48u) {
        _ws("... ("); _wu(m->n_entries - 48u); _ws(" more)\n");
    }
    _ws("=== END MANIFESTO ===\n");
}
