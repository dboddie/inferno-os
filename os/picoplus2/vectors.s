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
#define USBCTRL _usbctrl(SB)
#define UARTINTR _uart_intr(SB)

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
/* Interrupts */
    WORD    $DUMMY      /* TIMER0_IRQ_0 */      /* 0 */
    WORD    $DUMMY      /* TIMER0_IRQ_1 */
    WORD    $DUMMY      /* TIMER0_IRQ_2 */
    WORD    $DUMMY      /* TIMER0_IRQ_3 */
    WORD    $DUMMY      /* TIMER1_IRQ_0 */
    WORD    $DUMMY      /* TIMER1_IRQ_1 */
    WORD    $DUMMY      /* TIMER1_IRQ_2 */
    WORD    $DUMMY      /* TIMER1_IRQ_3 */
    WORD    $DUMMY      /* PWM_IRQ_WRAP_0 */
    WORD    $DUMMY      /* PWM_IRQ_WRAP_1 */
    WORD    $DUMMY      /* DMA_IRQ_0 */         /* 10 */
    WORD    $DUMMY      /* DMA_IRQ_1 */
    WORD    $DUMMY      /* DMA_IRQ_2 */
    WORD    $DUMMY      /* DMA_IRQ_3 */
    WORD    $USBCTRL    /* USBCTRL_IRQ */       /* 14 */
    WORD    $DUMMY      /* PIO0_IRQ0 */
    WORD    $DUMMY      /* PIO0_IRQ1 */
    WORD    $DUMMY      /* PIO1_IRQ0 */
    WORD    $DUMMY      /* PIO1_IRQ1 */
    WORD    $DUMMY      /* PIO2_IRQ0 */
    WORD    $DUMMY      /* PIO2_IRQ1 */         /* 20 */
    WORD    $DUMMY      /* IO_IRQ_BANK0 */
    WORD    $DUMMY      /* IO_IRQ_BANK0_NS */
    WORD    $DUMMY      /* IO_IRQ_QSPI */
    WORD    $DUMMY      /* IO_IRQ_QSPI_NS */
    WORD    $DUMMY      /* SIO_IRQ_FIFO */
    WORD    $DUMMY      /* SIO_IRQ_BELL */
    WORD    $DUMMY      /* SIO_IRQ_FIFO_NS */
    WORD    $DUMMY      /* SIO_IRQ_BELL_NS */
    WORD    $DUMMY      /* SIO_IRQ_MTIMECMP */
    WORD    $DUMMY      /* CLOCKS_IRQ */        /* 30 */
    WORD    $DUMMY      /* SPI0_IRQ */
    WORD    $DUMMY      /* SPI1_IRQ */
    WORD    $UARTINTR   /* UART0_IRQ */         /* 33 */
    WORD    $UARTINTR   /* UART1_IRQ */         /* 34 */
    WORD    $DUMMY      /* ADC_IRQ_FIFO */
    WORD    $DUMMY      /* I2C0_IRQ */
    WORD    $DUMMY      /* I2C1_IRQ */
    WORD    $DUMMY      /* OTP_IRQ */
    WORD    $DUMMY      /* TRNG_IRQ */
    WORD    $DUMMY      /* PROC0_IRQ_CTI */     /* 40 */
    WORD    $DUMMY      /* PROC1_IRQ_CTI */
    WORD    $DUMMY      /* PLL_SYS_IRQ */
    WORD    $DUMMY      /* PLL_USB_IRQ */
    WORD    $DUMMY      /* POWMAN_IRQ_POW */
    WORD    $DUMMY      /* POWMAN_IRQ_TIMER */
/* Spare IRQs (internal to cores) */
    WORD    $DUMMY      /* SPAREIRQ_IRQ_0 */
    WORD    $DUMMY      /* SPAREIRQ_IRQ_1 */
    WORD    $DUMMY      /* SPAREIRQ_IRQ_2 */
    WORD    $DUMMY      /* SPAREIRQ_IRQ_3 */
    WORD    $DUMMY      /* SPAREIRQ_IRQ_4 */    /* 50 */
    WORD    $DUMMY      /* SPAREIRQ_IRQ_5 */

#include "bootblock.s"
