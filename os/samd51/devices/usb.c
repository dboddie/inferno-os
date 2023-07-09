#include "fns.h"
#include "../thumb2.h"
#include "usb.h"

extern void* memcpy(void *s1, void *s2, long n);
extern void usb_kbdput(char *chars, int l);

#define USB_pin_DN 24
#define USB_pin_DP 25

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

static const char CDC_Device_Desc[] = {
    0x12,       /* length */
    0x01,       /* type */
    0x10, 0x02, /* usb_version */
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
    0xfa        /* max_power: 250 * 2mA */
};

static const char CDC_Interface_Association_Desc[] = {
    0x08,   /* length */
    0x0b,   /* type */
    0x00,   /* first_iface */
    0x02,   /* iface_count */
    0x02,   /* fn_class */
    0x02,   /* fn_subclass */
    0x00,   /* fn_protocol */
    0x00    /* string_index: no string description */
};

static const char CDC_Comms_Interface_Desc[] = {
    0x09,   /* length */
    0x04,   /* type */
    0x00,   /* number */
    0x00,   /* alternative */
    0x01,   /* endpoints: 1 interrupt IN */
    0x02,   /* class: communications interface */
    0x02,   /* subclass: subclass code for abstract control model (ACM) */
    0x00,   /* no protocol */
    0x00    /* no string description */
};

static const char CDC_CS_Interface_Desc[] = {
    0x05,       /* length */
    0x24,       /* type: CS_INTERFACE */
    0x00,       /* subtype: header (first of these interfaces) */
    0x10, 0x01  /* BCD rel number */
};

static const char CDC_CS_ACM_Interface_Desc[] = {
    0x04,   /* length */
    0x24,   /* type: CS_INTERFACE */
    0x02,   /* subtype: ACM functional descriptor */
    0x06    /* bmCapabilities: Table 28, Universal Serial Bus Class Definitions
               for Communications Devices*/
};

static const char CDC_CS_Union_Interface_Desc[] = {
    0x05,   /* length */
    0x24,   /* type: CS_INTERFACE */
    0x06,   /* subtype: union */
    0x00,   /* bMasterIface: the comms interface */
    0x01    /* bSlaveIface0: the data interface */
};

static const char CDC_CS_Call_Management_Interface_Desc[] = {
    0x05,   /* length */
    0x24,   /* type: CS_INTERFACE */
    0x01,   /* subtype: call management */
    0x03,   /* bmCapabilities: Table 27, Universal Serial Bus Class Definitions
               for Communications Devices */
    0x01    /* bDataIface: the data interface */
};

static const char CDC_Comms_Endpoint_In_Desc[] = {
    0x07,       /* length */
    0x05,       /* type */
    0x83,       /* address: IN 3 */
    0x03,       /* attributes: interrupt (0x3) */
    0x08, 0x00, /* max_pktsize */
    0xff        /* interval */
};

static const char CDC_Data_Interface_Desc[] = {
    0x09,   /* length */
    0x04,   /* type */
    0x01,   /* number */
    0x00,   /* alternative */
    0x02,   /* endpoints: 2 endpoints (1 IN, 1 OUT) */
    0x0a,   /* class: communications interface */
    0x00,   /* subclass */
    0x00,   /* protocol */
    0x00    /* no string description */
};

static const char CDC_Data_Endpoint_In_Desc[] = {
    0x07,       /* length */
    0x05,       /* type */
    0x81,       /* address: IN 1 */
    0x02,       /* attributes: bulk (0x2) */
    0x40, 0x00, /* max_pktsize */
    0x00        /* interval */
};

static const char CDC_Data_Endpoint_Out_Desc[] = {
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
    "\x0e\x03S\x00A\x00M\x00D\x005\x001\x00",
    "\x0e\x031\x002\x003\x004\x005\x006\x00"
    };

/* Internal USB handling */

