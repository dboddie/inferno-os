# Inferno for the STM32F405 MCU

This directory contains a port of Inferno to the STM32F405 MCU provided with
the [SparkFun MicroMod STM32](https://www.sparkfun.com/products/21326)
processor board.

## Current state

The port provides the following features, which generally work, but should be
considered prototypes:

* Basic multitasking.
* Keyboard input and text output via a UART.
* Floating point arithmetic support.
* Access to the status LED on the MicroMod STM32 board.
* I2C and SPI interface support.
* An SPI-based driver for the UltraChip UC8159 display driver provided with
  the [Inky Impression](https://shop.pimoroni.com/products/inky-impression-4?variant=39599238807635)
  e-ink display.
* An SPI-based driver for the Winbond W25Q128 flash memory chip on the MicroMod
  STM32 board.

There are some known issues:

* Text input is handled via a "fake" keyboard device, `kbd.c`, which is not
  very capable or flexible. A proper UART driver would be better.
* Input/output requires a USB-to-serial adaptor.

Hopefully, this can be resolved in time.

## Building

From the root directory of the Inferno repository, build a hosted installation
of Inferno in the usual way:

* Configure the values in the `mkconfig` file for your system.
* Run the `makemk.sh` script to build the `mk` executable. On x86-based Linux
  systems, this will be in the `Linux/386/bin` directory.
* Ensure that the `mk` executable is on the run path, typically by including
  its parent directory in the list of paths in the `PATH` environment variable.
* Run `mk nuke` then `mk mkdirs` to prepare the build environment.
* Run `mk install` to build a hosted system.

If this was successful, it should be possible to enter the `os/stm32f405`
directory and build the port:

* Run `mk appl` to build any port-specific tools.
* Run `mk` to build the port.

This should result in a `kernel` file in the current directory.

## Installing

For the MicroMod STM32 board, you will need to use the `dfu-util` tool to
install the `kernel` file. On Debian, this is provided by the `dfu-util`
package.

Connect the board to your computer using an USB-C cable, then hold down the
BOOT button on the board and tap the RESET button. The status LED should be
lit.

Check the devices listed by the `lsusb` command and make a note of the
vendor:product identifier for the device, such as this one:

    Bus 001 Device 014: ID 0483:df11 STMicroelectronics STM Device in DFU Mode

Run the `dfu-util` tool to list the connected DFU-enabled devices:

    dfu-util -l

This should show the device with the same vendor:product identifier as before,
and give a serial number, as in this example:

    Found DFU: [0483:df11] ver=2200, devnum=14, cfg=1, intf=0, path="1-4", alt=3, name="@Device Feature/0xFFFF0000/01*004 e", serial="207A3082434E"

To upload the `kernel` file run the `dfu-util` tool using `sudo` where
necessary, specifying the appropriate vendor:product identifier and serial
number:

    sudo dfu-util -d 0483:df11 -D kernel -s 0x08000000 -S "207A3082434E" -a 0

This should result in output that ends with something like the following:

    DfuSe interface name: "Internal Flash  "
    Downloading to address = 0x08000000, size = 479288
    Download        [=========================] 100%       479288 bytes
    Download done.

Once this has finished, press the RESET button on the board if you already
have a USB-to-serial adaptor connected.

## Connecting a serial adaptor

You will need a USB-to-serial adaptor to access USART3 on the processor.
Unplug the USB-C cable before attaching the USB-to-serial adaptor. If the
adaptor does not provide power, the USB-C cable can be connected again later.

The adapter's four wires need to access USART3 via connections on a carrier
board. This can either be done via the TX, RX and GND pins, or via a suitable
JST-SH cable to a QWIIC header that exposes the same connections.

Once the cable is connected to the board, connect it to your computer to power
the board.

## Using Inferno

You should now be able to access the Inferno environment by running a terminal
emulator, as in this example where `/dev/ttyUSB0` is the device file that is
created when the board is connected in non-bootloader mode:

    picocom -b 115200 /dev/ttyUSB0

And you should see something like this:

    Initial Dis: "/osinit.dis"
    **
    ** Inferno
    ** Vita Nuova
    **
    stm32f405$     

Please see the general Inferno documentation for information about how to use
Inferno. Note that man pages are not included in the default installation.
