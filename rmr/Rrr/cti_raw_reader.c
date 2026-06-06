/**
 * cti_raw_reader.c — CTI BITSTACK: deterministic multi-mode raw scanner
 * SPDX-License-Identifier: GPL-3.0-only
 *
 * Architecture:
 *   - Any file is a raw byte stream; format (JPEG/GIF/PNG/ZIP/RAW) is
 *     detected from the magic header but bytes are always read at block
 *     granularity (CTI_BLOCK_SIZE = 4096 bytes).
 *   - Each block produces one index entry: idx, size, ts, fid_crc32,
 *     entropy, flags, xbad, miss_score.
 *   - Five traversal modes give different orderings of the same blocks:
 *       SEQ          — baseline, block 0…N-1
 *       SPIRAL       — 2-D counter-clockwise spiral from grid centre
 *       TOROID       — coprime-stride walk; gcd(stride,N)=1 → full cover
 *       RANDOM_PERM  — xorshift64 pseudo-random mapping keyed by seed
 *       DELTA_MISS   — SEQ + live miss-score vs expected chain-CRC
 *
 * Zero malloc. Static arrays only. write(1,…) I/O. CRC32C inline.
 */
#define _POSIX_C_SOURCE 200809L
#include "cti_raw_reader.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/* ── CRC32C (Castagnoli, reflected polynomial 0x82F63B78) ───────────────── */
static uint32_t _crc_tbl[256];
static int _crc_ready = 0;
static void _crc_init(void) {
    for (uint32_t i = 0u; i < 256u; i++) {
        uint32_t v = i;
        for (int j = 0; j < 8; j++) v = (v & 1u) ? (v >> 1) ^ 0x82F63B78u : (v >> 1);
        _crc_tbl[i] = v;
    }
    _crc_ready = 1;
}
static uint32_t _crc32c(const uint8_t *p, uint32_t n) {
    uint32_t c = ~0u;
    while (n--) c = (c >> 8) ^ _crc_tbl[(c ^ *p++) & 0xFFu];
    return ~c;
}

/* ── Shannon entropy estimate, scaled × 1000 ────────────────────────────── */
/*
 * H = -Σ p·log₂(p) where p = hist[b]/len.
 * Approximation: for Q16 value p_q16 = (hist[b]<<16)/len,
 *   the number of leading zero bits lz gives log₂(p) ≈ (15-lz) - 16 = -(lz+1)
 *   so -log₂(p) ≈ lz + 1.
 * Special cases: p=0 → skip; p=1 (flat buffer) → contribution 0, skip.
 */
uint32_t cti_entropy(const uint8_t *buf, uint32_t len) {
    if (!len) return 0u;
    uint32_t hist[256];
    memset(hist, 0, sizeof(hist));
    for (uint32_t i = 0u; i < len; i++) hist[buf[i]]++;
    uint32_t e = 0u;
    for (int b = 0; b < 256; b++) {
        if (!hist[b] || hist[b] == len) continue;
        uint32_t p_q16 = (uint32_t)(((uint64_t)hist[b] << 16) / len);
        if (!p_q16) continue;
        /* count leading zeros from bit 15 down */
        uint32_t x = p_q16;
        uint32_t lz = 0u;
        while (!(x & 0x8000u) && lz < 16u) { x <<= 1; lz++; }
        /* -log2(p) ≈ lz + 1 */
        uint32_t neg_log2 = lz + 1u;
        e += (uint32_t)(((uint64_t)hist[b] * neg_log2 * 1000u) / len);
    }
    return e > 8000u ? 8000u : e;
}

/* ── Format detection ───────────────────────────────────────────────────── */
uint8_t cti_detect_fmt(const uint8_t *hdr, uint32_t len) {
    if (!hdr || len < 2u) return CTI_FMT_RAW;
    /* JPEG: FF D8 */
    if (hdr[0] == 0xFFu && hdr[1] == 0xD8u)
        return CTI_FMT_JPEG;
    /* PNG: 89 50 4E 47 0D 0A 1A 0A (check first 4 bytes) */
    if (len >= 4u && hdr[0] == 0x89u && hdr[1] == 0x50u
                  && hdr[2] == 0x4Eu && hdr[3] == 0x47u)
        return CTI_FMT_PNG;
    /* GIF: GIF87a or GIF89a (check 6-byte magic) */
    if (len >= 6u && hdr[0] == 'G' && hdr[1] == 'I' && hdr[2] == 'F'
                  && hdr[3] == '8'
                  && (hdr[4] == '7' || hdr[4] == '9')
                  && hdr[5] == 'a')
        return CTI_FMT_GIF;
    /* ZIP: PK\x03\x04 local-file-header signature */
    if (len >= 4u && hdr[0] == 'P' && hdr[1] == 'K'
                  && hdr[2] == 0x03u && hdr[3] == 0x04u)
        return CTI_FMT_ZIP;
    return CTI_FMT_RAW;
}

