#ifndef THUMB2_H
#define THUMB2_H

/* ARM Architecture Reference Manual Thumb-2 Supplement, page 4-76, T1 */
#define CPS(disable, bit) \
    WORD $(0xb660 | ((disable & 1) << 4) | (bit & 7))

#define CPS_A 4
#define CPS_I 2
#define CPS_F 1

/* ARM Architecture Reference Manual Thumb-2 Supplement, page 4-76, T2, M=1 */
#define CPSW(mode) \
    WORD $(0x8000f3af | (1 << 24) | ((mode & 0x1f) << 16))

#define MOVT(d, imm) \
    WORD $(0xf2c0 | ((imm & 0xf000) >> 12) | ((imm & 800) >> 1) | \
           ((imm & 0x700) << 20) | ((d & 0xf) << 24) | ((imm & 0xff) << 16))

/* ARM Architecture Reference Manual Thumb-2 Supplement, page 4-76, T2, M=1 */
#define SVC(n) \
    WORD $(0xdf00 | (n & 0xff))

/* ARM Architecture Reference Manual Thumb-2 Supplement, page 4-173, T1 */
#define MRC2(coproc, opc1, Rt, CRn, CRm, opc2) \
    WORD $(0x0010fe10 | ((coproc & 0xf) << 24) | ((opc1 & 7) << 5) | \
           ((Rt & 0xf) << 28) | (CRn & 0xf) | ((CRm & 0xf) << 16) | \
           ((opc2 & 7) << 21))

/* ARMv7-M Architecture Reference Manual, A7.7.81 */
#define MRS(Rd, spec) \
    WORD $(0x8000f3ef | ((Rd & 0xf) << 24) | ((spec & 0xff) << 16))

/* ARMv7-M Architecture Reference Manual, A7.7.82, mask=2 */
#define MSR(Rn, spec) \
    WORD $(0x8000f380 | (Rn & 0xf) | ((2 & 3) << 26) | ((spec & 0xff) << 16))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.34 */
#define DMB WORD $0x8f5ff3bf

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.35 */
#define DSB WORD $0x8f4ff3bf

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.38 */
#define ISB WORD $0x8f6ff3bf

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.25 */
#define CLREX WORD $0x8f2ff3bf

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.51 */
#define LDREX(Rt, Rn, offset) \
     WORD $(0x0f00e850 | ((Rt & 0xf) << 28) | (Rn & 0xf) | (((offset >> 2) & 0xff) << 16))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.168 */
#define STREX(Rd, Rt, Rn, offset) \
     WORD $(0x0000e840 | ((Rt & 0xf) << 28) | (Rn & 0xf) | ((Rd & 0xf) << 24) | \
                         (((offset >> 2) & 0xff) << 16))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.99, T2 */
#define PUSH(regs, lr) \
    WORD $(0x0000e92d | ((lr & 1) << 30) | ((regs & 0x1fff) << 16))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.98, T2 */
#define POP(regs, pc) \
    WORD $(0x0000e8bd | ((pc & 1) << 31) | ((regs & 0x1fff) << 16))

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

/* ARM Architecture Reference Manual Thumb-2 Supplement, page A7-534 */
#define VMRS(r) WORD $(0x0a10eef1 | (r)<<28) /* FP to ARM */
/* ARM Architecture Reference Manual Thumb-2 Supplement, page A7-535 */
#define VMSR(r) WORD $(0x0a10eee1 | (r)<<28) /* ARM to FP */

/* ARM Architecture Reference Manual Thumb-2 Supplement, page A7-557 */
#define VSTR(fp, Rn, offset) WORD $(0x0b00ed00 | (fp & 0xf) << 28 | ((offset >> 2) & 0xff) << 16 | (Rn))
/* ARM Architecture Reference Manual Thumb-2 Supplement, page A7-521 */
#define VLDR(fp, Rn, offset) WORD $(0x0b00ed10 | (fp & 0xf) << 28 | ((offset >> 2) & 0xff) << 16 | (Rn))

/* System control and ID registers
   ARMv7-M Architecture Reference Manual, B3.2.2 */
#define CPUID_ADDR 0xe000ed00
#define CCR_ADDR   0xe000ed14
#define CCR_DIV_0_TRP (1 << 4)
#define SHCSR_ADDR 0xe000ed24
#define SHCSR_USGFAULTENA (1 << 18)
#define SHCSR_MEMFAULTENA (1 << 16)
/* UFSR is the top 16 bits of CFSR */
#define CFSR_ADDR  0xe000ed28
#define UFSR_ADDR  0xe000ed2a
#define UFSR_UNDEFINSTR  1
#define HFSR_ADDR  0xe000ed2c
#define MMFAR_ADDR 0xe000ed34
#define CPACR_ADDR 0xe000ed88

#define FPCCR_ADDR 0xe000ef34
#define FPCAR_ADDR 0xe000ef38
#define FPDSCR_ADDR 0xe000ef3c
#define MVFR0_ADDR 0xe000ef40
#define MVFR1_ADDR 0xe000ef44

/* MRS and MSR encodings, ARMv7-M Architecture Reference Manual, B5.1.1 */
#define MRS_MSP 8
#define MRS_PSP 9
#define MRS_CONTROL 20

#endif
