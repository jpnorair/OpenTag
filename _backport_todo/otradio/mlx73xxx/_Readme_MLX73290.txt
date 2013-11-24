MLX73xxx Radio
==============
The MLX73xxx series of RF transceivers are multi-band, sub 1GHz devices that
work with DASH7 & OpenTag.  They have an SPI-based interface, with 4 GPIO IRQ
sources.  They also have a lot of internal features that enable optimized
implementations, such as polling timers and rx termination timers.  Regular
OpenTag implementations do not use these features, but they are able to be used
with optimized builds (in other words, they fit fine into DASH7).


Todos
=====
1. Improve SPI interface with STM32, so that it uses the hardware DMA at
maximum speed. (mlx73xx_STM32.c)

2. Improve radio features to make usage of RSSI (not currently characterized)

3. Improve the state-change sequences, especially before TX, that were 
developed with the prototype HW.

4. Remove the sequences that write blocks of default data, especially to Bank0.
Additionally, remove the manual configuration of the PA via Bank0.  These 
implementations were made with the Prototype HW in mind.


Other Features
==============
The MLX73290 includes dual RF Front Ends and an LF front end.  A lot of cool
things can be done with these features.