/* Configure the endpoint descriptor base address to point to an array of */
/* 4 blocks, each containing 8 32-bit words. */
static int desc_array[32];
/* Reserve buffers for the control endpoint data. */
static char ctl_in_array[128];
static char ctl_out_array[64];
/* the current request waiting for transfer complete */
static int usb_request;
static int device_address;
/* Reserve buffers for the endpoint data. */
static char ep1_bank1_array[64];
static char ep2_bank0_array[64];
static char ep3_bank1_array[8];
/* Implementation-specific properties */
static char serial_buffer[256];
static int bufstart, bufend;
/* Transfer properties */
static int data_bits, parity, stop_bits, data_rate;

static void usb_reset_peripheral(void);
static void usb_enable(int enable);
static void usb_config_memory(void);
static void usb_attach_device(int enable);
static void usb_enable_interrupts(void);
static USB_endpoint *usb_endp_addr(int n);
static USB_endpoint_desc *usb_endp_desc(int n, int high);
static void usb_init_control_endp(void);
static void usb_reset_byte_count(int endp, int bank);
static void usb_send_zlp(int endp);
static int usb_write_string_desc(int descIndex, void *addr);
static void usb_send_bytes(int endp, void *addr, int size);
static void usb_handle_control(void);
static void usb_respond(int endp, void *addr, int length, int wLength);
static void usb_handle_in(void);
static void usb_handle_out(void);
static void usb_handle_int(void);
static void usb_handle_setup(int size);
static int usb_prepare_config(char *addr);
static void usb_debug_bytes(char *bytes, int size);
static void usb_debug_chars(char *chars, int size);

void usb_init(void)
{
    enable_PORT();

    /* Setting the pin direction may not be needed. */
    const int pins = (1 << USB_pin_DN) | (1 << USB_pin_DP);
    PORT *port = (PORT *)PORT_base;
    port->dirset = pins;
    port->out &= ~pins;

    /* PA24,25 need to be set to function H (0x7) as described in the */
    /* I/O Multiplexing and Considerations chapter of the data sheet. */
    /* Each byte contains the functions for two pins, so byte 12 contains the */
    /* functions for these pins. */
    char *pmux = (char *)PORT_pmux;
    pmux[12] = 0x77;

    /* Configure the pins to use alternative peripheral control. */
    char *pincfg = (char *)PORT_pincfg;
    pincfg[USB_pin_DN] = PORT_pincfg_pmuxen;
    pincfg[USB_pin_DP] = PORT_pincfg_pmuxen;

    /* Enable generator 3 for the peripheral. */
    int *genctrl = (int *)GCLK_GENCTRL_base;
    genctrl[3] = GCLK_GENCTRL_idc | GCLK_GENCTRL_genen | GCLK_GENCTRL_src_DFLL;

    /* Use generator 3 for the peripheral. */
    int *pchctrl = (int *)GCLK_PCHCTRL_base;
    pchctrl[GCLK_USB] = GCLK_PCHCTRL_chen | 3;

    /* Enable the USB clock. */
    *(int *)MCLK_AHB_mask |= MCLK_AHB_USB;
    *(int *)MCLK_APBB_mask |= MCLK_APBB_USB;

    usb_reset_peripheral();

    /* Configure link power management handshake. */
    USB_device *dev = (USB_device *)USB_base;
    dev->ctrlb &= ~USB_ctrlb_lpmhdsk_mask;

    /* Set the USB calibration values. */
    int cdata = *(int *)NVM_calibration_usb;
    int transn = (cdata & NVM_calibration_usb_transn_mask) >> NVM_calibration_usb_transn_shift;
    int transp = (cdata & NVM_calibration_usb_transp_mask) >> NVM_calibration_usb_transp_shift;
    int trim = (cdata & NVM_calibration_usb_trim_mask) >> NVM_calibration_usb_trim_shift;
    dev->padcal = (transn << USB_padcal_transn_shift) |
                  (transp << USB_padcal_transp_shift) |
                  (trim << USB_padcal_trim_shift);

    /* Set up endpoint data structures and interrupts. */
    usb_config_memory();
    usb_enable_interrupts();

    /* Configure and enable the USB peripheral. */
    usb_enable(1);

    /* Attach the device to the bus. This should cause the reset interrupt to occur. */
    usb_attach_device(1);
}

