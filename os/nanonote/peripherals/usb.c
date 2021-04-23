#include "u.h"
#include "../../port/lib.h"
#include "../dat.h"
#include "../mem.h"
#include "../fns.h"

#include "../hardware.h"
#include "usb.h"

static int req_state;
static int req_current;
static int configuration;

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

static uchar _DevDesc[] = {
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
    0x40,           /* Manufacturer string descriptor index */
    0x41,           /* Product string descriptor index */
    0x42,           /* Serial number string descriptor index */
    0x01,           /* Number of configurations */
};

static uchar _ConfDesc[] = {
    0x09,           /* Length */
    0x02,           /* Descriptor (configuration) */
    0x08, 0x00,     /* Total length including other trailing descriptors */
    0x00,           /* Number of interfaces */
    0x01,           /* Configuration number/value (0 is unconfigured) */
    0x00,           /* Configuration descriptor index */
    0x40,           /* Attributes (self-powered) */
    0x01            /* Maximum power consumption (in 2mA units) */
};

/* The language string descriptor is needed if other string descriptors are used */
static uchar _LanguageDesc[] = {
    0x04,           /* Length */
    0x03,           /* Descriptor (string) */
    0x09, 0x04      /* Language code zero (US English) */
                    /* Other codes can follow... */
};

static uchar _ManufacturerDesc[] = {
    0x10,                               /* Length (2 + 14) */
    0x03,                               /* Descriptor (string) */
    'I', 0x00,                          /* UTF-16-LE string */
    'n', 0x00,
    'f', 0x00,
    'e', 0x00,
    'r', 0x00,
    'n', 0x00,
    'o', 0x00
};

static uchar _ProductDesc[] = {
    0x12,                               /* Length (2 + 16) */
    0x03,                               /* Descriptor (string) */
    'N', 0x00,                          /* UTF-16-LE string */
    'a', 0x00,
    'n', 0x00,
    'o', 0x00,
    'N', 0x00,
    'o', 0x00,
    't', 0x00,
    'e', 0x00
};

static uchar _SerialDesc[] = {
    0x0a,                               /* Length (2 + 8) */
    0x03,                               /* Descriptor (string) */
    '1', 0x00,                          /* UTF-16-LE string */
    '2', 0x00,
    '3', 0x00,
    '4', 0x00
};

static void write_descriptor(Request *req, uchar *fifo)
{
    /* Check the descriptor type */
    switch (req->value >> 8)
    {
    case DeviceDesc:
    {
        for (int i = 0; i < _DevDesc[0]; i++)
            fifo[0] = _DevDesc[i];
        break;
    }
    case ConfigurationDesc:
    {
        for (int i = 0; i < _ConfDesc[0]; i++)
            fifo[0] = _ConfDesc[i];
        break;
    }
    case StringDesc:
    {
        switch (req->value & 0xff)
        {
            case 0:
                for (int i = 0; i < _LanguageDesc[0]; i++)
                    fifo[0] = _LanguageDesc[i];
                break;
            case 0x40:
                for (int i = 0; i < _ManufacturerDesc[0]; i++)
                    fifo[0] = _ManufacturerDesc[i];
                break;
            case 0x41:
                for (int i = 0; i < _ProductDesc[0]; i++)
                    fifo[0] = _ProductDesc[i];
                break;
            case 0x42:
                for (int i = 0; i < _SerialDesc[0]; i++)
                    fifo[0] = _SerialDesc[i];
                break;
            default:
                print("Invalid string descriptor: %2.2ux\n", req->value & 0xff);
                break;
        }
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
            /* Expecting an interrupt to confirm that a response was sent */
            req_state = 0;
            return;
        }

        usb_read_msg(usb->fifo[0], &req);

        /* Indicate that the message has been received */
        usb->csr |= USB_ServicedOutPktRdy;

        switch (req.request)
        {
        case GetDescriptor:
            print(" GetDescriptor: %d %d\n", req.value >> 8, req.value & 0xff);
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
        case SetConfiguration:
            configuration = req.value;
            print(" SetConfiguration: %4.4ux\n", req.value);
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
