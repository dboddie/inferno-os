#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"
#include "thumb2.h"
#include "../port/error.h"

int
fpithumb2(Ereg *eregs)
{
    print("pc=%lux\n", eregs->pc);
    print("%04ux %04ux\n", *(ushort *)eregs->pc, *((ushort *)eregs->pc + 1));

    ushort w0 = *(ushort *)eregs->pc;
    ushort w1 = *(ushort *)eregs->pc + 1;
    uint imm;

    switch (w0 & 0xff00) {
    case 0xee00:
        switch (w0 & 0xb0) {
        case 0x00:              // MOVWF
        case 0x10:              // MOVFW
        case 0x20:              // MULF/MULD
        case 0x30:              // ADDF|ADDD|SUBF|SUBD
        case 0x80:              // DIVF|DIVD
            break;
        case 0xb0:              // CMPF|CMPD|MOVF|MOVD
            imm = ((w0 & 0xf) << 4) | (w1 & 0xf);
            eregs->pc += 4;
            print("pc=%lux\n", eregs->pc);
            return 1;
        default:
            return 0;
        }
        break;
    case 0xfe00:                // MOVFD|MOVDF
        break;
    case 0xeb00:                // MOVF|MOVD
        break;
    default:
        return 0;
    }

    return 0;
}