static void usb_reset_peripheral(void)
{
    USB_device *dev = (USB_device *)USB_base;
    dev->ctrla = USB_ctrla_swrst;

    while ((dev->ctrla & USB_ctrla_swrst) && (dev->syncbusy & USB_syncbusy_swrst));
}

static void usb_enable(int enable)
{
    USB_device *dev = (USB_device *)USB_base;
    if (enable)
        dev->ctrla = USB_ctrla_enable;
    else
        dev->ctrla = 0;

    while (dev->syncbusy & USB_syncbusy_enable);
}

static void usb_config_memory(void)
{
    /* Refer to the contents of the array. */
    USB_device *dev = (USB_device *)USB_base;
    dev->descadd = (int)desc_array;

    /* Link the descriptors to the buffers. */
    USB_endpoint_desc *desc = usb_endp_desc(0, 0);
    desc->addr = (int)ctl_out_array;
    desc->pcksize = USB_endp_pcksize_size_64B;
    desc = usb_endp_desc(0, 1);
    desc->addr = (int)ctl_in_array;
    desc->pcksize = USB_endp_pcksize_size_64B;

    /* Link the descriptors to the buffers. */
    /* Endpoint 1 */
    desc = usb_endp_desc(1, 1);
    desc->addr = (int)ep1_bank1_array;
    desc->pcksize = USB_endp_pcksize_size_64B;
    /* Endpoint 2 */
    desc = usb_endp_desc(2, 0);
    desc->addr = (int)ep2_bank0_array;
    desc->pcksize = (64 << USB_endp_pcksize_multi_shift) | USB_endp_pcksize_size_64B;

    /* Endpoint 3 */
    desc = usb_endp_desc(3, 1);
    desc->addr = (int)ep3_bank1_array;
    desc->pcksize = USB_endp_pcksize_size_8B | (8 << USB_endp_pcksize_multi_shift);
}

static void usb_attach_device(int enable)
{
    USB_device *dev = (USB_device *)USB_base;
    if (enable)
        dev->ctrlb &= ~USB_ctrlb_detach;
    else
        dev->ctrlb |= USB_ctrlb_detach;
}

static void usb_enable_interrupts(void)
{
    /* Interrupt line 80 for USB interrupts. */
    *(int *)NVIC_ISER2 = (0xd << 16);

    /* Enable only the EORST interrupt. */
    USB_device *dev = (USB_device *)USB_base;
    dev->intenclr = 0xffff;
    dev->intenset = USB_int_eorst;
    /* Clear all pending interrupts. */
    dev->intflag = 0xffff;
}

void usb_intr(void)
{
    USB_device *dev = (USB_device *)USB_base;
    int v = dev->intflag;

    if (v & USB_int_eorst) {
        /* Reset the endpoints. */
        usb_init_control_endp();
        dev->intflag = USB_int_eorst;
        v &= ~USB_int_eorst;
    }

    /* Clear the interrupt to acknowledge it. */
    dev->intflag = v;

    int epintsmy = dev->epintsmry;
    if (epintsmy == 0)
        return;

    /* Handle the control, in and out endpoints. */
    if (epintsmy & 0x1)
        usb_handle_control();
        
    if (epintsmy & 0x2)
        usb_handle_in();

    if (epintsmy & 0x4)
        usb_handle_out();

    if (epintsmy & 0x8)
        usb_handle_int();
}

static USB_endpoint *usb_endp_addr(int n)
{
    return (USB_endpoint *)(USB_epbase + (n * 0x20));
}

static USB_endpoint_desc *usb_endp_desc(int n, int high)
{
    /* The address points to the contents of a byte array (the first element). */
    USB_device *dev = (USB_device *)USB_base;
    return (USB_endpoint_desc *)(dev->descadd + (n * 0x20) + (high * 0x10));
}

