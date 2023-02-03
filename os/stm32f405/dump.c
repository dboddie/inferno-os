#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "ureg.h"
#include "thumb2.h"

void dumpstack(void)
{
    ulong sp = getsp();
    for (int i = 0; i < 512; i+=4) {
        wrhex(*(int *)(sp + i));
        if (i % 16 == 12)
            newline();
        else
            wrch(' ');
    }
}
