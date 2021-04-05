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
    InterruptTCU0 = 0x00800000
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
