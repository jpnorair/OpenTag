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

# This is the default app to compile, if one is not supplied
APP ?= demo_xrttest

# If a target is not supplied, then use the local machine
THISMACHINE ?= $(shell uname -srm | sed -e 's/ /-/g')
THISSYSTEM  ?= $(shell uname -s)
TARGET      ?= $(THISMACHINE)

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
VERSION     ?= 0.1.0

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
	#else ifeq ($(THISSYSTEM),CYGWIN_NT-10.0)
	#    PRODUCT := $(APP).exe
	else
		error "THISSYSTEM set to unknown value: $(THISSYSTEM)"
	endif
	
	# 
	CFLAGS      ?= -std=gnu99 -O3 -pthread -fPIC
	LIBMODULES  := OTEAX
	
	
	OT_CC	    := $(CC)
	OT_LIBTOOL  := libtool
	OT_CFLAGS   := $(CFLAGS)
	OT_SFLAGS   := -mcpu=cortex-m0plus -g3 -x assembler-with-cpp --specs=nano.specs -mfloat-abi=soft -mthumb
	OT_DEF      := $(EXT_DEF)
	OT_INC      := -I$(DEFAULT_INC) $(EXT_INC)
	OT_LIB      := -L./../_hbsys/$(TARGET)/lib $(EXT_LIB) -loteax $(EXT_LIBS)
	PLATFORM    := ./platform/posix_c
	
	# TODO put these in the app dir
	BUILDDIR    := build/$(THISMACHINE)
	PRODUCTDIR  := bin/$(THISMACHINE)

else ifeq ($(TARGET),nucleo_LRWAN1)
    # nucleo_LRWAN1 uses STM32L0 (ARM Cortex-M0+) and requires GNU ARM Embedded Toolchain
    # to be setup on the host computer.
    PRODUCT         := $(APP).hex
    
    #Todo 
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
	OT_DEF          := -D... (todo) $(EXT_DEF)
	
	# todo make this have ST libs
	OT_INC      := -I$(TICC_DIR)/include -I$(C2000_WARE) -I$(DEFAULT_INC) -I./../_hbsys/$(OT_PLATFORM)/include $(EXT_INC)
	
	# todo make this have ST libs
	OT_LIB      := -Wl,-Bstatic -L$(TICC_DIR)/lib -L./ $(EXT_LIB) $(EXT_LIBS)
	
else
	error "TARGET set to unknown value: $(TARGET)"
endif

# Export the following variables to the shell: will affect submodules
#OT_BUILDDIR := build/$(TARGET)
#OT_PRODUCTDIR := bin/$(TARGET)

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
SUBMODULES  := apps/$(APP) $(TRG_SUBMODULES) m2 otlib otsys platform/$(OT_PLATFORM) extensions/applets_capi extensions/applets_std
INCDEP      := -I$(DEFAULT_INC)

# Global vars that get exported to sub-makefiles
deps: $(LIBMODULES)
all: $(PRODUCT)
remake: cleaner all
#pkg: deps lib install


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

# Test
#test: $(PRODUCT_LIBS)
#	$(eval MKFILE := $(notdir $@))
#	cd ./$@ && $(MAKE) -f $(MKFILE).mk obj




# TODO BUILD ALL THE OBJECT FILES INTO THE TARGET variant




#Library dependencies (not in otfs sources)
$(LIBMODULES): %: 
	cd ./../$@ && $(MAKE) pkg TARGET=$(OT_PLATFORM)

#libotfs submodules
$(SUBMODULES): %: directories
	$(eval MKFILE := $(notdir $@))
	cd ./$@ && $(MAKE) -f $(MKFILE).mk obj

#Non-File Targets
.PHONY: all lib pkg remake clean cleaner

