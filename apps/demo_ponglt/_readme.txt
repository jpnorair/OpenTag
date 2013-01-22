About demo_ponglt
=================
PongLT is a simple, OpenTag demo application for peer-to-peer pinging for link
testing purposes (hence LT).  It is not a low-power demo, as the devices are
always pinging, ponging, or listening.


Functional purpose of this demo
===============================
 - Show a simple way how anycast queries can be used
 - Show how a command session can be initiated by a button press
 - Provide a simple way to do link testing between a demonstration system of 
     two or more devices


Application Description
=======================
Pong for Link Testing is a masterless application that allows one device to
send a query and all the others to respond.  It does not use any sort of power
conservation measures -- all the devices are listening all the time.  This is
what makes it good for link testing.  It is also an incredibly simple starting 
point for OpenTag users.

The Ping is a UDP-with-inventory packet on port 255, which uses the "User ID" 
ISF to store a small cookie that qualifies devices that have the string 
"APP=PongLT" somewhere in that file.  Only these devices will fully process the
Ping request and produce a Pong response.


Stock Applets Used
==================
Many of the stock applets from /otlibext/stdapplets/ are used as signal 
callbacks from the protocol layers.  You can look at extf_config.h to see which 
ones are enabled (or, to specify new ones).


This Application Recommends
===========================
CC430/MSP430F5: ~24KB Flash, ~1.5KB SRAM
Cortex M3:      ~32KB Flash, ~2KB SRAM
 - MPipe connection to show pong responses
 - Two LEDs to show RX/TX activity
 - One input source to send pings (a button), or sending by ALP over MPipe.


Supported Boards & Platforms
============================
See /apps/demo_opmode/code/platform_config.h for board & platform support.
NOTE: Different boards may support different methods of input and output.


Supported Project IDEs
======================
Currently, TI CCS v5 is supported for MSP430 builds (proj_ccsv5).  Raisonance 
RIDE7 is supported for Cortex M3 builds.  By the time you read this, there may
also be support for the SAT (Summon ARM Toolchain), which is a toolchain using
OpenOCD and Linaro-GCC.


Wiki Link & More Information
============================
http://www.indigresso.com/wiki/doku.php?id=opentag:apps:demo_ponglt
http://wiki.indigresso.com


--------------------------------------------------------------------------------
Usage Notes
===========
Also see the wiki (above) for instructions on how you can use the demo.

Compiler Constants:
This project's makefile or IDE configuration should pass several constants into
the compiler.  These constants are required for basic configuration.

__[partnum]__       (mandatory)     The part number of the platform being used.
                                    E.g. for MSP430F5529 = __MSP430F5529__, for
                                    STM32L151C8 = __STM32L151C8__.__

__LITTLE_ENDIAN__   (mandatory)     Endian must be configured properly. 
__BIG_ENDIAN__                      All supported cores are __LITTLE_ENDIAN__
    
__CCSv5__           (mandatory)     Compiler must be configured properly.  
__GCC__                             RIDE and SAT both use GCC.

__RESTRICT=int      (optional)      Restricts flash allocation to lower [int]
                                    bytes.  Helpful for using low-cost tools
                                    that have code restrictions.

__LARGE_MEMORY__    (optional)      Ignored on Cortex-M.  Enables 20 bit memory
                                    addressing on MSP430X2 devices, which does
                                    incur a performance penalty.  Do not use it
                                    unless you absolutely need it.

__DEBUG__           (optional)      Enable debugging features in the code.

__PROTO__           (optional)      Enable prototyping features in the code.
                                    __DEBUG__ enables these features, too, plus
                                    more.  An example of a __PROTO__ feature is 
                                    ID generation at startup, which is not used
                                    in final products that get IDs flashed-in
                                    during manufacturing.