/* ── Bad-byte run counter ───────────────────────────────────────────────── */
/* Counts runs of >3 identical boundary bytes (0x00 or 0xFF) as bad events. */
static uint8_t _xbad(const uint8_t *buf, uint32_t len) {
    uint32_t bad = 0u, i = 0u;
    while (i < len) {
        uint8_t b = buf[i];
        if (b == 0x00u || b == 0xFFu) {
            uint32_t j = i;
            while (j < len && buf[j] == b) j++;
            if (j - i > 3u) bad++;
            i = j;
        } else {
            i++;
        }
    }
    return (uint8_t)(bad > 255u ? 255u : bad);
}

/* ── Spiral traversal: i-th block in 2-D counter-clockwise spiral ───────── */
/*
 * Builds a centre-out spiral on a side×side grid (side = ceil(sqrt(n))).
 * Start at (side/2, side/2); alternate segment lengths 1,1,2,2,3,3,…
 * with 90° left turns.  Never clamps — returns fallback i%n for any step
 * that falls outside the grid (outer ring overhang).
 */
static uint32_t _spiral_idx(uint32_t i, uint32_t n) {
    if (!n) return 0u;
    uint32_t side = 1u;
    while (side * side < n) side++;

    int32_t x = (int32_t)(side / 2u);
    int32_t y = (int32_t)(side / 2u);
    if (i == 0u) {
        uint32_t idx = (uint32_t)y * side + (uint32_t)x;
        return (idx < n) ? idx : 0u;
    }

    /* Walk i steps along the spiral from the centre */
    int32_t dx = 1, dy = 0;              /* initial direction: right */
    uint32_t seg_len = 1u;               /* current segment length */
    uint32_t seg_step = 0u;              /* steps taken in this segment */
    uint32_t seg_count = 0u;             /* how many segments completed */

    for (uint32_t k = 0u; k < i; k++) {
        x += dx; y += dy;
        seg_step++;
        if (seg_step == seg_len) {
            /* 90° left turn: (dx,dy) → (-dy,dx) */
            int32_t tmp = dx; dx = -dy; dy = tmp;
            seg_step = 0u;
            seg_count++;
            /* segment length increases every 2 completed segments */
            if ((seg_count & 1u) == 0u) seg_len++;
        }
    }

    /* If outside grid, fall back to modular wrap */
    if (x < 0 || y < 0 || (uint32_t)x >= side || (uint32_t)y >= side)
        return i % n;
    uint32_t idx = (uint32_t)y * side + (uint32_t)x;
    return (idx < n) ? idx : (i % n);
}

/* ── Smallest coprime stride ≥ 2 (for TOROID full coverage) ─────────────── */
static uint32_t _coprime_stride(uint32_t n) {
    if (n <= 2u) return 1u;
    for (uint32_t s = 2u; s < n; s++) {
        uint32_t a = s, b = n;
        while (b) { uint32_t t = b; b = a % b; a = t; }
        if (a == 1u) return s;
    }
    return 1u;
}

/* ── xorshift64 pseudo-random block mapping ─────────────────────────────── */
/*
 * Maps scan index i to a physical block index in [0, n).
 * NOT a bijection — use CTI_TOROID when unique coverage is required.
 * Provides a different traversal order sensitive to (seed, i).
 */
static uint32_t _perm_idx(uint32_t i, uint32_t n, uint32_t seed) {
    /* Explicit parentheses: XOR first, then add the LCG increment */
    uint64_t s = ((uint64_t)seed ^ ((uint64_t)i * 6364136223846793005ULL))
                 + 1442695040888963407ULL;
    s ^= s >> 12; s ^= s << 25; s ^= s >> 27;
    s *= 2685821657736338717ULL;
    return (uint32_t)(s % (uint64_t)n);
}

/* ── Read exactly n bytes from fd ───────────────────────────────────────── */
static uint32_t _read_exact(int fd, uint8_t *buf, uint32_t n) {
    uint32_t done = 0u;
    while (done < n) {
        ssize_t r = read(fd, buf + done, (size_t)(n - done));
        if (r <= 0) break;   /* EOF (0) or I/O error (<0) */
        done += (uint32_t)r;
    }
    return done;
}

