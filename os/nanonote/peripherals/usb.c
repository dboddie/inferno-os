#include "u.h"
#include "../../port/lib.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "../hardware.h"
#include "usb.h"

static int req_state;
static int req_current;

void usb_init(void)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);

    /* Reset the state of request processing */
    req_state = 0;
    req_current = -1;

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
    /* Negotiate for high speed */
    usb->power |= 0x20;
    /* Enable soft connection to enable the PHY */
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

static void usb_read_fifo(uchar *fifo, uchar req[], ulong count)
{
    for (int i = 0; i < count; i++)
        req[i] = fifo[0];
}

static void usb_read_msg(uchar *fifo, Request *req)
{
    req->requesttype = fifo[0];
    req->request = fifo[0];
    req->value = fifo[0];
    req->value |= fifo[0] << 8;
    req->index = fifo[0];
    req->index |= fifo[0] << 8;
    req->length = fifo[0];
    req->length |= fifo[0] << 8;
}

static uchar _DevDesc[0x12] = {
    0x12,           /* Length */
    0x01,           /* Descriptor (device) */
    0x00, 0x02,     /* USB 2.0 */
    0xff,           /* Class (vendor) */
    USB_NoSubclass, /* Subclass */
    USB_NoProtocol, /* Protocol */
    0x20,           /* Max packet size in bytes */
    0x55, 0xf0,     /* Vendor ID */
    0x37, 0x13,     /* Product ID */
    0x00, 0x01,     /* Device version (1.0) */
    0x00,           /* Manufacturer string descriptor index */
    0x00,           /* Product string descriptor index */
    0x00,           /* Serial number string descriptor index */
    0x01,           /* Number of configurations */
};

static uchar _ConfDesc[0x9] = {
    0x09,           /* Length */
    0x02,           /* Descriptor (configuration) */
    0x08, 0x00,     /* Total length including other trailing descriptors */
    0x00,           /* Number of interfaces */
    0x00,           /* Configuration number/value */
    0x00,           /* Configuration descriptor index */
    0x40,           /* Attributes (self-powered) */
    0x01            /* Maximum power consumption (in 2mA units) */
};

static void write_descriptor(Request *req, uchar *fifo)
{
    /* Check the descriptor type */
    switch (req->value >> 8)
    {
    case DeviceDesc:
    {
        for (int i = 0; i < 0x12; i++)
            fifo[0] = _DevDesc[i];
        break;
    }
    case ConfigurationDesc:
    {
        for (int i = 0; i < 0x9; i++)
            fifo[0] = _ConfDesc[i];
        break;
    }
    default:
        print("Invalid descriptor: %2.2ux\n", req->value >> 8);
        break;
    }
}

void usb_intr(void)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    switch (usb->intr_usb)
    {
    case 4: /* reset */
        req_state = 0;
        print("Reset\n");
        break;
    default:
        ;
    }

    /* Read the endpoints causing interrupts - this also clears the flags */
    ushort in = usb->intr_in;
    Request req;

    /* Endpoint 0 */
    if (in & 0x1) {
        /* Read the incoming data */
        usb->index = 0;

        if (req_state != 0) {
            print(" OK\n");
            req_state = 0;
            return;
        }

        usb_read_msg(usb->fifo[0], &req);

        /* Indicate that the message has been received */
        usb->csr |= USB_ServicedOutPktRdy;

        switch (req.request)
        {
        case GetDescriptor:
            print(" GetDescriptor\n");
            /* Write the appropriate descriptor to the output FIFO */
            write_descriptor(&req, usb->fifo[0]);
            /* Let the host know that an IN packet is ready */
            usb->csr |= USB_InPktRdy | USB_DataEnd;
            req_state++;
            break;
        case SetAddress:
            usb->faddr = req.value;
            print(" SetAddress: %4.4ux\n", req.value);
            req_state++;
            break;
        default:
            print("%2.2ux %2.2ux %4.4ux %4.4ux %4.4ux\n",
                  req.requesttype, req.request, req.value, req.index, req.length);
            req_state = 0;
            break;
        }
    }
}
