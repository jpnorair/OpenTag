COMPILER=gcc

PROJ = ../..
OTLIB = $(PROJ)/OTlib
APP = $(PROJ)/Apps/Testbed_Protocol/Code
PLATFORM = $(PROJ)/Platforms/POSIX

#OT_HEADERS := $(OTLIB)/$(wildcard *.h)
#PL_HEADERS := $(PLATFORM)/$(wildcard *.h)

#NOTE: I don't use wildcards in the build strings, because I like to keep the
#      compilations selective.

TBPROT_AP_C =   $(APP)/input_parser.c $(APP)/tbprot_main.c 

TBPROT_PL_C =   $(PLATFORM)/platform_POSIX.c $(PLATFORM)/veelite_core_X2.c 
                    
TBPROT_OT_C =   $(OTLIB)/auth.c $(OTLIB)/buffers.c $(OTLIB)/crc16.c $(OTLIB)/external.c \
                $(OTLIB)/M2_network.c $(OTLIB)/M2QP.c $(OTLIB)/OT_utils.c \
                $(OTLIB)/queue.c $(OTLIB)/session.c $(OTLIB)/veelite.c 
                    
TBPROT_O =      input_parser.o tbprot_main.o \
                platform_POSIX.o veelite_core_X2.o \
                auth.o buffers.o crc16.o external.o M2_network.o M2QP.o \
                OT_utils.o queue.o session.o veelite.o


INCLUDES = -I$(OTLIB) -I$(PLATFORM) -I$(APP)
FLAGS = -O

all: tbprot_out
tbprotocol: tbprot_out


tbprot_out: tbprot_o $(TBPROT_O)
	$(COMPILER) $(INCLUDES) -o tbprotocol $(TBPROT_O)

tbprot_o: $(TBPROT_AP_C) $(TBPROT_PL_C) $(TBPROT_OT_C)
	$(COMPILER) $(FLAGS) $(INCLUDES) -c $(TBPROT_AP_C) $(TBPROT_PL_C) $(TBPROT_OT_C)


clean:
	rm -f *.o 
	rm -f *.gch

install: clean