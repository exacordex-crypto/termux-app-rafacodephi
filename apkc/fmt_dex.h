/* fmt_dex.h — Minimal classes.dex generator.
 * Produces the smallest valid DEX 035 file (header + map list only).
 * Includes Adler-32 (checksum) and SHA-1 (signature) from scratch.
 * No malloc. No libc. */
#pragma once
#include "mem.h"

/* ── Adler-32 (DEX checksum, bytes [12..end]) ────────────────────────── */
static inline u32 adler32(const u8 *data, sz len) {
    u32 s1 = 1u, s2 = 0u;
    for (sz i = 0; i < len; i++) {
        s1 = (s1 + data[i]) % 65521u;
        s2 = (s2 + s1)      % 65521u;
    }
    return (s2 << 16) | s1;
}

/* ── SHA-1 (DEX signature, bytes [32..end]) ──────────────────────────── */
typedef struct { u32 h[5]; u8 buf[64]; u64 bits; u32 blen; } SHA1Ctx;

static inline u32 _sha1_rot(u32 v, u8 n) { return (v<<n)|(v>>(32u-n)); }

static inline void sha1_init(SHA1Ctx *c) {
    c->h[0]=0x67452301u; c->h[1]=0xEFCDAB89u;
    c->h[2]=0x98BADCFEu; c->h[3]=0x10325476u;
    c->h[4]=0xC3D2E1F0u;
    c->bits = 0; c->blen = 0;
}
static inline void _sha1_block(SHA1Ctx *c, const u8 *blk) {
    u32 w[80], a,b,d,e,f,k,t;
    for (u32 i=0;i<16u;i++)
        w[i]=((u32)blk[i*4]<<24)|((u32)blk[i*4+1]<<16)|
             ((u32)blk[i*4+2]<<8)|(u32)blk[i*4+3];
    for (u32 i=16u;i<80u;i++)
        w[i]=_sha1_rot(w[i-3]^w[i-8]^w[i-14]^w[i-16],1u);
    a=c->h[0];b=c->h[1];u32 cc=c->h[2];d=c->h[3];e=c->h[4];
    for (u32 i=0;i<80u;i++) {
        if      (i<20u){f=(b&cc)|(~b&d);k=0x5A827999u;}
        else if (i<40u){f=b^cc^d;       k=0x6ED9EBA1u;}
        else if (i<60u){f=(b&cc)|(b&d)|(cc&d);k=0x8F1BBCDCu;}
        else           {f=b^cc^d;       k=0xCA62C1D6u;}
        t=_sha1_rot(a,5u)+f+e+k+w[i];
        e=d;d=cc;cc=_sha1_rot(b,30u);b=a;a=t;
    }
    c->h[0]+=a;c->h[1]+=b;c->h[2]+=cc;c->h[3]+=d;c->h[4]+=e;
}
static inline void sha1_update(SHA1Ctx *c, const u8 *data, sz len) {
    c->bits += (u64)len * 8u;
    for (sz i=0;i<len;i++) {
        c->buf[c->blen++]=data[i];
        if (c->blen==64u){_sha1_block(c,c->buf);c->blen=0;}
    }
}
static inline void sha1_final(SHA1Ctx *c, u8 out[20]) {
    u8 b80=0x80u;
    sha1_update(c,&b80,1u);
    while(c->blen!=56u){u8 z=0;sha1_update(c,&z,1u);}
    for(i32 i=7;i>=0;i--){u8 b=(u8)(c->bits>>(u32)(i*8));sha1_update(c,&b,1u);}
    for(i32 i=0;i<5;i++){
        out[i*4]  =(u8)(c->h[i]>>24); out[i*4+1]=(u8)(c->h[i]>>16);
        out[i*4+2]=(u8)(c->h[i]>>8);  out[i*4+3]=(u8)c->h[i];
    }
}

/* ── DEX format constants ────────────────────────────────────────────── */
#define DEX_HEADER_SZ   0x70u        /* 112 bytes */
#define DEX_ENDIAN_TAG  0x12345678u

/* TYPE_* values used in map list */
#define DEX_TYPE_HEADER   0x0000u
#define DEX_TYPE_MAPLIST  0x1000u

/* ── DEX generator ───────────────────────────────────────────────────── */
/*
 * Produces the minimum valid DEX 035:
 *   [0x000..0x06F]  header   (112 bytes)
 *   [0x070..0x08B]  map list (28 bytes: 4 + 2*12)
 * Total: 140 bytes.
 * Caller must supply a buffer of at least 140 bytes.
 * Returns actual size written.
 */
static inline u32 dex_build(u8 *out) {
    const u32 TOTAL    = DEX_HEADER_SZ + 28u; /* 140 = 0x8C */
    const u32 MAP_OFF  = DEX_HEADER_SZ;       /* 0x70 */

    m_set(out, 0, (sz)TOTAL);

    /* magic: "dex\n035\0" */
    out[0]='d'; out[1]='e'; out[2]='x'; out[3]='\n';
    out[4]='0'; out[5]='3'; out[6]='5'; out[7]='\0';

    /* [8..11]  checksum — filled in below */
    /* [12..31] SHA-1 signature — filled in below */
    w32(out+32, TOTAL);           /* file_size */
    w32(out+36, DEX_HEADER_SZ);  /* header_size */
    w32(out+40, DEX_ENDIAN_TAG); /* endian_tag */
    /* link_size, link_off = 0 (already zeroed) */
    w32(out+48, MAP_OFF);        /* map_off */
    /* string/type/proto/field/method/class counts+offs = 0 */
    /* data_size, data_off = 0 */

    /* Map list at offset MAP_OFF */
    u8 *mp = out + MAP_OFF;
    w32(mp, 2u);                  /* 2 map entries */
    /* Entry 0: TYPE_HEADER_ITEM, count=1, offset=0 */
    w16(mp+4,  DEX_TYPE_HEADER); w16(mp+6,  0u);
    w32(mp+8,  1u);               w32(mp+12, 0u);
    /* Entry 1: TYPE_MAP_LIST, count=1, offset=MAP_OFF */
    w16(mp+16, DEX_TYPE_MAPLIST);w16(mp+18, 0u);
    w32(mp+20, 1u);               w32(mp+24, MAP_OFF);

    /* SHA-1 signature (bytes [32..TOTAL-1]) */
    SHA1Ctx sc;
    sha1_init(&sc);
    sha1_update(&sc, out+32, (sz)(TOTAL-32u));
    sha1_final(&sc, out+12);

    /* Adler-32 checksum (bytes [12..TOTAL-1]) */
    u32 ck = adler32(out+12, (sz)(TOTAL-12u));
    w32(out+8, ck);

    return TOTAL;
}
