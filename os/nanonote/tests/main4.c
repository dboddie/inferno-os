#include "mem.h"

#define Orange  0xff7f00

void draw(unsigned int v)
{
    unsigned int *LCDSA0 = (unsigned int *)0xb3050044;
    unsigned int *addr = (unsigned int *)(*LCDSA0 | KSEG1);
    unsigned int i;

    for (i = 0; i < 0x9800; i++) {
        addr[i] = v;
    }
}

void main(void)
{
//  draw(Orange);
    fbdraw(Orange);
}
