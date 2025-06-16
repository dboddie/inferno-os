#include "u.h"
#include "../../port/lib.h"
#include "picoplus2.h"

#define USB_SET_ADDRESS 0x05
#define USB_GET_DESCRIPTOR 0x06
#define USB_SET_CONFIGURATION 0x09

/* Some descriptor types */
#define USB_DESCTYPE_DEVICE 1
#define USB_DESCTYPE_CONFIG 2
#define USB_DESCTYPE_STRING 3
#define USB_DESCTYPE_INTERF 4
#define USB_DESCTYPE_ENDPNT 5
#define USB_DESCTYPE_DEVICE_QUALIFIER 6

#define USB_GET_STATUS 0

#define USB_DEVICE_IN 0x80
#define USB_DEVICE_OUT 0x0

#define USB_CDC_SET_LINE_CODING 0x20
#define USB_CDC_SET_CONTROL_LINE_STATE 0x22

static const char Device_Desc[] = {
    0x12,       // length
    0x01,       // type: device
    0x10, 0x01, // usb_version (1.1)
    0x00,       // class: defined by interface
    0x00,       // subclass: defined by interface
    0x00,       // protocol: defined by interface
    0x40,       // max_pkt
    0xdb, 0xdb, // vendor
    0x78, 0x56, // product
    0x00, 0x01, // dev_version
    0x01,       // man_index
    0x02,       // prod_index
    0x03,       // ser_index
    0x01        // configs
};

static const char DeviceQualifier_Desc[] = {
    0x0a,       // length
    0x06,       // type: device qualifier
    0x10, 0x01, // usb_version (1.1)
    0x00,       // class: defined by interface
    0x00,       // subclass: defined by interface
    0x00,       // protocol: defined by interface
    0x40,       // max_pkt
    0x01,       // configs
    0x00
};

static const char Config_Desc[] = {
    0x09,       // length
    0x02,       // type: configuration
    0x4b, 0x00, // total_length: (this and all of the following descriptors)
    0x02,       // interfaces
    0x01,       // config_value
    0x00,       // string_index: no string description
    0x80,       // attributes: reserved bit
    0xfa,       // max_power: 250 * 2mA

    // Interface association, interface 0 of 1, class=subclass=2, no protocol
    0x08, 0x0b, 0x00, 0x02, 0x02, 0x02, 0x00, 0x00,

    // Interface 0, alt 0, 1 endpoint, Communications, Abstract, no protocol
    0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02, 0x00, 0x00,

    0x05, 0x24, 0x00, 0x10, 0x01,
    0x04, 0x24, 0x02, 0x02,
    0x05, 0x24, 0x06, 0x00, 0x01,
    0x05, 0x24, 0x01, 0x03, 0x01,
    0x07, 0x05, 0x83, 0x03, 0x08, 0x00, 0xff,

    // Interface 1, alt 0, 2 endpoints, CDC Data class (10), no subclass or protocol
    0x09, 0x04, 0x01, 0x00, 0x02, 0x0a, 0x00, 0x00, 0x00,
    // Endpoint 1 (IN), bulk, 64 bytes
    0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x00,
    // Endpoint 2 (OUT), bulk, 64 bytes
    0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x00,
};

/* String descriptors */
static const char *strings[] = {
    "\x04\x03\x09\x04",
    "\x10\x03T\x00e\x00s\x00t\x00i\x00n\x00g\x00",
    "\x0e\x03P\x00i\x00c\x00o\x002\x00W\x00",
    "\x0e\x031\x002\x003\x004\x005\x006\x00"
    };

static int ep_pid[3] = {0, 0, 0};

unsigned int
usb_pid(int i)
{
    int ep = ep_pid[i];
    ep_pid[i] ^= 1;
    return (ep == 0) ? USB_BCR_DATA0 : USB_BCR_DATA1;
}

void
usb_init(void)
{
    Clocks *usbclk = (Clocks *)CLK_USB_ADDR;
    usbclk->ctrl |= CLK_CTRL_ENABLE;

    Resets *clrreset = (Resets *)RESETS_CLR_BASE;
    Resets *resets = (Resets *)RESETS_BASE;
    clrreset->reset = RESETS_USBCTRL;
    while (!(resets->reset_done & RESETS_USBCTRL));

    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;
    for (int i = 0; i < 1024; i++)
        dpsram[i] = 0;

    NVIC *nvic = (NVIC *)NVIC_ISER;
    nvic->iser0_31 |= (1 << USBCTRL_IRQ);

    USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
    regs->usb_muxing = USB_MUXING_SOFTCON | USB_MUXING_TO_PHY;
    regs->usb_pwr = USB_PWR_VBUS_DETECT_OVERRIDE_EN | USB_PWR_VBUS_DETECT;
    regs->main_ctrl = USB_MAIN_CTRL_CONTROLLER_EN;
    regs->sie_ctrl = USB_SIE_CTRL_EP0_INT_1BUF;
    regs->inte = USB_INT_BUS_RESET | USB_INT_SETUP_REQ | USB_INT_BUFF_STATUS;

    // Enable EP1 for in bulk transfers with a buffer at offset 0x180.
    dpsram[USB_EP1_IN_EPCTL] = USB_ECR_EN | USB_ECR_INTEN | USB_ECR_BULK | 0x180;
    // Enable EP2 for out bulk transfers with a buffer at offset 0x200.
    dpsram[USB_EP2_OUT_EPCTL] = USB_ECR_EN | USB_ECR_INTEN | USB_ECR_BULK | 0x200;
    dpsram[USB_EP2_OUT_BUFCTL] = 64 | usb_pid(2) | USB_BCR_AVAIL;

    // Enable full speed device.
    USBregs *setregs = (USBregs *)USBCTRL_REGS_SET_BASE;
    setregs->sie_ctrl = USB_SIE_CTRL_PULLUP_EN;
}

