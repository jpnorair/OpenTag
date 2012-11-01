About OpenTag:
OpenTag is an open-source RTOS that implements the DASH7 standard for wireless 
sensor networking (WSN) and M2M comms.  It began in 2008 as a project for 
active RFID "tags," but it has morphed into a much more sophisticated system
for deploying most sorts of WSN, IoT, M2M, or A-RFID devices.


Quick facts:
  * Uses DASH7: http://www.indigresso.com/wiki/doku.php?id=dash7_mode_2:main
  * Written chiefly in C -- only C and ASM allowed in primary codebase
  * Device builds target 8/16/32bit MCUs.  Client builds target POSIX / Std C
  * Configurable for implementing DASH7 endpoints (tags, nodes), subcontrollers
      (routers), gateways (servers) -- everything
  * Can be specially compiled as a client API that communicates across a 
      message pipe (MPipe) with a gateway device


Project Directories:
apps:           Various applications for OpenTag to compile & deploy
board:          Boards supported by OpenTag (not all apps support all boards)
otkernel:       OpenTag's exokernel scheduler & task manager code
otlib:          Main OpenTag libraries & API (platform independent)
otlibext:       Extended libraries & applets (usually platform independent)
otplatform:     Platform-dependent libraries & drivers
otradio:        DASH7 PHY/MAC & platform-dependent RF transceiver driver
sandbox:        Code that existed prior to the directory restructuring
supplements:    Reference materials and support code


Licensing:
OpenTag is licensed via the OpenTag License, which is a permissive open source 
license much like the well-known BSD license.  The difference is that the 
OpenTag License is explicit about how intellectual property must be treated, 
worldwide.  In the EU and USA, the BSD license is ruled to implicitly provide 
the same terms as the OpenTag License (so it is the same in practice), but in 
other markets the local laws may be different.  Thus, the OpenTag license 
applies EU and USA rulings of BSD to the whole world.
http://github.com/jpnorair/OpenTag/wiki/OpenTag-License-Information.


Where to get more information:
Wiki:           http://www.indigresso.com/wiki
Repository:     http://www.github.com/jpnorair/opentag
Pro Support:    http://www.haystacktechnologies.com
DASH7:          http://www.dash7.org


--------------------------------------------------------------------------------
Version History (abridged)
Versions released prior to 01.2011 were internal alphas.


Version 0.4.0:  01.11.2012 
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
