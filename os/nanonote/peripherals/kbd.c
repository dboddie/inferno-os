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
    Ctrl    = Spec|0x62,
    Latin   = Spec|0x63,
    Alt     = Latin,
    Caps    = Spec|0x64,
    No      = 0x00,

    Up      = KF|14,
    Vup     = KF|15,
    Left    = View,
    Right   = View,
    Down    = View,
    Vdown   = KF|16,
    Del     = 0x7F,
    Sym     = No,
    Qi      = No,
    Fn      = No,

    Esc     = No
};

static kbd_column;
static uchar kbd_state[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}
};

void kbdinit(void)
{
    GPIO *c_func = (GPIO *)(GPIO_PORT_C_FUNC | KSEG1);
    c_func->clear = GPIO_Keyboard_Out_Mask; /* GPIO/interrupt */

    GPIO *c_sel = (GPIO *)(GPIO_PORT_C_SEL | KSEG1);
    c_sel->clear = GPIO_Keyboard_Out_Mask;  /* GPIO */

    GPIO *c_dir = (GPIO *)(GPIO_PORT_C_DIR | KSEG1);
    c_dir->clear = GPIO_Keyboard_Out_Mask;  /* input */

    GPIO *c_pull = (GPIO *)(GPIO_PORT_C_PULL | KSEG1);
    c_pull->set = GPIO_Keyboard_Out_Mask;   /* no pull up/down */

    GPIO *d_func = (GPIO *)(GPIO_PORT_D_FUNC | KSEG1);
    d_func->clear = GPIO_Keyboard_In_Mask; /* GPIO/interrupt */

    GPIO *d_sel = (GPIO *)(GPIO_PORT_D_SEL | KSEG1);
    d_sel->clear = GPIO_Keyboard_In_Mask;  /* GPIO */

    GPIO *d_dir = (GPIO *)(GPIO_PORT_D_DIR | KSEG1);
    d_dir->clear = GPIO_Keyboard_In_Mask;  /* input */

    GPIO *d_pull = (GPIO *)(GPIO_PORT_D_PULL | KSEG1);
    d_pull->clear = GPIO_Keyboard_In_Mask; /* pull up/down */

    kbdq = qopen(4*1024, 0, 0, 0);
    qnoblock(kbdq, 1);

    kbd_column = 0;
}

static int kbd_c_values[8] = {10,11,12,13,14,15,16,17};
static int kbd_d_values[8] = {18,19,20,21,22,23,24,26};

static uchar keys[8][8] = {
    { KF|1, KF|2, KF|3, KF|4, KF|5, KF|6,  KF|7, No    },
    { 'q',  'w',  'e',  'r',  't',  'y',   'u',  'i'   },
    { 'a',  's',  'd',  'f',  'g',  'h',   'j',  'k'   },
    { Esc,  'z',  'x',  'c',  'v',  'b',   'n',  'm'   },
    { '\t', Caps, '\\', '\'', ',',  '.',   '/',  Up    },
    { 'o',  'l',  '=',  Sym,  ' ',  Qi,    Ctrl, 'l'   },
    { KF|8, 'p',  '\b',  '\n', Vup,  Vdown, Down, Right },
    { Shift, Alt, Fn,   No,   No,   No,    No,   No    },
};

static uchar shift_keys[8][8] = {
    { KF|1, KF|2, KF|3, KF|4, KF|5, KF|6,  KF|7, No    },
    { 'Q',  'W',  'E',  'R',  'T',  'Y',   'U',  'I'   },
    { 'A',  'S',  'D',  'F',  'G',  'H',   'J',  'K'   },
    { Esc,  'Z',  'X',  'C',  'V',  'B',   'N',  'M'   },
    { '\t', Caps, '|',  '`',  ';',  ':',   '?',  Up    },
    { 'O',  'L',  '+',  Sym,  ' ',  Qi,    Ctrl, 'L'   },
    { KF|8, 'P',  '\b', '\n', Vup,  Vdown, Down, Right },
    { Shift, Alt, Fn,   No,   No,   No,    No,   No    },
};

static uchar sym_keys[8][8] = {
    { KF|1, KF|2, KF|3, KF|4, KF|5, KF|6,  KF|7, No    },
    { '!',  '@',  '#',  '$',  '%',  '^',   '&',  '*'   },
    { 'A',  'S',  'D',  '-',  '_',  '{',   '[',  ']'   },
    { Esc,  'Z',  'X',  'C',  'V',  'B',   '<',  '>'   },
    { '\t', Caps, '\\', '`', '\'',  '"',   '/',  Up    },
    { '(',  '}',  '=',  Sym,  ' ',  Qi,    Ctrl, 'L'   },
    { KF|8, ')',  '\b', '\n', Vup,  Vdown, Down, Right },
    { Shift, Alt, Fn,   No,   No,   No,    No,   No    },
};

static uchar fn_keys[8][8] = {
    { KF|1, KF|2, KF|3, KF|4, KF|5, KF|6,  KF|7, No    },
    { 'q',  'w',  'e',  'r',  't',  'y',   '7',  '8'   },
    { 'a',  's',  'd',  'f',  'g',  'h',   '4',  '5'   },
    { Esc,  'z',  'x',  'c',  'v',  'b',   '1',  '2'   },
    { '\t', Caps, '\\', '\'', ',',  '.',   '0',  Up    },
    { '9',  '6',  '3',  Sym,  ' ',  Qi,    Ctrl, 'l'   },
    { KF|8, 'p',  '\b',  '\n', Vup,  Vdown, Down, Right },
    { Shift, Alt, Fn,   No,   No,   No,    No,   No    },
};

void kbdpoll(void)
{
    GPIO *c_dir = (GPIO *)(GPIO_PORT_C_DIR | KSEG1);
    GPIO *c_data = (GPIO *)(GPIO_PORT_C_DATA | KSEG1);
    GPIO *d_pin = (GPIO *)(GPIO_PORT_D_PIN | KSEG1);

    int bitfield = 1 << kbd_c_values[kbd_column];
    c_dir->set = bitfield;
    c_data->clear = bitfield;

    for (int row = 0; row < 8; row++)
    {
        /* Check whether the key was pressed (bit goes low) */
        uchar pressed = (d_pin->data & (1 << kbd_d_values[row])) ? 0 : 1;
        uchar was_pressed = kbd_state[row][kbd_column];

        kbd_state[row][kbd_column] = pressed;

        if (row == 7 && (kbd_column < 3))
            continue;   /* shift/alt/fn */
        else if (row == 5 && (kbd_column == 3 || kbd_column == 6))
            continue;   /* symbol/ctrl */

        if (pressed && !was_pressed)
        {
            /* Keys that are pressed now, but weren't before */
            if (kbd_state[7][0] != 0)       /* shift */
                kbdputc(kbdq, shift_keys[row][kbd_column]);
            else if (kbd_state[5][3] != 0)  /* symbol */
                kbdputc(kbdq, sym_keys[row][kbd_column]);
            else if (kbd_state[7][2] != 0)  /* fn */
                kbdputc(kbdq, fn_keys[row][kbd_column]);
            else
                kbdputc(kbdq, keys[row][kbd_column]);
        }
    }
    c_dir->clear = bitfield;

    kbd_column = (kbd_column + 1) & 7;
}
