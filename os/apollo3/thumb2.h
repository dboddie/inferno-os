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

/* ARMv7-M Architecture Reference Manual, A7.7.82, mask=3 (nzcvqg bits)
   See B5.1.1 for information about the bits. */
#define MSR(Rn, spec) \
    WORD $(0x8000f380 | (Rn & 0xf) | ((3 & 3) << 26) | ((spec & 0xff) << 16))

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

#define POP_LR_PC(regs, lr, pc) \
    WORD $(0x0000e8bd | ((lr & 1) << 30) | ((pc & 1) << 31) | ((regs & 0x1fff) << 16))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.43, T3 */
#define LDR_imm(Rt, Rn, imm12) \
    WORD $(0x0000f8d0 | (Rn & 0xf) | ((imm12 & 0xfff) << 16) | ((Rt & 0xf) << 28))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.45, T2 */
#define LDR2(Rt, Rn, Rm, shift) \
    WORD $(0x0000f850 | (Rn & 0xf) | ((Rm & 0xf) << 16) | ((shift & 0x3) << 20) | ((Rt & 0xf) << 28))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.162, T3 */
#define STR_imm(Rt, Rn, imm12) \
    WORD $(0x0000f8c0 | (Rn & 0xf) | ((imm12 & 0xfff) << 16) | ((Rt & 0xf) << 28))

/* ARM Architecture Reference Manual Thumb-2 Supplement, 4.6.163, T2 */
#define STR2(Rt, Rn, Rm, shift) \
    WORD $(0x0000f840 | (Rn & 0xf) | ((Rm & 0xf) << 16) | ((shift & 0x3) << 20) | ((Rt & 0xf) << 28))

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

/* ARMv7-M Architecture Reference Manual, A7.7.256 */
#define VSTR(fp, Rn, offset) WORD $(0x0b00ed80 | (fp & 0xf) << 28 | ((offset >> 2) & 0xff) << 16 | (Rn))
/* ARMv7-M Architecture Reference Manual, A7.7.233 */
#define VLDR(fp, Rn, offset) WORD $(0x0b00ed90 | (fp & 0xf) << 28 | ((offset >> 2) & 0xff) << 16 | (Rn))

/* ARMv7-M Architecture Reference Manual, A7.7.248, T1 */
#define VPUSH(first_reg, number_of_double_regs) \
    WORD $(0x0b00ed2d | (((first_reg & 0x10) << 2) | ((first_reg & 0xf) << 28) | \
           ((number_of_double_regs & 0x7f) << 17)))
/* ARMv7-M Architecture Reference Manual, A7.7.249, T1 */
#define VPOP(first_reg, number_of_double_regs) \
    WORD $(0x0b00ecbd | (((first_reg & 0x10) << 2) | ((first_reg & 0xf) << 28) | \
           ((number_of_double_regs & 0x7f) << 17)))

/* System control and ID registers
   ARMv7-M Architecture Reference Manual, B3.2.2 */
#define CPUID_ADDR 0xe000ed00
#define CCR_ADDR   0xe000ed14
#define CCR_STKALIGN (1 << 9)
#define CCR_DIV_0_TRP (1 << 4)
#define CCR_UNALIGN_TRP (1 << 3)
#define SHCSR_ADDR 0xe000ed24
#define SHCSR_USGFAULTENA (1 << 18)
#define SHCSR_MEMFAULTENA (1 << 16)
/* UFSR is the top 16 bits of CFSR */
#define CFSR_ADDR  0xe000ed28
#define UFSR_ADDR  0xe000ed2a
#define UFSR_UNDEFINSTR  1
#define HFSR_ADDR  0xe000ed2c
#define MMFAR_ADDR 0xe000ed34
#define BFAR_ADDR  0xe000ed38
#define CPACR_ADDR 0xe000ed88

#define FPCCR_ADDR 0xe000ef34
#define FPCAR_ADDR 0xe000ef38
#define FPDSCR_ADDR 0xe000ef3c
#define MVFR0_ADDR 0xe000ef40
#define MVFR1_ADDR 0xe000ef44

#define FPCCR_ASPEN 0x80000000
#define FPCCR_LSPEN 0x40000000

/* MRS and MSR encodings, ARMv7-M Architecture Reference Manual, B5.1.1 */
#define MRS_MSP 8
#define MRS_PSP 9
#define MRS_PRIMASK 16
#define MRS_BASEPRI 17
#define MRS_FAULTMASK 19
#define MRS_CONTROL 20

#define CONTROL_NPRIV 1
#define CONTROL_SPSEL 2
#define CONTROL_FPCA 4

/* System handler priority register 3 (SysTick is in top 4/8 bits) and
   equivalent for IRQs 36-39 (UART3 is 39, so in the top 4/8 bits). */
#define SHPR1 0xe000ed18
#define SHPR2 0xe000ed1c
#define SHPR3 0xe000ed20
#define NVIC_IPR9 0xe000e424

/* System control block (STM32 Cortex-M4 programming manual, section 4.4.5) */
#define SCB_VTOR 0xe000ed08
#define SCB_AIRCR 0xe000ed0c
#define SCB_AIRCR_VECTKEYRESET 0x05fa0000
#define SCB_AIRCR_SYSRESETREQ 0x04
#define SCB_AIRCR_VECTRESET 0x01

/* ARMv7-M Architecture Reference Manual, B3.2.4 */
#define ICSR 0xe000ed04
#define VECTACTIVE_MASK 0x1ff

/* ARMv7-M Architecture Reference Manual, B1.4.2 */
#define EPSR_T 0x01000000

/* Floating point features (ARMv7-M Architecture Reference Manual, B4.7) */
#define MVFR0 0xe000ef40
#define MVFR1 0xe000ef44
#define MVFR2 0xe000ef48

#endif
