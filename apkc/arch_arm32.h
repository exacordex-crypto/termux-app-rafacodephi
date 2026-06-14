/* arch_arm32.h — ARMv7-A A32 instruction encoder.
 * All functions return 32-bit A32 machine words.
 * No Thumb encoding. No heap. No imports. */
#pragma once
#include "mem.h"

/* ── Condition codes ─────────────────────────────────────────────────── */
#define A32_EQ 0x0u
#define A32_NE 0x1u
#define A32_CS 0x2u
#define A32_CC 0x3u
#define A32_MI 0x4u
#define A32_PL 0x5u
#define A32_HI 0x8u
#define A32_LS 0x9u
#define A32_GE 0xAu
#define A32_LT 0xBu
#define A32_GT 0xCu
#define A32_LE 0xDu
#define A32_AL 0xEu  /* Always (unconditional) */

/* ── Register aliases ────────────────────────────────────────────────── */
#define A32_R0  0u
#define A32_R1  1u
#define A32_R2  2u
#define A32_R3  3u
#define A32_R4  4u
#define A32_R5  5u
#define A32_R6  6u
#define A32_R7  7u
#define A32_R8  8u
#define A32_SP  13u
#define A32_LR  14u
#define A32_PC  15u

typedef struct { u8 *p; sz cap; sz pos; } A32Buf;
static inline void a32_emit(A32Buf *b, u32 w) { w32(b->p + b->pos, w); b->pos += 4; }
static inline u32  a32_mark(A32Buf *b)        { return (u32)b->pos; }

/* ── Fixed words ─────────────────────────────────────────────────────── */
#define A32_NOP  0xE320F000u
#define A32_BXLR 0xE12FFF1Eu  /* BX LR */

/* ── Branches ────────────────────────────────────────────────────────── */
/* BX  Rm  — branch-and-exchange (change to Thumb if Rm[0]=1) */
static inline u32 a32_bx(u8 rm, u8 cc) {
    return ((u32)cc<<28)|0x012FFF10u|(u32)rm;
}
/* B   #off24  signed, PC-relative in 4-byte units */
static inline u32 a32_b(i32 off4, u8 cc) {
    return ((u32)cc<<28)|0x0A000000u|((u32)off4&0x00FFFFFFu);
}
/* BL  #off24 */
static inline u32 a32_bl(i32 off4, u8 cc) {
    return ((u32)cc<<28)|0x0B000000u|((u32)off4&0x00FFFFFFu);
}
/* BLX Rm (always) */
static inline u32 a32_blx(u8 rm) { return 0xE12FFF30u|(u32)rm; }
/* SWI #imm24 (Linux syscall) */
static inline u32 a32_swi(u32 imm24, u8 cc) {
    return ((u32)cc<<28)|0x0F000000u|(imm24&0x00FFFFFFu);
}

/* ── Move ─────────────────────────────────────────────────────────────── */
/* MOV  Rd, #imm8rot  (imm8 rotated right 2*rot) */
static inline u32 a32_mov_imm(u8 rd, u8 imm8, u8 rot, u8 s, u8 cc) {
    return ((u32)cc<<28)|0x03A00000u|((u32)s<<20)|((u32)rd<<12)|
           ((u32)(rot&0xFu)<<8)|(u32)imm8;
}
/* MOVW Rd, #imm16  (ARMv6T2+) */
static inline u32 a32_movw(u8 rd, u16 imm, u8 cc) {
    return ((u32)cc<<28)|0x03000000u|
           (((u32)imm>>12)<<16)|((u32)rd<<12)|((u32)imm&0xFFFu);
}
/* MOVT Rd, #imm16  (top 16 bits) */
static inline u32 a32_movt(u8 rd, u16 imm, u8 cc) {
    return ((u32)cc<<28)|0x03400000u|
           (((u32)imm>>12)<<16)|((u32)rd<<12)|((u32)imm&0xFFFu);
}
/* MOV  Rd, Rm  (shift=0) */
static inline u32 a32_mov_reg(u8 rd, u8 rm, u8 s, u8 cc) {
    return ((u32)cc<<28)|0x01A00000u|((u32)s<<20)|((u32)rd<<12)|(u32)rm;
}
/* MVN  Rd, #imm8rot */
static inline u32 a32_mvn_imm(u8 rd, u8 imm8, u8 rot, u8 cc) {
    return ((u32)cc<<28)|0x03E00000u|((u32)rd<<12)|((u32)(rot&0xFu)<<8)|(u32)imm8;
}
/* Emit full 32-bit constant via MOVW+MOVT */
static inline void a32_mov32(A32Buf *b, u8 rd, u32 v, u8 cc) {
    a32_emit(b, a32_movw(rd,(u16)v,cc));
    if (v >> 16) a32_emit(b, a32_movt(rd,(u16)(v>>16),cc));
}

/* ── Data processing ─────────────────────────────────────────────────── */
#define _A32_DP(opc,s,cc,rn,rd,op2) \
    (((u32)(cc)<<28)|((u32)(opc)<<21)|((u32)(s)<<20)|((u32)(rn)<<16)|((u32)(rd)<<12)|(op2))
