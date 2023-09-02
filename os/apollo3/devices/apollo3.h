typedef struct {
    unsigned int control;
    unsigned int reload;
    unsigned int current;
    unsigned int calibration;
} SysTick;

#define SYSTICK     0xe000e010

#define CACHECTRL_CACHECFG 0x40018000
#define CACHECTRL_FLASHCFG 0x40018004
#define CACHECTRL_CTRL 0x40018008

#define SRAMMODE 0x40020284

#define PWR_DEVPWREN 0x40021008
#define PWR_UART1 0x100
#define PWR_UART0 0x80
#define PWR_IOM4 0x20
#define PWR_IOM3 0x10
#define PWR_MEMPWRSTATUS 0x40021014
#define PWR_DEVPWRSTATUS 0x40021018
#define PWR_HCPC 0x10               /* includes IOM 3,4,5 */
#define PWR_HCPA 0x04               /* includes UARTs */
#define PWR_MCUL 0x01               /* DMA and Fabrics */

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
#define GPIO_padregA 0x40010000
#define GPIO_padregE 0x40010010
#define GPIO_padregJ 0x40010024
#define GPIO_padregK 0x40010028
#define GPIO_padregM 0x40010030

#define GPIO_cfgA 0x40010040
#define GPIO_cfgC 0x40010048
#define GPIO_cfgE 0x40010050
#define GPIO_cfgF 0x40010054
#define GPIO_cfgG 0x40010058

#define GPIO_padkey 0x40010060

#define GPIO_rdA 0x40010080
#define GPIO_rdB 0x40010084
#define GPIO_wtA 0x40010088
#define GPIO_wtsA 0x40010090
#define GPIO_wtsB 0x40010094
#define GPIO_wtcA 0x40010098
#define GPIO_wtcB 0x4001009c
#define GPIO_enA 0x400100a0
#define GPIO_ensA 0x400100a8
#define GPIO_encA 0x400100b4

#define GPIO_altpadcfgA 0x400100e0
#define GPIO_altpadcfgE 0x400100f0
#define GPIO_altpadcfgJ 0x40010104
#define GPIO_altpadcfgK 0x40010108
#define GPIO_altpadcfgM 0x40010110
