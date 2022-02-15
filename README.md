# About OpenTag

OpenTag is an open-source RTOS-like firmware platform for implementing embedded networking stacks on microcontrollers.  It implements, specifically, the DASH7 Mode 2 networking stack, but it is being updated in 2022 to provide more generic features for networking stack synthesis.  

In the 2022+ timeframe, OpenTag's main value-add is that it provides a very resource constrained, low-power, low-latency platform for implementing IoT & LPWAN networking stacks on the 2nd core of dual-core wireless MCUs such as the STM32WL55.

## Some History

Development began in 2008, to implement ISO 18000-7 (DASH7 Mode 1) on the TI CC430.  This MCU contained 4KB SRAM, 32KB Flash, and a relatively slow CPU (~4 DMIPS).  There was no generic RTOS available at that time (and there still isn't) that can operate in such a resource constrained environment and provide sufficiently low latencies to perform the duties of a networking stack.  TinyOS was an interesting option, and it along with NanoRK have many architectural similarities as OpenTag, but so much hacking would have been needed to do to either of these, that it made sense to start OpenTag from the ground-up.

### DASH7

DASH7 is a wireless standard that was borne out of the US Military, and it is designed for *extreme* low-power and low-latency wireless communication, so it is great for communication between moving
things or for anything that can benefit from a lower power requirement.  DASH7 is also designed to finally eliminate "Application Profiles" as it introduces a filesystem and presentation-layer methodology not typically available in WSN/M2M/IoT implementations.  

## Maintainership & Licensing

OpenTag was created and is maintained by JP Norair.  The core parts of the code are BSD licensed, and other parts of the code -- typically networking stack modules -- may feature the OpenTag License.  The OpenTag License is similar to BSD, except that it is explicit about how IP ownership of implemented software shall be handled.

## Supported Platforms

OpenTag is designed to run on a hardware platform that includes a low-power microcontroller plus an RF transceiver.  However, much of it can also be compiled and run in a POSIX standard-C environment. ARM Cortex-M is the typical CPU family, although older versions have also run on CC430, MSP430F5, MSP430F6, and RF430 devices from Texas Instruments.  

At present, there is no roadmap to support MCUs that are not Cortex-M.  However, OpenTag is very portable, and community porting efforts to other platforms are welcome. 

## Version History (Abridged)

The latest version is 0.5.0.  Version 2.0 is scheduled for 03.2014 release.
Version 1.x is skipped because it should have been used somewhere in the 0.2 to
0.3 timeframe.  Several proprietary stacks exist, forked from those versions.

#### Version 2.0: 2022 [underway]

- STM32WL55 port
- ALP model replaced with more netports-style model
- Increased modularization of networking stacks
- New Filesystem implementation

#### Versions 1.1, 1.2:  2019

- Additional APIs added
- New devices supported (many)
- MSP430 platforms no longer supported
- LoRa supported as the radio
- Many demo & example apps added

#### Version 1.0:   09.2014

- Stress-tested version of 0.5.0
- New organization of files to make easier to integrate other libraries

#### Version 0.5.0:  01.2014

- Hicculp Kernel introduced for Cortex-M devices
- Implementation of final-draft Mode 2 communications specification
- MPipe v2: abstract data encapsulation and crypto support
- ALP-core v2: out-of-order messaging and IPC functionality
- Improvements to internal ALP protocols
- Platforms added: STM32L
- Radios added: SPIRIT1

#### Version 0.4.x:  12.2012

- Exokernel architecture formalized
- Kernel scheduling is now managed entirely in hardware
- Data Link Layer and MPipe modules are rearchitected as preemptive "exotasks"
- "Idle Time" tasks are rearchitected as cooperative "kernel tasks"
- Further improvements to the radio driver, especially CSMA and flooding
- RTC-based event synchronization introduced
- True Random Number Generator (TRNG) introduced

#### Version 0.3.x:  5.2012 - 10.2012

- MSP430F5 platform added
- CC1101 radio added
- USB-CDC MPipe support formalized
- Support for TI PaLFI wakeup module introduced
- Cooperative multitasking formalized
- Applet architecture introduced for system callbacks & session management
- General improvements & bug fixes from 0.2.x versions

#### Version 0.2.x:  1.2012 - 4.2012

- Directory restructuring
- Filesystem improvements
- Kernel resembles a normal kernel
- Support for STM32 platforms
- Support for new radios (non-CC430)
- DASH7 Session management formally introduced
- DASH7 PHY/MAC support expanded
- DASH7 queries working

#### Version 0.1.x:  8.2011 - 12.2011

- Projects are cleaned-up for TI CCS and other build tools
- MPipe, ALP, NDEF developed for client communication
- Client-Server API developed
- Improvements to early kernel
- Improvements to radio PHY/MAC and drivers
- Improvements to Networking and Transport layers

#### Version 0.0.x:  1.2011 - 7.2011

- otlib developed
- Early CC430 RF driver and PHY/MAC developed
- Early platform support
- Logic testing of otlib and timing testing of platform

