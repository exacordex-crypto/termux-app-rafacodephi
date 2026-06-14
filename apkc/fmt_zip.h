/* fmt_zip.h — ZIP archive writer (STORE method, no compression).
 * CRC-32 computed with Sarwate table (generated at first use).
 * No heap. Writes into caller-supplied buffer. */
#pragma once
#include "mem.h"

/* ── CRC-32 (ISO 3309 / ZIP)  poly = 0xEDB88320 (bit-reversed) ────────── */
static u32 _crc_tab[256];
static u8  _crc_rdy = 0;

static inline void _crc_init(void) {
    if (_crc_rdy) return;
    for (u32 i = 0; i < 256u; i++) {
        u32 c = i;
        /* 8 iterations, branchless via arithmetic mask */
        for (u32 j = 0; j < 8u; j++) {
            u32 mask = (u32)(-(i32)(c & 1u));
            c = (c >> 1) ^ (0xEDB88320u & mask);
        }
        _crc_tab[i] = c;
    }
    _crc_rdy = 1;
}
static inline u32 crc32(const u8 *buf, sz len) {
    _crc_init();
    u32 c = 0xFFFFFFFFu;
    for (sz i = 0; i < len; i++)
        c = _crc_tab[(u8)(c ^ buf[i])] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

/* ── ZIP writer ──────────────────────────────────────────────────────── */
#define ZIP_MAX  64u

typedef struct {
    const char *name;
    const u8   *data;
    u32         size;
    u32         crc;
    u32         lfh_off; /* offset of Local File Header in output */
} ZipEnt;

typedef struct {
    u8     *buf;
    sz      cap;
    sz      pos;
    ZipEnt  ent[ZIP_MAX];
    u32     n;
} ZipWr;

static inline void zip_init(ZipWr *z, u8 *buf, sz cap) {
    z->buf = buf; z->cap = cap; z->pos = 0; z->n = 0;
}

/* internal: raw append */
static inline void _zp(ZipWr *z, const void *src, sz n) {
    m_cpy(z->buf + z->pos, src, n); z->pos += n;
}
static inline void _z16(ZipWr *z, u16 v){ u8 b[2]; w16(b,v); _zp(z,b,2); }
static inline void _z32(ZipWr *z, u32 v){ u8 b[4]; w32(b,v); _zp(z,b,4); }

/* Add a file to the archive (STORE, no compression) */
static inline void zip_add(ZipWr *z, const char *name, const u8 *data, u32 size) {
    u32  crc     = crc32(data, (sz)size);
    u16  namelen = (u16)s_len(name);
    ZipEnt *e   = &z->ent[z->n++];
    e->name    = name;
    e->data    = data;
    e->size    = size;
    e->crc     = crc;
    e->lfh_off = (u32)z->pos;

    /* Local File Header */
    _z32(z, 0x04034B50u);  /* signature */
    _z16(z, 20u);           /* version needed */
    _z16(z, 0u);            /* flags */
    _z16(z, 0u);            /* compression = STORE */
    _z16(z, 0u);            /* mod time */
    _z16(z, 0u);            /* mod date */
    _z32(z, crc);
    _z32(z, size);
    _z32(z, size);
    _z16(z, namelen);
    _z16(z, 0u);            /* extra len */
    _zp(z, name, (sz)namelen);
    _zp(z, data, (sz)size);
}

/* Finalise: write Central Directory + EOCD.  Returns total byte count. */
static inline sz zip_finish(ZipWr *z) {
    u32 cd_off = (u32)z->pos;
    for (u32 i = 0; i < z->n; i++) {
        ZipEnt *e   = &z->ent[i];
        u16 namelen = (u16)s_len(e->name);
        _z32(z, 0x02014B50u); /* Central Dir Record sig */
        _z16(z, 20u);          /* version made by */
        _z16(z, 20u);          /* version needed */
        _z16(z, 0u);           /* flags */
        _z16(z, 0u);           /* compression */
        _z16(z, 0u);           /* mod time */
        _z16(z, 0u);           /* mod date */
        _z32(z, e->crc);
        _z32(z, e->size);
        _z32(z, e->size);
        _z16(z, namelen);
        _z16(z, 0u);           /* extra len */
        _z16(z, 0u);           /* comment len */
        _z16(z, 0u);           /* disk start */
        _z16(z, 0u);           /* internal attr */
        _z32(z, 0u);           /* external attr */
        _z32(z, e->lfh_off);   /* LFH offset */
        _zp(z, e->name, (sz)namelen);
    }
    u32 cd_size = (u32)z->pos - cd_off;
    /* End of Central Directory */
    _z32(z, 0x06054B50u);
    _z16(z, 0u);               /* disk */
    _z16(z, 0u);               /* disk with CD */
    _z16(z, (u16)z->n);
    _z16(z, (u16)z->n);
    _z32(z, cd_size);
    _z32(z, cd_off);
    _z16(z, 0u);               /* comment len */
    return z->pos;
}
