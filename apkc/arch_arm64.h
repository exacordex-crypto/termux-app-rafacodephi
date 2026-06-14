/* arch_arm64.h — AArch64 (ARMv8-A) A64 instruction encoder.
 * Every function returns the 32-bit machine word (little-endian).
 * No heap. No imports beyond mem.h. Pure bit arithmetic. */
#pragma once
#include "mem.h"

/* ── Register numbers ────────────────────────────────────────────────── */
#define R0  0u  /* also XZR in some contexts */
#define R1  1u
#define R2  2u
#define R3  3u
#define R4  4u
#define R5  5u
#define R6  6u
#define R7  7u
#define R8  8u
#define R9  9u
#define R10 10u
#define R16 16u
#define R29 29u  /* FP */
#define R30 30u  /* LR */
#define RSP 31u  /* SP in load/store */
#define RZR 31u  /* XZR in data proc */

/* ── Condition codes ─────────────────────────────────────────────────── */
#define CC_EQ 0x0u
#define CC_NE 0x1u
#define CC_CS 0x2u
#define CC_CC 0x3u
#define CC_MI 0x4u
#define CC_PL 0x5u
#define CC_HI 0x8u
#define CC_LS 0x9u
#define CC_GE 0xAu
#define CC_LT 0xBu
#define CC_GT 0xCu
#define CC_LE 0xDu
#define CC_AL 0xEu

/* ── Emitter context ─────────────────────────────────────────────────── */
typedef struct { u8 *p; sz cap; sz pos; } CodeBuf;
static inline void cb_emit(CodeBuf *b, u32 insn) {
    w32(b->p + b->pos, insn); b->pos += 4;
}
static inline u32 cb_mark(CodeBuf *b) { return (u32)b->pos; }

/* ── Fixed words ─────────────────────────────────────────────────────── */
#define A64_NOP  0xD503201Fu
#define A64_RET  0xD65F03C0u  /* ret x30 */
#define A64_BRK0 0xD4200000u  /* brk #0 */

/* ── Branches ────────────────────────────────────────────────────────── */
/* B  #imm26  (PC-relative, unit = 4 bytes) */
static inline u32 a64_b  (i32 off4) { return 0x14000000u|((u32)off4&0x3FFFFFFu); }
/* BL #imm26 */
static inline u32 a64_bl (i32 off4) { return 0x94000000u|((u32)off4&0x3FFFFFFu); }
/* BR  Xn */
static inline u32 a64_br (u8 rn)    { return 0xD61F0000u|((u32)rn<<5); }
/* BLR Xn */
static inline u32 a64_blr(u8 rn)    { return 0xD63F0000u|((u32)rn<<5); }
/* RET Xn (default LR=x30) */
static inline u32 a64_ret(u8 rn)    { return 0xD65F0000u|((u32)rn<<5); }
/* B.cond #imm19 */
static inline u32 a64_bcond(u8 cc, i32 off4) {
    return 0x54000000u|(((u32)off4&0x7FFFFu)<<5)|(u32)cc;
}
/* CBZ/CBNZ  Xn, #imm19 */
static inline u32 a64_cbz (u8 rn, i32 off4, u8 sf) {
    return ((u32)sf<<31)|0x34000000u|((((u32)off4)&0x7FFFFu)<<5)|(u32)rn;
}
static inline u32 a64_cbnz(u8 rn, i32 off4, u8 sf) {
    return ((u32)sf<<31)|0x35000000u|((((u32)off4)&0x7FFFFu)<<5)|(u32)rn;
}
/* SVC #imm16 */
static inline u32 a64_svc(u16 imm) { return 0xD4000001u|((u32)imm<<5); }

