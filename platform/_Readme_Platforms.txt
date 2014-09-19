Readme for Platform Directory
12 Sept 2014


DEFINITION OF PLATFORMS
=======================
The Platforms Directory should be located at the path: OpenTag/platform

This directory should contain OpenTag-specific program code that is platform-
dependent, and it should be arranged in one directory per platform.

Therefore, in OpenTag language a "platform" defines a computing platform.  A
"computing platform" implies a combination of CPU, working memory, storage 
memory, and intrinsic I/O mechanisms.  In most cases where OpenTag is deployed, 
a "computing platform" refers to a microcontroller (MCU), although it could 
also be a higher-level construct, such as POSIX.


NAMING CONVENTIONS
==================
The platform names are used as subdirectories in other OpenTag directories, as
well, namely in IDE project folders and the /include/board directory.  In the 
former, the IDE should have a separate project for each platform it supports.  
In the latter, all boards that use the respective platform should have a board 
header file stored in a subdirectory named the same as this platform.


AVAILABLE PLATFORMS
===================
At the time of writing, OpenTag supports the platforms in the table below. 
For microcontrollers, there are no other prerequistes or libraries needed, just 
what is provided in the OpenTag source tree.  For the POSIX-STDC build, you 
need a POSIX system with STD C libs.

Platform Name  | Description                    | Latest stable version
---------------+--------------------------------+------------------------------
STM32L1xx      | STMicro STM32L1xx series MCUs  | 2.x
STM32L0xx      | STMicro STM32L0xx series MCUs  | 2.x (experimental)
STM32F0xx      | STMicro STM32F0xx series MCUs  | 2.x (experimental)
STM32F10x      | STMicro STM32F10x series MCUs  | 2.x (experimental)
POSIX STD-C    | POSIX with STD C               | 2.x (experimental)
---------------+--------------------------------+------------------------------
MSP430F5       | TI MSP430F5 series MCUs        | 0.4.2
CC430          | TI CC430 series RF-MCUs        | 0.4.2

Certain, special-purpose testbeds and simulators may also support POSIX as a 
platform, although they may not offer complete simulation of the OpenTag stack 
at this time.  Check with the testbed/simulator documentation.
