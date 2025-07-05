# Inferno for RP2350-based boards

This directory contains a port of Inferno to the Raspberry Pi Pico 2 and
related development boards that use the RP2350 microcontroller.

## Current state

The port provides the following features, which generally work, but should be
considered prototypes:

* Basic multitasking.
* Floating point arithmetic support.
* Access to the status LED.
* USB device support, functioning as a serial-like device.
* Optional keyboard input and text output via a UART.

There are some known issues:

* Text input is handled via a "fake" keyboard device, `kbd.c`, which is not
  very capable or flexible. A proper UART driver would be better.

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

If this was successful, it should be possible to enter the `os/rp2350`
directory and build the port:

* Run `mk appl` to build any port-specific tools.
* Run `mk CONF=<file>` to build the port, where `<file>` is the name of the
  configuration file for a particular board. For example, `picoplus2.conf` is
  the file to use for the Pimoroni Pico Plus 2.

This should result in a `inferno.uf2` file in the current directory. This can
be installed on the device.

## Installing

Connect the Pico Plus 2 board to your computer using a USB-C cable. The board
should appear as a mass storage device called `RP2350` which you can mount and
open in a file browser. You should see files called `INFO_UF2.TXT` and
`INDEX.HTM` in the root directory of the device.

Copy the `inferno.uf2` file to the root directory of the device. This will take a
few seconds. The device will reset when this is complete.

## Using Inferno via the USB-C connection

Inferno will automatically act as a USB communication device. On systems that
provide it, running the `lsusb -v` command will show a line like this to
indicate that the device has been found:

    Bus 001 Device 010: ID dbdb:5678 Testing Pico2W

A TTY device file should have been created by the operating system, and this
can be used to access the console on the device. For example, on Debian, the
`picocom` tool can be used to do this. In this example command, `ttyACM0` is
the appropriate device file, but this will depend on your system:

    picocom -b 115200 --echo --imap lfcrlf --omap crlf /dev/ttyACM0

The mapping options help to ensure a conventional terminal experience.

Once you have opened the terminal, it is possible to export a namespace over
the USB connection with

    export / /dev/usb/data

or, alternatively

    export / #u/data

The connection can be used from Inferno hosted on Linux by first ensuring that
the TTY device raw is configured to be used for raw data. On Debian, the `stty`
command is used for this; for example:

    stty /dev/ttyACM0 raw -icanon -iexten -echo

This can also be done from within the hosted environment, using the `os`
command in the Inferno shell:

    os stty /dev/ttyACM0 raw -icanon -iexten -echo

Note that the `stty` command is using paths in the external Linux environment.

If the device file is symlinked from Linux into a hosted Inferno directory
structure, such as with the command

    ln -s /dev/ttyACM0 acm

the `mount` command can then be used to access the namespace exported by the
device:

    mount -A acm /n/pico

This should run successfully and return immediately, allowing access to the
device's namespace via the `/n/pico` directory. If the `mount` command stalls,
it is possible that the TTY is not configured correctly and the bytes are being
transformed in transit.

## Using Inferno via a serial adaptor

By default, the keyboard interface that accepts input via a UART is disabled
by the `-DNO_KEYBOARD_IN_SWITCHER` definition in the `mkfile`. If this is
commented out or removed, Inferno will provide this interface.

Inferno uses UART1 to present a terminal to the user via the QW/ST connector on
the board. You will need a USB-to-serial adaptor and a JST-SH cable to access
the TX, RX and GND lines exposed by this connector. The power line does not
need to be connected because power is supplied by the USB connection.

Once the adaptor is connected to the board, connect it to your computer.

You should now be able to access the Inferno environment by running a terminal
emulator, as in this example where `/dev/ttyACM0` is the device file in use:

    picocom -b 115200 /dev/ttyACM0

If you press the reset button on the device, the terminal emulator should show
something like this:

    Initial Dis: "/osinit.dis"
    **
    ** Inferno
    ** Vita Nuova
    **
    picoplus2$

If you set up the terminal emulator before copying the `out.uf2` file to the
device's storage, this text will appear automatically and there is no need to
reset the device to see it.

Please see the general Inferno documentation for information about how to use
Inferno. Note that man pages are not included in the default installation.
