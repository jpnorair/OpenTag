About PingPong Demo:
PingPong for OpenTag is quite advanced, compared to other PingPongs.  I hope
you think it is the world's coolest PingPong demo.  If you use it just with two
devices, it is just like any typical PingPong.

1. It is the same app for all devices (master-master)
2. It can work with more than two devices.  Practically, I'll say 16, but this
   can be increased by multiplying the timeout value.  Maybe it does more by
   nature, but I haven't tested it for throughput.
3. It does a 32bit anycast query ("PING") to validate that devices involved in 
   the pingpong game should be involved.  This is handled below the application
   layer, so any other DASH7 devices will ignore the PING unless they strangley
   have "PING" stored in bytes 0-3 the Routing Code ISF.
4. The PingPong command is a "UDP with Inventory" command, on the Routing Code
   ISF, looking for "PING".
5. The application code applies a random offset (following the response window).
   The value is less-than or equal-to (<=) the length of the response window.
   Thus, the earliest offset in the group is the first device to respond, and 
   it becomes the next "PING".
6. Otherwise, the devices just listen, so this is not a low-power app (it is a 
   high throughput app).

HW/board requirements: 
- Any device officially supported by OpenTag (any board in the "board" folder), 
  although for early versions (anything you download before 9/2012), only CC430
  and MSP430+CC1101 platforms are confirmed.  This is only because those boards
  were up-to-date when the app was first written.
- To get any feedback to your client (typically OTcom), you need a board with 
  MPipe output capability.  In some cases, you may need to rebuild the firmware 
  for the board in order to enable MPipe output.

Where to get more information:
http://www.indigresso.com/wiki/doku.php?id=opentag:apps:demo_pingpong
http://wiki.indigresso.com

--------------------------------------------------------------------------------
Usage Notes:
See the wiki (above) for instructions on how you can use the demo.

Compiler Constants
==================
This project's makefile or IDE configuration should pass several constants into
the compiler.  These constants are required for basic configuration.

__[partnum]__       (mandatory)     The part number of the platform being used.
                                    E.g. for MSP430F5529, it is __MSP430F5529__

__LITTLE_ENDIAN__   (mandatory)     Endian must be configured properly. 
__BIG_ENDIAN__                      All known cores are __LITTLE_ENDIAN__
    
__CCSv4__           (mandatory)     Compiler must be configured properly.
__CCSv5__                           CCSv5 is recommended for MSP430/CC430.
__GCC__                             GCC is recommended for ARM MCUs.
__IAR__

__LARGE_MEMORY__    (optional)      On MSP430F5 this enables large memory model.
                                    Code will be more compact and portable to 
                                    CC430 if you do *not* enable.  Recommended
                                    to use only for big filesystem storage apps.

__DEBUG__           (optional)      Two definitions that do the same thing, i.e.
DEBUG_ON [legacy]                   enable some debugging features in the code
