#ifndef STM32F405_H
#define STM32F405_H

typedef unsigned int uint;

typedef struct {
    uint moder;
    uint otyper;
    uint ospeedr;
    uint pupdr;
    uint idr;
    uint odr;
    uint bsrr;
    uint lckr;
    uint afrl;
    uint afrh;
} GPIO;

enum GPIO_OutputType {
    GPIO_PushPull = 0,
    GPIO_OpenDrain = 1
};

enum GPIO_Mode {
    GPIO_Input = 0,
    GPIO_Output = 1,
    GPIO_Alternate = 2,
    GPIO_Analog = 3
};

enum GPIO_Speed {
    GPIO_LowSpeed = 0,
    GPIO_MediumSpeed = 1,
    GPIO_HighSpeed = 2,
    GPIO_VeryHighSpeed = 3
};

enum GPIO_Pull {
    GPIO_NoPull = 0,
    GPIO_PullUp = 1,
    GPIO_PullDown = 2
};

#define GPIO_A   0x40020000
#define GPIO_B   0x40020400
#define GPIO_C   0x40020800

typedef struct {
    uint cr;
    uint pllcfgr;
    uint cfgr;
    uint cir;
    uint ahb1rstr;
    uint ahb2rstr;
    uint ahb3rstr;
    uint padding0;
    uint apb1rstr;
    uint apb2rstr;
    uint padding1;
    uint padding2;
    uint ahb1enr;
    uint ahb2enr;
    uint ahb3enr;
    uint padding3;
    uint apb1enr;
    uint apb2enr;
} RCC;

#define RCC_CR   0x40023800

enum RCC_CR_Flags {
    RCC_PLLRDY = 0x02000000,
    RCC_PLLON  = 0x01000000,
    RCC_HSE_Bypass = 0x40000,
    RCC_HSE_Ready = 0x20000,
    RCC_HSE_On  = 0x10000,
    RCC_HSI_Ready = 0x2,
    RCC_HSI_On  = 0x1
};

enum RCC_PLLSource {
    RCC_PLL_HSI_Source = 0,
    RCC_PLL_HSE_Source = 0x00400000
};

#define RCC_CFGR 0x40023808

enum RCC_CFGR_SW {
    RCC_CFGR_SW_HSI = 0,
    RCC_CFGR_SW_HSE = 1,
    RCC_CFGR_SW_PLL = 2,
    RCC_CFGR_SWS_Mask = 0xc,
    RCC_CFGR_SWS_PLL = 8
};

#define RCC_AHB1_RESET_GPIOA 1
#define RCC_AHB1_RESET_GPIOB 2
#define RCC_AHB1_RESET_GPIOC 4

#define RCC_AHB1_ENABLE_GPIO_A 1
#define RCC_AHB1_ENABLE_GPIO_B 2
#define RCC_AHB1_ENABLE_GPIO_C 4

#define RCC_APB1_ENABLE_USART3  0x00040000
#define RCC_APB1_ENABLE_I2C1    0x00200000

#define RCC_APB2_SPI1 0x00001000
#define RCC_APB1_SPI3 0x00008000

typedef struct {
    uint control;
    uint reload;
    uint current;
    uint calibration;
} SysTick;

#define SYSTICK     0xe000e010

typedef struct {
    uint sr;
    uint dr;
    uint brr;
    uint cr1;
    uint cr2;
    uint cr3;
    uint gtpr;
} USART;

#define USART3      0x40004800

enum USART_Status {
    USART_TransComplete = 0x40,
    USART_ReadNotEmpty = 0x20
};

enum USART_Control1 {
    USART_Over8 = 0x8000,
    USART_Enable = 0x2000,
    USART_WordLength = 0x1000,
    USART_TransEnable = 0x8,
    USART_RecvEnable = 0x4
};

enum USART_Control2 {
    USART_StopBits = 0x3000
};

typedef struct {
    uint cr1;
    uint cr2;
    uint oar1;
    uint oar2;
    uint dr;
    uint sr1;
    uint sr2;
    uint ccr;
    uint trise;
    uint fltr;
} I2C;

#define I2C1 0x40005400

enum I2C_Constants {
    I2C1_cr1_swrst      = 0x8000,
    I2C1_cr1_pos        = 0x800,
    I2C1_cr1_ack        = 0x400,
    I2C1_cr1_stop       = 0x200,
    I2C1_cr1_start      = 0x100,
    I2C1_cr1_pe         = 0x1,
    I2C1_cr2_freq_mask  = 0x3f,
    I2C1_sr1_ackfail    = 0x400,
    I2C1_sr1_txe        = 0x80,
    I2C1_sr1_rxne       = 0x40,
    I2C1_sr1_btf        = 0x4,
    I2C1_sr1_addr       = 0x2,
    I2C1_sr1_start      = 0x1,
    I2C1_sr2_busy       = 0x2
};

#define SPI1 0x40013000
#define SPI3 0x40003c00

typedef struct {
    uint cr1;
    uint cr2;
    uint sr;
    uint dr;
    uint crcpr;
    uint rxcrcr;
    uint txcrcr;
    uint i2scfgr;
    uint i2spr;
} SPI;

enum SPI_Constants {
    SPI_bidimode = 0x8000,      /* Control register 1 flags */
    SPI_bdioe    = 0x4000,
    SPI_crcen    = 0x2000,
    SPI_crcnext  = 0x1000,
    SPI_dff      = 0x800,
    SPI_rxonly   = 0x400,
    SPI_ssm      = 0x200,
    SPI_ssi      = 0x100,
    SPI_lsbfirst = 0x80,
    SPI_spe      = 0x40,
    SPI_br_shift = 3,
    SPI_br_mask  = 0x7,
    SPI_mstr     = 0x4,
    SPI_cpol     = 0x2,
    SPI_cpha     = 0x1,
    SPI_bsy      = 0x80,        /* Status register flags */
    SPI_txe      = 0x02,
    SPI_rxne     = 0x01
};

/* Interrupts (NVIC) */
typedef struct {
    uint iser0_31;
    uint iser32_63;
    uint iser64_95;
} NVIC;

#define NVIC_ISER 0xe000e100
#define NVIC_ISER0 NVIC_ISER    // interrupts 0-31
#define NVIC_ISER1 0xe000e104   // interrupts 32-63
#define NVIC_ISER2 0xe000e108   // interrupts 64-95

typedef struct {
    uint icer0_31;
    uint icer32_63;
    uint icer64_95;
} NVIC_clear;

#define NVIC_ICER 0xe000e180
#define NVIC_ICER0 NVIC_ICER    // interrupts 0-31
#define NVIC_ICER1 0xe000e184   // interrupts 32-63
#define NVIC_ICER2 0xe000e188   // interrupts 64-95

#endif
