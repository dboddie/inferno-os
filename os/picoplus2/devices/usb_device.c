#include "u.h"
#include "../../port/lib.h"
#include "picoplus2.h"

void
usb_init(void)
{
    Clocks *usbclk = (Clocks *)CLK_USB_ADDR;
    usbclk->ctrl |= CLK_CTRL_ENABLE;

    Resets *clrreset = (Resets *)RESETS_CLR_BASE;
    Resets *resets = (Resets *)RESETS_BASE;
    clrreset->reset = RESETS_USB;
    while (!(resets->reset_done & RESETS_USB));

    NVIC *nvic = (NVIC *)NVIC_ISER;
    nvic->iser0_31 |= (1 << USBCTRL_IRQ);

    USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
    regs->usb_muxing = USB_MUXING_SOFTCON | USB_MUXING_TO_PHY;
    regs->usb_pwr = USB_PWR_VBUS_DETECT_OVERRIDE_EN | USB_PWR_VBUS_DETECT;
    regs->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN;
    regs->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF;
    regs->inte = USB_INT_SETUP_REQ | USB_INT_BUFF_STATUS | USB_INT_BUS_RESET;

    regs->sie_ctrl |= USB_SIE_CTRL_PULLUP_EN;
}

void
usb_info(char *buf, int n)
{
    USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
    NVIC *nvic = (NVIC *)NVIC_ISER;
    PLL *pll = (PLL *)PLL_USB_BASE;
    int i = 0;
/*
    i += snprint(buf + i, n, "%08ux ", regs->main_ctrl);
    i += snprint(buf + i, n, "%08ux ", regs->sie_ctrl);
    i += snprint(buf + i, n, "%08ux ", regs->usb_muxing);
    i += snprint(buf + i, n, "%08ux ", regs->usb_pwr);
    i += snprint(buf + i, n, "%08ux ", regs->inte);
    i += snprint(buf + i, n, "%08ux ", nvic->iser0_31);
    i += snprint(buf + i, n, "%08ux ", *(unsigned int *)(NVIC_IPR + 12));
    i += snprint(buf + i, n, "%08ux ", *(unsigned int *)NVIC_ICPR0);
    i += snprint(buf + i, n, "%08ux ", pll->cs);
    i += snprint(buf + i, n, "%08ux ", pll->pwr);
    i += snprint(buf + i, n, "%08ux ", pll->fbdiv_int);
    i += snprint(buf + i, n, "%08ux ", pll->prim);
*/

    Clocks *usbclk = (Clocks *)CLK_USB_ADDR;
    i += snprint(buf + i, n, "%08ux ", usbclk->ctrl);
    i += snprint(buf + i, n, "%08ux ", usbclk->div);
}

void usbctrl(void)
{
    USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
    USBregs *clrregs = (USBregs *)USBCTRL_REGS_CLR_BASE;
    print("%08x\n", regs->ints);
    if (regs->ints & USB_INT_BUS_RESET) {
        clrregs->sie_status = USB_SIE_STATUS_BUS_RESET;
    } else if (regs->ints & USB_INT_SETUP_REQ) {
        clrregs->sie_status = USB_SIE_STATUS_SETUP_REC;
    }
}
