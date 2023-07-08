typedef struct {
    unsigned int control;
    unsigned int reload;
    unsigned int current;
    unsigned int calibration;
} SysTick;

#define SYSTICK     0xe000e010

/* Clock control and masks */
#define GCLK_base 0x40001c00
#define GCLK_GENCTRL_base 0x40001c20
#define GCLK_GENCTRL_idc 0x200
#define GCLK_GENCTRL_genen 0x100
#define GCLK_GENCTRL_src_mask 0x1f
#define GCLK_GENCTRL_src_XOSC32K 5
#define GCLK_GENCTRL_src_DFLL 6
#define GCLK_PCHCTRL_base 0x40001c80
#define GCLK_PCHCTRL_chen 0x40
#define GCLK_PCHCTRL_gen_mask 0x0f
#define GCLK_USB 10

#define MCLK_AHB_mask 0x40000810
#define MCLK_AHB_USB 0x400
#define MCLK_APBA_mask 0x40000814
#define MCLK_APBA_SERCOM1 0x2000
#define MCLK_APBB_mask 0x40000818
#define MCLK_APBB_PORT 0x10
#define MCLK_APBB_USB 0x01
#define MCLK_APBD_mask 0x40000820
#define MCLK_APBD_SERCOM5 0x2

/* PORT registers */
#define PORT_base   0x41008000
#define PORT_dir 	0x41008000
#define PORT_dirclr 0x41008004
#define PORT_dirset 0x41008008
#define PORT_dirtgl 0x4100800c
#define PORT_out 	0x41008010
#define PORT_outclr 0x41008014
#define PORT_outset 0x41008018
#define PORT_outtgl 0x4100801c
#define PORT_in     0x41008020
#define PORT_ctrl   0x41008024

typedef struct {
    int dir;
    int dirclr;
    int dirset;
    int dirtgl;
    int out;
    int outclr;
    int outset;
    int outtgl;
    int in;
    int ctrl;
    int wrconfig;
    int evctrl;
} PORT;

#define PORT_pmux   0x41008030
#define PORT_pincfg 0x41008040
#define PORT_pincfg_drvstr 0x40
#define PORT_pincfg_pmuxen 0x01

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
