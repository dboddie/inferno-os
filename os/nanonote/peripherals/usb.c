#include "u.h"
#include "../../port/lib.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "../hardware.h"

void usb_init(void)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);

    /* Flush FIFOs
    for (int i = 0; i < 16; i++) {
        usb->index = i;
        usb->in_csr |= 0x08;
        usb->out_csr |= 0x10;
    } */

    /* Propagate the UDC clock by clearing the appropriate bit */
    *(ulong*)(CGU_CLKGR | KSEG1) &= ~CGU_UDC;
    /* Leave UDC suspend mode */
    *(ulong*)(CGU_SCR | KSEG1) |= CGU_SPENDN;

    ic->mask_clear = InterruptUDC;

    usb->intr_usb_enable = 0x4;
    usb->intr_in_enable = 0x1;
    usb->power |= 0x20;
    usb->power |= 0x40;
}

void usb_info(char *buf, int n)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    snprint(buf, n,
            "%2.2ux %2.2ux %1.1ux\n"
            "%4.4ux %4.4ux %4.4ux %4.4ux",
            usb->faddr, usb->power, usb->index,
            usb->intr_in_enable, usb->intr_out_enable, usb->intr_in, usb->intr_out);
}

void usb_intr(void)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    print("%1.1ux %2.2ux \n", usb->intr_usb, usb->intr_in);
}
