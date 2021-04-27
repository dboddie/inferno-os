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

    /* Propagate the UDC clock by clearing the appropriate bit */
    *(ulong*)(CGU_CLKGR | KSEG1) &= ~CGU_UDC;
    /* Leave UDC suspend mode */
    *(ulong*)(CGU_SCR | KSEG1) |= CGU_SPENDN;

    ic->mask_clear = InterruptUDC;

    usb->intr_usb_enable = USB_Reset;
    /* Enable ep0 and ep1 input interrupts - the NanoNote supports 2 IN endpoints */
    usb->intr_in_enable = USB_Endpoint_IN0 | USB_Endpoint_IN2;
    /* Enable ep1 output interrupts - the NanoNote supports 1 OUT endpoint  */
    usb->intr_out_enable = USB_Endpoint_OUT1;
    /* Negotiate for high speed */
    usb->power |= USB_Power_HighSpeed;
    /* Enable soft connection to enable the PHY */
    usb->power |= USB_Power_SoftConn;
}

void usb_info(char *buf, int n)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    USBDeviceConfig *uc = (USBDeviceConfig *)(USB_DEVICE_CONFIG_BASE | KSEG1);
    snprint(buf, n,
            "%2.2ux %2.2ux %1.1ux\n"
            "%4.4ux %4.4ux %4.4ux %4.4ux\n"
            "%2.2ux %2.2ux",
            usb->faddr, usb->power, usb->index,
            usb->intr_in_enable, usb->intr_out_enable, usb->intr_in, usb->intr_out,
            uc->ep_info, uc->ram_info);
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

static char* usb_request_type[4] = {
    "Standard", "Class", "Endpoint", "Reserved"
};

static char* usb_request_dest[4] = {
    "Device", "Interface", "Endpoint", "Other"
};

static char* usb_request_names[] = {
    "GET_STATUS",        "CLEAR_FEATURE",     "?",              "SET_FEATURE",
    "?",                 "SET_ADDRESS",       "GET_DESCRIPTOR", "SET_DESCRIPTOR",
    "GET_CONFIGURATION", "SET_CONFIGURATION", "GET_INTERFACE",  "?",
    "?",                 "SET_INTERFACE",     "SYNCH_FRAME"
};

static void print_request(Request *req)
{
/*
    int type = (req->requesttype >> 5) & 0x3;
    print("%s ", usb_request_type[type]);
    int dest = req->requesttype & 0x1f;
    if (dest < 4)
        print("%s ", usb_request_dest[dest]);
    else
        print("Reserved ");

    if (req->request < 0x13)
        print("%s ", usb_request_names[req->request]);
    else
        print("? ");
*/
    print("%2.2ux %2.2ux %4.4ux %4.4ux %4.4ux\n",
          req->requesttype, req->request, req->value, req->index, req->length);
}

static uchar _DevDesc[] = {
    0x12,               /* Length */
    DeviceDesc,
    0x00, 0x02,         /* USB 2.0 */
    USB_Class_Vendor,
    USB_NoSubclass,     /* Subclass */
    USB_NoProtocol,     /* Protocol */
    USB_MAXP_SIZE_ENDP, /* Max packet size in bytes */
    0x55, 0xf0,         /* Vendor ID */
    0x37, 0x13,         /* Product ID */
    0x00, 0x01,         /* Device version (1.0) */
    0x40,               /* Manufacturer string descriptor index */
    0x41,               /* Product string descriptor index */
    0x42,               /* Serial number string descriptor index */
    0x01,               /* Number of configurations */
};

static uchar _ConfDesc[] = {
    0x09,           /* Length */
    ConfigurationDesc,
    0x20, 0x00,     /* Total length including other trailing descriptors such
                       as the interface and endpoint descriptors */
    0x01,           /* Number of interfaces (there must be at least 1) */
    0x01,           /* Configuration number/value (0 is unconfigured) */
    0x00,           /* Configuration descriptor index */
    0xc0,           /* Attributes (bus-powered, self-powered) */
    0x01            /* Maximum power consumption (in 2mA units) */
};

