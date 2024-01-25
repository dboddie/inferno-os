# Inferno for the Teensy 4.1/MicroMod

This directory contains a port of Inferno to the i.MX RT1062 SoC provided
with the [SparkFun MicroMod Teensy](https://www.sparkfun.com/products/16402)
processor board.

## Current state

The port provides the following features, which generally work, but should be
considered prototypes:

* Basic multitasking.
* Keyboard input and text output via a UART.
* Floating point arithmetic support.
* Access to the status LED on the MicroMod Teensy board.

There are some known issues:

* The system occasionally encounters a lock loop during preemption, causing a
  panic and a reboot.
* Text input is handled via a "fake" keyboard device, `kbd.c`, which is not
  very capable or flexible. A proper UART driver would be better.

Hopefully, these can be resolved later.

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

If this was successful, it should be possible to enter the `os/teensy41mm`
directory and build the port:

* Run `mk appl` to build any port-specific tools.
* Run `mk` to build the port.

This should result in a `kernel` file in the current directory.

## Installing

For the MicroMod Teensy board, you will need to use the `teensy_loader_cli`
tool to install the `kernel` file. This is available from its
[GitHub repository](https://github.com/PaulStoffregen/teensy_loader_cli).

Connect the board to your computer using an USB-C cable, then press and
release the BOOT button on the board. The PROG LED should be lit.

Check the devices listed by the `lsusb` command and check that the device is
present:

    Bus 001 Device 023: ID 16c0:0478 Van Ooijen Technische Informatica Teensy Halfkay Bootloader

To upload the `kernel` file run the `upload` rule:

    mk upload

This should result in output like the following:

    teensy_loader_cli --mcu=TEENSY_MICROMOD -v -w kernel.hex
    Teensy Loader, Command Line, Version 2.2
    Read "kernel.hex": 430680 bytes, 2.6% usage
    Found HalfKay Bootloader
    Programming.......................................................
    Booting

If programming results in an error that ends with

    Programming...error writing to Teensy

repeat the `mk upload` command. It may take two attempts to install the system.

If a serial-to-USB adaptor is already connected, you can skip the next section.

## Connecting a serial adaptor

You will need a USB-to-serial adaptor to access LPUART6 on the processor.
Unplug the USB-C cable before attaching the USB-to-serial adaptor. If the
adaptor does not provide power, the USB-C cable can be connected again later.

The adapter's four wires need to access LPUART6 via connections on a carrier
board. This can be done via the TX, RX and GND pins on the MicroMod ATP carrier
board.

Once the cable is connected to the board, connect it to your computer to
access the UART. If the cable has not been connected to any of the power pins
on the carrier board, connect the carrier board via USB-C to power it.

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
    teensy41mm$

Please see the general Inferno documentation for information about how to use
Inferno. Note that man pages are not included in the default installation.
