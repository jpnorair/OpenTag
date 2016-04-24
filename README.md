About
=====
OpenTag is an open-source RTOS that implements the DASH7 standard for wireless
sensor networking and M2M comms.  DASH7 is a wireless standard that was borne
out of the US Military, and it is designed for *extreme* low-power and low-
latency wireless communication, so it is great for communication between moving
things or for anything that can benefit from a lower power requirement.  DASH7
is also designed to finally eliminate "Application Profiles" as it introduces a
filesystem and data methodology not available to any other WSN/M2M/IoT
standard.  Most DASH7 devices can be re-mapped over-the-air to take-on vastly
different applications (sort of like the scene in "The Matrix" where Trinity
learns to fly a helicopter).

OpenTag was created and is maintained by JP Norair.  Norair also invented DASH7
during the same time, so there is a high degree of synergy between the standard
and the implementation.  Do not be fooled by the small build size: OpenTag is a
full-featured, real-time exokernel with a large API and library.  It is capable
of communicating with native DASH7 peers as well as UDP, SCTP (future), and
NDEF connections.


Supported Platforms
===================
OpenTag is designed to run on a hardware platform that includes a low-power
microcontroller plus a 433 MHz RF transceiver.  However, much of it can also be
compiled and run in a POSIX standard-C environment.  On the MSP430, a typical
endpoint build might require 16-24KB ROM (Flash) and 2KB RAM.  For gateway or
router projects, a typical MSP430 build requires 20-32KB ROM and 3-4KB RAM.  As
a "rule of thumb," Cortex-M builds tend to use about 12% more program memory
(flash) and 33% more RAM.

OpenTag is increasing targeting Cortex-M as the "standard" MCU, although it
also runs nicely on CC430, MSP430F5, MSP430F6, and RF430 devices from Texas
Instruments.  Officially, there is no roadmap to support MCUs that are not
Cortex-M or MSP430F5 derived.  However, OpenTag is very portable, and community
porting efforts to other platforms are welcome.  MCUs actively supported are:
ST STM32F10x, ST STM32L1xx, TI CC430F5/6, TI MSP430F5/6, TI RF430F5978.

The RF transceiver component for OpenTag must comply to the DASH7 requirements,
which means that it should support GFSK and MSK modulation in the 433 MHz band.
Transceivers that are actively supported at this time of writing are: ST
SPIRIT1, TI CC430, TI CC110x.  Several other devices have been supported in the
past, and many devices are suitable for use with OpenTag.


The Code
========
This is the OpenTag Mainline Distribution.  It is maintained by JP Norair,
the creator of OpenTag and inventor of the DASH7 wireless specification.

The Haystack Distribution of OpenTag (HDO) is a superset of the mainline
distribution, by Haystack Technologies (http://www.haystacktechnologies.com).
HDO includes a number of features, patches, and APIs that are not available in
the mainline distribution.  Haystack tends to release one of its proprietary
improvements back into the mainline distribution each six months.


Important URLs
==============
Repository:     http://www.github.com/jpnorair/opentag
Public Wiki:    http://www.indigresso.com/wiki/doku.php
Public Forum:   http://www.indigresso.com/forum/
Pro Support:    http://www.haystacktechnologies.com


License
=======
OpenTag uses the OpenTag License, which is an open source license similar to
ClearBSD, Analog Devices BSD, and some others.  You can read about the OpenTag
License on the Indigresso Wiki or on this GitHub project Wiki.


Version History (Abridged)
==========================
The latest version is 0.5.0.  Version 2.0 is scheduled for 03.2014 release.
Version 1.x is skipped because it should have been used somewhere in the 0.2 to
0.3 timeframe.  Several proprietary stacks exist, forked from those versions.

[PLANNED]
Version 2.x:    ???
- Additional APIs added
- Multi-threading added
- New "Jupiter" library that allows multiple Wiring/Duino sketches to run simultaneously.
- Many demo & example apps added
- MSP430 & CC430 platforms foreward-ported from 0.4.x to 2.x


Version 2.0:   09.2014
- Stress-tested version of 0.5.0
- New organization of files to make easier to integrate other libraries


Version 0.5.0:  01.2014
- Hicculp Kernel introduced for Cortex-M devices
- Implementation of final-draft Mode 2 communications specification
- MPipe v2: abstract data encapsulation and crypto support
- ALP-core v2: out-of-order messaging and IPC functionality
- Improvements to internal ALP protocols
- Platforms added: STM32L
- Radios added: SPIRIT1


Version 0.4.x:  12.2012
- Exokernel architecture formalized
- Kernel scheduling is now managed entirely in hardware
- Data Link Layer and MPipe modules are rearchitected as preemptive "exotasks"
- "Idle Time" tasks are rearchitected as cooperative "kernel tasks"
- Further improvements to the radio driver, especially CSMA and flooding
- RTC-based event synchronization introduced
- True Random Number Generator (TRNG) introduced


Version 0.3.x:  5.2012 - 10.2012
- MSP430F5 platform added
- CC1101 radio added
- USB-CDC MPipe support formalized
- Support for TI PaLFI wakeup module introduced
- Cooperative multitasking formalized
- Applet architecture introduced for system callbacks & session management
- General improvements & bug fixes from 0.2.x versions


Version 0.2.x:  1.2012 - 4.2012
- Directory restructuring
- Filesystem improvements
- Kernel resembles a normal kernel
- Support for STM32 platforms
- Support for new radios (non-CC430)
- DASH7 Session management formally introduced
- DASH7 PHY/MAC support expanded
- DASH7 queries working


Version 0.1.x:  8.2011 - 12.2011
- Projects are cleaned-up for TI CCS and other build tools
- MPipe, ALP, NDEF developed for client communication
- Client-Server API developed
- Improvements to early kernel
- Improvements to radio PHY/MAC and drivers
- Improvements to Networking and Transport layers


Version 0.0.x:  1.2011 - 7.2011
- otlib developed
- Early CC430 RF driver and PHY/MAC developed
- Early platform support
- Logic testing of otlib and timing testing of platform

