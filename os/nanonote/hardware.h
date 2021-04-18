#define CGU_CCR   0x10000000
#define CGU_CLKGR 0x10000020

enum CGUGates {
    CGU_RTC = 4,
    CGU_TCU = 2
};

#define RTC_BASE 0x10003000

struct RTC {
    ulong  control;
    ulong  second;
    ulong  second_alarm;
    ulong  regulator;
};
typedef struct RTC RTC;

#define INTERRUPT_BASE 0x10001000

struct InterruptCtr {
    ulong source;                   /* ICSR */
    ulong mask;                     /* ICMR */
    ulong mask_set;                 /* ICMSR */
    ulong mask_clear;               /* ICMCR */
    ulong pending;                  /* ICPR */
};
typedef struct InterruptCtr InterruptCtr;

enum InterruptSource {
    InterruptTCU0  = 0x00800000,
    InterruptGPIO0 = 0x10000000,
    InterruptGPIO1 = 0x08000000,
    InterruptGPIO2 = 0x04000000,
    InterruptGPIO3 = 0x02000000
};

#define TIMER_BASE 0x10002010

struct JZTimer {
    ulong counter_enable;           /* TER */
    ulong counter_enable_set;       /* TESR */
    ulong counter_enable_clear;     /* TECR */
    ulong stop;                     /* TSR */

    ulong flag;                     /* TFR */
    ulong flag_set;                 /* TFSR */
    ulong flag_clear;               /* TFCR */
    ulong stop_set;                 /* TSSR */

    ulong mask;                     /* TMR */
    ulong mask_set;                 /* TMSR */
    ulong mask_clear;               /* TMCR */
    ulong stop_clear;               /* TSCR */

    ulong data_full0;               /* TDFR0 */
    ulong data_half0;               /* TDHR0 */
    ulong counter0;                 /* TCNT0 */
    ulong control0;                 /* TCSR0 */
    /* ... until TCSR7 */
};
typedef struct JZTimer JZTimer;

enum TimerCounterEnable {
    TimerCounter0 = 0x01,
    TimerCounter1 = 0x02,
    TimerCounter2 = 0x04,
    TimerCounter3 = 0x08,
    TimerCounter4 = 0x10,
    TimerCounter5 = 0x20,
    TimerCounter6 = 0x40,
    TimerCounter7 = 0x80,
    TimerCounterAll = 0xff
};

enum TimerPrescale {
    TimerPrescale1 = 0,
    TimerPrescale4 = 0x08,
    TimerPrescale16 = 0x10,
    TimerPrescale64 = 0x18,
    TimerPrescale256 = 0x20,
    TimerPrescale1024 = 0x28
};

enum TimerSource {
    TimerSourceExt = 4,
    TimerSourceRTC = 2,
    TimerSourcePCLK = 1
};

#define GPIO_PORT_C_PIN     0x10010200
#define GPIO_PORT_C_DATA    0x10010210
#define GPIO_PORT_C_INTMASK 0x10010220
#define GPIO_PORT_C_PULL    0x10010230
#define GPIO_PORT_C_FUNC    0x10010240
#define GPIO_PORT_C_SEL     0x10010250
#define GPIO_PORT_C_DIR     0x10010260
#define GPIO_PORT_C_TRIG    0x10010270
#define GPIO_PORT_C_FLAG    0x10010280

#define GPIO_PORT_D_PIN     0x10010300
#define GPIO_PORT_D_DATA    0x10010310
#define GPIO_PORT_D_INTMASK 0x10010320
#define GPIO_PORT_D_PULL    0x10010330
#define GPIO_PORT_D_FUNC    0x10010340
#define GPIO_PORT_D_SEL     0x10010350
#define GPIO_PORT_D_DIR     0x10010360
#define GPIO_PORT_D_TRIG    0x10010370
#define GPIO_PORT_D_FLAG    0x10010380

enum GPIOPins {
    GPIO_Power = 0x20000000,
    GPIO_Keyboard_In_Mask = 0x05fc0000,
    GPIO_Keyboard_Out_Mask = 0x0003fc00
};

struct GPIO {
    ulong data;
    ulong set;
    ulong clear;
};
typedef struct GPIO GPIO;
