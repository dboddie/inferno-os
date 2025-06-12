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

#define USB_CDC_SET_LINE_CODING 0x20
#define USB_CDC_SET_CONTROL_LINE_STATE 0x22

#define USB_DIR_IN 0x80
#define USB_DIR_OUT 0x0

static const char CDC_Device_Desc[] = {
    0x12,       /* length */
    0x01,       /* type */
    0x00, 0x02, /* usb_version (2.0) */
    0xef,       /* class: miscellaneous device */
    0x02,       /* subclass */
    0x01,       /* protocol (interface association descriptor) */
    0x40,       /* max_pkt */
    0xdb, 0xdb, /* vendor */
    0x78, 0x56, /* product */
    0x00, 0x01, /* dev_version */
    0x01,       /* man_index */
    0x02,       /* prod_index */
    0x03,       /* ser_index */
    0x01        /* configs */
};

static const char CDC_Config_Desc[] = {
    0x09,       /* length */
    0x02,       /* type */
    0x4b, 0x00, /* total_length: (this and all of the following descriptors) */
    0x02,       /* interfaces */
    0x01,       /* config_value */
    0x00,       /* string_index: no string description */
    0x80,       /* attributes: reserved bit */
    0xfa,       /* max_power: 250 * 2mA */
//};
//
//static const char CDC_Interface_Association_Desc[] = {
    0x08,   /* length */
    0x0b,   /* type */
    0x00,   /* first_iface */
    0x02,   /* iface_count */
    0x02,   /* fn_class */
    0x02,   /* fn_subclass */
    0x00,   /* fn_protocol */
    0x00,   /* string_index: no string description */
//};
//
//static const char CDC_Comms_Interface_Desc[] = {
    0x09,   /* length */
    0x04,   /* type */
    0x00,   /* number */
    0x00,   /* alternative */
    0x01,   /* endpoints: 1 interrupt IN */
    0x02,   /* class: communications interface */
    0x02,   /* subclass: subclass code for abstract control model (ACM) */
    0x01,   /* V.25ter */
    0x00,   /* no string description */
//};
//
//static const char CDC_CS_Interface_Desc[] = {
    0x05,       /* length */
    0x24,       /* type: CS_INTERFACE */
    0x00,       /* subtype: header (first of these interfaces) */
    0x10, 0x01, /* BCD rel number */
//};
//
//static const char CDC_CS_ACM_Interface_Desc[] = {
    0x04,   /* length */
    0x24,   /* type: CS_INTERFACE */
    0x02,   /* subtype: ACM functional descriptor */
    0x06,   /* bmCapabilities: Table 28, Universal Serial Bus Class Definitions
               for Communications Devices*/
//};
//
//static const char CDC_CS_Union_Interface_Desc[] = {
    0x05,   /* length */
    0x24,   /* type: CS_INTERFACE */
    0x06,   /* subtype: union */
    0x00,   /* bMasterIface: the comms interface */
    0x01,   /* bSlaveIface0: the data interface */
//};
//
//static const char CDC_CS_Call_Management_Interface_Desc[] = {
    0x05,   /* length */
    0x24,   /* type: CS_INTERFACE */
    0x01,   /* subtype: call management */
    0x03,   /* bmCapabilities: Table 27, Universal Serial Bus Class Definitions
               for Communications Devices */
    0x01,   /* bDataIface: the data interface */
//};
//
//static const char CDC_Comms_Endpoint_In_Desc[] = {
    0x07,       /* length */
    0x05,       /* type */
    0x83,       /* address: IN 3 */
    0x03,       /* attributes: interrupt (0x3) */
    0x08, 0x00, /* max_pktsize */
    0xff,       /* interval */
//};
//
//static const char CDC_Data_Interface_Desc[] = {
    0x09,   /* length */
    0x04,   /* type */
    0x01,   /* number */
    0x00,   /* alternative */
    0x02,   /* endpoints: 2 endpoints (1 IN, 1 OUT) */
    0x0a,   /* class: communications interface */
    0x00,   /* subclass */
    0x00,   /* protocol */
    0x00,   /* no string description */
//};
//
//static const char CDC_Data_Endpoint_In_Desc[] = {
    0x07,       /* length */
    0x05,       /* type */
    0x81,       /* address: IN 1 */
    0x02,       /* attributes: bulk (0x2) */
    0x40, 0x00, /* max_pktsize */
    0x00,       /* interval */
//};
//
//static const char CDC_Data_Endpoint_Out_Desc[] = {
    0x07,       /* length */
    0x05,       /* type */
    0x02,       /* address: OUT 2 */
    0x02,       /* attributes: bulk (0x2) */
    0x40, 0x00, /* max_pktsize */
    0x00        /* interval */
};

/* String descriptors */
static const char *strings[] = {
    "\x04\x03\x09\x04",
    "\x10\x03T\x00e\x00s\x00t\x00i\x00n\x00g\x00",
    "\x0e\x03P\x00i\x00c\x00o\x002\x00W\x00",
    "\x0e\x031\x002\x003\x004\x005\x006\x00"
    };

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

    // Enable EP1 for in bulk transfers with a buffer at offset 0x200.
    dpsram[USB_EP1_IN_EPCTL] = USB_ECR_EN | USB_ECR_INTEN | USB_ECR_BULK | 0x200;
    // Enable EP2 for out bulk transfers with a buffer at offset 0x200.
    dpsram[USB_EP2_OUT_EPCTL] = USB_ECR_EN | USB_ECR_INTEN | USB_ECR_BULK | 0x300;
    dpsram[USB_EP2_OUT_BUFCTL] = USB_BCR_AVAIL;

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

