#define CGU_CPCCR 0x10000000
#define CGU_CLKGR 0x10000020
#define CGU_SCR   0x10000024

enum CGUGates {
    CGU_UDC = 0x800,
    CGU_MSC = 0x080,
    CGU_RTC = 0x004,
    CGU_TCU = 0x002
};

#define CGU_SPENDN 0x40

#define RTC_BASE 0x10003000

typedef struct {
    ulong  control;
    ulong  second;
    ulong  second_alarm;
    ulong  regulator;
} RTC;

#define INTERRUPT_BASE 0x10001000

typedef struct {
    ulong source;                   /* ICSR */
    ulong mask;                     /* ICMR */
    ulong mask_set;                 /* ICMSR */
    ulong mask_clear;               /* ICMCR */
    ulong pending;                  /* ICPR */
} InterruptCtr;

enum InterruptSource {
    InterruptTCU0  = 0x00800000,
    InterruptGPIO0 = 0x10000000,
    InterruptGPIO1 = 0x08000000,
    InterruptGPIO2 = 0x04000000,
    InterruptGPIO3 = 0x02000000,
    InterruptUDC   = 0x01000000,
    InterruptMSC   = 0x00004000,
    InterruptUHC   = 0x00000008,
};

#define TIMER_BASE 0x10002010

typedef struct {
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
} JZTimer;

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

typedef struct {
    ulong data;
    ulong set;
    ulong clear;
} GPIO;

#define WATCHDOG_BASE 0x10002000

typedef struct {
    ulong data;         /* TDR */
    ulong enable;       /* TCER */
    ulong counter;      /* TCNT */
    ulong control;      /* TCSR */
} Watchdog;

enum WatchdogCtl {
    WD_PrescaleDiv1    = 0x00,
    WD_PrescaleDiv4    = 0x08,
    WD_PrescaleDiv16   = 0x10,
    WD_PrescaleDiv64   = 0x18,
    WD_PrescaleDiv256  = 0x20,
    WD_PrescaleDiv1024 = 0x28,
    WD_ExtEnable       = 4,
    WD_RTCEnable       = 2,
    WD_PCLKEnable      = 1
};

#define MSC_BASE 0x10021000

typedef struct {
    ulong clock_control;    /* STRPCL */
    ulong status;           /* STAT */
    ulong clock_rate;       /* CLKRT */
    ulong cmd_control;      /* CMDAT */
    ulong resp_time_out;    /* RESTO */
    ulong read_time_out;    /* RDTO */
    ulong block_length;     /* BLKLEN */
    ulong number_of_blocks; /* NOB */
    ulong success_blocks;   /* SNOB */
    ulong mask;             /* IMASK */
    ulong interrupt;        /* IREG */
    ulong cmd_index;        /* CMD */
    ulong cmd_arg;          /* ARG */
    ushort resp_fifo;       /* RES */
    ushort padding;
    ulong rec_data_fifo;    /* RXFIFO */
    ulong trans_data_fifo;  /* TXFIFO */
} MSC;

enum {
    MSC_ClockCtrl_Reset         = 0x08,
    MSC_ClockCtrl_StartOp       = 0x04,
    MSC_ClockCtrl_StartClock    = 0x02,
    MSC_ClockCtrl_StopClock     = 0x01,
    MSC_Status_IsResetting      = 0x8000,
    MSC_Status_DataTranDone     = 0x1000,
    MSC_Status_EndCmdRes        = 0x0800,
    MSC_Status_DataFIFOEmpty    = 0x0040,
    MSC_Status_CRCResError      = 0x0020,
    MSC_Status_CRCReadError     = 0x0010,
    MSC_Status_ClockEnabled     = 0x100,
    MSC_CmdCtrl_BusWidth        = 0x600,
    MSC_CmdCtrl_BusShift        = 9,
    MSC_CmdCtrl_OneBit          = 0,
    MSC_CmdCtrl_FourBit         = 2,
    MSC_CmdCtrl_Init            = 0x80,
    MSC_CmdCtrl_Busy            = 0x40,
    MSC_CmdCtrl_StreamBlock     = 0x20,
    MSC_CmdCtrl_WriteRead       = 0x10,
    MSC_CmdCtrl_DataEnabled     = 0x8,
    MSC_CmdCtrl_ResponseFormat  = 0x7,
};

#define USB_DEVICE_BASE 0x13040000

typedef struct {
    /* Common */
    uchar faddr;            /* function address */
    uchar power;
    ushort intr_in;
    ushort intr_out;
    ushort intr_in_enable;
    ushort intr_out_enable;
    uchar intr_usb;
    uchar intr_usb_enable;
    ushort frame;
    uchar index;            /* 0-15 to select the bank of registers below */
    uchar test_mode;

    /* Indexed - setting the index register to a value from 0 to 15 gives
       access to the bank of registers for the corresponding endpoint */
    ushort in_max_p;        /* 0x10 */
    ushort csr;             /* 0x12 */
    ushort out_max_p;       /* 0x14 */
    ushort out_csr;         /* 0x16 */
    ushort count;           /* 0x18 */

    ushort padding[3];

    /* FIFOs */
    uchar fifo[16][4];      /* 0x20-0x5f */
} USBDevice;

enum {
    /* INTRUSBE flags */
    USB_Reset                   = 0x4,

    /* Power flags */
    USB_Power_SoftConn          = 0x40,
    USB_Power_HighSpeed         = 0x20,

    /* CSR0 flags */
    USB_Ctrl_ServicedOutPktRdy  = 0x40,
    USB_Ctrl_SendStall          = 0x20,
    USB_Ctrl_DataEnd            = 0x08,
    USB_Ctrl_InPktRdy           = 0x02,
    USB_Ctrl_OutPktRdy          = 0x01,

    /* INCSR flags */
    USB_InAutoSet               = 0x8000,
    USB_InISO                   = 0x4000,
    USB_InMode                  = 0x2000,
    USB_InClrDataTog            = 0x40,
    USB_InSentStall             = 0x20,
    USB_InSendStall             = 0x10,
    USB_InFlushFIFO             = 0x08,
    USB_InUnderRun              = 0x04,
    USB_InFIFONotEmpty          = 0x02,
    USB_InPktRdy                = 0x01,
    /* OUTCSR flags */
    USB_OutClrDataTog           = 0x80,
    USB_OutFlushFIFO            = 0x10,
    USB_OutPktRdy               = 0x01,
};

#define USB_DEVICE_CONFIG_BASE 0x13040078

typedef struct {
    /* Configuration */
    uchar ep_info;
    uchar ram_info;
} USBDeviceConfig;

/* intr_in_enable and intr_out_enable mask values */
enum {
    USB_Endpoint_IN0 = 1,
    USB_Endpoint_IN1 = 2,
    USB_Endpoint_IN2 = 4,
    USB_Endpoint_OUT1 = 2,
};
