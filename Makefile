# Copyright 2020, JP Norair
#
# Licensed under the OpenTag License, Version 1.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.indigresso.com/wiki/doku.php?id=opentag:license_1_0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


# USAGE NOTE:
# The two primary things to supply via command line are:
# APP="name-of-app"
# TARGET="name-of-board"



# Get the information of the local machine
THISMACHINE ?= $(shell uname -srm | sed -e 's/ /-/g')
THISSYSTEM  ?= $(shell uname -s)

# This is the default app to compile, if one is not supplied
APP ?= demo_blink

# Target to compile-for, if one is not supplied
TARGET ?= $(THISMACHINE)

# These should be the compiler & linker for the local machine
CC := gcc
LD := ld

# Try to get git HEAD commit value
ifneq ($(INSTALLER_HEAD),)
    GITHEAD := $(INSTALLER_HEAD)
else
    GITHEAD := $(shell git rev-parse --short HEAD)
endif

EXT_DEF     ?= 
EXT_INC     ?= 
EXT_LIB     ?=
EXT_LIBS    ?= 
VERSION     ?= 3.0.$(GITHEAD)

DEFAULT_INC := ./include
SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

# Conditional Settings per Target
ifeq ($(TARGET),$(THISMACHINE))
	ifeq ($(THISSYSTEM),Darwin)
	    PRODUCT := $(APP)
	else ifeq ($(THISSYSTEM),Linux)
		PRODUCT := $(APP)
	else
		error "THISSYSTEM set to unsupported value: $(THISSYSTEM)"
	endif
	
	# 
	CFLAGS          ?= -std=gnu99 -O3 -pthread -fPIC
	LIBMODULES      := OTEAX
	
	OT_PLATFORM     := posix_c
	OT_MCU          := posix_c
	
	OT_BUILDDIR     := build/$(THISMACHINE)
	OT_PRODUCTDIR   := bin/$(THISMACHINE)
	
	OT_CC	        := $(CC)
	OT_LIBTOOL      := libtool
	OT_CFLAGS       := $(CFLAGS)
	OT_SFLAGS       := 
	OT_DEF          := $(EXT_DEF)
	OT_INC          := -I$(DEFAULT_INC) $(EXT_INC)
	OT_LIB          := -L./../_hbsys/$(TARGET)/lib $(EXT_LIB) -loteax $(EXT_LIBS)
	
else ifeq ($(TARGET),discovery_LoRa)
    # discovery_LoRa uses STM32L0 (ARM Cortex-M0+) and requires GNU ARM Embedded Toolchain
    # to be setup on the host computer.
    PRODUCT         := $(APP).elf
    
    #Todo 
	LIBMODULES      := OTEAX
	
	TRG_SUBMODULES  := io/sx127x
	
	# Todo get these from board
	OT_PLATFORM     := stm32l0xx
	OT_MCU          := stm32l072
	
	OT_BUILDDIR     := build/$(TARGET)
	OT_PRODUCTDIR   := bin/$(TARGET)
	
	OT_CC	        := arm-none-eabi-gcc
	OT_LIBTOOL      := arm-none-eabi-ar
	OT_CFLAGS       := --c99 -O2 ..... (todo)
	OT_SFLAGS       := -mcpu=cortex-m0plus -g3 -x assembler-with-cpp --specs=nano.specs -mfloat-abi=soft -mthumb
	OT_DEF          := -D... (todo) $(EXT_DEF)
	
	# todo make this have ST libs
	OT_INC          := -I$(TICC_DIR)/include -I$(C2000_WARE) -I$(DEFAULT_INC) -I./../_hbsys/$(OT_PLATFORM)/include $(EXT_INC)
	
	# todo make this have ST libs
	OT_LIB          := -Wl,-Bstatic -L$(TICC_DIR)/lib -L./ $(EXT_LIB) $(EXT_LIBS)
	