/* ── Move immediate ──────────────────────────────────────────────────── */
/* MOVZ  Rd, #imm16 LSL (hw*16)  — sf=1 → 64-bit, sf=0 → 32-bit */
static inline u32 a64_movz(u8 rd, u16 imm, u8 hw, u8 sf) {
    return ((u32)sf<<31)|(0x2u<<29)|(0x25u<<23)|((u32)hw<<21)|((u32)imm<<5)|(u32)rd;
}
/* MOVK  Rd, #imm16 LSL (hw*16)  — keep other bits */
static inline u32 a64_movk(u8 rd, u16 imm, u8 hw, u8 sf) {
    return ((u32)sf<<31)|(0x3u<<29)|(0x25u<<23)|((u32)hw<<21)|((u32)imm<<5)|(u32)rd;
}
/* MOVN  Rd, #imm16 LSL (hw*16)  — inverted */
static inline u32 a64_movn(u8 rd, u16 imm, u8 hw, u8 sf) {
    return ((u32)sf<<31)|(0x0u<<29)|(0x25u<<23)|((u32)hw<<21)|((u32)imm<<5)|(u32)rd;
}
/* Emit 64-bit constant into Xd using MOVZ + up to 3× MOVK */
static inline void a64_mov64(CodeBuf *b, u8 rd, u64 v) {
    u8 first = 1;
    for (u8 hw = 0; hw < 4; hw++) {
        u16 part = (u16)(v >> (hw * 16));
        if (!part && !first) continue;
        if (first) { cb_emit(b, a64_movz(rd, part, hw, 1)); first = 0; }
        else         cb_emit(b, a64_movk(rd, part, hw, 1));
    }
    if (first) cb_emit(b, a64_movz(rd, 0, 0, 1)); /* v == 0 */
}

/* ── Data processing (register) ──────────────────────────────────────── */
/* ORR Xd, Xn, Xm  →  MOV Xd,Xm when Xn=XZR */
static inline u32 a64_orr_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x2Bu<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_and_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x0Au<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_eor_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x4Au<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_add_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x0Bu<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_sub_reg(u8 rd, u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x4Bu<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)rd;
}
/* MOV Xd, Xm  (alias: ORR Xd, XZR, Xm) */
static inline u32 a64_mov_reg(u8 rd, u8 rm, u8 sf) {
    return a64_orr_reg(rd, RZR, rm, sf);
}

/* ── Data processing (immediate) ─────────────────────────────────────── */
/* ADD/SUB Xd, Xn, #imm12 [shift=0 or shift=1 (LSL#12)] */
static inline u32 a64_add_imm(u8 rd, u8 rn, u16 imm12, u8 sh, u8 sf) {
    return ((u32)sf<<31)|(0x11u<<24)|((u32)sh<<22)|((u32)imm12<<10)|((u32)rn<<5)|(u32)rd;
}
static inline u32 a64_sub_imm(u8 rd, u8 rn, u16 imm12, u8 sh, u8 sf) {
    return ((u32)sf<<31)|(0x51u<<24)|((u32)sh<<22)|((u32)imm12<<10)|((u32)rn<<5)|(u32)rd;
}
/* CMP Xn, #imm12  (SUBS XZR, Xn, #imm) */
static inline u32 a64_cmp_imm(u8 rn, u16 imm12, u8 sf) {
    return ((u32)sf<<31)|(0x71u<<24)|((u32)imm12<<10)|((u32)rn<<5)|(u32)RZR;
}
/* CMP Xn, Xm */
static inline u32 a64_cmp_reg(u8 rn, u8 rm, u8 sf) {
    return ((u32)sf<<31)|(0x6Bu<<24)|((u32)rm<<16)|((u32)rn<<5)|(u32)RZR;
}

/* ── Branchless select ────────────────────────────────────────────────── */
/* CSEL  Xd, Xn, Xm, cond  — Xd = (cond) ? Xn : Xm */
static inline u32 a64_csel(u8 rd, u8 rn, u8 rm, u8 cc, u8 sf) {
    return ((u32)sf<<31)|(0x1Au<<24)|(1u<<23)|((u32)rm<<16)|((u32)cc<<12)|((u32)rn<<5)|(u32)rd;
}
/* CSINC Xd, Xn, Xm, cond  — used for CSET (rd,xzr,xzr,inv_cond) */
static inline u32 a64_csinc(u8 rd, u8 rn, u8 rm, u8 cc, u8 sf) {
    return ((u32)sf<<31)|(0x1Au<<24)|(1u<<23)|((u32)rm<<16)|((u32)cc<<12)|(1u<<10)|((u32)rn<<5)|(u32)rd;
}

