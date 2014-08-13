COMPILER=gcc

PROJ = ../..
OTLIB = $(PROJ)/otlib

#OT_HEADERS := $(OTLIB)/$(wildcard *.h)
#PL_HEADERS := $(PLATFORM)/$(wildcard *.h)

#NOTE: I don't use wildcards in the build strings, because I like to keep the
#      compilations selective.

TBCRC_AP_C =   crc16_test.c
                    
TBCRC_OT_C =   $(OTLIB)/crc16.c $(OTLIB)/queue.c 
                    
TBCRC_O =      crc16.o queue.o


INCLUDES = -I$(OTLIB)
FLAGS = -O

all: tbcrc_out
tbcrcocol: tbcrc_out


tbcrc_out: tbcrc_o $(TBCRC_O)
	$(COMPILER) $(INCLUDES) -o tbcrcocol $(TBCRC_O)

tbcrc_o: $(TBCRC_AP_C) $(TBCRC_OT_C)
	$(COMPILER) $(FLAGS) $(INCLUDES) -c $(TBCRC_AP_C) $(TBCRC_OT_C)


clean:
	rm -f *.o 
	rm -f *.gch

install: clean