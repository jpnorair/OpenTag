# HT Linktest (ht_linktest) User Guide

HT Linktest is a simple, demo application for master-slave link testing.  It is not a low-power demo, as the devices are always listening or transmitting.  This guide assumes you are using as STMicro STM32 Discovery B072-LRWAN1 developers kit.  Others may be supported too, but this guide treats the B072-LRWAN1 kits.

## Purpose

* The purpose is to provide PER (Packet Error Rate) by having one side send a sequence of many packets and the other side receive them.
* 100 packets are sent for each initiation

## Initial HW Setup

1. Both dev kits should be loaded with the application firmware.
2. Make sure antennas are attached.  Without antennas, the app will still work, but the radio range will be very poor.
3. One kit should be plugged into a computer, via USB, with the `otter` application on it.  This device will be called the "Gateway."
4. The other kit can be plugged-in or powered with batteries.  This device will be called the "Endpoint."

## Otter Setup

The otter program is a binary, client-side terminal console.  It has many features.  For this demo, all it needs to do is read information from the "Gateway" and print it to screen.

1) Open a Terminal emulator on your mac or linux PC.  This guide assumes the reader knows how to do this.

2) Make sure "Gateway" is plugged-in via USB, and determine its the USB TTY interface file path.  This may take some trial and error.  Plug-in and unplug the device to check, and run the command below.

**Mac**

```
$ ls /dev/tty.usbmodem*
```

**Linux**

```
$ ls /dev/ttyACM*
```

3) The output of `ls` should be something like `/dev/tty.usbmodem1412` on the mac or `/dev/ttyACM1` on Linux.

4) Move to the directory `otter` is in (example `~/bin`), and call it with the above USB TTY file (example `/dev/tty.usbmodem1412`).  An example is below.  You may store otter in a different location on your system, and you may have a different TTY filename.  Substitute your values as necessary.  The baudrate should be always 115200.

```
$ cd ~/bin
$ ./otter /dev/tty.usbmodem1412 115200
```

5) Once you startup otter, otter will give you a blank command line and you have no way to enter text.  Push the ESCAPE button to open a prompt (looks like `otter~`).  To quit, open a prompt via ESCAPE and type "quit".  Ctrl+C also works and is safe.  Apart from `quit`, you don't actually need to use any of the otter command features for this demo.

```
otter~ quit
```


## Kit Runtime Configuration

### Buttons

There are two buttons on the B072-LRWAN1: one black, one blue.  The black button is a reset button.  The Blue button is an application button.

#### Black Button Test (Reset)

Push the black button.  Some red and green LEDs should blink, briefly.

#### Gateway Configuration via Buttons

To configure a board as a Gateway, use the buttons.  Gateways will open the serial port.  Endpoints will not.  You want the serial port open in order to get the results of the link test.

The Gateway-entry process is like this:

1. Hold Black button down.
2. While holding Black button, also hold Blue button down.
3. Release Black button (keep holding Blue button).
4. After you see red and green LEDs flash, release Blue button.
5. Now it is in Gateway mode.

#### Data Rate Control

The link test supports three data rates.  LoRa supports some very slow data rates and some relatively slow data rates.  This demo can do:

* ~0.8 kbps
* ~3.8 kbps
* ~15.6 kbps

1. Hold the Blue button for several seconds.
2. A green LED on the right will blink once (keep holding).
3. The green LED will blink twice (keep holding).
4. The green LED will blink three times (release!)

If you release the button after one blink, the board will use the low data rate (0.8 kbps).  Two blinks, the middle rate (3.8 kbps).  Three blinks, the high rate (15.6 kbps).

**Special note: make sure both boards use the same data rate!**


## Running Application Demo

### Press and Release Blue Button

To run the demo, it's very easy.  Press the blue button on the Endpoint, and release immediately.  100 packets will be sent in a series.  The Gateway will print a message to otter after the series completes.