void
usb_info(char *buf, int n)
{
    USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
    NVIC *nvic = (NVIC *)NVIC_ISER;
    PLL *pll = (PLL *)PLL_USB_BASE;
    int i = 0;

    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;
    i += snprint(buf + i, n, "%08ux\n", dpsram[USB_EP1_IN_EPCTL]);
    i += snprint(buf + i, n, "%08ux\n", dpsram[USB_EP2_OUT_EPCTL]);
    i += snprint(buf + i, n, "%08ux\n", dpsram[USB_EP2_OUT_BUFCTL]);
/*
    i += snprint(buf + i, n, "%08ux ", regs->main_ctrl);
    i += snprint(buf + i, n, "%08ux ", regs->sie_ctrl);
    i += snprint(buf + i, n, "%08ux ", regs->usb_muxing);
    i += snprint(buf + i, n, "%08ux ", regs->usb_pwr);
    i += snprint(buf + i, n, "%08ux ", regs->inte);
    i += snprint(buf + i, n, "%08ux ", nvic->iser0_31);
//    i += snprint(buf + i, n, "%08ux ", *(unsigned int *)(NVIC_IPR + 12));
//    i += snprint(buf + i, n, "%08ux ", *(unsigned int *)NVIC_ICPR0);
    i += snprint(buf + i, n, "%08ux ", pll->cs);
    i += snprint(buf + i, n, "%08ux ", pll->pwr);
    i += snprint(buf + i, n, "%08ux ", pll->fbdiv_int);
    i += snprint(buf + i, n, "%08ux ", pll->prim);
*/
/*
    Clocks *usbclk = (Clocks *)CLK_USB_ADDR;
    i += snprint(buf + i, n, "%08ux ", usbclk->ctrl);
    i += snprint(buf + i, n, "%08ux ", usbclk->div);
*/
}

void
usb_recv_ack(void)
{
    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;

    // Mark the buffer as available in the control register for a data packet.
    dpsram[USB_EP0_OUT_BUFCTL] = 0 | usb_pid(0) | USB_BCR_AVAIL;
//    print("ep0 out bufctl %08ux\n", dpsram[USB_EP0_OUT_BUFCTL]);
}

void
usb_send_ack(void)
{
    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;

    // Mark the buffer as available in the control register for a data packet.
    dpsram[USB_EP0_IN_BUFCTL] = 0 | usb_pid(0) | USB_BCR_AVAIL | USB_BCR_FULL;
//    print("ep0 in bufctl %08ux\n", dpsram[USB_EP0_IN_BUFCTL]);
}

void
usb_send_stall(void)
{
    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;

    // Mark the buffer as available in the control register for a data packet.
    dpsram[USB_EP0_IN_BUFCTL] = USB_BCR_STALL;
    USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
    regs->ep_stall_arm = 1;
//    print("stall ep0 in\n");
}

char *ep0_src;
int ep0_len;

void
usb_send_data(int bufctl, char *bufaddr, int ep, int bit, char *src, int len)
{
    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;
    int flags, n;
//    print("send %d\n", len);
    ep0_src = src;
    ep0_len = len;

    flags = 0;
    if (len > 64)
        len = 64;
    else
        flags = USB_BCR_LAST;

    // Copy the data into the buffer in DPSRAM.
    memmove((void *)bufaddr, src, len);

    // Mark the buffer as available in the control register for a data packet.
    dpsram[bufctl] = len | usb_pid(ep) | USB_BCR_AVAIL | USB_BCR_FULL | flags;
//    print("%08ux\n", dpsram[bufctl]);

    ep0_src += len;
    ep0_len -= len;
}

typedef struct {
    unsigned char requestType;
    unsigned char request;
    unsigned short value;
    unsigned short index;
    unsigned short length;
} usb_setup_packet;

static int setting_addr = 0;
static short device_status = 0;

