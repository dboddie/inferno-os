typedef struct {
    unsigned int control;
    unsigned int reload;
    unsigned int current;
    unsigned int calibration;
} SysTick;

#define SYSTICK     0xe000e010

typedef struct {
    unsigned int proc0_ctrl;
    unsigned int proc0_cycles;
    unsigned int proc0_count;
    unsigned int proc1_ctrl;
    unsigned int proc1_cycles;
    unsigned int proc1_count;
    unsigned int timer0_ctrl;
    unsigned int timer0_cycles;
    unsigned int timer0_count;
    unsigned int timer1_ctrl;
    unsigned int timer1_cycles;
    unsigned int timer1_count;
    unsigned int watchdog_ctrl;
    unsigned int watchdog_cycles;
    unsigned int watchdog_count;
    unsigned int riscv_ctrl;
    unsigned int riscv_cycles;
    unsigned int riscv_count;
} Ticks;

#define TICKS_BASE0 0x40108000

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

#define NVIC_ICPR0 0xe000e280
#define NVIC_ICPR1 0xe000e284
#define NVIC_IPR 0xe000e400

/* GPIO */
#define IO_BANK0_BASE 0x40028000
#define GPIO4_IO_ADDR (IO_BANK0_BASE + 0x20)
#define GPIO5_IO_ADDR (IO_BANK0_BASE + 0x28)
#define GPIO25_IO_ADDR (IO_BANK0_BASE + 0xc8)
#define GPIO47_IO_ADDR (IO_BANK0_BASE + 0x178)

typedef struct {
    unsigned int status;
    unsigned int ctrl;
} GPIOctrl;

#define PADS_BANK0_BASE 0x40038000
#define GPIO4_PAD_ADDR (PADS_BANK0_BASE + 0x14)
#define GPIO5_PAD_ADDR (PADS_BANK0_BASE + 0x18)
#define GPIO25_PAD_ADDR (PADS_BANK0_BASE + 0x68)
#define GPIO47_PAD_ADDR (PADS_BANK0_BASE + 0xc0)

enum {
    PADS_IE = 0x40,
    PADS_DRIVE_12mA = 0x30,
    PADS_DRIVE_8mA = 0x20,
    PADS_DRIVE_4mA = 0x10,
    PADS_DRIVE_2mA = 0x00,
    PADS_PUE = 8,
    PADS_PDE = 4,
    PADS_SCHMITT = 2,
    PADS_SLEWFAST = 1
};

#define SIO_BASE 0xd0000000
#define SIO_NONSEC_BASE 0xd0020000

typedef struct {
    unsigned int cpuid;
    unsigned int gpio_in;
    unsigned int gpio_hi_in;
    unsigned int _0;
    unsigned int gpio_out;
    unsigned int gpio_hi_out;
    unsigned int gpio_out_set;
    unsigned int gpio_hi_out_set;
    unsigned int gpio_out_clr;
    unsigned int gpio_hi_out_clr;
    unsigned int gpio_out_xor;
    unsigned int gpio_hi_out_xor;
    unsigned int gpio_oe;
    unsigned int gpio_hi_oe;
    unsigned int gpio_oe_set;
    unsigned int gpio_hi_oe_set;
    unsigned int gpio_oe_clr;
    unsigned int gpio_hi_oe_clr;
    unsigned int gpio_oe_xor;
    unsigned int gpio_hi_oe_xor;
} SIOregs;

#define UART0_BASE 0x40070000
#define UART1_BASE 0x40078000

typedef struct {
    unsigned int dr;
    unsigned int rsr;
    unsigned int _0;
    unsigned int _1;
    unsigned int _2;
    unsigned int _3;
    unsigned int fr;
    unsigned int _4;
    unsigned int ilpr;
    unsigned int ibrd;
    unsigned int fbrd;
    unsigned int lcr_h;
    unsigned int cr;
    unsigned int ifls;
    unsigned int imsc;
    unsigned int ris;
    unsigned int mis;
    unsigned int icr;
    unsigned int dmacr;
} UART;

enum {
    UARTCR_RXE = 0x200,
    UARTCR_TXE = 0x100,
    UARTCR_EN = 0x001,

    UARTLCR_H_WLEN_8 = 0x60,
    UARTLCR_H_FEN = 0x10,

    UARTFR_TXFE = 0x80,
    UARTFR_RXFF = 0x40,
    UARTFR_TXFF = 0x20,
    UARTFR_RXFE = 0x10,
    UARTFR_BUSY = 0x08,
};

#define UART0_IRQ 33
#define UART1_IRQ 34

#define PLL_SYS_BASE 0x40050000
#define PLL_USB_BASE 0x40058000

typedef struct {
    unsigned int cs;
    unsigned int pwr;
    unsigned int fbdiv_int;
    unsigned int prim;
    unsigned int intr;
    unsigned int inte;
    unsigned int intf;
    unsigned int ints;
} PLL;

enum {
    PLL_CS_LOCK = (1 << 31),
    PLL_PWR_VCOPD = 0x20,
    PLL_PWR_POSTDIVD = 0x08,
    PLL_PWR_PD = 0x01,
};

#define RESETS_BASE 0x40020000
#define RESETS_CLR_BASE 0x40023000