static void usb_init_control_endp(void)
{
    /* Configure endpoint zero descriptors. */
    USB_endpoint_desc *desc = usb_endp_desc(0, 0);
    /* OUT endpoint handles 64 byte packets - also reset byte count to zero. */
    desc->pcksize = USB_endp_pcksize_size_64B;

    /* IN endpoint handles 64 byte packets - also reset byte count to zero. */
    desc = usb_endp_desc(0, 1);
    desc->pcksize = USB_endp_pcksize_size_64B;

    /* Configure the control endpoint. */
    USB_endpoint *endp = usb_endp_addr(0);
    endp->cfg = (USB_epcfg_ctl << USB_epcfg_in_shift) |
                (USB_epcfg_ctl << USB_epcfg_out_shift);

    /* Enable endpoint interrupts. */
    endp->intenclr = 0xff;
    endp->intenset = USB_epint_rxstp | USB_epint_trcpt0 | USB_epint_trcpt1;
    /* Clear pending interrupts. */
    endp->intflag = 0xff;
}

static void usb_reset_byte_count(int endp, int bank)
{
    /* Reset the byte count in the endpoint descriptor's packet size register. */
    USB_endpoint_desc *desc = usb_endp_desc(endp, bank);
    desc->pcksize &= ~USB_endp_pcksize_count_mask;
}

static void usb_send_zlp(int endp)
{
    USB_endpoint *ep = usb_endp_addr(endp);
    ep->intenclr = USB_epint_trcpt1;

    /* Use any reasonable address. No data will be read from it, anyway. */
    usb_send_bytes(endp, desc_array, 0);

    ep->intenset = USB_epint_trcpt1;
    ep->statusset = USB_epstatus_bk1ready;
}

static int usb_write_string_desc(int descIndex, void *addr)
{
    if (descIndex > 3) descIndex = 0;

    const char *s = strings[descIndex];
    memcpy(addr, s, s[0]);
    return s[0];
}

static void usb_send_bytes(int endp, void *addr, int size)
{
    /* Get the address of the endpoint's IN memory registers. */
    USB_endpoint_desc *desc = usb_endp_desc(endp, 1);
    desc->addr = (int)addr;
    desc->pcksize = USB_endp_pcksize_size_64B | size;
}

static void usb_respond(int endp, void *addr, int length, int wLength)
{
/*    int mask = USB_epint_trcpt1 | USB_epint_trfail1; */

    if (wLength < length)
        length = wLength;

    usb_send_bytes(endp, addr, length);

    /* Clear the bk0ready status bit to indicate space for new data. */
    /* Set the bl1ready status bit to indicate data for transfer. */
    USB_endpoint *ep = usb_endp_addr(endp);
    ep->statusclr = USB_epstatus_bk0ready;
    ep->statusset = USB_epstatus_bk1ready;
}

static void usb_handle_control(void)
{
    USB_endpoint_desc *desc;
    int osize;
    USB_endpoint *ep = (USB_endpoint *)USB_epbase;
    int epv = ep->intflag;

    if (epv & USB_epint_trcpt0) {
        if (usb_request == 0x20) {
            USB_endpoint_desc *desc = usb_endp_desc(0, 0);
            osize = desc->pcksize & USB_endp_pcksize_count_mask;
            if (osize == 7) {
                data_rate = ctl_out_array[0];
                stop_bits = ctl_out_array[4];
                parity = ctl_out_array[5];
                data_bits = ctl_out_array[6];
            }

            usb_send_zlp(0);
            usb_request = 0;
        }
    }

    if (epv & USB_epint_rxstp) {
        ep->intflag = USB_epint_rxstp;

        /* Read the number of bytes received from the bank memory block. */
        desc = usb_endp_desc(0, 0);
        osize = desc->pcksize & USB_endp_pcksize_count_mask;

        usb_handle_setup(osize);
    }

    if (epv & USB_epint_trcpt1) {
        if (usb_request == USB_SET_ADDRESS) {
            /* Set the address and enable it. */
            USB_device *dev = (USB_device *)USB_base;
            dev->dadd = device_address | USB_dadd_adden;
            /* Reset the byte count for the endpoint. */
            usb_reset_byte_count(0, 0);
            usb_request = 0;
        }
    }

    /* Clear the interrupt to acknowledge it. */
    ep->intflag = epv;
}

