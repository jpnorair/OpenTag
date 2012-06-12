Readme for Platforms Directory
07 Feb 2012


DEFINITION OF PLATFORMS
=======================

The Platforms Directory should be located at the path:
(PROJECT_ROOT)/OTplatform

This directory should contain the following types of files, ideally grouped 
into subdirectories based on the platform-type:
1. General platform/FW libraries for computing platforms (often MCUs)
2. OpenTag-specific program code that is platform-dependent

Therefore, in OpenTag language a "platform" defines a computing platform.  A
"computing platform" implies a combination of CPU, working memory, storage 
memory, and intrinsic I/O mechanisms.  In most cases where OpenTag is deployed, 
a "computing platform" refers to a microcontroller (MCU), although it could 
also be a higher-level construct, such as POSIX.


NAMING CONVENTIONS
==================

By convention, general libraries are in subdirectories preceded with a "~"
character.  OpenTag-specific program code subdirectories are not preceeded with
any character.  The names given to these subdirectories are the platform names
themselves.

The platform names are used as subdirectories in other OpenTag directories, as
well, namely in IDE project folders and the /Board directory.  In the former,
the IDE should have a separate project for each platform it supports.  In the
latter, all boards that use the respective platform should have a board header
file stored in a subdirectory named the same as this platform.


AVAILABLE PLATFORMS
===================

At the time of writing, OpenTag supports three platforms maturely, all of which
are microcontrollers with no other prerequisites (i.e. no OS required).

Platform Name   | Description
----------------|--------------------------------------------------------------
CC430           | TI CC430 series RF-MCUs
STM32F10x       | STMicro STM32F10x series MCUs
STM32L1xx       | STMicro STM32L1xx series MCUs

Certain testbeds and simulators may also support POSIX as a platform, although
it does not offer complete support of the OpenTag stack at this time.  These
testbeds and simulators are limited to partial usage of the OpenTag stack.