static uchar _IfaceDesc[] = {
    0x09,           /* Length */
    InterfaceDesc,
    0x00,           /* Interface number (starting at 0) */
    0x00,           /* Alternate setting */
    0x02,           /* Number of endpoints */
    USB_Class_Vendor,
    USB_NoSubclass,
    USB_NoProtocol,
    0x43
};

static uchar _EndOutDesc[] = {
    0x07,                       /* Length */
    EndpointDesc,
    0x01,                       /* Endpoint 1 (OUT) */
    Endpoint_Bulk,
    USB_MAXP_SIZE_HIGH & 0xff,  /* Maximum packet size */
    USB_MAXP_SIZE_HIGH >> 8,
    0x00                        /* Interval */
};

static uchar _EndInDesc[] = {
    0x07,                       /* Length */
    EndpointDesc,
    0x82,                       /* Endpoint 2 (IN) */
    Endpoint_Bulk,
    USB_MAXP_SIZE_HIGH & 0xff,  /* Maximum packet size */
    USB_MAXP_SIZE_HIGH >> 8,
    0x00                        /* Interval */
};

/* The language string descriptor is needed if other string descriptors are used */
static uchar _LanguageDesc[] = {
    0x04,           /* Length */
    StringDesc,     /* Descriptor (string) */
    0x09, 0x04      /* Language code zero (US English) */
                    /* Other codes can follow... */
};

static uchar _ManufacturerDesc[] = {
    0x10,       /* Length (2 + 14) */
    StringDesc,
    'I', 0x00,  /* UTF-16-LE string */
    'n', 0x00,
    'f', 0x00,
    'e', 0x00,
    'r', 0x00,
    'n', 0x00,
    'o', 0x00
};

static uchar _ProductDesc[] = {
    0x12,       /* Length (2 + 16) */
    StringDesc,
    'N', 0x00,  /* UTF-16-LE string */
    'a', 0x00,
    'n', 0x00,
    'o', 0x00,
    'N', 0x00,
    'o', 0x00,
    't', 0x00,
    'e', 0x00
};

static uchar _SerialDesc[] = {
    0x0a,       /* Length (2 + 8) */
    StringDesc,
    '1', 0x00,  /* UTF-16-LE string */
    '2', 0x00,
    '3', 0x00,
    '4', 0x00
};

static uchar _IfaceStringDesc[] = {
    0x16,       /* Length (2 + 20) */
    StringDesc,
    'A', 0x00,  /* UTF-16-LE string */
    'T', 0x00,
    ' ', 0x00,
    'C', 0x00, 'o', 0x00, 'm', 0x00, 'm', 0x00, 'a', 0x00, 'n', 0x00, 'd', 0x00
};

static uchar _FallbackStringDesc[] = {
    0x02,
    StringDesc
};

static uchar* StringDescriptors[4] = {
    _ManufacturerDesc, _ProductDesc, _SerialDesc, _IfaceStringDesc
};

static void write_descriptor(uchar type, uchar index, uchar *fifo, ushort length)
{
    //print("GetDescriptor: %d %d %d\n", type, index, length);
    /* Check the descriptor type */
    switch (type)
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

        if (length > _ConfDesc[0]) {
            /* Also return the interface and endpoint descriptors */
            write_descriptor(InterfaceDesc, 0, fifo, 0xffff);
            write_descriptor(EndpointDesc, 0, fifo, 0xffff);
            write_descriptor(EndpointDesc, 1, fifo, 0xffff);
        }
        break;
    }
    case InterfaceDesc:
    {
        for (int i = 0; i < _IfaceDesc[0]; i++)
            fifo[0] = _IfaceDesc[i];
        break;
    }
    case EndpointDesc:
    {
        uchar *desc;
        if (index == 0)
            desc = _EndInDesc;
        else
            desc = _EndOutDesc;

        for (int i = 0; i < desc[0]; i++)
            fifo[0] = desc[i];
        break;
    }
    case StringDesc:
    {
        uchar *desc;
        if (index == 0)
            desc = _LanguageDesc;
        else if (index >= 0x40 && index < 0x44)
            desc = StringDescriptors[index - 0x40];
        else {
            desc = _FallbackStringDesc;
            print("Invalid string descriptor: %2.2ux\n", index);
        }

        for (int i = 0; i < desc[0]; i++)
            fifo[0] = desc[i];

        break;
    }
    default:
        print("Invalid descriptor: %2.2ux\n", type);
        break;
    }
}

