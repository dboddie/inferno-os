#define CGU_CFCR    0x10000000
#define CGU_PLCR    0x10000010
#define CGU_OCR     0x1000001c
#define CGU_CFCR2   0x10000060

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

#define TIMER_OTER 0x10002000
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

/* Ports A to D are sometimes referred to as ports 0 to 3 */
#define GPIO_PORT_A_BASE    0x10010000
#define GPIO_PORT_B_BASE    0x10010030
#define GPIO_PORT_C_BASE    0x10010060
#define GPIO_PORT_D_BASE    0x10010090

enum GPIOPins {
    GPIO_A_CapsLED            = 0x08000000,   /* port A/0 bit 27 */
    GPIO_C_NumLED             = 0x00400000,   /* port C/2 bit 22 */
    GPIO_A_ScrollLED          = 0x00000200,   /* port A/0 bit 9 */
    GPIO_A_Keyboard_In_Mask   = 0x000000ff,   /* port A/0 */
    GPIO_D_Keyboard_Out_Mask  = 0x2000ffff    /* port D/3 */
};

typedef struct {
    ulong data;
    ulong dir;
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
    ulong duty'     /* DUT0/1 bits 0-10 */
} PWM;

enum PWMBits {
    PWM_CtrEn           = 0x80,
    PWM_CtrShutdown     = 0x40,
    PWM_CtrPrescaleMask = 0x3f,
    PWM_PeriodMask      = 0x1ff,
    PWM_FullDuty        = 0x200,
    PWM_DutyMask        = 0x1ff
}

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
