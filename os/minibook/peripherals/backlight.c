#include "u.h"
#include "mem.h"
#include "../hardware.h"

void backlight_init(void)
{
    /* Set up backlight pin functions and PWM */
    GPIO *gpioc = (GPIO *)(GPIO_PORT_C_BASE | KSEG1);
    gpioc->dir |= GPIO_C_PWM0 | GPIO_C_Backlight;
    gpioc->data |= GPIO_C_Backlight;

    /* Enable PWM0 */
    // Perhaps also ensure that the PWM0 clock is running
    PWM *pwm = (PWM *)(PWM0_BASE | KSEG1);
    pwm->period = 299;
    pwm->duty = 50;
    pwm->control = PWM_CtrEn | 47;
}

ulong backlight_get_brightness(void)
{
    PWM *pwm = (PWM *)(PWM0_BASE | KSEG1);
    return pwm->duty;
}

void backlight_set_brightness(int brightness)
{
    PWM *pwm = (PWM *)(PWM0_BASE | KSEG1);
    pwm->duty = brightness;
}
