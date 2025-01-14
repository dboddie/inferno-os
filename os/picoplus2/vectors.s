#define STACKTOP 0x1005fffc
#define DUMMY _dummy(SB)
#define RESET _start(SB)
#define SYSTICK _systick(SB)
#define HARD_FAULT _hard_fault(SB)
#define USAGE_FAULT _usage_fault(SB)
#define SECURE_FAULT _secure_fault(SB)
#define NMI _nmi(SB)
#define MEM_MANAGE _mem_manage(SB)
#define BUS_FAULT _bus_fault(SB)
#define SVCALL _svcall(SB)
#define DEBUGMON _debugmon(SB)
#define PENDSV _pendsv(SB)

TEXT vectors(SB), $0
    WORD    $STACKTOP
    WORD    $RESET
    WORD    $NMI
    WORD    $HARD_FAULT
    WORD    $MEM_MANAGE
    WORD    $BUS_FAULT
    WORD    $USAGE_FAULT
    WORD    $DUMMY
    WORD    $0
    WORD    $0
    WORD    $0
    WORD    $SVCALL
    WORD    $DUMMY
    WORD    $0
    WORD    $PENDSV
    WORD    $SYSTICK

    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY
    WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY; WORD    $DUMMY

#include "bootblock.s"
