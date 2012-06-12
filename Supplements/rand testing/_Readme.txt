Readme for: Random Number generator testing
===========================================

If you are doing cryptography or collision avoidance (OpenTag does both) it is 
good to have a random number generator.  For cryptography in particular, you 
want a really-fucking-good random number generator, or else your crypto system 
can be exploited.

This supplement contains a POSIX C program called "ent" that runs on a POSIX 
shell.  This program is by John Walker of fourmilab.ch (and incidentally the 
founder of AutoCAD), but I have added the part that takes-in hex files.  

As I have found, the entropy of the least-significant 4 bits of a 12bit ADC capture of a floating pin is really high, and suitable as a random number generator.  The onchip temperature sensor is not really good enough.


THE BASICS
==========

Here's how you make "ent":
$ make ent

Here's how you can use it on a hex file.  The number is how many bits you want 
to extract from each hex number in the file, starting from the least-significant
bit (LSB).  The smaller this is, usually the more random.
$ ./ent --hex=4 adc12_floating_pin.txt

Here's how you can use it on a binary file.  You will get the same results as 
above, because "ent" will produce an intermediary binary file if you run it in 
hex mode.
$ ./ent -b adc12_floating_pin_4.bin


ABOUT THE INCLUDED DATA
=======================

There are a bunch of included data files you can test "ent" with:
- ent: (pre-compiled binary)
  This may work if you are running Darwin (OS X) on an x86_64
- adc12_floating_pin.txt: (hex file, extract from .7z)
  300,000 captures from a floating pin
- adc12_tempsensor.txt: (hex file, extract from .7z)
  50,000 captures from temperature sensor
- adc12_floating_pin_1.bin:
  Binary file using 1 LSB from the floating pin captures
- adc12_floating_pin_4.bin:
  Binary file using 4 LSB's from the floating pin captures 

All test data came from an ADC12 peripheral on an STM32F, running at maximum
capture rate, set-up in uncalibrated mode.