void usb_intr(void)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    static int amount = 4;

    switch (usb->intr_usb)
    {
    case USB_Reset:
        req_state = 0;
        print("Reset\n");
        break;
    default:
        ;
    }

    /* Read the endpoints causing interrupts - this also clears the flags */
    ushort in = usb->intr_in;
    ushort out = usb->intr_out;
    Request req;

//    print("%4.4ux %4.4ux %4.4ux %4.4ux\n", usb->intr_in_enable,
//          usb->intr_out_enable, in, out);

    /* Endpoint 0 */
    if (in & USB_Endpoint_IN0) {
        /* Read the incoming data */
        usb->index = 0;

        if (req_state != 0) {
            /* Expecting an interrupt to confirm that a response was sent */
            req_state = 0;
            return;
        }

        /* Ignore spurious requests, perhaps from the controller */
        if (!(usb->csr & USB_Ctrl_OutPktRdy)) {
            return;
        }

        usb_read_msg(usb->fifo[0], &req);

        switch (req.request)
        {
        case GetDescriptor:
            /* Indicate that the message has been received */
            usb->csr |= USB_Ctrl_ServicedOutPktRdy;

            /* Write the appropriate descriptor to the output FIFO */
            write_descriptor(req.value >> 8, req.value & 0xff, usb->fifo[0], req.length);
            /* Let the host know that an IN packet is ready */
            usb->csr |= USB_Ctrl_InPktRdy | USB_Ctrl_DataEnd;
            req_state++;
            break;
        case SetAddress:
            /* Indicate that the message has been received */
            usb->csr |= USB_Ctrl_ServicedOutPktRdy;
            usb->csr |= USB_Ctrl_DataEnd;

            usb->faddr = req.value;
            //print(" SetAddress: %4.4ux\n", req.value);
            req_state++;
            break;
        case SetConfiguration:
            /* Indicate that the message has been received */
            usb->csr |= USB_Ctrl_ServicedOutPktRdy;
            usb->csr |= USB_Ctrl_DataEnd;

            configuration = req.value;
            //print(" SetConfiguration: %4.4ux\n", req.value);
            req_state++;

            usb->index = 1;
            usb->out_max_p = USB_MAXP_SIZE_HIGH;
            usb->out_csr |= USB_OutClrDataTog;

            usb->index = 2;
            usb->in_max_p = USB_MAXP_SIZE_HIGH;
            usb->csr &= ~USB_InISO;
            usb->csr |= USB_InMode | USB_InClrDataTog;

            while (usb->csr & USB_InFIFONotEmpty) {
                print("Flushing IN FIFO\n");
                usb->csr |= USB_InFlushFIFO;
            }
            usb->csr |= USB_InPktRdy | USB_InFlushFIFO;

            print("EP1 OUT EP2 IN\n");
            break;

        default:
            print_request(&req);
            req_state = 0;
            break;
        }
    }

    if (in & USB_Endpoint_IN2) {
        usb->index = 2;

        if (usb->csr & USB_InSentStall) {
            usb->csr &= ~(USB_InSentStall | USB_InSendStall);
            usb->csr |= USB_InMode | USB_InClrDataTog;
        }

        if (usb->csr & USB_InUnderRun)
            usb->csr &= ~USB_InUnderRun;

        print("amount: %d\n", amount);
        while (amount > 0) {
            usb->fifo[2][0] = 'X';
            amount--;
        }

        usb->csr |= USB_InPktRdy;
    }

    if (out & USB_Endpoint_OUT1) {
        usb->index = 1;

        if (usb->out_csr & USB_OutPktRdy) {
            ushort count = usb->count;
            print("%4.4ux bytes received:\n", count);

            for (int i = 0; i < count; i++)
                print("%2.2ux ", usb->fifo[1][0]);

            print("\n");

            /* Indicate that the message has been received */
            usb->out_csr &= ~USB_OutPktRdy;

            /* Write some data to the IN endpoint's FIFO */
            amount = 10;
            while (amount > 0) {
                usb->fifo[2][0] = 'X';
                amount--;
            }

            usb->index = 2;
            usb->csr |= USB_InPktRdy;
        }
    }
}
