# Readme for STM32L4xx

STM32L4xx is an excellent choice for OpenTag.  The sleep modes are extremely low-power, the run modes have really good DMIPS/mW, and the price is low.  Moreover, it has a strong CPU with DSP instructions, so it is very useful for doing sophisticated error correction in firmware.

## STM32L4 vs. L1 vs. L0

STM32L4 is more optimized for OpenTag than L0 or L1 is.  The STM32L422KB is the best choice, because it combines enough Flash (128KB) to do A/B sector bootloading, plus 40KB SRAM, which is more than enough.  Unlike L0 and L1, L4 devices do not contain EEPROM, but they do have a lot more SRAM.  OpenTag Filesystem contents can be stored in SRAM and backed-up on Flash at shutdown events -- this is actually preferable in most application to using EEPROM.  On the other hand, if your application needs EEPROM, then you should consider the L0 or L1 devices.


## Color Coding of Files

If you have a Mac and you have kept the .DS_Store file in this directory, each of the files in this directory has a color "label" on it.  This color corresponds to how easy it is to port this to another platform.  If you cannot see the color labels, the same information should be written in the top comments of each of the files.

* Grey    - Porting to any other Cortex-M device is trivial
* Purple  - Porting to any other STM32 device is trivial
* Blue    - This file is portable across some STM32 family lines, but not all
* Green   - This file is for STM32L1 only.
