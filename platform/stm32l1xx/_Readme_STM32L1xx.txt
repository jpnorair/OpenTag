Readme for STM32L1xx
====================
STM32L1xx is an excellent choice for OpenTag.  The sleep modes are quite
low-power, the run modes have really good DMIPS/mW, there are a lot of
variants, and the price is low.  Thus, the STM32L1xx is the de-facto choice for
OpenTag, and it has the best support.  Every feature for OpenTag is available
on STM32L1, with no major caveats.

Support for Different STM32L Densities
======================================
Medium Density is the defacto type supported.  This includes STM32L1xx devices
Below 256KB Flash.  Medium Density Plus and High Density devices have more 
Flash, more RAM, and often a few extra peripherals.  These are not supported at
the moment, but they are very easy to support.

Color Coding of Files
=====================
If you are lucky enough to have a Mac and you have kept the .DS_Store file in
this directory, each of the files in this directory has a color "label" on it.
This color corresponds to how easy it is to port this to another platform.  If
you cannot see the color labels, the same information is written in the top
comments of each of the files.

Grey    - Porting to any other Cortex-M device is trivial
Purple  - Porting to any other STM32 device is trivial
Blue    - This file is portable across some STM32 family lines, but not all
Green   - This file is for STM32L1 only.
