# Blink

Just a test project that blinks LEDs via a kernel task.  It is intended for 
the purpose of testing the platform code and scheduler related to core RTOS features.


## This Application Recommends

Cortex-M: ≥ 32KB Flash, ≥ 2KB SRAM, ≥ 8MHz


## Supported Boards & Platforms

[unspecified in readme]

## Supported Project IDEs

* Makefile
* STM32CubeIDE


## Wiki Links & More Information

http://www.indigresso.com/wiki/doku.php?id=opentag:apps:blink
http://wiki.indigresso.com


## Usage Notes

Also see the wiki (above) for instructions on how you can use the demo.

#### Compiler Constants:

This project's makefile or IDE configuration should pass several constants into
the compiler.  These constants are required for basic configuration.

```
__[partnum]__       (mandatory)     The part number of the platform being used.
                                    E.g. for MSP430F5529 = __MSP430F5529__, for
                                    STM32L151C8 = __STM32L151C8__.__
    
__GCC__             (mandatory)     Compiler must be configured properly.  
                                    GCC is the supported compiler.

__RESTRICT=int      (optional)      Restricts flash allocation to lower [int]
                                    bytes.  Helpful for using low-cost tools
                                    that have code restrictions.

__LARGE_MEMORY__    (optional)      Ignored on Cortex-M.  Enables 20 bit memory
                                    addressing on MSP430X2 devices, which does
                                    incur a performance penalty.  Do not use it
                                    unless you absolutely need it.

__DEBUG__           (optional)      Enable debugging/prototyping features in
__PROTO__                           the code, or enable release features.  See
__RELEASE__                         app documentation for exactly what these 
                                    features enable/disable.
```
