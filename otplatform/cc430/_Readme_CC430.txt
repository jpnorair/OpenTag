CC430 Platform subdirectory:
This platform supports all CC430s, although OpenTag is only guaranteed to work 
with the CC430F6137 or CC430F5137, because those are the only ones I've tested.
If you are using other variants of the CC430, you may need to change some files 
here and there.

The Texas Instruments CC430 is an SoC (single die) that includes a 5-series 
MSP430 and a CC1101 radio core.  Its sensitivity is not great compared to other 
parts, but it is the smallest, lowest-cost solution available.  Additionally,
the power usage is quite low for low-duty-cycle applications because the CC430 
radio core is very intelligent and can execute its channel scan cycles very 
quickly.  The CC430 is an excellent choice for "tags" or remote wireless 
sensors.