# Inferno for the SAMD51 MCU

This directory contains a port of Inferno to the Microchip ATSAMD51J20 MCU
provided with the [SparkFun MicroMod SAMD51](https://www.sparkfun.com/products/16791)
processor board.

## Current state

The port provides the following features, which generally work, but should be
considered prototypes:

* Basic multitasking.
* Keyboard input and text output via a UART.
* Access to the status LED on the MicroMod SAMD51 board.

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

If this was successful, it should be possible to enter the `os/samd51`
directory and build the port:

* Run `mk appl` to build any port-specific tools.
* Run `mk` to build the port.

This should result in a `kernel` file in the current directory.

## Installing

*This section is vague because I can no longer test the port. Use caution and
consult the [Hookup Guide](https://learn.sparkfun.com/tutorials/micromod-samd51-processor-board-hookup-guide/all)
where necessary.*

For the MicroMod SAMD51 board, you will need to use the `bossac` tool to
install the `kernel` file. On Debian, this is provided by the `bossa-cli`
package.

Connect the board to your computer using an USB-C cable, then double tap the
RESET button. The status LED should blink or fade in and out.

To upload the `kernel` file run the `bossac` tool using `sudo` where
necessary:

    bossac -i --port=ttyACM0 -U -i --offset=0x4000 -w -v kernel

Once this has finished, press the RESET button on the board if you already
have a USB-to-serial adaptor connected.

## Connecting a serial adaptor

You will need a USB-to-serial adaptor to access SERCOM1 on the processor.
Unplug the USB-C cable before attaching the USB-to-serial adaptor. If the
adaptor does not provide power, the USB-C cable can be connected again later.

The adapter's four wires need to access SERCOM1 via connections on a carrier
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
    samd51$

Please see the general Inferno documentation for information about how to use
Inferno. Note that man pages are not included in the default installation.
