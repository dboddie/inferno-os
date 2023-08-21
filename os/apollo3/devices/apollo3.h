typedef struct {
    unsigned int control;
    unsigned int reload;
    unsigned int current;
    unsigned int calibration;
} SysTick;

#define SYSTICK     0xe000e010

#define PWR_DEVPWREN 0x40021008
#define PWR_UART1 0x100
#define PWR_UART0 0x80
#define PWR_DEVPWRSTATUS 0x40021018
#define PWR_HCPA 0x04               /* includes UARTs */

/* Clock control and masks */
#define CLKGEN_CLKOUT 0x40004010
#define CLKGEN_CLKSEL_HFRC 0x19     /* in Arduino code, not in the datasheet */
#define CLKGEN_CKEN 0x80
#define CLKGEN_CLKKEY 0x40004014
#define CLKGEN_CCTRL 0x40004018
#define CLKGEN_CLOCKEN2STAT 0x4000402c
#define CLKGEN_STIMER_CNT_CLKEN 0x1000
#define CLKGEN_UART0HF_CLKEN 0x4000
#define CLKGEN_UART1HF_CLKEN 0x8000

/* Interrupts (NVIC) */
typedef struct {
    unsigned int iser0_31;
    unsigned int iser32_63;
    unsigned int iser64_95;
} NVIC;

#define NVIC_ISER 0xe000e100
#define NVIC_ISER0 NVIC_ISER    // interrupts 0-31
#define NVIC_ISER1 0xe000e104   // interrupts 32-63
#define NVIC_ISER2 0xe000e108   // interrupts 64-95

typedef struct {
    unsigned int icer0_31;
    unsigned int icer32_63;
    unsigned int icer64_95;
} NVIC_clear;

#define NVIC_ICER 0xe000e180
#define NVIC_ICER0 NVIC_ICER    // interrupts 0-31
#define NVIC_ICER1 0xe000e184   // interrupts 32-63
#define NVIC_ICER2 0xe000e188   // interrupts 64-95

/* GPIO */
#define GPIO_padregE 0x40010010
#define GPIO_padregM 0x40010030

#define GPIO_cfgC 0x40010048
#define GPIO_cfgG 0x40010058

#define GPIO_padkey 0x40010060

#define GPIO_wtA 0x40010088
#define GPIO_wtsA 0x40010090
#define GPIO_wtcA 0x40010098
#define GPIO_enA 0x400100a0
#define GPIO_ensA 0x400100a8
#define GPIO_encA 0x400100b4

#define GPIO_altpadcfgE 0x400100f0
#define GPIO_altpadcfgM 0x40010110
