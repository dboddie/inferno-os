#define BASE 0x4000
#define STACKTOP 0x20020000
#define DUMMY _dummy(SB)
#define RESET _start(SB)
#define SYSTICK _systick(SB)

/* See page 372 of the STM32F405/415 Reference Manual RM0090 */

TEXT vectors(SB), $0
    WORD    $STACKTOP
    WORD    $RESET
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $DUMMY
    WORD    $0
    WORD    $0
    WORD    $0
    WORD    $0
    WORD    $DUMMY
    WORD    $0
    WORD    $0
    WORD    $DUMMY
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
