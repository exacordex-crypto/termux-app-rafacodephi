/* mem.h — static bump allocator, branchless utils, endian writers.
 * No malloc. No heap. All pools live in BSS (zero-init). */
#pragma once
#include "sys.h"

/* ── Capacity constants (all hex) ─────────────────────────────────────── */
#define APK_CAP  0x600000u   /* 6 MiB — APK output buffer                */
#define SRC_CAP  0x100000u   /* 1 MiB — source input                     */
#define TMP_CAP  0x080000u   /* 512 KiB — scratch / temp                 */

static u8 _apk_buf[APK_CAP];
static u8 _src_buf[SRC_CAP];
static u8 _tmp_buf[TMP_CAP];
static sz _apk_pos, _src_pos, _tmp_pos;

static inline u8  *apk_ptr(void)  { return _apk_buf; }
static inline sz   apk_used(void) { return _apk_pos; }
static inline void apk_reset(void){ _apk_pos = 0; }
static inline void tmp_reset(void){ _tmp_pos = 0; }

static inline u8 *apk_alloc(sz n) {
    u8 *p = _apk_buf + _apk_pos;
    _apk_pos += (n + 7u) & ~7u;
    return p;
}
static inline u8 *tmp_alloc(sz n) {
    u8 *p = _tmp_buf + _tmp_pos;
    _tmp_pos += (n + 7u) & ~7u;
    return p;
}

/* ── Memory ops ──────────────────────────────────────────────────────── */
static inline void *m_set(void *d, u8 v, sz n) {
    u8 *p = (u8*)d;
    u64 vv = (u64)v * 0x0101010101010101ULL;
    sz i = 0;
    for (; i + 8 <= n; i += 8) *(u64*)(p+i) = vv;
    for (; i < n; i++) p[i] = v;
    return d;
}
static inline void *m_cpy(void *d, const void *s, sz n) {
    u8 *dp = (u8*)d; const u8 *sp = (const u8*)s;
    sz i = 0;
    for (; i + 8 <= n; i += 8) *(u64*)(dp+i) = *(const u64*)(sp+i);
    for (; i < n; i++) dp[i] = sp[i];
    return d;
}
/* branchless byte-level equality: returns 1 if equal */
static inline u8 m_eq(const void *a, const void *b, sz n) {
    const u8 *x = (const u8*)a, *y = (const u8*)b;
    u8 d = 0;
    for (sz i = 0; i < n; i++) d |= x[i] ^ y[i];
    return !d;
}

/* ── String ops ──────────────────────────────────────────────────────── */
static inline sz s_len(const char *s) {
    const char *p = s; while (*p) p++; return (sz)(p - s);
}
static inline u8 s_eq(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *a == *b;
}
static inline void s_cpy(char *d, const char *s) { while ((*d++ = *s++)); }
static inline u8 s_pfx(const char *s, const char *pfx) {
    while (*pfx && *s == *pfx) { s++; pfx++; }
    return !*pfx;
}

/* ── Branchless arithmetic ───────────────────────────────────────────── */
static inline u32 u32_min(u32 a, u32 b) {
    u32 d = a - b; return b + (d & (u32)((i32)d >> 31));
}
static inline u32 u32_max(u32 a, u32 b) {
    u32 d = b - a; return a + (d & (u32)((i32)d >> 31));
}
static inline u32 u32_aln(u32 v, u32 a) { return (v + a - 1u) & ~(a - 1u); }
/* bitmask: all-1s if cond true, else 0 — branchless select */
static inline u32 u32_mask(u32 cond) { return (u32)(-(i32)!!cond); }
static inline u32 u32_sel(u32 cond, u32 a, u32 b) {
    u32 m = u32_mask(cond); return (a & m) | (b & ~m);
}

/* ── Endian-safe writers / readers (unaligned OK) ────────────────────── */
static inline void w16(u8 *p, u16 v) { p[0]=(u8)v; p[1]=(u8)(v>>8); }
static inline void w32(u8 *p, u32 v) {
    p[0]=(u8)v; p[1]=(u8)(v>>8); p[2]=(u8)(v>>16); p[3]=(u8)(v>>24);
}
static inline void w64(u8 *p, u64 v) { w32(p,(u32)v); w32(p+4,(u32)(v>>32)); }
static inline u16 r16(const u8 *p) { return (u16)(p[0]|(u16)(p[1]<<8)); }
static inline u32 r32(const u8 *p) {
    return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);
}

/* ── I/O helpers ─────────────────────────────────────────────────────── */
static inline void pr(const char *s)     { os_write(1, s, s_len(s)); }
static inline void pr_err(const char *s) { os_write(2, s, s_len(s)); }
static inline void pr_nl(void)           { os_write(1, "\n", 1); }
static inline void pr_hex8(u32 v) {
    static const char H[] = "0123456789abcdef";
    char b[10]; b[0]='0'; b[1]='x';
    for (int i = 0; i < 8; i++) b[2+i] = H[(v>>(28-i*4))&0xF];
    os_write(1, b, 10);
}
static inline void pr_dec(u64 v) {
    if (!v) { os_write(1, "0", 1); return; }
    char b[20]; i32 i = 20;
    while (v) { b[--i] = '0' + (char)(v % 10); v /= 10; }
    os_write(1, b+i, (sz)(20-i));
}
