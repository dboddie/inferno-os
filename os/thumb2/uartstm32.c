/* This file must be given a name that begins with "uart". */

#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/uart.h"

#include "devices/fns.h"

/* This file will get built as a result of its entry in the misc section of the
   thumb2.conf file. */

/* Forward declaration of the API structure. The name must end in "uart". */
extern PhysUart stm32physuart;

static Uart uart3 = {
    .name = "uart3",
    .phys = &stm32physuart,
};

static Uart* pnp(void)
{
    Uart *uart;

    uart = &uart3;
    if(uart->console == 0 && kbdq == 0) {
        kbdq = qopen(64, 0, nil, nil);
        qnoblock(kbdq, 1);
    }
    return uart;
}

static void enable(Uart *, int ie)
{
    enable_usart_intr(ie);
//    setup_usart();
}

static void disable(Uart *) {}
static void kick(Uart *) {}

static int stop(Uart *uart, int n)
{
    if(n != 1)
        return -1;
    uart->stop = n;
    return 0;
}

static long status(Uart *, void *, long, long)
{
    return 0;
}

void putc(Uart*, int c)
{
    wrch(c);
}

int getc(Uart*)
{
    return rdch_wait();
}

static void donothing(Uart*, int) {}

static int donothing_ret(Uart *, int)
{
    return 0;
}

void uartconsinit(void)
{
    Uart *uart;

    uart = &uart3;

    if(!uart->enabled)
        (*uart->phys->enable)(uart, 0);
    uartctl(uart, "b115200 l8 pn s1");

    consuart = uart;
    uart->console = 1;
}

PhysUart stm32physuart = {
	.name		= "uart3",
	.pnp		= pnp,
	.enable		= enable,
	.disable	= disable,
	.kick		= kick,
	.dobreak	= donothing,
	.baud		= donothing_ret,
	.bits		= donothing_ret,
	.stop		= donothing_ret,
	.parity		= donothing_ret,
	.modemctl	= donothing,
	.rts		= donothing,
	.dtr		= donothing,
	.status		= status,
	.fifo		= donothing,
	.getc		= getc,
	.putc		= putc,
};

/* Not part of the above API. Called from the l.s file. */
void uart3_intr(void)
{
    Uart *uart = &uart3;

    if (uart->console) {
        if (uart->opens == 1)
            uart->putc = kbdcr2nl;
        else
            uart->putc = nil;
    }

    uartrecv(uart, rdch());
}
