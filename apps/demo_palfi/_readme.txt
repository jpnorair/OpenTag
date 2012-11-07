About Demo_PaLFi:
Demo_PaLFi is an OpenTag demo application including a PaLFi Master and Slave.  
The Master and Slave are two independent applications, and they are compiled 
onto different devices (Master and Slave).  Additionally, there is a project
called "Slave2" that can be used for DASH7 (UHF) link testing.

Master: 
- MSP430F55xx + CC1101 (e.g. MSP-EXP430F5529 board + CC1101EMK)
- used as a DASH7 Gateway
- Connected to PC client over USB-CDC or Serial
- Future variants may include TMS3705 driver as well

Slave:
- CC430+PalFi SiP (RF430F5978)
- used as DASH7 endpoint, with PaLFi transponder interface
- Not connected to PC client over wireline

Slave2:
- Same as normal slave, except PaLFI interface is not implemented
- Instead of PaLFI, button push sends a sensor reading.

The Project folder (CCSv5_proj, in future also CCSv4_proj and GCC) contains a
project file for master and one for slave.  The code for the Master is in the
folder "Master" and the code for Slave in "Slave."  They are two independent
application codebases.


Known, Supported Boards:
For the Master, any hardware that supports OpenTag gateway will support this 
application.  In the original demo, a TI MSP-EXP430F5529 mainboard with 
CC1101EMK daughterboard was used.  Pick any board with enough resources to meet
your needs.  The TI RF430 USB Gateway module (part #) can be considered the 
minimum requirement.

For the Slave, a PaLFi transponder IC is required.  The only supported platform
at the moment is a CC430-variant (SiP) that has this PaLFi transponder inside
the same chip package (part #)


Additional Components:
You will need the RI-ACC-ADR2 kit from Texas Instruments, or something similar
to this, to act as the PaLFi interrogator.  This kit includes an interface
daughterboard that connects to a PC client via USB, and it also includes a
Windows PC GUI program that enables you to send PaLFi commands to the device.


Where to get more information:
http://www.indigresso.com/wiki/doku.php?id=opentag:board:tiproto_exp5529
http://www.indigresso.com/wiki/doku.php?id=opentag:apps:demo_palfi
http://wiki.indigresso.com

--------------------------------------------------------------------------------
Usage Notes:
See the wiki (above) for instructions on how you can use the demo.

Compiler Constants
==================
This project's makefile or IDE configuration should pass several constants into
the compiler.  These constants are required for basic configuration.

__[partnum]__       (mandatory)     The part number of the platform being used.
                                    For MSP430F5529, it is __MSP430F5529__

__LITTLE_ENDIAN__   (mandatory)     Endian must be configured properly. 
__BIG_ENDIAN__                      All MSP430F5 cores are __LITTLE_ENDIAN__
    
__CCSv4__           (mandatory)     Compiler must be configured properly.
__CCSv5__                           CCSv5 is recommended.
__GCC__
__IAR__

__LARGE_MEMORY__    (optional)      On MSP430F5 this enables large memory model.
                                    Code will be more compact and portable to 
                                    CC430 if you do *not* enable.  Recommended
                                    to use only for big filesystem storage apps.

__DEBUG__           (optional)      Two definitions that do the same thing, i.e.
DEBUG_ON [legacy]                   enable some debugging features in the code
