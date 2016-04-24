MPipe Bridge
============
Here is a build of OpenTag that does not implement DASH7!

This app implements a bridge between two MPipe interfaces.  For example, one
could be USB and the other UART.  There is an option to do authentication, 
cryptography, and intermediate protocol processing on the Bridge platform.  
This is a fine way to offload those requirements from a network processor type
device.


Application Description
=======================
A typical application is bridging USB to UART.  The bridge in this case might
also act as an authentication co-processor or a protocol converter, which, for
example, is very helpful for interfacing Android clients with OpenTag Hosts.

+------------+             +------------+              +------------+ 
|   Client   |             |   Bridge   |              |   OpenTag  |
|    e.g.    | <---USB---> |    e.g.    | <---UART---> |    Host    |
|  Android   |             |  STM32F0   |              |            |
+------------+             +------------+              +------------+


This Application Recommends
===========================
Cortex-M: ≥ 32KB Flash, ≥ 2KB SRAM, ≥ 8MHz
 + USB interface
 + UART interface


Supported Boards & Platforms
============================
The STM32F07 discovery board is the first one specifically intended for demoing
this application, although any board supporting two MPipe interfaces should be
adequate.  The STM32F0 and L0 are great chips for this app, because they can
support "Crystal-less USB" as well as small IC packages, and as such the 
solution cost is low and the size is very small. 


Supported Project IDEs
======================
Raisonance RIDE7 + GCC is supported for Cortex M builds.  Since GCC is used,
a pure GNU build and debug environment is certainly possible, but it is not
supported at the moment.


Wiki Link & More Information
============================
http://www.indigresso.com/wiki/doku.php?id=opentag:apps:mpipe_bridge
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
    
__GCC__             (mandatory)     Compiler must be configured properly.  
                                    GCC is the supported compiler.

__RESTRICT=int      (optional)      Restricts flash allocation to lower [int]
                                    bytes.  Helpful for using low-cost tools
                                    that have code restrictions.

__LARGE_MEMORY__    (optional)      Ignored on Cortex-M.  Enables 20 bit memory
                                    addressing on MSP430X2 devices, which does
                                    incur a performance penalty.  Do not use it
                                    unless you absolutely need it.

__DEBUG__           (optional)      Enable debugging/prototyping features in
__PROTO__                           the code, or enable release features.  See
__RELEASE__                         app documentation for exactly what these 
                                    features enable/disable.