/* ── Seek to absolute offset ─────────────────────────────────────────────── */
static int _seek_to(int fd, uint64_t off) {
    off_t r = lseek(fd, (off_t)off, SEEK_SET);
    return (r == (off_t)off) ? 0 : -1;
}

/* ── Main scan ──────────────────────────────────────────────────────────── */
int cti_scan_fd(CtiScanner *sc, int fd, CtiMode mode, uint32_t seed) {
    if (!sc || fd < 0) return -1;
    if (!_crc_ready) _crc_init();

    memset(sc, 0, sizeof(*sc));
    sc->mode       = mode;
    sc->seed       = seed;
    sc->block_size = CTI_BLOCK_SIZE;

    struct stat st;
    if (fstat(fd, &st) < 0) return -1;
    sc->file_size = (uint64_t)st.st_size;
    if (sc->file_size == 0u) return -1;

    sc->n_blocks = (uint32_t)((sc->file_size + CTI_BLOCK_SIZE - 1u) / CTI_BLOCK_SIZE);

    /* detect format from first 8 bytes */
    uint8_t hdr[8];
    uint32_t hdr_got = _read_exact(fd, hdr, 8u);
    sc->file_fmt = cti_detect_fmt(hdr, hdr_got);
    if (_seek_to(fd, 0u) < 0) return -1;

    uint32_t n_scan = sc->n_blocks < CTI_MAX_ENTRIES ? sc->n_blocks : CTI_MAX_ENTRIES;
    uint32_t stride = _coprime_stride(n_scan);

    /* Static block buffer — never on the stack (no stack blow-up). */
    static uint8_t blk[CTI_BLOCK_SIZE];
    uint32_t prev_crc = 0u;
    uint32_t chain    = ~0u;

    for (uint32_t si = 0u; si < n_scan; si++) {
        uint32_t bi;
        switch (mode) {
            case CTI_SEQ:
            case CTI_DELTA_MISS:  bi = si;                                break;
            case CTI_SPIRAL:      bi = _spiral_idx(si, n_scan);           break;
            case CTI_TOROID:      bi = (si * stride) % n_scan;            break;
            case CTI_RANDOM_PERM: bi = _perm_idx(si, n_scan, seed);       break;
            default:              bi = si;
        }

        uint64_t offset = (uint64_t)bi * CTI_BLOCK_SIZE;
        if (_seek_to(fd, offset) < 0) continue;

        uint32_t want = CTI_BLOCK_SIZE;
        if (offset + want > sc->file_size)
            want = (uint32_t)(sc->file_size - offset);

        uint32_t got = _read_exact(fd, blk, want);
        if (got == 0u) continue;

        CtiEntry *e  = &sc->entries[sc->n_entries];
        e->idx        = bi;
        e->size       = got;
        e->ts         = (uint64_t)si;
        e->fid_crc32  = _crc32c(blk, got);
        e->entropy    = cti_entropy(blk, got);
        e->flags      = sc->file_fmt;
        e->xbad       = _xbad(blk, got);
        e->miss_score = 0;

        if (mode == CTI_DELTA_MISS) {
            /* expected: deterministic function of position and seed;
             * use (seed|1) to avoid degenerate zero-multiplication when seed=0 */
            uint32_t expected = ((prev_crc ^ (bi * 0x9E3779B9u)) * (seed | 1u)) & 0xFFFFu;
            int32_t  actual   = (int32_t)(e->fid_crc32 & 0xFFFFu);
            int32_t  delta    = actual - (int32_t)expected;
            if (e->xbad > 0u) delta += (delta < 0) ? -100 : +100;
            e->miss_score = (int16_t)(delta < -32768 ? -32768 : delta > 32767 ? 32767 : delta);
        }

        chain = (chain >> 8) ^ _crc_tbl[(chain ^ (e->fid_crc32 & 0xFFu)) & 0xFFu];

        sc->total_bad += e->xbad;
        prev_crc       = e->fid_crc32;
        sc->n_entries++;
    }
    sc->chain_crc = ~chain;
    return 0;
}

