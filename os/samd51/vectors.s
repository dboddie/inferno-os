#define STACKTOP 0x200007fc
#define DUMMY _dummy(SB)
#define RESET _start(SB)
#define SYSTICK _systick(SB)
#define HARD_FAULT _hard_fault(SB)
#define USAGE_FAULT _usage_fault(SB)
#define NMI _nmi(SB)
#define MEM_MANAGE _mem_manage(SB)
#define BUS_FAULT _bus_fault(SB)
#define SVCALL _svcall(SB)
#define PENDSV _pendsv(SB)
#define SERCOM5_RXC_INT _sercom5_rxc_intr(SB)

/* See page 372 of the STM32F405/415 Reference Manual RM0090 */

TEXT vectors(SB), $0
    WORD    $STACKTOP
    WORD    $RESET
    WORD    $NMI
    WORD    $HARD_FAULT
    WORD    $MEM_MANAGE
    WORD    $BUS_FAULT
    WORD    $USAGE_FAULT
    WORD    $0
    WORD    $0
    WORD    $0
    WORD    $0
    WORD    $SVCALL
    WORD    $0
    WORD    $0
    WORD    $PENDSV
    WORD    $SYSTICK

/* Handlers for peripherals start at 0x40 from the start of the vector table */

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

/* 0x100 after vector table start */

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $SERCOM5_RXC_INT
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

/* 0x200 after vector table start */

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY

/* 0x264 after vector table start */