static int ep0_pid = 0;

unsigned int
ep0_next_pid(void)
{
    ep0_pid = ep0_pid ^ 1;
    // Return a flag for the previous value.
    return ep0_pid == 0 ? USB_BCR_DATA1 : USB_BCR_DATA0;
}

void
usb_recv_ack(void)
{
    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;

    // Mark the buffer as available in the control register for a data packet.
    dpsram[USB_EP0_OUT_BUFCTL] = 0 | ep0_next_pid() | USB_BCR_AVAIL;
    print("ep0 out bufctl %08ux\n", dpsram[USB_EP0_OUT_BUFCTL]);
}

void
usb_send_ack(void)
{
    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;

    // Mark the buffer as available in the control register for a data packet.
    dpsram[USB_EP0_IN_BUFCTL] = 0 | ep0_next_pid() | USB_BCR_AVAIL | USB_BCR_FULL;
    print("ep0 in bufctl %08ux\n", dpsram[USB_EP0_IN_BUFCTL]);
}

void
usb_send_descriptor(const char *src, int len)
{
    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;
    int flags, n;

    for (int i = 0; i < len; i += 64) {
        n = len - i;
        flags = 0;
        if (n > 64)
            n = 64;
        else
            flags = USB_BCR_LAST;

        // Copy the data into the buffer in DPSRAM.
        memmove((void *)USB_DPSRAM_EP0_BUF, src, n);
        src += n;
        // Mark the buffer as available in the control register for a data packet.
        dpsram[USB_EP0_IN_BUFCTL] = n | ep0_next_pid() | USB_BCR_AVAIL |
                                    USB_BCR_FULL | flags;
        print("%08ux\n", dpsram[USB_EP0_IN_BUFCTL]);

        USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
        while (regs->buff_status & 1 == 0);
    }
}

void
usb_cdc_set_line_coding(void)
{
    unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;
    print("%08ux\n", *(unsigned int *)&dpsram[USB_EP0_OUT_BUFCTL]);
    //USB_DPSRAM_EP0_BUF
    usb_send_ack();
}

typedef struct {
    unsigned char requestType;
    unsigned char request;
    unsigned short value;
    unsigned short index;
    unsigned short length;
} usb_setup_packet;

static int setting_addr = 0;

void
usb_handle_setup(void)
{
    usb_setup_packet *p = (usb_setup_packet *)0x50100000;
    print("%02ux %02ux %04ux %04ux %04ux\n", p->requestType,
          p->request, p->value, p->index, p->length);

    ep0_pid = 1;

    if (p->requestType == USB_DIR_IN) {
        switch (p->request) {
        case USB_GET_DESCRIPTOR: {
            unsigned char descIndex = p->value & 0xff;
            unsigned char descType = p->value >> 8;
            unsigned int descLength;
            switch (descType) {
            case USB_DESCTYPE_DEVICE:
                usb_send_descriptor(CDC_Device_Desc, sizeof(CDC_Device_Desc));
                break;
            case USB_DESCTYPE_CONFIG:
                if (p->length == CDC_Config_Desc[0])
                    usb_send_descriptor(CDC_Config_Desc, 9);
                else if (p->length == 75)
                    usb_send_descriptor(CDC_Config_Desc, 75);
                else
                    print("? %d\n", p->length);
                break;
            case USB_DESCTYPE_STRING:
                if (descIndex > 3) descIndex = 0;
                usb_send_descriptor(strings[descIndex], strings[descIndex][0]);
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
    } else {
        switch (p->request) {
        case USB_SET_ADDRESS:
            // Send an ack (zero length response).
            print("set address %d\n", p->value);
            setting_addr = p->value;
            usb_send_ack();
            break;
        case USB_SET_CONFIGURATION:
            usb_send_ack();
            break;
        case USB_CDC_SET_LINE_CODING:
            usb_cdc_set_line_coding();
            break;
        case USB_CDC_SET_CONTROL_LINE_STATE:
            usb_send_ack();
            break;
        }
    }
}

void usbctrl(void)
{
    USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
    int status = regs->ints;
    //print("s %08ux\n", status);
    if (status & USB_INT_BUS_RESET) {
        regs->sie_status = USB_SIE_STATUS_BUS_RESET;
        regs->addr_endp[0] = 0;
    }
    if (status & USB_INT_SETUP_REQ) {
        regs->sie_status = USB_SIE_STATUS_SETUP_REC;
        usb_handle_setup();
    }
    if (status & USB_INT_BUFF_STATUS) {
        // EP0 in
        if (regs->buff_status & 1) {
            regs->buff_status = 1;
            if (setting_addr) {
                USBregs *regs = (USBregs *)USBCTRL_REGS_BASE;
                regs->addr_endp[0] = setting_addr;
                setting_addr = 0;
            } else
                usb_recv_ack();
        }
        // EP0 out
        if (regs->buff_status & 2)
            regs->buff_status = 2;
        // EP1 in
        if (regs->buff_status & 4)
            regs->buff_status = 4;
        // EP2 out
        if (regs->buff_status & 32) {
            regs->buff_status = 32;
            unsigned int *dpsram = (unsigned int *)USB_DPSRAM_BASE;
            dpsram[USB_EP2_OUT_BUFCTL] &= ~USB_BCR_FULL;
            dpsram[USB_EP2_OUT_BUFCTL] |= USB_BCR_AVAIL;
        }
    }
}
