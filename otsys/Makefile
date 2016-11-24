COMPILER=gcc

FLAGS=

PROJ = ../
INC_DIR = $(PROJ)/include 
APP_DIR = $(PROJ)/apps/null_posix

#OT_HEADERS := $(OTLIB)/$(wildcard *.h)
#PL_HEADERS := $(PLATFORM)/$(wildcard *.h)
#NOTE: I don't use wildcards in the build strings, because I like to keep the
#      compilations selective.

SYSQUEUE_C = ./sysqueue.c
SYSQUEUE_O = ./sysqueue.o


INCLUDES = -I$(INC_DIR) -I$(APP_DIR) 
FLAGS = -O -D __UNITTEST_SYSQUEUE__

all: sysqueue_out
sysqueue: sysqueue_out


sysqueue_out: sysqueue_o $(SYSQUEUE_O)
	$(COMPILER) $(INCLUDES) -o sysqueue_t.out $(SYSQUEUE_O)

sysqueue_o: $(SYSQUEUE_C)
	$(COMPILER) $(FLAGS) $(INCLUDES) -c $(SYSQUEUE_C)


clean:
	rm -f *.o 
	rm -f *.gch

install: clean