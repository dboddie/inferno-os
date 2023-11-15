# Inferno for the Apollo3 Blue System on Chip

This directory contains a port of Inferno to the Ambiq Apollo3 SoC provided
with the [SparkFun MicroMod Artemis](https://www.sparkfun.com/products/16401)
processor board.

## Current state

The port provides the following features, which generally work, but should be
considered prototypes:

* Basic multitasking.
* Keyboard input and text output via a UART.
* Floating point arithmetic support.
* Access to the status LED on the MicroMod Artemis board.
* I2C and SPI interface support.
* An SPI-based driver for the UltraChip UC8159 display driver provided with
  the [Inky Impression](https://shop.pimoroni.com/products/inky-impression-4?variant=39599238807635)
  e-ink display.

There are some known issues:

* Text input is handled via a "fake" keyboard device, `kbd.c`, which is not
  very capable or flexible. A proper UART driver would be better.
* The current bootloader only writes to the lowest 512K of flash memory.
  This typically means that the `kernel` file should not exceed around 448K
  (512K - 64K) in size. If too many files are added to the root namespace,
  this limit is exceeded and the resulting system will fail to boot.

Hopefully, these can be resolved in time.

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

If this was successful, it should be possible to enter the `os/apollo3`
directory and build the port:

* Run `mk appl` to build any port-specific tools.
* Run `mk` to build the port.

This should result in a `kernel` file in the current directory.

## Installing

For the MicroMod Artemis board, you will need the
[svl.py](https://raw.githubusercontent.com/sparkfun/Apollo3_Uploader_SVL/main/svl.py)
tool to install the `kernel` file.

Connect the board to your computer using an USB-C cable. The board should
appear as a USB device. On Linux, a device file such as `/dev/ttyUSB0` should
have been created.

Run the `svl.py` script to upload the `kernel` file, using the appropriate
device file for your board if `/dev/ttyUSB0` is not relevant:

    svl.py -f kernel /dev/ttyUSB0

This should result in output like the following:

    Artemis SVL Bootloader
    Got SVL Bootloader Version: 5
    [##################################################]Upload complete

You should now be able to access the Inferno environment by running a terminal
emulator, as in this example where `/dev/ttyUSB0` is the device file in use:

    picocom -b 115200 /dev/ttyUSB0

And you should see something like this:

    Initial Dis: "/osinit.dis"
    **
    ** Inferno
    ** Vita Nuova
    **
    apollo3$

Please see the general Inferno documentation for information about how to use
Inferno. Note that man pages are not included in the default installation.
