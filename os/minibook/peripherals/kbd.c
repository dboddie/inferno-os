#include "u.h"
#include "../../port/lib.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "hardware.h"

enum
{
    Spec    = 0x80,

    View    = Spec|0x00,    /* view (shift window up) */
    KF      = Spec|0x40,    /* function key */
    Shift   = Spec|0x60,
    LShift  = Shift,
    RShift  = Shift,
    Ctrl    = Spec|0x62,
    Latin   = Spec|0x63,
    Alt     = Latin,
    Caps    = Spec|0x64,
    Num     = Spec|0x65,
    No      = 0x00,

    Up      = KF|14,
    Vup     = KF|15,
    Left    = View,
    Right   = View,
    Down    = View,
    Vdown   = KF|16,
    Ins     = KF|20,
    Del     = 0x7F,
    Sym     = No,
    Qi      = No,
    Fn      = No,

    Esc     = No,
    Zzz     = No,
    Pause   = No,
    Menu    = No,
    PrtSc   = No,
    Pgup    = KF|15,
    Pgdown  = View,
    Home    = KF|13,
    End     = '\r'
};

static kbd_column;
static uchar kbd_state[8][17] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

void kbdinit(void)
{
    /* Set up touch pad button input and LED pins */
    GPIO *gpioa = (GPIO *)(GPIO_PORT_A_BASE | KSEG1);
    gpioa->dir &= ~GPIO_A_TouchLeft;
    gpioa->dir |= GPIO_A_CapsLED;
    gpioa->pull |= GPIO_A_Keyboard_In_Mask;

    /* Make all keyboard pins input pins */
    GPIO *gpiod = (GPIO *)(GPIO_PORT_D_BASE | KSEG1);
    gpioa->dir &= ~GPIO_A_Keyboard_In_Mask;
    gpiod->dir &= ~GPIO_D_Keyboard_Out_Mask;

    kbdq = qopen(4*1024, 0, 0, 0);
    qnoblock(kbdq, 1);

    kbd_column = 0;
}

static uchar keys[8][17] = {
    { Pause,  'q',  'w',  'e',  'r', 'u',  'i', 'o',   No,   No,   No,    'p',  No,    No,   No,  No,    No },
    { No,     '\t', Caps, KF|3, 't', 'y',  ']', KF|7,  No,   '\b', No,    '[',  Zzz,   No,   No,  No,    LShift },
    { No,     'a',  's',  'd',  'f',  'j', 'k',  'l',  No,   No,   No,    ';',  No,    No,   No,  Up,    RShift },
    { No,     Esc,  '\\', KF|4, 'g',  'h', KF|6, No,   ' ',  No,   Alt,   '\'', No,    No,   No,  Down,  No },
    { No,     'z',  'x',  'c',  'v',  'm', ',',  '.',  Num,  '\n', No,    '\\', No,    No,   No,  Left,  No },
    { No,     No,   No,   No,   'b',  'n', No,   Menu, No,   No,   No,    '/',  No,    No,   No,  Right, No },
    { Ctrl,   '`',  No,   No,   '5',  '6', '=',  KF|8, '\b', KF|9, No,    '-',  No,    KF|2, Ins, No,    KF|1 },
    { KF|5,   '1',  '2',  '3',  '4',  '7', '8',  '9',  No,   No,   PrtSc, '0',  KF|10, No,   No,  No,    Fn },
};

static uchar shift_keys[8][17] = {
    { Pause,  'Q',  'W',  'E',  'R', 'U',  'I', 'O',   No,   No,   No,    'P',  No,    No,   No,  No,    No },
    { No,     '\t', Caps, KF|3, 'T', 'Y',  '}', KF|7,  No,   '\b', No,    '{',  Zzz,   No,   No,  No,    LShift },
    { No,     'A',  'S',  'D',  'F',  'J', 'K',  'L',  No,   No,   No,    ':',  No,    No,   No,  Up,    RShift },
    { No,     Esc,  '|',  KF|4, 'G',  'H', KF|6, No,   ' ',  No,   Alt,   '"',  No,    No,   No,  Down,  No },
    { No,     'Z',  'X',  'C',  'V',  'M', '<',  '>',  Num,  '\n', No,    '|',  No,    No,   No,  Left,  No },
    { No,     No,   No,   No,   'B',  'N', No,   Menu, No,   No,   No,    '?',  No,    No,   No,  Right, No },
    { Ctrl,   '`',  No,   No,   '%',  '^', '=',  KF|8, '\b', KF|9, No,    '_',  No,    KF|2, Ins, No,    KF|1 },
    { KF|5,   '!',  '@',  '#',  '$',  '&', '*',  '(',  No,   No,   PrtSc, ')',  KF|10, No,   No,  No,    Fn },
};

static uchar fn_keys[8][17] = {
    { No, No, No, No, No, No, No, No, No, No, No, No, No, No,    No, No,     No },
    { No, No, No, No, No, No, No, No, No, No, No, No, No, No,    No, No,     No },
    { No, No, No, No, No, No, No, No, No, No, No, No, No, No,    No, Pgup,   No },
    { No, No, No, No, No, No, No, No, No, No, No, No, No, No,    No, Pgdown, No },
    { No, No, No, No, No, No, No, No, No, No, No, No, No, No,    No, Home,   No },
    { No, No, No, No, No, No, No, No, No, No, No, No, No, No,    No, End,    No },
    { No, No, No, No, No, No, No, No, No, No, No, No, No, KF|12, No, No,     KF|11 },
    { No, No, No, No, No, No, No, No, No, No, No, No, No, No,    No, No,     No },
};

static int kbd_d_values[17] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,29};

void kbdpoll(void)
{
    GPIO *gpioa = (GPIO *)(GPIO_PORT_A_BASE | KSEG1);
    GPIO *gpiod = (GPIO *)(GPIO_PORT_D_BASE | KSEG1);

    /* Make a column pin an output and bring it low */
    int bitfield = 1 << kbd_d_values[kbd_column];
    gpiod->dir |= bitfield;
    gpiod->data &= ~bitfield;

    for (int row = 0; row < 8; row++)
    {
        /* Check whether the key was pressed (bit goes low) */
        uchar pressed = (gpioa->data & (1 << row)) ? 0 : 1;
        uchar was_pressed = kbd_state[row][kbd_column];

        kbd_state[row][kbd_column] = pressed;
        uchar k = keys[row][kbd_column];

        if (k == Shift || k == Alt || k == Fn || k == Ctrl)
            continue;

        if (pressed && !was_pressed)
        {
            /* Keys that are pressed now, but weren't before */
            if ((kbd_state[1][16] != 0) || (kbd_state[2][16] != 0))
                kbdputc(kbdq, shift_keys[row][kbd_column]);
            else if (kbd_state[7][16] != 0)
                kbdputc(kbdq, fn_keys[row][kbd_column]);
            else
                kbdputc(kbdq, keys[row][kbd_column]);
        }
    }
    /* Make the column pin an input again */
    gpiod->dir &= ~bitfield;

    if (kbd_column++ == 17) kbd_column = 0;
}
