/* sys.h — freestanding syscall layer: arm64 / arm32 Android/Linux
 * svc #0 (arm64) / swi #0 (arm32). No libc. No crt. Pure inline ASM. */
#pragma once

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;
typedef signed char         i8;
typedef signed short        i16;
typedef signed int          i32;
typedef signed long long    i64;
#ifdef __aarch64__
typedef unsigned long       uptr;
typedef long                iptr;
#else
typedef unsigned int        uptr;
typedef int                 iptr;
#endif
typedef __SIZE_TYPE__       sz;

#define NULL ((void*)0)

/* open(2) flags */
#define O_RDONLY  0x00
#define O_WRONLY  0x01
#define O_RDWR    0x02
#define O_CREAT   0x40
#define O_TRUNC   0x200
#define O_CLOEXEC 0x80000

/* ═══════════════════════════ ARM64 ════════════════════════════════════════ */
#ifdef __aarch64__

#define _NR_read    63
#define _NR_write   64
#define _NR_openat  56
#define _NR_close   57
#define _NR_lseek   62
#define _NR_fstat   80
#define _NR_exit    93

static __attribute__((always_inline)) i64
_sc1(i64 n, i64 a) {
    register i64 x8 __asm__("x8") = n;
    register i64 x0 __asm__("x0") = a;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8) : "memory","cc");
    return x0;
}
static __attribute__((always_inline)) i64
_sc3(i64 n, i64 a, i64 b, i64 c) {
    register i64 x8 __asm__("x8") = n;
    register i64 x0 __asm__("x0") = a;
    register i64 x1 __asm__("x1") = b;
    register i64 x2 __asm__("x2") = c;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8),"r"(x1),"r"(x2) : "memory","cc");
    return x0;
}
static __attribute__((always_inline)) i64
_sc4(i64 n, i64 a, i64 b, i64 c, i64 d) {
    register i64 x8 __asm__("x8") = n;
    register i64 x0 __asm__("x0") = a;
    register i64 x1 __asm__("x1") = b;
    register i64 x2 __asm__("x2") = c;
    register i64 x3 __asm__("x3") = d;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x8),"r"(x1),"r"(x2),"r"(x3) : "memory","cc");
    return x0;
}

static inline i64  os_read (i32 fd, void *b, sz n)       { return _sc3(_NR_read, (i64)fd,(i64)(uptr)b,(i64)n); }
static inline i64  os_write(i32 fd, const void *b, sz n) { return _sc3(_NR_write,(i64)fd,(i64)(uptr)b,(i64)n); }
static inline i32  os_open (const char *p, i32 f, i32 m) { return (i32)_sc4(_NR_openat,-100LL,(i64)(uptr)p,(i64)f,(i64)m); }
static inline i32  os_close(i32 fd)                      { return (i32)_sc1(_NR_close,(i64)fd); }
static inline __attribute__((noreturn)) void os_exit(i32 c) {
    _sc1(_NR_exit,(i64)c); __builtin_unreachable();
}

/* ═══════════════════════════ ARM32 ════════════════════════════════════════ */
#else

#define _NR_exit   1
#define _NR_read   3
#define _NR_write  4
#define _NR_open   5
#define _NR_close  6
#define _NR_lseek  19

static __attribute__((always_inline)) i32
_sc1_32(i32 n, i32 a) {
    register i32 r7 __asm__("r7") = n;
    register i32 r0 __asm__("r0") = a;
    __asm__ volatile("swi #0" : "+r"(r0) : "r"(r7) : "memory");
    return r0;
}
static __attribute__((always_inline)) i32
_sc3_32(i32 n, i32 a, i32 b, i32 c) {
    register i32 r7 __asm__("r7") = n;
    register i32 r0 __asm__("r0") = a;
    register i32 r1 __asm__("r1") = b;
    register i32 r2 __asm__("r2") = c;
    __asm__ volatile("swi #0" : "+r"(r0) : "r"(r7),"r"(r1),"r"(r2) : "memory");
    return r0;
}

static inline i32  os_read (i32 fd, void *b, sz n)       { return _sc3_32(_NR_read, fd,(i32)(uptr)b,(i32)n); }
static inline i32  os_write(i32 fd, const void *b, sz n) { return _sc3_32(_NR_write,fd,(i32)(uptr)b,(i32)n); }
static inline i32  os_open (const char *p, i32 f, i32 m) { return _sc3_32(_NR_open,(i32)(uptr)p,f,m); }
static inline i32  os_close(i32 fd)                      { return _sc1_32(_NR_close,fd); }
static inline __attribute__((noreturn)) void os_exit(i32 c) {
    _sc1_32(_NR_exit,c); __builtin_unreachable();
}
#endif
