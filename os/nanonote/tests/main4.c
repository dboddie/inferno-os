#include "mem.h"

extern void fbdraw(unsigned int v);

#define Pink    0xff80ff

unsigned int main(void)
{
/*
    unsigned int *LCDSA0 = (unsigned int *)0xb3050044;
    unsigned int addr[] = *LCDSA0 | KSEG1;

    for (unsigned int i = 0; i < 0x9800; i++) {
        addr[i] = 0xff4477cc;
    }

    for (;;) {}
*/
//    fbdraw(0xff4477cc); /* blue */

    return Pink;
}
