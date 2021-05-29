# Notes

Starting a port of Inferno to the Letux 400 Minibook in parallel to the port of
the Qi Hardware Ben NanoNote.

## Memory map

Linux kernels appear to be loaded at 0x80010000 by U-Boot, according to the
uImage file I found. Looking at some kernel sources, I see that the file
`arch/mips/boot/Makefile` specifies a `LOADADDR` that matches this.

The memory map at https://projects.goldelico.com/p/letux400/page/MemoryMap/

## GPIO

The lists at
https://web.archive.org/web/20120214203947/http://projects.kwaak.net/twiki/bin/view/Epc700/GPIO
and the contents of the `arch/mips/jz4730/proc.c` file in old Letux 400 kernels
show the following:

 * Caps Lock LED is on port 0/A pin 27
 * Num Lock LED is on port 2/C pin 22
 * Scroll Lock LED is on port 0/A pin 9

These appear to be active low.

The Linux kernel uses the GPIO_GPAUR(n) macro
(defined in include/asm-mips/mach-jz4730/regs.h) in the __gpio_as_pwn() macro
(defined in include/asm-mips/mach-jz4730/ops.h) to set the flags in the
high selection register of port 2/C at 0x10010060 + 0x14, clearing bits 28-31
and setting bits 28 and 30 so that the SEL31 and SEL30 pairs of bits both
contain a value of 1. This sets pins 30 and 31 of port 2/C to have function 1.

## LCD

The Linux kernel (in the arch/mips/jz4730/board-minipc.c file) calls the
__gpio_as_lcd_master() macro (defined in include/asm-mips/mach-jz4730/ops.h)
which calls REG_GPIO_GPALR(1), REG_GPIO_GPALR(1), REG_GPIO_GPAUR(1) and
REG_GPIO_GPAUR(1) (defined in include/asm-mips/mach-jz4730/regs.h) to set up
the low and high selection registers of port 1/B at 0x10010030 + 0x10 and 0x14.

The effect of this is to keep the functions of bits 0-7, setting the functions
of bits 8-23 and 27-31 to 1, and setting the function of 24-26 to 2.

	REG_GPIO_GPALR(1) &= 0x0000FFFF;	\
	REG_GPIO_GPALR(1) |= 0x55550000;	\
	REG_GPIO_GPAUR(1) &= 0x00000000;	\
	REG_GPIO_GPAUR(1) |= 0x556A5555;	\

Either the LCD PCLK or the output of the PLL supplies the LCD pixel clock
(LPCLK) and LCD device clock (LDCLK) via separate dividers.

Landfall defines the dimensions, depth, frame rate and other parameters for the
Minibook panel in pkg/devices/lib/panel/src/letux400/panel-letux400.c

    static struct Jz4740_lcd_panel panel = {
      .config = (
          Jz4740_lcd_mode_tft_generic
        | Jz4740_lcd_pclock_negative
        | Jz4740_lcd_hsync_negative
        | Jz4740_lcd_vsync_negative
        | Jz4740_lcd_de_positive),

      .width = 800,
      .height = 480,
      .bpp = 16,
      .frame_rate = 60,
      .hsync = 80,
      .vsync = 20,
      .line_start = 0,
      .line_end = 0,
      .frame_start = 0,
      .frame_end = 0,
    };

The config constants are defined in the pkg/devices/lib/lcd/include/lcd-jz4740-config.h
file.

It also provides the get_pixel_clock() function to calculate the pixel clock in
pkg/devices/lib/lcd/src/jz4740/lcd-jz4740.cc which performs these calculations:

  multiplier = have_serial_tft() ? 3 : 1;

  pclk = _panel->frame_rate *
         (_panel->width * multiplier +
          _panel->hsync + _panel->line_start + _panel->line_end) *
         (_panel->height +
          _panel->vsync + _panel->frame_start + _panel->frame_end);

There are also checks for the panel mode.

Assuming that the panel is driven in parallel, the pixel clock frequency will
be

    f = 60 * (800 + 80) * (480 + 20) = 26400000

Clocks need to be configured to obtain this frequency, and this is done in the
pkg/devices/lib/cpm/src/jz4730.cc file, which contains the mechanics of how the
PLL is configured.

According to the JZ4730 documentation, if EXCLK is 3686400 Hz then it can drive
the USB clock at 47.9232 MHz, this involves multiplying the frequency by 13.
It also recommends not setting the raw PLL output to less than 100 MHz.

    PLL_freq_raw = EXCLK * M/N
    PLL_freq = PLL_freq_raw / [1,2,2,4]

Reading the value of the PLCR register on a running system results in a value
of 0x5a000520 being obtained. This is decoded to

    PLLFD = 180     multiply by 182
    PLLRD = 0       divide by 2
    PLLOD = 0       divide by 1
    PLLS  = 1       PLL is on and stable
    PLLBP = 0       no bypass
    PLLEN = 1       PLL is enabled
    PLLST = 32      stabilize time

Calculating the PLL output frequency

    PLL_freq = (EXCLK * (PLLFD + 2) / (PLLRD + 2)) / od[PLLOD]
             = (3686400 * 182 / 2)
             = 335462400 Hz
             = 335.4624 MHz

This frequency can be divided by 7 to obtain a USB frequency of 47.9232 MHz.

The LCD device clock is derived from this via the LFR bits in the CFCR register.
The LCD pixel clock is derived from this via the PXFR bits in the CFCR register.

The CFCR register appears to contain a value of 0x0d522220. This is decoded to

    SCS = LCS = I2CS = UCS = 0
    UFR = 6                     (USB divider = 7)
    MCS = 1                     (support 24 MHz MSC clock)
    SCKLOOE = 1
    UPE = 1                     (update division ratios immediately)
    MFR = 2                     (memory clock divider = 3)
    LFR = 2                     (LCD device clock divider = 3)
    PFR = 2                     (peripheral clock divider = 3)
    HFR = 2                     (system clock divider = 3)
    CFR = 0                     (CPU clock divider = 2)

### Pixel and device clock ratio

In pkg/devices/lcd/src/jz4740/lcd-jz4740-device.cc the set_timing() function
stops the LCD clock, sets the frequencies using a 3/1 ratio for the device to
pixel clocks:

    cpm_device->set_lcd_frequencies(pclk, 3);
    cpm_device->update_output_frequency();
    cpm_device->start_lcd();

Later, in the enable() function, it sets up the DMA descriptors:

    chip->config((struct Jz4740_lcd_descriptor *) desc_vaddr,
                 (struct Jz4740_lcd_descriptor *) desc_paddr,
                 fb_paddr);

The set_lcd_frequencies(pclk, 3) call refers to a function in the
pkg/devices/lib/cpm/src/jz4730.cc file:

    uint32_t out = get_output_frequency(),
             lcd = pclk * ratio;

    set_lcd_pixel_divider(out / pclk);

    // Limit the device frequency to 150MHz.

    if (lcd > 150000000) lcd = 150000000;

    set_lcd_device_divider(out / lcd);

This calculates the LCD device clock (lcd) from the pixel clock (pclk), then
finds the appropriate divider for the PLL output frequency to produce the pixel
clock frequency.

For the PLL_freq value we have obtained (335462400), plus the pixel clock value
(26400000), the LCD pixel divider will be 12.7069... which truncates to 12 for
a faster than necessary clock. The LCD device clock divider will be 4.

The LCD device clock divider is not specified using a linear scale (see
https://projects.goldelico.com/p/letux400/page/Clocks/ for details) but a value
of 4 is encoded as 3 in any case.
