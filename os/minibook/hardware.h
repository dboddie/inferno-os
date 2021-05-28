#define CGU_CFCR    0x10000000
#define CGU_PLCR    0x10000010
#define CGU_OCR     0x1000001c
#define CGU_MSCR    0x10000020
#define CGU_CFCR2   0x10000060

#define EXTAL_RATE 3686400

enum CGU_CFCR_Fields {
    CGU_LCS         = 0x40000000,   /* LCD clock source (0=PLL, 1=LCD_PCLK) */
    CGU_MCS         = 0x01000000,   /* MSC clock selection (0 = 16 MHz, 1 = 24 MHz) */
    CGU_LFR_Mask    = 0x0000f000,
    CGU_LFR_Shift   = 12,
};

enum CGU_PLCR_Fields {
    CGU_PLLFD_Mask  = 0xff800000,   /* feedback divider */
    CGU_PLLRD_Mask  = 0x007c0000,   /* input divider */
    CGU_PLLOD_Mask  = 0x00030000,   /* divider [1, 2, 2, 4] */
    CGU_PLLS        = 0x00000400,   /* status */
    CGU_PLLBP       = 0x00000200,   /* bypass */
    CGU_PLLEN       = 0x00000100,   /* enable */
    CGU_PLLST_Mask  = 0x000000ff    /* stabilize time */
};

enum CGU_OCR_Fields {
    CGU_OCR_O1ST_Mask   = 0x00ff0000,   /* stabilize time */
    CGU_OCR_O2SE        = 0x00000100,   /* select (0=EXCLK/128, 1=32768Hz clock) */
    CGU_OCR_SPEND1      = 0x00000080,   /* USB port 1 suspend */
    CGU_OCR_SPEND0      = 0x00000040    /* USB port 0 suspend */
};

enum CGU_MCR_Gates {
    CGU_UDC     = 0x01000000,
    CGU_MSC     = 0x00002000,
    CGU_PWM0    = 0x00000400,
    CGU_LCD     = 0x00000080,
    CGU_OST     = 0x00000008
};

enum CGU_CFCR2_Fields {
    CGU_PXFR = 0x1ff     /* LCD pixel clock divider (= 1+PXFR) */
};

#define INTERRUPT_BASE 0x10001000

typedef struct {
    ulong source;                   /* ICSR */
    ulong mask;                     /* ICMR */
    ulong mask_set;                 /* ICMSR */
    ulong mask_clear;               /* ICMCR */
    ulong pending;                  /* ICPR */
} InterruptCtr;

enum InterruptSource {
    InterruptGPIO0 = 0x10000000,
    InterruptGPIO1 = 0x08000000,
    InterruptGPIO2 = 0x04000000,
    InterruptGPIO3 = 0x02000000,
    InterruptOST0  = 0x01000000,
    InterruptOST1  = 0x00800000,
    InterruptOST2  = 0x00400000,
    InterruptCIM   = 0x00040000,
    InterruptMSC   = 0x00004000,
    InterruptUHC   = 0x00002000,
    InterruptUDC   = 0x00001000,
};

#define TIMER_OTER  0x10002000
#define TIMER_BASE0 0x10002010
#define TIMER_BASE1 0x10002030
#define TIMER_BASE2 0x10002050

typedef struct {
    ulong data;     /* OTDR - contains the reload value*/
    ulong counter;  /* OTCNT */
    ulong control;  /* OTCSR */
    ulong read;     /* OTCRD */
} JZTimer;

enum TimerEnable {
    Timer0 = 0x01,
    Timer1 = 0x02,
    Timer2 = 0x04,
    TimerAll = 0x07
};

enum TimerControl {
    TimerSwitch     = 0x80,
    TimerUnder      = 0x40,
    TimerUnderIntEn = 0x20,
    TimerPCLK4   = 0,
    TimerPCLK16  = 1,
    TimerPCLK64  = 2,
    TimerPCLK256 = 3,
    TimerRTCCLK  = 4,
    TimerEXCLK   = 5
};

enum TimerSource {
    TimerSourceExt = 4,
    TimerSourceRTC = 2,
    TimerSourcePCLK = 1
};

/* Ports A to D are sometimes referred to as ports 0 to 3 */
#define GPIO_PORT_A_BASE    0x10010000
#define GPIO_PORT_B_BASE    0x10010030
#define GPIO_PORT_C_BASE    0x10010060
#define GPIO_PORT_D_BASE    0x10010090

enum GPIOPins {
    GPIO_A_CapsLED            = 0x08000000,   /* port A/0 bit 27 */
    GPIO_A_TouchLeft          = 0x00010000,   /* port A/0 bit 16 */
    GPIO_A_TouchRight         = 0x00002000,   /* port A/0 bit 13 */
    GPIO_A_ScrollLED          = 0x00000200,   /* port A/0 bit 9 */
    GPIO_A_Keyboard_In_Mask   = 0x000000ff,   /* port A/0 */
    GPIO_C_PWM0               = 0x40000000,   /* port C/2 bit 30 */
    GPIO_C_NumLED             = 0x00400000,   /* port C/2 bit 22 */
    GPIO_D_Keyboard_Out_Mask  = 0x2000ffff    /* port D/3 */
};

typedef struct {
    ulong data;
    ulong dir;
    ulong _padding;
    ulong pull;
    ulong sel_low;
    ulong sel_high;
    ulong trig_low;
    ulong trig_high;
    ulong int_en;
    ulong int_mask;
    ulong int_flag;
} GPIO;

#define PWM0_BASE 0x10050000
#define PWM1_BASE 0x10051000

typedef struct {
    ulong control;  /* CTR0/1 lowest 8 bits */
    ulong period;   /* PER0/1 bits 0-9 */
    ulong duty;     /* DUT0/1 bits 0-10 */
} PWM;

enum PWMBits {
    PWM_CtrEn           = 0x80,
    PWM_CtrShutdown     = 0x40,
    PWM_CtrPrescaleMask = 0x3f,
    PWM_PeriodMask      = 0x1ff,
    PWM_FullDuty        = 0x200,
    PWM_DutyMask        = 0x1ff
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
    USB_OutSentStall            = 0x40,
    USB_OutSendStall            = 0x20,
    USB_OutFlushFIFO            = 0x10,
    USB_OutFIFOFull             = 0x02,
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

long usb_read(void* a, long n, vlong offset);
long usb_write(void* a, long n, vlong offset);

#define LCD_BASE 0x13050000

typedef struct {
    ulong config;
    ulong vsync;
    ulong hsync;
} LCD;
