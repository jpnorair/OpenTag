Readme for POSIX C
==================
In case you don’t know (which I find to be unlikely), C is a programming 
language and POSIX is a system standard for operating systems.  Therefore, 
“POSIX C” stipulates a C-based programming architecting using POSIX libraries.
Basically, it just means UNIX with C.

POSIX C is an available platform for OpenTag, although it’s not real-time, so
it’s only really suitable for simulation of OpenTag.  This can be a nice 
feature for testing OpenTag code in an environment that’s much faster than
embedded, and it even allows scripting and various other high-level programming
techniques to be spliced-in, for the purpose of testing and potentially 
brainstorming new technologies.  It is also nice for doing networking stress
tests, or other such optimization of integration parameters.  For example, 
gathering and deploying 1000+ devices to perform parameter benchmarking is not
practical, but doing this on the network simulator is quite easy.

What You Need
=============
A Mac or Linux computer, or possibly some environment in Windows that provides
POSIX libraries.  But seriously, just get a Mac if you’re going to spend a lot 
of time working with the network simulator.  Mac has a nice GUI, and it’s easy
to develop apps for it, so you can expect that Mac will be the target for any
GUI-driven applications we deploy officially with OpenTag in the future.  Of 
course, everything we do will always support a CLI version that will work with
any type of POSIX system.

Notes
=====
The POSIX C platform is under development at this time.  It is non-functional
at time of writing, and there is a way to go in order to get the POSIX timers
completely functional as simulators for hardware timers, and then to simulate
the NVIC interrupt controller of the ARM Cortex-M3 (which is the default 
platform for OpenTag, and our golden reference).