/* ── write()-only output helpers ────────────────────────────────────────── */
static void _ws(const char *s) { write(1, s, strlen(s)); }
static void _wu(uint32_t v) {
    char b[12]; int i = 11; b[i] = '\0';
    if (!v) { b[--i] = '0'; }
    else { while (v) { b[--i] = (char)('0' + v % 10u); v /= 10u; } }
    _ws(b + i);
}
static void _wi(int32_t v) {
    if (v < 0) {
        _ws("-");
        /* avoid UB for INT32_MIN by casting before negation */
        uint32_t u = (v == (int32_t)0x80000000) ? 0x80000000u : (uint32_t)(-v);
        _wu(u);
    } else {
        _wu((uint32_t)v);
    }
}
static const char _HX[] = "0123456789abcdef";
static void _wh(uint32_t v) {
    char b[9]; b[8] = '\0';
    for (int i = 0; i < 8; i++) b[i] = _HX[(v >> (28 - i * 4)) & 0xFu];
    _ws(b);
}
/* Proper 64-bit decimal output (no split-high/low decimal concatenation). */
static void _wu64(uint64_t v) {
    char b[21]; int i = 20; b[i] = '\0';
    if (!v) { b[--i] = '0'; }
    else { while (v) { b[--i] = (char)('0' + (int)(v % 10u)); v /= 10u; } }
    _ws(b + i);
}

static const char *_fmt_name(uint8_t f) {
    switch (f) {
        case CTI_FMT_JPEG: return "JPEG";
        case CTI_FMT_GIF:  return "GIF ";
        case CTI_FMT_PNG:  return "PNG ";
        case CTI_FMT_ZIP:  return "ZIP ";
        default:           return "RAW ";
    }
}
static const char *_mode_name(CtiMode m) {
    switch (m) {
        case CTI_SEQ:         return "SEQ";
        case CTI_SPIRAL:      return "SPIRAL";
        case CTI_TOROID:      return "TOROID";
        case CTI_RANDOM_PERM: return "RANDOM_PERM";
        case CTI_DELTA_MISS:  return "DELTA_MISS";
        default:              return "?";
    }
}

void cti_print_report(const CtiScanner *sc) {
    if (!sc) return;
    _ws("=== CTI BITSTACK SCAN REPORT ===\n");
    _ws("format:     "); _ws(_fmt_name(sc->file_fmt)); _ws("\n");
    _ws("mode:       "); _ws(_mode_name(sc->mode));    _ws("\n");
    _ws("file_size:  "); _wu64(sc->file_size);         _ws(" bytes\n");
    _ws("n_blocks:   "); _wu(sc->n_blocks);            _ws("\n");
    _ws("scanned:    "); _wu(sc->n_entries);           _ws("\n");
    _ws("chain_crc:  0x"); _wh(sc->chain_crc);         _ws("\n");
    _ws("total_bad:  "); _wu(sc->total_bad);           _ws("\n\n");
    _ws("idx      size  fid_crc32          E     F  xbad  miss\n");
    _ws("------   ----  --------  ------  ----  -  ----  -----\n");
    uint32_t show = sc->n_entries < 32u ? sc->n_entries : 32u;
    for (uint32_t i = 0u; i < show; i++) {
        const CtiEntry *e = &sc->entries[i];
        _wu(e->idx);                     _ws("  ");
        _wu(e->size);                    _ws("  0x"); _wh(e->fid_crc32); _ws("  ");
        _wu(e->entropy);                 _ws("  ");
        _wu(e->flags);                   _ws("  ");
        _wu(e->xbad);                    _ws("  ");
        _wi((int32_t)e->miss_score);     _ws("\n");
    }
    if (sc->n_entries > 32u) {
        _ws("... ("); _wu(sc->n_entries - 32u); _ws(" more entries)\n");
    }
    _ws("=== END CTI REPORT ===\n");
}

/* ── Optional standalone CLI entry point ────────────────────────────────── */
#ifdef CTI_BUILD_MAIN
#include <fcntl.h>
int main(int argc, char **argv) {
    if (argc < 2) { write(1, "usage: cti_scan <file> [mode] [seed]\n", 37); return 1; }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) { write(1, "cannot open file\n", 17); return 1; }
    CtiMode mode = (argc >= 3) ? (CtiMode)argv[2][0] - '0' : CTI_SEQ;
    uint32_t seed = (argc >= 4) ? (uint32_t)argv[3][0] : 0u;
    if (mode < 0 || mode >= CTI_MODE_COUNT) mode = CTI_SEQ;
    /* ZrManifest is ~60KB — allocate as static to avoid stack overflow */
    static CtiScanner sc;
    if (cti_scan_fd(&sc, fd, mode, seed) < 0) {
        write(1, "scan failed\n", 12); return 1;
    }
    cti_print_report(&sc);
    return 0;
}
#endif
