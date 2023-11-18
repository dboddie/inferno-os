#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/uart.h"

#include "devices/fns.h"

void kbdinit(void)
{
    kbdq = qopen(128, 0, nil, nil);
    qnoblock(kbdq, 1);
    enable_usart_intr(0);
}

void uart3_intr(void)
{
    int c;
    if (!rdch_ready())
        return;

    c = rdch();
    /* Filter backspace */
    if (c == 127) {
        kbdputc(kbdq, 8);
        wrch(8);
        wrch(32);
        wrch(8);
        return;
    }
    if (c != 13) {
        /* Process characters normally */
        kbdputc(kbdq, c);
        wrch(c);
    } else {
        /* Add additional newlines for carriage returns */
        char buf[2];
        buf[0] = 13; buf[1] = 10;
        qproduce(kbdq, buf, 2);
        wrch(13);
        wrch(10);
    }
}
