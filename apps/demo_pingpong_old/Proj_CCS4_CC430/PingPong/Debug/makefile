################################################################################
# Automatically-generated file. Do not edit!
################################################################################

EMPTY := 
SPACE := $(EMPTY) $(EMPTY)

ORDERED_OBJS += \
$(GEN_CMDS_QUOTED) \
"./App/~demo_pingpong_CC430.obj" \
"./CC430/cc430_lib/cc430_ucs.obj" \
"./CC430/cc430_lib/cc430_tim.obj" \
"./CC430/cc430_lib/cc430_rtc.obj" \
"./CC430/cc430_lib/cc430_rf.obj" \
"./CC430/cc430_lib/cc430_pmm.obj" \
"./CC430/cc430_lib/cc430_gpio.obj" \
"./CC430/cc430_lib/cc430_flash.obj" \
"./CC430/cc430_lib/cc430_dma.obj" \
"./CC430/cc430_lib/cc430_crc.obj" \
"./CC430/veelite_core_X2_CC430.obj" \
"./CC430/radio_CC430.obj" \
"./CC430/platform_CC430.obj" \
"./CC430/mpipe_CC430.obj" \
"./OTlib/veelite.obj" \
"./OTlib/system.obj" \
"./OTlib/session.obj" \
"./OTlib/queue.obj" \
"./OTlib/ndef.obj" \
"./OTlib/external.obj" \
"./OTlib/encode_M2.obj" \
"./OTlib/crc16.obj" \
"./OTlib/buffers.obj" \
"./OTlib/auth.obj" \
"./OTlib/alp_sensor.obj" \
"./OTlib/alp_security.obj" \
"./OTlib/alp_main.obj" \
"./OTlib/alp_logger.obj" \
"./OTlib/alp_filedata.obj" \
"./OTlib/alp_dashforth.obj" \
"./OTlib/alp_api_server.obj" \
"./OTlib/alp_api_client.obj" \
"./OTlib/OT_utils.obj" \
"./OTlib/OTAPI_logger.obj" \
"./OTlib/M2_network.obj" \
"./OTlib/M2QP.obj" \
-l"libc.a" \
"../lnk_cc430_4+32.cmd" \

-include ../makefile.init

RM := DEL /F

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include OTlib/subdir.mk
-include CC430/subdir.mk
-include CC430/cc430_lib/subdir.mk
-include App/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: PingPong.out

# Tool invocations
PingPong.out: $(GEN_CMDS) $(CMD_SRCS) $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Linker'
	$(shell echo --silicon_version=mspx > ccsLinker.opt)
	$(shell echo -g >> ccsLinker.opt)
	$(shell echo -O0 >> ccsLinker.opt)
	$(shell echo --opt_for_speed=3 >> ccsLinker.opt)
	$(shell echo --diag_warning=225 >> ccsLinker.opt)
	$(shell echo --small_enum >> ccsLinker.opt)
	$(shell echo --plain_char=unsigned >> ccsLinker.opt)
	$(shell echo --printf_support=minimal >> ccsLinker.opt)
	$(shell echo -z >> ccsLinker.opt)
	$(shell echo -m"PingPong.map" >> ccsLinker.opt)
	$(shell echo --stack_size=80 >> ccsLinker.opt)
	$(shell echo --heap_size=80 >> ccsLinker.opt)
	$(shell echo --use_hw_mpy=F5 >> ccsLinker.opt)
	$(shell echo --warn_sections >> ccsLinker.opt)
	$(shell echo -i"C:/Program Files/Texas Instruments/ccsv4/msp430/include" -i"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/lib" -i"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/include" >> ccsLinker.opt)
	$(shell echo --reread_libs >> ccsLinker.opt)
	$(shell echo --rom_model >> ccsLinker.opt)
	$(shell type ccsObjs.opt >> ccsLinker.opt)
	"C:/Program Files/Texas Instruments/ccsv4/tools/compiler/msp430/bin/cl430" -@ccsLinker.opt -o "PingPong.out"
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(MSP430_EXECUTABLE_OUTPUTS__QTD) "PingPong.out"
	-$(RM) ".\OTlib\M2QP.obj" ".\OTlib\M2_network.obj" ".\OTlib\OTAPI_logger.obj" ".\OTlib\OT_utils.obj" ".\OTlib\alp_api_client.obj" ".\OTlib\alp_api_server.obj" ".\OTlib\alp_dashforth.obj" ".\OTlib\alp_filedata.obj" ".\OTlib\alp_logger.obj" ".\OTlib\alp_main.obj" ".\OTlib\alp_security.obj" ".\OTlib\alp_sensor.obj" ".\OTlib\auth.obj" ".\OTlib\buffers.obj" ".\OTlib\crc16.obj" ".\OTlib\encode_M2.obj" ".\OTlib\external.obj" ".\OTlib\ndef.obj" ".\OTlib\queue.obj" ".\OTlib\session.obj" ".\OTlib\system.obj" ".\OTlib\veelite.obj" ".\CC430\mpipe_CC430.obj" ".\CC430\platform_CC430.obj" ".\CC430\radio_CC430.obj" ".\CC430\veelite_core_X2_CC430.obj" ".\CC430\cc430_lib\cc430_crc.obj" ".\CC430\cc430_lib\cc430_dma.obj" ".\CC430\cc430_lib\cc430_flash.obj" ".\CC430\cc430_lib\cc430_gpio.obj" ".\CC430\cc430_lib\cc430_pmm.obj" ".\CC430\cc430_lib\cc430_rf.obj" ".\CC430\cc430_lib\cc430_rtc.obj" ".\CC430\cc430_lib\cc430_tim.obj" ".\CC430\cc430_lib\cc430_ucs.obj" ".\App\~demo_pingpong_CC430.obj" 
	-$(RM) ".\OTlib\M2QP.pp" ".\OTlib\M2_network.pp" ".\OTlib\OTAPI_logger.pp" ".\OTlib\OT_utils.pp" ".\OTlib\alp_api_client.pp" ".\OTlib\alp_api_server.pp" ".\OTlib\alp_dashforth.pp" ".\OTlib\alp_filedata.pp" ".\OTlib\alp_logger.pp" ".\OTlib\alp_main.pp" ".\OTlib\alp_security.pp" ".\OTlib\alp_sensor.pp" ".\OTlib\auth.pp" ".\OTlib\buffers.pp" ".\OTlib\crc16.pp" ".\OTlib\encode_M2.pp" ".\OTlib\external.pp" ".\OTlib\ndef.pp" ".\OTlib\queue.pp" ".\OTlib\session.pp" ".\OTlib\system.pp" ".\OTlib\veelite.pp" ".\CC430\mpipe_CC430.pp" ".\CC430\platform_CC430.pp" ".\CC430\radio_CC430.pp" ".\CC430\veelite_core_X2_CC430.pp" ".\CC430\cc430_lib\cc430_crc.pp" ".\CC430\cc430_lib\cc430_dma.pp" ".\CC430\cc430_lib\cc430_flash.pp" ".\CC430\cc430_lib\cc430_gpio.pp" ".\CC430\cc430_lib\cc430_pmm.pp" ".\CC430\cc430_lib\cc430_rf.pp" ".\CC430\cc430_lib\cc430_rtc.pp" ".\CC430\cc430_lib\cc430_tim.pp" ".\CC430\cc430_lib\cc430_ucs.pp" ".\App\~demo_pingpong_CC430.pp" 
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