typedef struct {
    unsigned int reset;
    unsigned int wdsel;
    unsigned int reset_done;
} Resets;

enum {
    RESETS_USB = (1 << 28),
    RESETS_UART1 = (1 << 27),
    RESETS_PLL_USB = (1 << 15),
    RESETS_PLL_SYS = (1 << 14),
    RESETS_IO_BANK0 = (1 << 6),
};

// See section 8.1.6 in the RP2350 datasheet.
#define CLOCKS_BASE 0x40010000
#define CLK_REF_ADDR (CLOCKS_BASE + 0x30)
#define CLK_SYS_ADDR (CLOCKS_BASE + 0x3c)
#define CLK_PERI_ADDR (CLOCKS_BASE + 0x48)
#define CLK_USB_ADDR (CLOCKS_BASE + 0x60)

typedef struct {
    unsigned int ctrl;
    unsigned int div;
    unsigned int selected;
} Clocks;

#define CLK_REF_CTRL_XOSC_CLKSRC 2

#define CLK_CTRL_ENABLE (1 << 11)

#define CLK_PERI_CTRL_ENABLE CLK_CTRL_ENABLE
#define CLK_PERI_CTRL_CLKSRC_PLL_SYS (1 << 5)
#define CLK_PERI_CTRL_XOSC_CLKSRC (4 << 5)
#define CLK_USB_CTRL_XOSC_CLKSRC (3 << 5)
#define CLK_USB_CTRL_CLKSRC_PLL_USB 0
#define CLK_SYS_CTRL_CLKSRC_PLL_SYS (0 << 5)
#define CLK_SYS_CTRL_CLKSRC_PLL_USB (1 << 5)
#define CLK_SYS_CTRL_CLKSRC_CLK_SYS_AUX 1

#define CLK_SYS_RESUS_CTRL (CLOCKS_BASE + 0x84)

#define XOSC_BASE 0x40048000

typedef struct {
    unsigned int ctrl;
    unsigned int status;
    unsigned int dormant;
    unsigned int startup;
    unsigned int count;
} XOSC;

enum {
    XOSC_DISABLE = (0xd1e << 12),
    XOSC_ENABLE = (0xfab << 12),
    XOSC_1_15MHZ = 0xaa0,
    XOSC_10_30MHZ = 0xaa1,
    XOSC_25_60MHZ = 0xaa2,
    XOSC_40_100MHZ = 0xaa3,

    XOSC_STABLE = (1 << 31),

    XOSC_X4 = (1 << 20),
};

#define XOSC_FREQ 12000000

#define USBCTRL_REGS_BASE 0x50110000
#define USBCTRL_REGS_CLR_BASE 0x50113000
#define USB_DPRAM_BASE 0x50100000

typedef struct {
    unsigned int addr_endp[16];
    unsigned int main_ctrl;
    unsigned int sof_wr;
    unsigned int sof_rd;
    unsigned int sie_ctrl;
    unsigned int sie_status;
    unsigned int int_ep_ctrl;
    unsigned int buff_status;
    unsigned int buff_cpu_should_handle;
    unsigned int ep_abort;
    unsigned int ep_abort_done;
    unsigned int ep_stall_arm;
    unsigned int nak_poll;
    unsigned int ep_status_stall_nak;
    unsigned int usb_muxing;
    unsigned int usb_pwr;
    unsigned int usbphy_direct;
    unsigned int usbphy_direct_override;
    unsigned int usbphy_trim;
    unsigned int linestate_tuning;
    unsigned int intr;
    unsigned int inte;
    unsigned int intf;
    unsigned int ints;
    unsigned int sof_timestamp_raw;
    unsigned int sof_timestamp_last;
    unsigned int sm_state;
    unsigned int ep_tx_error;
    unsigned int ep_rx_error;
    unsigned int dev_sm_watchdog;
} USBregs;

enum {
    USB_MAIN_CTRL_PHY_ISO = 0x4,
    USB_MAIN_CTRL_HOST_NDEVICE = 0x2,
    USB_MAIN_CTRL_CONTROLLER_EN = 0x1,
    USB_MUXING_SOFTCON = 0x8,
    USB_MUXING_TO_PHY = 0x1,
    USB_SIE_CTRL_EP0_INT_1BUF = 0x20000000,
    USB_SIE_CTRL_PULLUP_EN = 0x10000,
    USB_SIE_STATUS_BUS_RESET = 0x80000,
    USB_SIE_STATUS_SETUP_REC = 0x20000,
    USB_INT_SETUP_REQ = 0x10000,
    USB_INT_BUS_RESET = 0x1000,
    USB_INT_BUFF_STATUS = 0x10,
    USB_PWR_VBUS_DETECT_OVERRIDE_EN = 0x8,
    USB_PWR_VBUS_DETECT = 0x4,
};

#define USBCTRL_DEVREGS_BASE (USBCTRL_REGS_BASE + 0x100)

typedef struct {
    unsigned int sof_timestamp_raw;
    unsigned int sof_timestamp_last;
    unsigned int sm_state;
    unsigned int ep_tx_error;
    unsigned int ep_rx_error;
    unsigned int dev_sm_watchdog;
} USBdevregs;

#define USBCTRL_IRQ 14
