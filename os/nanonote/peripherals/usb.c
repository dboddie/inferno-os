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

typedef struct {
    uchar* a;
    long i, n;
    Lock lock;
    ushort ready;
} in_transfer;

static in_transfer in_tr;

typedef struct {
    uchar buf[USB_MAXP_SIZE_HIGH];
    long i;
    int full;
    Lock lock;
} out_transfer;

static out_transfer out_tr;

void usb_init(void)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    InterruptCtr *ic = (InterruptCtr *)(INTERRUPT_BASE | KSEG1);

    /* Initialise the state of request processing */
    req_state = 0;
    req_current = -1;
    /* Initialise transfers */
    in_tr.n = 0;
    in_tr.ready = 0;
    out_tr.i = 0;
    out_tr.full = 0;

    /* Propagate the UDC clock by clearing the appropriate bit */
    *(ulong*)(CGU_CLKGR | KSEG1) &= ~CGU_UDC;
    /* Leave UDC suspend mode */
    *(ulong*)(CGU_SCR | KSEG1) |= CGU_SPENDN;

    ic->mask_clear = InterruptUDC;

    usb->intr_usb_enable = USB_Reset;
    /* Enable ep0 and ep1 input interrupts - the NanoNote supports 2 IN endpoints */
    usb->intr_in_enable = USB_Endpoint_IN0 | USB_Endpoint_IN1;
    /* Enable ep1 output interrupts - the NanoNote supports 1 OUT endpoint  */
    usb->intr_out_enable = USB_Endpoint_OUT1;
    /* Negotiate for high speed */
    usb->power |= USB_Power_HighSpeed;
    /* Enable soft connection to enable the PHY */
    usb->power |= USB_Power_SoftConn;
}

