About test_swcodec:
test_swcodec is a test application for measuring the performance of the software
encoder & decoder on hardware.  The system uses a trigger pulse (rising-edge
initiated) to measure the duration of the encoder and decoder processes, so you
will need a logic analyzer to go with it.  Fortunately, there are several low
cost PC-based products (check-out the "Logic 8") that can be used for this 
purpose.  (JP note: I personally use a Intronix LogicPort)

test_swcodec uses the Null radio driver, so it doesn't actually send data over 
the radio.

Master: 
- MSP430F55xx + CC1101 (e.g. MSP-EXP430F5529 board + CC1101EMK)
- used as a DASH7 Gateway
- Connected to PC client over USB-CDC or Serial
- Future variants may include TMS3705 driver as well

Slave:
- CC430+PalFi SiP (part number not known)
- used as DASH7 endpoint, with PaLFi transponder interface
- Not connected to PC client over wireline

The Project folder (CCSv5_proj, in future also CCSv4_proj and GCC) contains a
project file for master and one for slave.  The code for the Master is in the
folder "Master" and the code for Slave in "Slave."  They are two independent
application codebases.


Known, Supported Boards:
Any board that presently is supported by OpenTag can be ported easily to work
with test_swcodec.  You need to do two things:

1. Define "__NULL_RADIO__" in your compiler, or otherwise make sure that the
   Null Radio driver is compiled and linked to the build
   
2. Make sure trig1 is mapped to the pin you are connecting to your logic 
   analyzer.  It can also connect to an LED if you wish.  Null Radio will call
   otapi_led1_on() & otapi_led1_off() to drive this pin.

Where to get more information:
http://www.indigresso.com/wiki/doku.php?id=opentag:apps:test_swcodec
http://wiki.indigresso.com

--------------------------------------------------------------------------------
Usage Notes:
See the wiki (above) for instructions on how you can use the demo.

Compiler Constants
==================
This project's makefile or IDE configuration should pass several constants into
the compiler.  These constants are required for basic configuration.

__NULL_RADIO__      (mandatory)     Tells the preprocessor to build the Null
                                    Radio driver instead of the normal driver.

__[partnum]__       (mandatory)     The part number of the platform being used.
                                    e.g. for MSP430F5529, it is __MSP430F5529__

__LITTLE_ENDIAN__   (mandatory)     Endian must be configured properly. 
__BIG_ENDIAN__                      All supported cores are __LITTLE_ENDIAN__
    
__CCSv4__           (mandatory)     Compiler must be configured properly, based
__CCSv5__                           on what system you are using.
__GCC__
__IAR__

__LARGE_MEMORY__    (optional)      Some cores (MSP430F5) have a large memory
                                    option.  For most cores it is ignored.

__DEBUG__           (optional)      Two definitions that do the same thing, i.e.
DEBUG_ON [legacy]                   enable some debugging features in the code
