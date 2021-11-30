About sim_network_load
======================
Network Load is a simulator that runs entirely in a POSIX pthreads environment.
It simulates a piece of OpenTag hardware on a computer.  Each piece of OpenTag HW
that is simulated exists as an independent process using multiple threads.


Functional purpose of this app
==============================
 - Testing features of OpenTag without going to HW
 - Testing network load of different communication patterns and MAC configurations
 - Demonstrating the efficiency of the DASH7 Query Transport Layer without having
   to ship thousands of devices.


Application Description
=======================



Stock Applets Used
==================
Many of the stock applets from /otlibext/stdapplets/ are used as signal 
callbacks from the protocol layers.  You can look at extf_config.h to see which 
ones are enabled (or, to specify new ones).


This Application Recommends
===========================
A modern computer running Linux or Mac OS X.  The project was created in XCode on
Mac OS X, but there is also a generic Makefile.

Supported Boards & Platforms
============================
- Mac OS X 10.10
- Linux support is unofficial, but it works fine.


Supported Project IDEs
======================
- XCode
- GNU build tools


Wiki Link & More Information
============================
http://www.indigresso.com/wiki/doku.php?id=opentag:apps:sim_network_load
http://wiki.indigresso.com


--------------------------------------------------------------------------------
Usage Notes
===========
Also see the wiki (above) for instructions on how you can use the demo.


