#ifndef THUMB2_H
#define THUMB2_H

/* ARM Architecture Reference Manual Thumb-2 Supplement, page 4-76 */
#define CPS(disable, bit) \
    WORD $(0xb660 | ((disable & 1) << 4) | (bit & 7))

#define CPS_A 4
#define CPS_I 2
#define CPS_F 1

#define MOVT(d, imm) \
    WORD $(0xf2c0 | ((imm & 0xf000) >> 12) | ((imm & 800) >> 1) | \
           ((imm & 0x700) << 20) | ((d & 0xf) << 24) | ((imm & 0xff) << 16))

/*
 * Coprocessors
 */
#define CpSC        15          /* System Control */

/*
 * Primary (CRn) CpSC registers.
 */
#define CpID        0           /* ID and cache type */
#define CpCONTROL   1           /* miscellaneous control */

/*
 * CpCONTROL op2 codes, op1==0, Crm==0.
 */
#define CpMainctl   0

#define CPUID_ADDR 0xe000ed00
#define CPACR_ADDR 0xe000ed88

/* ARM Architecture Reference Manual Thumb-2 Supplement, page A7-534 */
#define VMRS(r) WORD $(0x0a10eef1 | (r)<<28) /* FP to ARM */
/* ARM Architecture Reference Manual Thumb-2 Supplement, page A7-535 */
#define VMSR(r) WORD $(0x0a10eee1 | (r)<<28) /* ARM to FP */

/* ARM Architecture Reference Manual Thumb-2 Supplement, page A7-557 */
#define VSTR(fp, Rn, offset) WORD $(0x0b00ed00 | (fp & 0xf) << 28 | ((offset >> 2) & 0xff) << 16 | (Rn))
/* ARM Architecture Reference Manual Thumb-2 Supplement, page A7-521 */
#define VLDR(fp, Rn, offset) WORD $(0x0b00ed10 | (fp & 0xf) << 28 | ((offset >> 2) & 0xff) << 16 | (Rn))

#define FPCCR_ADDR 0xe000ef34
#define FPCAR_ADDR 0xe000ef38
#define FPDSCR_ADDR 0xe000ef3c
#define MVFR0_ADDR 0xe000ef40
#define MVFR1_ADDR 0xe000ef44

#endif