void usb_info(char *buf, int n)
{
    snprint(buf, n,
            "IN:  %6ld %6ld %1d\n"
            "OUT: %1d\n",
            in_tr.i, in_tr.n, in_tr.ready,
            out_tr.full);
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

static DeviceDescriptor _DevDesc = {
    .length = 0x12,
    .type = DeviceDesc,
    .version = 0x200,   /* USB 2.0 */
    .class = USB_Class_Vendor,
    .subclass = USB_NoSubclass,
    .protocol = USB_NoProtocol,
    .maxpacketsize = USB_MAXP_SIZE_ENDP,
    .vendor = 0xf055,
    .product = 0x1337,
    .device = 0x100,    /* Device version (1.0) */
    .manufacturer_idx = 0x40,
    .product_idx = 0x41,
    .serialnumber_idx = 0x42,
    .configurations = 1
};

static ConfigDescriptor _ConfDesc = {
    .length = 0x09,
    .type = ConfigurationDesc,
    .totallength = 0x0020,      /* including other trailing descriptors such
                                   as the interface and endpoint descriptors */
    .interfaces = 1,            /* (there must be at least 1) */
    .configuration = 0x01,      /* (1-indexed, 0 is unconfigured) */
    .configuration_idx = 0,     /* no configuration string descriptor */
    .attributes = 0xc0,         /* (bus-powered, self-powered) */
    .maxpower = 0x01            /* Maximum power consumption (in 2mA units) */
};

static InterfaceDescriptor _IfaceDesc = {
    .length = 0x09,
    .type = InterfaceDesc,
    .number = 0x00,             /* (0-indexed) */
    .alternate = 0x00,
    .endpoints = 2,
    .class = USB_Class_Vendor,
    .subclass = USB_NoSubclass,
    .protocol = USB_NoProtocol,
    .index = 0x43               /* string descriptor index */
};

static EndpointDescriptor _EndOutDesc = {
    .length = 0x07,
    .type = EndpointDesc,
    .endpoint = 0x01,
    .attributes = Endpoint_Bulk,
    .maxpacketsize = USB_MAXP_SIZE_HIGH,
    .interval = 0
};

static EndpointDescriptor _EndInDesc = {
    .length = 0x07,
    .type = EndpointDesc,
    .endpoint = 0x81,
    .attributes = Endpoint_Bulk,
    .maxpacketsize = USB_MAXP_SIZE_HIGH,
    .interval = 0
};

/* The language string descriptor is needed if other string descriptors are used */
static ushort _langstr[] = {0x409}; /* Language code zero (US English) */
                                    /* Other codes can follow... */

static struct {
    uchar length;
    uchar type;
    ushort string[1];
} _LanguageDesc = {
    0x04, StringDesc, {0x0409}
};

static struct {
    uchar length;
    uchar type;
    ushort string[7];
} _ManufacturerDesc = {
    0x10, StringDesc, {'I', 'n', 'f', 'e', 'r', 'n', 'o'}
};

static struct {
    uchar length;
    uchar type;
    ushort string[8];
} _ProductDesc = {
    0x12, StringDesc, {'N', 'a', 'n', 'o', 'N', 'o', 't', 'e'}
};

static struct {
    uchar length;
    uchar type;
    ushort string[4];
} _SerialDesc = {
    0x0a, StringDesc, {'1', '2', '3', '4'}
};

static struct {
    uchar length;
    uchar type;
    ushort string[6];
} _IfaceStringDesc = {
    0x0e, StringDesc, {'C', 'u', 's', 't', 'o', 'm'}
};

static struct {
    uchar length;
    uchar type;
} _FallbackStringDesc = {
    0x02, StringDesc
};

static StringDescriptor* StringDescriptors[4] = {
    (StringDescriptor*)&_ManufacturerDesc,
    (StringDescriptor*)&_ProductDesc,
    (StringDescriptor*)&_SerialDesc,
    (StringDescriptor*)&_IfaceStringDesc
};

static void write_descriptor(uchar type, uchar index, uchar *fifo, ushort length)
{
    //print("GetDescriptor: %d %d %d\n", type, index, length);
    /* Check the descriptor type */
    switch (type)
    {
    case DeviceDesc:
    {
        for (int i = 0; i < _DevDesc.length; i++)
            fifo[0] = ((uchar *)&_DevDesc)[i];
        break;
    }
    case ConfigurationDesc:
    {
        for (int i = 0; i < _ConfDesc.length; i++)
            fifo[0] = ((uchar *)&_ConfDesc)[i];

        if (length > _ConfDesc.length) {
            /* Also return the interface and endpoint descriptors */
            write_descriptor(InterfaceDesc, 0, fifo, 0xffff);
            write_descriptor(EndpointDesc, 0, fifo, 0xffff);
            write_descriptor(EndpointDesc, 1, fifo, 0xffff);
        }
        break;
    }
    case InterfaceDesc:
    {
        for (int i = 0; i < _IfaceDesc.length; i++)
            fifo[0] = ((uchar *)&_IfaceDesc)[i];
        break;
    }
    case EndpointDesc:
    {
        uchar *desc;
        if (index == 0)
            desc = (uchar *)&_EndInDesc;
        else
            desc = (uchar *)&_EndOutDesc;

        for (int i = 0; i < desc[0]; i++)
            fifo[0] = desc[i];
        break;
    }
    case StringDesc:
    {
        uchar *desc;
        if (index == 0)
            desc = (uchar *)&_LanguageDesc;
        else if (index >= 0x40 && index < 0x44)
            desc = (uchar *)StringDescriptors[index - 0x40];
        else {
            desc = (uchar *)&_FallbackStringDesc;
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

void usb_send_data(void)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    usb->index = 1;

    if (usb->csr & USB_InSentStall) {
        usb->csr &= ~(USB_InSentStall | USB_InSendStall);
        usb->csr |= USB_InMode | USB_InClrDataTog;
    }

    if (usb->csr & USB_InUnderRun)
        usb->csr &= ~USB_InUnderRun;

    /* Queue more data to be sent to the host by reading from the inpoint
       buffer - we may need a way to check whether the FIFO is full */
    ushort sent = 0;

    while (in_tr.i < in_tr.n && sent < USB_MAXP_SIZE_HIGH)
    {
        usb->fifo[1][0] = in_tr.a[in_tr.i++];
        sent++;
    }

    usb->csr |= USB_InPktRdy;
}

void usb_intr(void)
{
    USBDevice *usb = (USBDevice *)(USB_DEVICE_BASE | KSEG1);
    /* Record the current index so it can be restored when returning */
    uchar index = usb->index;

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
            usb->index = index; /* Restore the original index */
            return;
        }

        /* Ignore spurious requests, perhaps from the controller */
        if (!(usb->csr & USB_Ctrl_OutPktRdy)) {
            usb->index = index; /* Restore the original index */
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

            if (usb->out_csr & USB_OutPktRdy) {
                usb->out_csr |= USB_OutFlushFIFO;
                usb->out_csr |= USB_OutFlushFIFO;
            }

            usb->index = 1;
            usb->in_max_p = USB_MAXP_SIZE_HIGH;
            usb->csr &= ~USB_InISO;
            usb->csr |= USB_InMode | USB_InClrDataTog;

            while (usb->csr & USB_InFIFONotEmpty) {
                print("Flushing IN FIFO\n");
                usb->csr |= USB_InFlushFIFO;
            }

            break;

        case GetStatus:
        {
            ushort status = 0;
            /* Indicate that the message has been received */
            usb->csr |= USB_Ctrl_ServicedOutPktRdy;

            if (req.index == 1)
            {
                switch (req.value) {
                case 0:
                    /* Halt on endpoint 1? */
                    usb->index = 1;
                    /* Write the status to the endpoint's FIFO */
                    status = (usb->out_csr & (USB_OutSentStall | USB_OutSendStall)) ? 1 : 0;
                    usb->index = 0;
                    break;
                case 1:
                    /* Custom status - return the out buffer's full flag */
                    status = out_tr.full;
                    break;
                default:
                    ;
                }
            } else if (req.index == 2 && req.value == 2) {
                /* Custom status (2), endpoint 2 */
                status = in_tr.ready;
            }

            /* Write the low and high bytes */
            usb->fifo[0][0] = status & 0xff;
            usb->fifo[0][0] = status >> 8;

            /* Let the host know that an IN packet is ready */
            usb->csr |= USB_Ctrl_InPktRdy | USB_Ctrl_DataEnd;
            req_state++;
            break;
        }
        case ClearFeature:
            usb->csr |= USB_Ctrl_ServicedOutPktRdy;
            usb->csr |= USB_Ctrl_DataEnd;

            if (req.index == 1 && req.value == 0) {
                usb->index = 1;
                usb->out_csr &= ~(USB_OutSendStall | USB_OutSentStall);
                usb->out_csr |= USB_OutClrDataTog;
            }

            req_state = 0;
            break;

        default:
            print("%2.2ux %2.2ux %4.4ux %4.4ux %4.4ux\n",
                  req.requesttype, req.request, req.value, req.index, req.length);
            req_state = 0;
            break;
        }
    }

    if (in & USB_Endpoint_IN1)
    {
        /* This code is executed when there is a request for more data. */
        usb_send_data();
    }

    if (out & USB_Endpoint_OUT1) {
        usb->index = 1;

        if (usb->out_csr & USB_OutPktRdy)
        {
            /* Only write to memory if the transfer is set up and not complete */
            if (out_tr.full == 0)
            {
                /* Copy the contents of the packet into the buffer */
                while ((usb->count > 0) && (out_tr.i < USB_MAXP_SIZE_HIGH))
                    out_tr.buf[out_tr.i++] = usb->fifo[1][0];

                out_tr.full = 1;
            }

            /* Indicate that the message has been received */
            usb->out_csr &= ~USB_OutPktRdy;
        }
        else if (usb->out_csr & USB_OutSentStall)
            usb->out_csr |= USB_OutSentStall;
    }

    usb->index = index; /* Restore the original index */
}

/* Functions for integration with the device file */

/* Read data from an OUT endpoint */
long usb_read(void* a, long n, vlong offset)
{
    USED(offset);

    lock(&out_tr.lock);

    /* Wait while the interrupt handler transfers the data */
    while (out_tr.full == 0);

    /* Return buffered data */
    n = (out_tr.i < USB_MAXP_SIZE_HIGH) ? out_tr.i : USB_MAXP_SIZE_HIGH;
    uchar *b = (uchar *)a;

    for (int i = 0; i < n; i++)
        b[i] = out_tr.buf[i];

    out_tr.i = 0;
    out_tr.full = 0;

    unlock(&out_tr.lock);

    return n;
}

/* Write data to an IN endpoint */
long usb_write(void* a, long n, vlong offset)
{
    USED(offset);

    /* Stop other writes from accessing the transfer structure but allow
       interrupts to occur (unlike ilock) */
    lock(&in_tr.lock);

    in_tr.a = (uchar *)a;
    in_tr.n = n;
    in_tr.i = 0;
    in_tr.ready = 0;

    splhi();
    usb_send_data();
    spllo();

    in_tr.ready = 1;

    /* Wait until the whole block of data has been transferred - the interrupt
       handlers will take care of all blocks after the first */
    while (in_tr.i < in_tr.n);

    in_tr.ready = 0;

    /* Allow other calls to write to access the transfer structure again */
    unlock(&in_tr.lock);

    return n;
}
