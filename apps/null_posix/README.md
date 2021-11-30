About null_posix:
null_posix is a null application for OpenTag, for the purpose of compiling 
certain components of OpenTag onto POSIX clients.


--------------------------------------------------------------------------------
Usage Notes:
See the wiki (above) for instructions on how you can use the demo.

Compiler Constants
==================
This project's makefile or IDE configuration should pass several constants into
the compiler.  These constants are required for basic configuration.

__LITTLE_ENDIAN__   (mandatory)     Endian must be configured properly. 
__BIG_ENDIAN__                      All MSP430F5 cores are __LITTLE_ENDIAN__
    
__GCC__             (mandatory)     Compiler must be configured properly.
__CLANG__                           

__DEBUG__           (optional)      enable some debugging features in the code
