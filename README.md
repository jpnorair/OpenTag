About
=====
OpenTag is an open-source RTOS that implements the DASH7 standard for wireless sensor networking and M2M comms.  
DASH7 is a wireless standard that was borne out of the US Military, and it is designed for *extreme* low-power and 
low-latency wireless communication, so it is great for communication between moving things or for anything that can 
benefit from a lower power requirement.  DASH7 is also designed to finally eliminate "Application Profiles" as it 
introduces a filesystem and data methodology not available to any other WSN/M2M/IoT standard.  Most DASH7 devices 
can be re-mapped over-the-air to take-on vastly different applications (sort of like the scene in "The Matrix" where 
Trinity learns to fly a helicopter).

OpenTag was created and is maintained by JP Norair.  Norair also invented DASH7 during the same time, so there is a 
high degree of synergy between the standard and the implementation.  Do not be fooled by the small size: OpenTag is a 
full-featured, real-time exokernel with a large API and library.  It is capable of communicating with native DASH7 
peers as well as UDP, SCTP, and NDEF connections.

OpenTag is designed to run on low-power microcontrollers.  On the MSP430, a typical endpoint project might require 
16-24KB ROM (Flash) and 1-2KB RAM.  For gateway or router projects, a typical project MSP430 project is 20-32KB ROM 
and 2-4KB RAM.  Cortex M projects are typically about 12% larger.  Officially, there is no roadmap to support 
non-MSP430 or non-Cortex-M microcontrollers, but OpenTag is very portable, and there are community projects to 
port to, at least, AVR8 and PIC24.


The Code
========
This is the OpenTag Mainline Distribution.  It is maintained by JP Norair, 
the creator of OpenTag and inventor of the DASH7 wireless specification.
The Haystack Distribution of OpenTag (HDO) is a contributory distribution 
of this distribution.  (http://www.haystacktechnologies.com)


Public Wiki
===========
Indigresso Wiki: http://www.indigresso.com/wiki/doku.php


Public Support Forum
====================
Indigresso Forum: http://www.indigresso.com/forum/


Professional Support Partners 
=============================
Haystack Technologies: http://www.haystacktechnologies.com


License
=======
OpenTag uses the OpenTag License, which is an open source license similar to ClearBSD, Analog Devices BSD, 
and some others.  You can read about the OpenTag License on the Indigresso Wiki or on this GitHub project Wiki.