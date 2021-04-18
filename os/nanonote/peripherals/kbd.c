#include "u.h"
#include "../../port/lib.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "hardware.h"

static kbd_column;

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

void kbdpoll(void)
{
    GPIO *c_func = (GPIO *)(GPIO_PORT_C_FUNC | KSEG1);
    GPIO *c_sel = (GPIO *)(GPIO_PORT_C_SEL | KSEG1);
    GPIO *c_dir = (GPIO *)(GPIO_PORT_C_DIR | KSEG1);
    GPIO *c_pin = (GPIO *)(GPIO_PORT_C_PIN | KSEG1);
    GPIO *c_data = (GPIO *)(GPIO_PORT_C_DATA | KSEG1);
    GPIO *d_pin = (GPIO *)(GPIO_PORT_D_PIN | KSEG1);

    int bitfield = 1 << kbd_c_values[kbd_column];
    c_func->clear = bitfield;
    c_sel->clear = bitfield;
    c_dir->set = bitfield;
    c_data->clear = bitfield;
    fbprint(d_pin->data & GPIO_Keyboard_In_Mask, 1 + kbd_column, 0x808000);
    c_func->clear = bitfield;
    c_sel->clear = bitfield;
    c_dir->clear = bitfield;

    kbd_column = (kbd_column + 1) & 7;
}
