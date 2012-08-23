About BinTex:
http://www.indigresso.com/wiki/opentag:otlibext:bintex

BinTex is a very simple markup language for notating binary datastreams in 
human-readable text.  It might not even be considered a markup-language, but,
in any case, it is a nice way to write data.

About the code:
There are two variants of the code.

1. bintex_ot
This variant must be made (i.e. make ...) with a makefile including a POSIX
app for OpenTag, such as null_posix.  It uses the Queue Module, and the Queue
Module uses platform_memcpy(), so there is shared code.

2. bintex
This variant includes the queue module written directly inside bintex.  So you
can drop this .c/.h pair of files into whatever standard-C project you have.

