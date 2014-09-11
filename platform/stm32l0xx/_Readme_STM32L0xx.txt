Readme for STM32L0xx
====================
STM32L0xx is an excellent choice for OpenTag.  The sleep modes are extremely
low-power, the run modes have really good DMIPS/mW, and the price is low.  
STM32L0xx is not as mature as the STM32L1xx, it does not have a variant 
including 4KB EEPROM, and it is not quite as fast at doing Reed-Solomon error
correction (RS is only relevant to H-Builder version of OpenTag).  Otherwise,
it is the best chip in the universe for OpenTag.  The very best STM32L0 devices 
for OpenTag are the STM32L062 parts because they have an internal AES 
coprocessor.  This saves 9-22KB, depending on how you optimize OTEAX!  They 
also have USB, which is a nice bonus.  

STM32L0 vs. L1 vs. F0
=====================
STM32L0 is more optimized for OpenTag than L1 is, but at this time of writing 
it only comes in 64KB Flash + 8KB SRAM + 2KB EEPROM maximum resources.  If this
is enough for you, then use L0.  For DASH7 endpoints, this is often quite 
sufficient unless you want to run heavy applications like A/B over-the-air
firmware reprogramming or CoAP.

STM32L0 is vastly superior to F0 when it comes to running OpenTag.  Seriously,
if you are deciding between L0 and F0, choose L0 unless you have something very
specific in mind.  F0 is dirt-cheap, which is nice, but L0 is only a tiny bit 
more expensive and in practice it runs much faster with much less power.


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