/* ── Shifts ──────────────────────────────────────────────────────────── */
/* LSL Xd, Xn, #sh  (UBFM) */
static inline u32 a64_lsl_imm(u8 rd, u8 rn, u8 sh, u8 sf) {
    u8 bsz = sf ? 64u : 32u;
    return ((u32)sf<<31)|(0x53u<<23)|((u32)sf<<22)|
           ((u32)(bsz-sh)<<16)|((u32)(bsz-sh-1)<<10)|((u32)rn<<5)|(u32)rd;
}
/* LSR Xd, Xn, #sh  (UBFM) */
static inline u32 a64_lsr_imm(u8 rd, u8 rn, u8 sh, u8 sf) {
    u8 bsz = sf ? 63u : 31u;
    return ((u32)sf<<31)|(0x53u<<23)|((u32)sf<<22)|
           ((u32)sh<<16)|((u32)bsz<<10)|((u32)rn<<5)|(u32)rd;
}
/* ASR Xd, Xn, #sh  (SBFM) */
static inline u32 a64_asr_imm(u8 rd, u8 rn, u8 sh, u8 sf) {
    u8 bsz = sf ? 63u : 31u;
    return ((u32)sf<<31)|(0x13u<<23)|((u32)sf<<22)|
           ((u32)sh<<16)|((u32)bsz<<10)|((u32)rn<<5)|(u32)rd;
}

/* ── Load / Store ────────────────────────────────────────────────────── */
/* LDR Xt, [Xn, #off]  unsigned offset (off must be multiple of 8 for 64-bit) */
static inline u32 a64_ldr(u8 rt, u8 rn, u16 off, u8 sf) {
    u8 sz = sf ? 3u : 2u;
    u16 sc = sf ? (off>>3) : (off>>2);
    return ((u32)sz<<30)|(0x39u<<24)|(1u<<22)|((u32)sc<<10)|((u32)rn<<5)|(u32)rt;
}
/* STR Xt, [Xn, #off] */
static inline u32 a64_str(u8 rt, u8 rn, u16 off, u8 sf) {
    u8 sz = sf ? 3u : 2u;
    u16 sc = sf ? (off>>3) : (off>>2);
    return ((u32)sz<<30)|(0x39u<<24)|((u32)sc<<10)|((u32)rn<<5)|(u32)rt;
}
/* STP Xt1, Xt2, [Xn, #off7*8]  signed-offset form */
static inline u32 a64_stp(u8 t1, u8 t2, u8 rn, i8 off7, u8 sf) {
    u8 opc = sf ? 2u : 0u;
    return ((u32)opc<<30)|(0x15u<<26)|(2u<<23)|
           (((u32)off7&0x7Fu)<<15)|((u32)t2<<10)|((u32)rn<<5)|(u32)t1;
}
/* STP pre-index */
static inline u32 a64_stp_pre(u8 t1, u8 t2, u8 rn, i8 off7, u8 sf) {
    u8 opc = sf ? 2u : 0u;
    return ((u32)opc<<30)|(0x15u<<26)|(3u<<23)|
           (((u32)off7&0x7Fu)<<15)|((u32)t2<<10)|((u32)rn<<5)|(u32)t1;
}
/* LDP post-index */
static inline u32 a64_ldp_post(u8 t1, u8 t2, u8 rn, i8 off7, u8 sf) {
    u8 opc = sf ? 2u : 0u;
    return ((u32)opc<<30)|(0x15u<<26)|(1u<<23)|
           (((u32)off7&0x7Fu)<<15)|((u32)t2<<10)|((u32)rn<<5)|(u32)t1;
}
/* ADR  Xd, #byte_offset (PC-relative) */
static inline u32 a64_adr(u8 rd, i32 off) {
    return 0x10000000u|(((u32)off&3u)<<29)|(((u32)off>>2)&0x7FFFFu)<<5|(u32)rd;
}
/* ADRP Xd, #page_offset (page = 4 KiB) */
static inline u32 a64_adrp(u8 rd, i32 pg) {
    return 0x90000000u|(((u32)pg&3u)<<29)|(((u32)(pg>>2))&0x7FFFFu)<<5|(u32)rd;
}
