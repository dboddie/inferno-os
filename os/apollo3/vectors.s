#define STACKTOP 0x1005fffc
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

/* Handlers for peripherals start at 0x40 from the start of the vector table.
   See section 3.2 of the Apollo3 Blue MCU Data Sheet. */

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY      /* RTC (2) */
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
    WORD    $DUMMY      /* UART0 (15) */

    WORD    $DUMMY      /* UART1 (16) */
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY

    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY      /* STIMER_CMPA (23) */

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

/* 0xd0 after vector table start */