else ifeq ($(TARGET),nucleo_LRWAN1)
    # nucleo_LRWAN1 uses STM32L0 (ARM Cortex-M0+) and requires GNU ARM Embedded Toolchain
    # to be setup on the host computer.
    PRODUCT         := $(APP).elf
    
    # Todo 
	LIBMODULES      := OTEAX
	
	TRG_SUBMODULES  := io/sx127x
	
	# Todo get these from board
	OT_PLATFORM     := stm32l0xx
	OT_MCU          := stm32l073
	
	OT_BUILDDIR     := build/$(TARGET)
	OT_PRODUCTDIR   := bin/$(TARGET)
	
	OT_CC	        := arm-none-eabi-gcc
	OT_LIBTOOL      := arm-none-eabi-ar
	OT_CFLAGS       := --c99 -O2 ..... (todo)
	OT_SFLAGS       := -mcpu=cortex-m0plus -g3 -x assembler-with-cpp --specs=nano.specs -mfloat-abi=soft -mthumb
	OT_DEF          := -D... (todo) $(EXT_DEF)
	
	# todo make this have ST libs
	OT_INC      := -I$(TICC_DIR)/include -I$(C2000_WARE) -I$(DEFAULT_INC) -I./../_hbsys/$(OT_PLATFORM)/include $(EXT_INC)
	
	# todo make this have ST libs
	OT_LIB      := -Wl,-Bstatic -L$(TICC_DIR)/lib -L./ $(EXT_LIB) $(EXT_LIBS)
	
else
	error "TARGET set to unsupported value: $(TARGET)"
endif

# Export the following variables to the shell: will affect submodules
export OT_BUILDDIR
export OT_PRODUCTDIR
export OT_CC
export OT_CFLAGS
export OT_SFLAGS
export OT_DEF
export OT_INC
export OT_LIB
export OT_PLATFORM
export OT_MCU

# Derived Parameters
SUBMODULES  := m2 otlib otsys alp apps/$(APP) $(TRG_SUBMODULES) platform/$(OT_PLATFORM) 
INCDEP      := -I$(DEFAULT_INC)

# Global vars that get exported to sub-makefiles
deps: $(LIBMODULES)
all: $(OT_PRODUCTDIR)/$(PRODUCT)
hex: $(OT_PRODUCTDIR)/$(APP).hex
remake: cleaner all


# install: 
# 	@rm -rf $(PACKAGEDIR)
# 	@mkdir -p $(PACKAGEDIR)
# 	@cp -R ./$(PRODUCTDIR)/* ./$(PACKAGEDIR)/
# 	@rm -f $(PACKAGEDIR)/../libotfs
# 	@ln -s libotfs.$(VERSION) ./$(PACKAGEDIR)/../libotfs
# 	cd ../_hbsys && $(MAKE) sys_install INS_MACHINE=$(TARGET) INS_PKGNAME=libotfs
	
directories:
	@mkdir -p $(OT_PRODUCTDIR)
	@mkdir -p $(OT_BUILDDIR)

#Clean only Objects
clean:
	@$(RM) -rf $(BUILDDIR)
	@$(RM) -rf $(PRODUCTDIR)

#Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) -rf bin
	@$(RM) -rf build


# This allows conversion of the elf into hex
$(OT_PRODUCTDIR)/$(APP).hex: $(OT_PRODUCTDIR)/$(PRODUCT)


# TODO BUILD ALL THE OBJECT FILES INTO THE TARGET variant
#$(OT_PRODUCTDIR)/$(PRODUCT)


#Library dependencies (not in otfs sources)
$(LIBMODULES): %: 
	cd ./../$@ && $(MAKE) pkg TARGET=$(OT_PLATFORM)

#libotfs submodules
$(SUBMODULES): %: directories
	$(eval MKFILE := $(notdir $@))
	cd ./$@ && $(MAKE) -f $(MKFILE).mk obj

#Non-File Targets
.PHONY: all lib pkg remake clean cleaner