static void usb_handle_in(void)
{
    /* Handle endpoint 1 (IN). */
    USB_endpoint *ep = usb_endp_addr(1);
    int epv = ep->intflag;

    if ((epv & USB_epint_stall1) == 0) {
        /* Request a stall for the next transaction, but leave the bank ready. */
        ep->statusset = USB_epstatus_stallrq1;
        ep->statusclr = USB_epstatus_dtglin;
    }

    ep->statusset = USB_epstatus_bk1ready;

    /* Clear the interrupt to acknowledge it. */
    ep->intflag = epv;
}

static void usb_handle_out(void)
{
    /* Handle endpoint 2 (OUT). */
    USB_endpoint *ep = usb_endp_addr(2);
    int epstatus = ep->status;
    int epv = ep->intflag;
    USB_endpoint_desc *desc = usb_endp_desc(2, 0);

    int osize = desc->pcksize & USB_endp_pcksize_count_mask;
    char *chars = (char *)desc->addr;

    usb_kbdput(chars, osize);

    if (epstatus & USB_epstatus_bk0ready) {
        ep->statusclr = USB_epstatus_bk0ready;
        usb_reset_byte_count(2, 0);
    }

    /* Clear the interrupt to acknowledge it. */
    ep->intflag = epv;
}

static void usb_handle_int(void)
{
    wrstr("**int**\r\n");
    USB_endpoint *ep = usb_endp_addr(3);
    int epv = ep->intflag;
    ep->intflag = epv;
}

/* Handle a setup request supplied in the control endpoint's OUT array with the */
/* size specified in bytes. */

static void usb_handle_setup(int size)
{
    USB_endpoint *endp;
    USB_endpoint_desc *desc;
/*    usb_debug_bytes(ctl_out_array, size);*/

    /* Check what the packet contains. */
    USB_setup *pkt = (USB_setup *)ctl_out_array;

    if ((pkt->type == 0) && (pkt->request == USB_SET_CONFIGURATION)) {

        /* Configure the other endpoints. */
        endp = (USB_endpoint *)usb_endp_addr(1);
        endp->cfg = USB_epcfg_bulk << USB_epcfg_in_shift;
        endp->intenset = USB_epint_trcpt1 | USB_epint_stall1;
        endp->statusset = USB_epstatus_bk1ready | USB_epstatus_stallrq1;

        endp = (USB_endpoint *)usb_endp_addr(2);
        endp->cfg = USB_epcfg_bulk << USB_epcfg_out_shift;
        endp->intenset = USB_epint_trcpt0 | USB_epint_trfail0;

        endp = (USB_endpoint *)usb_endp_addr(3);
        endp->cfg = USB_epcfg_int << USB_epcfg_in_shift;
        endp->intenset = USB_epint_trcpt1;
        endp->statusset = USB_epstatus_bk1ready;
        usb_send_zlp(0);
        return;

    } else if (pkt->type == 0x21) {

        if (pkt->request == USB_CDC_SET_LINE_CODING) {
            /* Accept more data in bank 0. */
            endp = (USB_endpoint *)USB_epbase;
            endp->statusclr = USB_epstatus_bk0ready;
            desc = (USB_endpoint_desc *)usb_endp_desc(0, 0);
            desc->pcksize = USB_endp_pcksize_size_64B | (8 << USB_endp_pcksize_multi_shift);
            usb_request = pkt->request;

        } else if (pkt->request == USB_CDC_SET_CONTROL_LINE_STATE)
            usb_send_zlp(0);

        return;
    }

    /* The epstatus register for this endpoint should have bits */
    /* set:   DTGLOUT (0x01), DTGLIN (0x02), CURBK (0x04), BK0RDY (0x40) */
    /* clear: BK1RDY (0x80), STALLRQ0 (0x10), STALLRQ1 (0x20) */

    if (pkt->type & 0x80)
    {
        /* Device to host */
        if ((pkt->type & 0x7f) == 0)
        {
            /* Standard requests */
            if (pkt->request == USB_GET_DESCRIPTOR)
            {
                /* 0: requestType, 1: request, 2: index, 3: type, ... */
                int descIndex = pkt->value & 0xff;
                int descType = pkt->value >> 8;
                int descLength;

                if (descType == USB_DESCTYPE_DEVICE) {
                    descLength = 18;
                    memcpy(ctl_in_array, CDC_Device_Desc, descLength);

                } else if (descType == USB_DESCTYPE_CONFIG) {
                    descLength = usb_prepare_config(ctl_in_array);

                } else if (descType == USB_DESCTYPE_STRING) {
                    descLength = usb_write_string_desc(descIndex, ctl_in_array);

                } else if ((descType == 0x06) || (descType == 0x0f)) {
                    /* Send a stall to speed up the process of not responding */
                    /* to these requests. */
                    endp = (USB_endpoint *)USB_epbase;
                    endp->statusset = USB_epstatus_stallrq1;
                    return;
                } else
                    return;

                usb_respond(0, ctl_in_array, descLength, pkt->length);
            }
        }
    } else {
        /* Host to device */
        if (pkt->request == USB_SET_ADDRESS) {
            /* Record the address and send an acknowlegement. */
            /* Set the address when the response is delivered. */
            device_address = ctl_out_array[2] & USB_dadd_dadd_mask;
            usb_send_zlp(0);
            usb_request = pkt->request;
        }
    }
}

