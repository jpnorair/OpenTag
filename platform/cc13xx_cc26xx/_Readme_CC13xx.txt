Readme for CC13xx
=====================
CC1310 and other CC13xx parts are maybe the best options for OpenTag, at least
at this time of writing in late 2016.  

- Integrated DC-DC enables it to run straight from a Li-Ion, very efficiently.
- Very good radio front end with lots of possibilities.
- Very low-power Cortex M3 core
- Very efficient and useful driver library (unlike, ahem, ST’s useless libs)

The only let-down is that it requires an external USB controller, but not all 
devices need USB, and it is relatively cheap (and small) to add it if you do.


Notes
=====================



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
