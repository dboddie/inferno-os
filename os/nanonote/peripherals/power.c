#include "u.h"
#include "../../port/lib.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "../hardware.h"

void power_init(void)
{
    GPIO *d_func = (GPIO *)(GPIO_PORT_D_FUNC | KSEG1);
    GPIO *d_sel = (GPIO *)(GPIO_PORT_D_SEL | KSEG1);
    GPIO *d_dir = (GPIO *)(GPIO_PORT_D_DIR | KSEG1);
    GPIO *d_pull = (GPIO *)(GPIO_PORT_D_PULL | KSEG1);
    /* GPIO/interrupt function, GPIO selected, direction in, pull up/down */
    d_func->clear = GPIO_Power;
    d_sel->clear = GPIO_Power;
    d_dir->clear = GPIO_Power;
    d_pull->clear = GPIO_Power;
}

int power_button_pressed(void)
{
    GPIO *d_pin = (GPIO *)(GPIO_PORT_D_PIN | KSEG1);
    return (d_pin->data & GPIO_Power) ? 0: 1;
}

void power_check_reset(void)
{
}