static int usb_prepare_config(char *addr)
{
    memcpy(addr, CDC_Config_Desc, 9);
    memcpy(addr + 9, CDC_Interface_Association_Desc, 8);
    memcpy(addr + 17, CDC_Comms_Interface_Desc, 9);
    memcpy(addr + 26, CDC_CS_Interface_Desc, 5);
    memcpy(addr + 31, CDC_CS_ACM_Interface_Desc, 4);
    memcpy(addr + 35, CDC_CS_Union_Interface_Desc, 5);
    memcpy(addr + 40, CDC_CS_Call_Management_Interface_Desc, 5);
    memcpy(addr + 45, CDC_Comms_Endpoint_In_Desc, 7);
    memcpy(addr + 52, CDC_Data_Interface_Desc, 9);
    memcpy(addr + 61, CDC_Data_Endpoint_In_Desc, 7);
    memcpy(addr + 68, CDC_Data_Endpoint_Out_Desc, 7);
    return 75;
}

static void usb_debug_bytes(char *bytes, int size)
{
    wrstr("bytes: ");
    int i = 0;
    while (i < size) {
        _wrhex(bytes[i], 2);
        wrch(' ');
        i++;
    }
    newline();
}

static void usb_debug_chars(char *chars, int size)
{
    wrstr("chars: '");
    for (int i = 0; i < size; i++) {
        char ch = chars[i];
        if ((ch >= 32) && (ch < 127))
            wrch(ch);
        else {
            wrch('\\');
            _wrhex(ch, 2);
        }
        i++;
    }
    wrch('\'');
    newline();
}

void usb_serwrite(char *s, int n)
{
    usart_serwrite(s, n);
    int i = 0, j = 0;
    char c;
    USB_endpoint *ep = usb_endp_addr(1);

    while ((i < n) && (j < 64)) {
        c = s[i++];
        ep1_bank1_array[j++] = c;
        if (j == 64)
            break;

        if (c == '\n')
            ep1_bank1_array[j++] = '\r';
    }

    ep->statusclr = USB_epstatus_stallrq1;
    ep->statusset = USB_epstatus_bk1ready;

    USB_endpoint_desc *desc = usb_endp_desc(1, 1);
    desc->pcksize = USB_endp_pcksize_size_64B | j;
}
