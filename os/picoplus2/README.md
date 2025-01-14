# Inferno for the Pimoroni Pico 2 Plus

This directory contains a port of Inferno to the Pimoroni Pico 2 Plus. This is
a development board using the RP2350 microcontroller that is also found on the
Raspberry Pi Pico 2 and derivatives.

## Current state

The port provides the following features, which generally work, but should be
considered prototypes:

* Basic multitasking.
* Keyboard input and text output via a UART.
* Floating point arithmetic support.
* Access to the status LED.

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

If this was successful, it should be possible to enter the `os/picoplus2`
directory and build the port:

* Run `mk appl` to build any port-specific tools.
* Run `mk` to build the port.

This should result in a `kernel` file in the current directory. This will need
to be encoded as a UF2 file for installation on the device.

## Creating a UF2 file

Probably the easiest way to create a UF2 file from the `kernel` file is to use
the `uf2conv.py` utility. This can be found in [this repository](https://github.com/microsoft/uf2).

Clone the repository, then add the `utils` directory within to the `PATH`
environment variable.

You can then run the `uf2conv.py` script to create a UF2 file:

    uf2conv.py -c -b 0x10000000 -f 0xe48bff59 -o out.uf2 kernel

The `out.uf2` file can then be installed on the device.

## Installing

Connect the Pico Plus 2 board to your computer using a USB-C cable. The board
should appear as a mass storage device called `RP2350` which you can mount and
open in a file browser. You should see files called `INFO_UF2.TXT` and
`INDEX.HTM` in the root directory of the device.

Copy the `out.uf2` file to the root directory of the device. This will take a
few seconds. The device will reset when this is complete.

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
