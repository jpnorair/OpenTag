Readme for STM32F0xx
====================
STM32F0xx is *not* a great choice for running DASH7, but it is quite nice for 
some other uses of OpenTag.  The main problems with STM32F0xx are that it does 
not have as good low power support as the STM32L devices do and it is not easy 
to generate the 1024 Hz timebase that DASH7 requires.  That said, the STM32F0xx 
makes quite a good application bridge between a USB client and a device that 
does not support USB, because it is incredibly cheap, it requires no external 
crystal, and the IC packages can get very small.  This is the main usage for 
STM32F0xx with OpenTag.

More on Low Power Operation
===========================
It is possible to improve the power usage of STM32F0 through a system of 
elaborate hacks, but these are not implemented currently.  Presently, STM32F0
builds only use regular sleep, and the current draw can be expected to be 
between 1-5mA (sleep-run).  If you want to have good low-power features, use
the STM32L0 or L1 instead.  These can sleep using only ~1uA.  Consequently,
even some F1 devices (ones with TIM9/10/11) can sleep down to ~4uA.  So F0 is 
the worst power user of the STM32F0/F1/L0/L1 lines.


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
Green   - This file is for STM32F0 only.
