About app_7lan
==============
7LAN is an application that implements DASH7 in a manner similar to classical
networking.  A device powers-up, searches for a network, and then joins it. 
Therefore, it is connection-oriented, although connection maintenance overhead
is kept to a minimum.


Application Description
=======================
The 7LAN app specifies two kinds of devices: gateways and endpoints.  When a
device powers-up, it will look for the presence of an existing 7LAN network.
If one is detected, the device will join it as an endpoint.  If no network is
detected, the device will become the gateway for the network.  Additionally, it 
is possible to manually configure devices as an endpoint or gateway.  In future
versions of 7LAN, it may also be possible to configure routers and passive 
gateways, but these features are not included in the present implementation.

Communication of devices in the 7LAN is by standard DASH7 means.  Therefore, 
the 7LAN user may initiate a standard-addressed DASH7 communication with DASH7 
devices outside the 7LAN, using the universal OpenTag API, and the connection
to the 7LAN will remain alive.  However, doing this can be quite complicated 
for users that are not experts in DASH7, so the 7LAN app includes a simplified, 
port-based API for UDP-style communication between devices.  The only caveat is
that it is limited to communication among devices presently connected to the 
same 7LAN network.  Furthermore, CoAP supported is expected to be included in
the near future, offerring users an even further abstracted way to use the 7LAN.

Devices in the network are configured to periodically listen for activity.  The
duty cycle is configurable, but the default is 250ms for endpoints and 31ms for
gateways.  Therefore, the worst-case latency for a message to an endpoint is 
250ms, and 31ms for a message to a gateway.  Once the first communication is 
established, subsequent communications of the same session occur with minimal
latency (typ <5ms).  Future advancements to the 7LAN app will include options 
to introduce time-synchronization and guaranteed time slots for communication 
(i.e. TDMA).

Addressing and networking in the 7LAN is done using the 16 bit DASH7 VID and 
8 bit subnet specifier, used according to the DASH7 standard.  When a device is 
connected to the 7LAN, the controlling gateway provides it with a VID.  DASH7
addressing methods allow a maximum of 16 fully independent co-located 7LANs, 16 
maskable sub-LANs per 7LAN, and 65536 client devices per 7LAN.  However, the
gateway must store at least 16 bytes of information per device in the 7LAN, or
more if encryption is used, so the amount of devices per 7LAN is practically 
limited by the amount of RAM available on the gateway.  On a 10KB device like
the STM32L151C8 (commonly used in Haystack boards), a 7LAN is best kept smaller
than 256 devices.

As a special note, UDP on DASH7 may be used with unicast (point-to-point) 
addressing and broadcasting, as is normal for UDP, but it can also use special
multicast and anycast addressing.  This is unique to DASH7, which has a query-
based addressing model vastly more powerful that what IP has.  Queries may be
on tokens up to 16 bytes in length, and they are capable of searching through 
up to 4096 contiguous *or non-contiguous* bytes in the DASH7 filesystem.  The 
query functionality is much like what is available with SQL comparison 
operators, including arithmetic, logical, string, and partial-string searching.


Stock Applets Used
==================
Many of the stock applets from /otlibext/stdapplets/ are used as signal 
callbacks from the protocol layers.  You can look at extf_config.h to see which 
ones are enabled (or, to specify new ones).


This Application Recommends
===========================
Gateway or Endpoint:
MCU with 8KB SRAM & 32KB Flash, or greater.  E.g. STM32L151C8 + SPIRIT1

Endpoint only:
MCU with 2KB SRAM & 20KB Flash, or greater.  E.g. CC430



Supported Boards & Platforms
============================
The best boards to use for 7LAN are the Haystack Jupiter or Haytag boards.
See /apps/app_7lan/code/platform_config.h for complete board & platform support.



Supported Project IDEs
======================
Currently, TI CCS v5 is supported for MSP430 builds (proj_ccsv5).  Raisonance 
RIDE7 is supported for Cortex M3 builds.  By the time you read this, there may
also be support for the SAT (Summon ARM Toolchain), which is a toolchain using
OpenOCD and Linaro-GCC.


Wiki Link & More Information
============================
http://www.indigresso.com/wiki/doku.php?id=opentag:apps:app_7lan
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
