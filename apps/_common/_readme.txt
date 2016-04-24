About apps/_common
==================
This folder contains some configuration and startup files that may be used by
many of the demos and apps.  

These files are usually platform-independent.  If they are not, the 
documentation block at the top of the file, and the file name itself, should 
make it quite clear that it is platform-dependent.

The most important things in here are the filesystem (fs) setup files, named
as follows, where “xxx” is some name of a common FS distribution:
 - fs_xxx_config.h : the veelite filesystem configuration
 - fs_xxx_startup.c : the data in the veelite filesystem at startup

