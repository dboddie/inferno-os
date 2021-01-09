#include "mem.h"

extern void fbdraw(unsigned int v);

#define Pink    0xff80ff
#define Yellow  0xffff40

void main(void)
{
    unsigned int *LCDSA0 = (unsigned int *)0xb3050044;      // a3
    unsigned int *addr = (unsigned int *)(*LCDSA0 | KSEG1); // v1 gp -> t0
    unsigned int i;

    for (i = 0; i < 0x9800; i++) {                      // a3 t2
        addr[i] = Yellow;                               // t1
    }

//    fbdraw(0xff4477cc); /* blue */
}