void
usb_handle_setup(void)
{
    usb_setup_packet *p = (usb_setup_packet *)0x50100000;
    print("%02ux %02ux %04ux %04ux %04ux\n", p->requestType,
          p->request, p->value, p->index, p->length);

    ep_pid[0] = 1;

    switch (p->requestType) {
    case USB_DEVICE_IN:
    {
        switch (p->request) {
        case USB_GET_DESCRIPTOR: {
            unsigned char descIndex = p->value & 0xff;
            unsigned char descType = p->value >> 8;
            unsigned int descLength;
            switch (descType) {
            case USB_DESCTYPE_DEVICE:
                descLength = (p->length < Device_Desc[0]) ? p->length : Device_Desc[0];
                usb_send_data(USB_EP0_IN_BUFCTL, (char *)USB_DPSRAM_EP0_BUF, 0, 1,
                              Device_Desc, descLength);
                break;
            case USB_DESCTYPE_DEVICE_QUALIFIER:
                usb_send_data(USB_EP0_IN_BUFCTL, (char *)USB_DPSRAM_EP0_BUF, 0, 1,
                              DeviceQualifier_Desc, 10);
            case USB_DESCTYPE_CONFIG:
                descLength = (p->length < Config_Desc[2]) ? p->length : Config_Desc[2];
                usb_send_data(USB_EP0_IN_BUFCTL, (char *)USB_DPSRAM_EP0_BUF, 0, 1,
                              Config_Desc, descLength);
                break;
            case USB_DESCTYPE_STRING:
                if (descIndex > 4) descIndex = 0;
                usb_send_data(USB_EP0_IN_BUFCTL, (char *)USB_DPSRAM_EP0_BUF, 0, 1,
                              strings[descIndex], strings[descIndex][0]);
                break;
            default:
                usb_send_stall();
                break;
            }
            break;
        }
        case USB_GET_STATUS:
            usb_send_data(USB_EP0_IN_BUFCTL, (char *)USB_DPSRAM_EP0_BUF, 0, 1,
                          (char *)&device_status, 2);
        default:
            break;
        }
        break;
    }
    case USB_DEVICE_OUT:
    {
        switch (p->request) {
        case USB_SET_ADDRESS:
            // Send an ack (zero length response).
//            print("set address %d\n", p->value);
            setting_addr = p->value;
            usb_send_ack();
            break;
        case USB_SET_CONFIGURATION:
            usb_send_ack();
            break;
        default:
            usb_send_stall();
            break;
        }
        break;
    }
    case 0x21:
    {
        switch (p->request) {
        case USB_CDC_SET_LINE_CODING:
            usb_send_ack();
            break;
        case USB_CDC_SET_CONTROL_LINE_STATE:
            usb_send_ack();
            break;
        default:
            usb_send_stall();
            break;
        }
        break;
    }
    default:
        usb_send_stall();
        break;
    }
}

void usbctrl(void)
{
    USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
    USBregs *clrregs = (USBregs *)USBCTRL_REGS_CLR_BASE;
    int status = regs->ints;
    //print("s %08ux\n", status);

    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;

    if (status & USB_INT_BUS_RESET) {
        // Clear the status bit and reset the device address to zero.
        clrregs->sie_status = USB_SIE_STATUS_BUS_RESET;
        regs->addr_endp[0] = 0;
    }
    if (status & USB_INT_SETUP_REQ) {
        // Clear the status bit and handle the setup request.
        clrregs->sie_status = USB_SIE_STATUS_SETUP_REC;
        usb_handle_setup();
    }
    if (status & USB_INT_BUFF_STATUS) {
        // A buffer has finished transferring for one or more endpoints.
        unsigned int bs = regs->buff_status;
//        print("%08ux\n", bs);
        // Handle each endpoint in turn.
        if (bs & 0x01) {
            // EP0 in
            clrregs->buff_status = 1;
            if (setting_addr) {
                USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
                regs->addr_endp[0] = setting_addr;
                setting_addr = 0;
            } else if (ep0_len > 0) {
                usb_send_data(USB_EP0_IN_BUFCTL, (char *)USB_DPSRAM_EP0_BUF, 0, 1,
                              ep0_src, ep0_len);
            } else {
                usb_recv_ack();
            }
        }
        if (bs & 0x02) {
            // EP0 out
            //print("EP0 %08ux\n", dpsram[USB_EP0_OUT_BUFCTL]);
            clrregs->buff_status = 2;
        }
        // EP1 in
        if (bs & 0x04) {
            clrregs->buff_status = 4;
            //print("EP1 %08ux\n", dpsram[USB_EP1_IN_BUFCTL]);
            //usb_send_data(USB_EP1_IN_BUFCTL, (char *)USB_DPSRAM_EP1_BUF, 1, 4,
            //              "Testing\n", 8);
            dpsram[USB_EP2_OUT_BUFCTL] = 64 | usb_pid(2) | USB_BCR_AVAIL;
        }
        // EP2 out
        if (bs & 0x20) {
            //print("EP2 %08ux\n", dpsram[USB_EP2_OUT_BUFCTL]);
            clrregs->buff_status = 32;
            int len = dpsram[USB_EP2_OUT_BUFCTL] & USB_BCR_LEN_MASK;
            for (int i = 0; i < len; i++) {
                print("%c", *(unsigned char *)(USB_DPSRAM_EP2_BUF + i));
            }
            usb_send_data(USB_EP1_IN_BUFCTL, (char *)USB_DPSRAM_EP1_BUF, 1, 4,
                          (char *)USB_DPSRAM_EP2_BUF, len);
        }
    }
}