/* ADD  Rd, Rn, #imm8rot */
static inline u32 a32_add_imm(u8 rd,u8 rn,u8 imm8,u8 rot,u8 s,u8 cc){
    return _A32_DP(0x4u|0x40u,s,cc,rn,rd,((u32)(rot&0xFu)<<8)|(u32)imm8)|0x02000000u;
}
/* SUB  Rd, Rn, #imm8rot */
static inline u32 a32_sub_imm(u8 rd,u8 rn,u8 imm8,u8 rot,u8 s,u8 cc){
    return _A32_DP(0x2u|0x40u,s,cc,rn,rd,((u32)(rot&0xFu)<<8)|(u32)imm8)|0x02000000u;
}
/* ADD  Rd, Rn, Rm */
static inline u32 a32_add_reg(u8 rd,u8 rn,u8 rm,u8 s,u8 cc){
    return ((u32)cc<<28)|0x00800000u|((u32)s<<20)|((u32)rn<<16)|((u32)rd<<12)|(u32)rm;
}
/* SUB  Rd, Rn, Rm */
static inline u32 a32_sub_reg(u8 rd,u8 rn,u8 rm,u8 s,u8 cc){
    return ((u32)cc<<28)|0x00400000u|((u32)s<<20)|((u32)rn<<16)|((u32)rd<<12)|(u32)rm;
}
/* AND / ORR / EOR (register) */
static inline u32 a32_and_reg(u8 rd,u8 rn,u8 rm,u8 s,u8 cc){
    return ((u32)cc<<28)|0x00000000u|((u32)s<<20)|((u32)rn<<16)|((u32)rd<<12)|(u32)rm;
}
static inline u32 a32_orr_reg(u8 rd,u8 rn,u8 rm,u8 s,u8 cc){
    return ((u32)cc<<28)|0x01800000u|((u32)s<<20)|((u32)rn<<16)|((u32)rd<<12)|(u32)rm;
}
static inline u32 a32_eor_reg(u8 rd,u8 rn,u8 rm,u8 s,u8 cc){
    return ((u32)cc<<28)|0x00200000u|((u32)s<<20)|((u32)rn<<16)|((u32)rd<<12)|(u32)rm;
}
/* CMP  Rn, Rm */
static inline u32 a32_cmp_reg(u8 rn,u8 rm,u8 cc){
    return ((u32)cc<<28)|0x01500000u|((u32)rn<<16)|(u32)rm;
}
/* CMP  Rn, #imm8rot */
static inline u32 a32_cmp_imm(u8 rn,u8 imm8,u8 rot,u8 cc){
    return ((u32)cc<<28)|0x03500000u|((u32)rn<<16)|((u32)(rot&0xFu)<<8)|(u32)imm8;
}
/* LSL/LSR/ASR immediate (shift of MOV) */
static inline u32 a32_lsl_imm(u8 rd,u8 rm,u8 sh,u8 s,u8 cc){
    return ((u32)cc<<28)|0x01A00000u|((u32)s<<20)|((u32)rd<<12)|((u32)sh<<7)|(u32)rm;
}
static inline u32 a32_lsr_imm(u8 rd,u8 rm,u8 sh,u8 s,u8 cc){
    return ((u32)cc<<28)|0x01A00020u|((u32)s<<20)|((u32)rd<<12)|((u32)sh<<7)|(u32)rm;
}
static inline u32 a32_asr_imm(u8 rd,u8 rm,u8 sh,u8 s,u8 cc){
    return ((u32)cc<<28)|0x01A00040u|((u32)s<<20)|((u32)rd<<12)|((u32)sh<<7)|(u32)rm;
}
/* MUL  Rd, Rm, Rs */
static inline u32 a32_mul(u8 rd,u8 rm,u8 rs,u8 s,u8 cc){
    return ((u32)cc<<28)|0x00000090u|((u32)s<<20)|((u32)rd<<16)|((u32)rs<<8)|(u32)rm;
}

/* ── Load / Store ────────────────────────────────────────────────────── */
/* LDR  Rd, [Rn, #+off12]  (U=1: add, U=0: sub) */
static inline u32 a32_ldr_imm(u8 rd,u8 rn,u16 off12,u8 u,u8 cc){
    return ((u32)cc<<28)|0x05100000u|((u32)u<<23)|((u32)rn<<16)|((u32)rd<<12)|(u32)(off12&0xFFFu);
}
/* STR  Rd, [Rn, #+off12] */
static inline u32 a32_str_imm(u8 rd,u8 rn,u16 off12,u8 u,u8 cc){
    return ((u32)cc<<28)|0x05000000u|((u32)u<<23)|((u32)rn<<16)|((u32)rd<<12)|(u32)(off12&0xFFFu);
}
/* LDRB  Rd, [Rn, #+off] */
static inline u32 a32_ldrb_imm(u8 rd,u8 rn,u16 off,u8 u,u8 cc){
    return ((u32)cc<<28)|0x05500000u|((u32)u<<23)|((u32)rn<<16)|((u32)rd<<12)|(u32)(off&0xFFFu);
}
/* PUSH {reglist}  = STMDB SP!, {reglist} */
static inline u32 a32_push(u16 regs,u8 cc){
    return ((u32)cc<<28)|0x092D0000u|(u32)regs;
}
/* POP  {reglist}  = LDMIA SP!, {reglist} */
static inline u32 a32_pop(u16 regs,u8 cc){
    return ((u32)cc<<28)|0x08BD0000u|(u32)regs;
}
