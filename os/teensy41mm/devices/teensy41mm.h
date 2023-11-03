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

/* IOMUX and GPIO */
#define IOMUXC_SW_MUX_CTL_PAD_GPIO_B0_03 0x401f8148
#define IOMUXC_SW_PAD_CTL_PAD_GPIO_B0_03 0x401f8338

/* GPIO2 and GPIO7 mux selection (11.3.28) */
#define IOMUXC_GPR_GPR27 0x400ac06c

#define GPIO2_DR 0x401bc000
#define GPIO2_GDIR 0x401bc004

// The secondary UART is connected to pads 17 (TX1) and 19 (RX1).
// Internally, these are AD_B0_02 (TX1) and AD_B0_03 (RX1) GPIOs.
#define IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_02 0x401f80c4
#define IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_03 0x401f80c8
#define IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_02 0x401f82b4
#define IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_03 0x401f82b8

// LPUART6 needs to be selected in a daisy chain with these pins.
#define IOMUXC_LPUART6_RX_SELECT_INPUT 0x401f8550
#define IOMUXC_LPUART6_TX_SELECT_INPUT 0x401f8554

// CCM
#define CCM_CSCMR1 0x400fc01c
#define CCM_CSCMR1_PERCLK_PODF_MASK 0x3f
#define CCM_CSCMR1_PERCLK_SEL 0x40
#define CCM_CSCDR1 0x400fc024
#define CCM_CSCDR1_UART_CLK_PODF_MASK 0x3f
#define CCM_CSCDR1_UART_CLK_SEL 0x40

#define CCM_CCGR3 0x400fc074
// lpuart6 clock (lpuart6_clk_enable)
#define CCM_CCGR3_CG3 0xc0
