typedef struct {
    char ctrla;    /* byte */
    char _pad0;
    char syncbusy; /* byte */
    char _pad1;
    int _pad2;
    short ctrlb;   /* 2 bytes */
    char dadd;     /* device address (byte) */
    char _pad3;
    int _pad4;
    short fnum;    /* frame number (2 bytes) */
    short _pad5;
    int intenclr;  /* lower 2 bytes */
    int intenset;  /* lower 2 bytes */
    int intflag;   /* lower 2 bytes */
    int epintsmry; /* endpoint interrupts (2 bytes) */
    int descadd;   /* descriptor address (4 bytes) */
    short padcal;  /* calibration (2 bytes) */
} USB_device;

#define USB_base 0x41000000

enum {
    USB_ctrla_mode_mask = 0x80,
    USB_ctrla_mode_host = 0x80,
    USB_ctrla_runstdby = 0x04,
    USB_ctrla_enable = 0x02,
    USB_ctrla_swrst = 0x01,
    USB_syncbusy_enable = 0x02,
    USB_syncbusy_swrst = 0x01,
    USB_ctrlb_lpmhdsk_mask = 0x300,
    USB_ctrlb_lpmhdsk_shift = 10,
    USB_ctrlb_lpmhdsk_none = 0,
    USB_ctrlb_lpmhdsk_ack = 1,
    USB_ctrlb_lpmhdsk_nyet = 2,
    USB_ctrlb_detach = 0x1,
    USB_dadd_adden = 0x80,      /* device address enable */
    USB_dadd_dadd_mask = 0x7f,  /* device address mask */
    USB_fnum_fncerr = 0x80,
    USB_fnum_fnum_mask = 0x3ff8,
    USB_fnum_fnum_shift = 3,
    USB_fnum_mfnum_mask = 0x7,
    USB_fnum_mfnum_shift = 0,
    USB_int_wakeup = 0x10,
    USB_int_eorst = 0x08,       /* End of reset */
    USB_int_sof = 0x04,         /* Start of frame */
    USB_int_suspend = 0x01,
    USB_padcal_transn_shift = 6,
    USB_padcal_transp_shift = 0,
    USB_padcal_trim_shift = 12
};

/* Endpoint registers repeat every 0x20 bytes. */
#define USB_epbase        0x41000100

typedef struct {
    int cfg;         /* 0 (byte) */
    char statusclr;  /* 4 (byte) */
    char statusset;  /* 5 (byte) */
    char status;     /* 6 (byte) */
    char intflag;    /* 7 (byte) */
    char intenclr;   /* 8 (byte) */
    char intenset;   /* 9 (byte) */
} USB_endpoint;

#define USB_epcfg_in_mask 0x70
#define USB_epcfg_in_shift 4
#define USB_epcfg_out_mask 0x07
#define USB_epcfg_out_shift 0
#define USB_epcfg_disabled 0
#define USB_epcfg_ctl 1
#define USB_epcfg_iso 2
#define USB_epcfg_bulk 3
#define USB_epcfg_int 4
#define USB_epcfg_dual 5
#define USB_epstatus_bk1ready 0x80
#define USB_epstatus_bk0ready 0x40
#define USB_epstatus_stallrq1 0x20
#define USB_epstatus_stallrq0 0x10
#define USB_epstatus_curbk    0x04
#define USB_epstatus_dtglin   0x02
#define USB_epstatus_dtglout  0x01
#define USB_epint_stall1  0x40
#define USB_epint_stall0   0x20
#define USB_epint_rxstp   0x10
#define USB_epint_trfail1 0x08
#define USB_epint_trfail0 0x04
#define USB_epint_trfail  0x04
#define USB_epint_trcpt1  0x02
#define USB_epint_trcpt0  0x01
#define USB_epint_trcpt   0x01

/* Endpoint descriptors repeat every 0x10 bytes with OUT followed by IN. */
typedef struct {
    int addr;
    int pcksize;
    short extreg;
    char status_bk;
} USB_endpoint_desc;

#define USB_endp_addr 0x00          /* address of data block (4 bytes) */
#define USB_endp_pcksize 0x04       /* packet size information (4 bytes) */
#define USB_endp_extreg 0x08        /* 2 bytes */
#define USB_endp_status_bk 0x0a     /* byte */

#define USB_endp_pcksize_auto_zlp 0x80000000
#define USB_endp_pcksize_size_mask 0x70000000
#define USB_endp_pcksize_size_64B  0x30000000
#define USB_endp_pcksize_size_8B  0x00000000
#define USB_endp_pcksize_multi_mask 0x0fffc000
#define USB_endp_pcksize_multi_shift 14
#define USB_endp_pcksize_count_mask 0x3fff

/* NVM Software Calibration Area Mapping */
#define NVM_calibration 0x00800080
#define NVM_calibration_usb 0x00800084
#define NVM_calibration_usb_transn_mask 0x001f
#define NVM_calibration_usb_transp_mask 0x03e0
#define NVM_calibration_usb_trim_mask   0x1c00
#define NVM_calibration_usb_transn_shift 0
#define NVM_calibration_usb_transp_shift 5
#define NVM_calibration_usb_trim_shift   10

typedef struct {
    char type;
    char request;
    short value;
    short index;
    short length;
} USB_setup;
