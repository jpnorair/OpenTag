################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv5/tools/compiler/msp430

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./App_Code/main.obj" \
"./OTkernel/~Native/system.obj" \
"./OTlib/veelite.obj" \
"./OTlib/session.obj" \
"./OTlib/queue.obj" \
"./OTlib/ndef.obj" \
"./OTlib/external.obj" \
"./OTlib/encode_M2.obj" \
"./OTlib/crypto_aes128.obj" \
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
"./OTplatform/CC430/cc430_lib/cc430_ucs.obj" \
"./OTplatform/CC430/cc430_lib/cc430_tim.obj" \
"./OTplatform/CC430/cc430_lib/cc430_rtc.obj" \
"./OTplatform/CC430/cc430_lib/cc430_rf.obj" \
"./OTplatform/CC430/cc430_lib/cc430_pmm.obj" \
"./OTplatform/CC430/cc430_lib/cc430_gpio.obj" \
"./OTplatform/CC430/cc430_lib/cc430_flash.obj" \
"./OTplatform/CC430/cc430_lib/cc430_dma.obj" \
"./OTplatform/CC430/cc430_lib/cc430_crc.obj" \
"./OTplatform/CC430/veelite_core_X2_CC430.obj" \
"./OTplatform/CC430/platform_CC430.obj" \
"./OTplatform/CC430/mpipe_CC430.obj" \
"./OTradio/CC430/radio_CC430.obj" \
-l"libc.a" \
"../lnk_cc430_4+32.cmd" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include OTradio/CC430/subdir_vars.mk
-include OTplatform/CC430/subdir_vars.mk
-include OTplatform/CC430/cc430_lib/subdir_vars.mk
-include OTlib/subdir_vars.mk
-include OTkernel/~Native/subdir_vars.mk
-include App_Code/subdir_vars.mk
-include subdir_rules.mk
-include OTradio/CC430/subdir_rules.mk
-include OTplatform/CC430/subdir_rules.mk
-include OTplatform/CC430/cc430_lib/subdir_rules.mk
-include OTlib/subdir_rules.mk
-include OTkernel/~Native/subdir_rules.mk
-include App_Code/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: CC430.out

# Tool invocations
CC430.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: MSP430 Linker'
	"C:/ti/ccsv5/tools/compiler/msp430/bin/cl430" -vmspx --abi=coffabi --code_model=small --data_model=small -O1 --opt_for_speed=2 -g --define=__CCSv5__ --define=DEBUG_ON --define=__CC430F5137__ --diag_warning=225 --display_error_number --silicon_errata=CPU18 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --small_enum --plain_char=unsigned --single_inline --printf_support=minimal -z -m"CC430.map" --stack_size=80 --heap_size=80 --use_hw_mpy=F5 -i"C:/ti/ccsv5/ccs_base/msp430/include" -i"C:/ti/ccsv5/tools/compiler/msp430/lib" -i"C:/ti/ccsv5/tools/compiler/msp430/include" --reread_libs --warn_sections --rom_model -o "CC430.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(MSP430_EXECUTABLE_OUTPUTS__QUOTED) "CC430.out"
	-$(RM) "OTradio\CC430\radio_CC430.pp" "OTplatform\CC430\mpipe_CC430.pp" "OTplatform\CC430\platform_CC430.pp" "OTplatform\CC430\veelite_core_X2_CC430.pp" "OTplatform\CC430\cc430_lib\cc430_crc.pp" "OTplatform\CC430\cc430_lib\cc430_dma.pp" "OTplatform\CC430\cc430_lib\cc430_flash.pp" "OTplatform\CC430\cc430_lib\cc430_gpio.pp" "OTplatform\CC430\cc430_lib\cc430_pmm.pp" "OTplatform\CC430\cc430_lib\cc430_rf.pp" "OTplatform\CC430\cc430_lib\cc430_rtc.pp" "OTplatform\CC430\cc430_lib\cc430_tim.pp" "OTplatform\CC430\cc430_lib\cc430_ucs.pp" "OTlib\M2QP.pp" "OTlib\M2_network.pp" "OTlib\OTAPI_logger.pp" "OTlib\OT_utils.pp" "OTlib\alp_api_client.pp" "OTlib\alp_api_server.pp" "OTlib\alp_dashforth.pp" "OTlib\alp_filedata.pp" "OTlib\alp_logger.pp" "OTlib\alp_main.pp" "OTlib\alp_security.pp" "OTlib\alp_sensor.pp" "OTlib\auth.pp" "OTlib\buffers.pp" "OTlib\crc16.pp" "OTlib\crypto_aes128.pp" "OTlib\encode_M2.pp" "OTlib\external.pp" "OTlib\ndef.pp" "OTlib\queue.pp" "OTlib\session.pp" "OTlib\veelite.pp" "OTkernel\~Native\system.pp" 
	-$(RM) "App_Code\main.pp" 
	-$(RM) "OTradio\CC430\radio_CC430.obj" "OTplatform\CC430\mpipe_CC430.obj" "OTplatform\CC430\platform_CC430.obj" "OTplatform\CC430\veelite_core_X2_CC430.obj" "OTplatform\CC430\cc430_lib\cc430_crc.obj" "OTplatform\CC430\cc430_lib\cc430_dma.obj" "OTplatform\CC430\cc430_lib\cc430_flash.obj" "OTplatform\CC430\cc430_lib\cc430_gpio.obj" "OTplatform\CC430\cc430_lib\cc430_pmm.obj" "OTplatform\CC430\cc430_lib\cc430_rf.obj" "OTplatform\CC430\cc430_lib\cc430_rtc.obj" "OTplatform\CC430\cc430_lib\cc430_tim.obj" "OTplatform\CC430\cc430_lib\cc430_ucs.obj" "OTlib\M2QP.obj" "OTlib\M2_network.obj" "OTlib\OTAPI_logger.obj" "OTlib\OT_utils.obj" "OTlib\alp_api_client.obj" "OTlib\alp_api_server.obj" "OTlib\alp_dashforth.obj" "OTlib\alp_filedata.obj" "OTlib\alp_logger.obj" "OTlib\alp_main.obj" "OTlib\alp_security.obj" "OTlib\alp_sensor.obj" "OTlib\auth.obj" "OTlib\buffers.obj" "OTlib\crc16.obj" "OTlib\crypto_aes128.obj" "OTlib\encode_M2.obj" "OTlib\external.obj" "OTlib\ndef.obj" "OTlib\queue.obj" "OTlib\session.obj" 
	-$(RM) "OTlib\veelite.obj" "OTkernel\~Native\system.obj" "App_Code\main.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
