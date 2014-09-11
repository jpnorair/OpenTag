Readme for Protocol Testbed
===========================

What is Protocol Testbed?
Answer: a C program that runs on a POSIX shell for the purpose of stress-testing the protocol layers of OpenTag.  

What protocols does it test?
- M2NP (network layer)
- M2QP (transport layer)
- All ALPs supported by OpenTag


Features
========
- Command line user interface
- Communicates with a single server over an Mpipe connection.  This is implemented at present as a TTY device.


Requirements
============
- POSIX & GNU C libraries
- GCC 4 (older versions might work)
- A POSIX-compliant shell (e.g. sh, csh, bash, etc)


Installation & run
==================
1. cd (PROJECT ROOT)/Apps/Testbed_Protocol
2. make 
3. sudo make install
4. ./tbprotocol


Usage
=====
The "tbprotocol" program will work on stdio with prompts.  Alternatively you can feed it a script file.

To do something, type a command string into the prompt.  Start by typing "help."  Formal documentation will be put up on the wiki (indigresso.com/wiki).


Some Q & A
==========

Q: Can you help me with the installation instructions?
A: Open a shell and enter each line.  Replace "(PROJECT ROOT)" with the root directory path of the OpenTag code tree.  If you don't know what a "shell" is, I recommend that you wait for a major release or get somebody else to help you.

Q: How do I uninstall?
A: sudo make uninstall
A: sudo rm /usr/local/bin/otclient (alternate)

Q: Can I run this on a Mac?
A: Yes, install XCode (it is free and probably already there).

Q: How can I run this on Windows?
A: Get Cygwin or MinGW -- or just get Linux/Mac.

