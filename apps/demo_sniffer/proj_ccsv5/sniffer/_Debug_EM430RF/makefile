################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv5/tools/compiler/msp430_4.1.2

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./app_code/main.obj" \
"./applets/sys_sig_powerdown.obj" \
"./applets/sys_sig_panic.obj" \
"./applets/dll_sig_rfterminate_2.obj" \
"./applets/dll_sig_rfinit.obj" \
"./otkernel/gulp/system_gulp.obj" \
"./otlib/veelite.obj" \
"./otlib/session.obj" \
"./otlib/radio_task.obj" \
"./otlib/queue.obj" \
"./otlib/mpipe_task.obj" \
"./otlib/m2_transport.obj" \
"./otlib/m2_network.obj" \
"./otlib/m2_encode.obj" \
"./otlib/m2_dll_task.obj" \
"./otlib/external.obj" \
"./otlib/crypto_aes128.obj" \
"./otlib/crc16.obj" \
"./otlib/buffers.obj" \
"./otlib/auth.obj" \
"./otlib/alp_sensor.obj" \
"./otlib/alp_security.obj" \
"./otlib/alp_main.obj" \
"./otlib/alp_logger.obj" \
"./otlib/alp_filedata.obj" \
"./otlib/alp_dashforth.obj" \
"./otlib/alp_api_server.obj" \
"./otlib/alp_api_client.obj" \
"./otlib/OT_utils.obj" \
"./otlib/OTAPI_tasker.obj" \
"./otlib/OTAPI_logger.obj" \
"./otplatform/cc430/veelite_core_X2_CC430.obj" \
"./otplatform/cc430/platform_isr_CC430.obj" \
"./otplatform/cc430/platform_CC430.obj" \
"./otplatform/cc430/palfi_CC430_spi.obj" \
"./otplatform/cc430/mpipe_CC430_uart.obj" \
"./otplatform/cc430/mpipe_CC430_chronoslcd.obj" \
"./otplatform/cc430_lib/cc430_ucs.obj" \
"./otplatform/cc430_lib/cc430_tim.obj" \
"./otplatform/cc430_lib/cc430_rtc.obj" \
"./otplatform/cc430_lib/cc430_rf.obj" \
"./otplatform/cc430_lib/cc430_pmm.obj" \
"./otplatform/cc430_lib/cc430_map.obj" \
"./otplatform/cc430_lib/cc430_gpio.obj" \
"./otplatform/cc430_lib/cc430_flash.obj" \
"./otplatform/cc430_lib/cc430_dma.obj" \
"./otplatform/cc430_lib/cc430_crc.obj" \
"./otradio/cc430/radio_CC430.obj" \
-l"libc.a" \
"../lnk_cc430f6137_4+32.cmd" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include otradio/cc430/subdir_vars.mk
-include otplatform/cc430_lib/subdir_vars.mk
-include otplatform/cc430/subdir_vars.mk
-include otlib/subdir_vars.mk
-include otkernel/gulp/subdir_vars.mk
-include applets/subdir_vars.mk
-include app_code/subdir_vars.mk
-include subdir_rules.mk
-include otradio/cc430/subdir_rules.mk
-include otplatform/cc430_lib/subdir_rules.mk
-include otplatform/cc430/subdir_rules.mk
-include otlib/subdir_rules.mk
-include otkernel/gulp/subdir_rules.mk
-include applets/subdir_rules.mk
-include app_code/subdir_rules.mk
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
all: sniffer.out

# Tool invocations
sniffer.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: MSP430 Linker'
	"C:/ti/ccsv5/tools/compiler/msp430_4.1.2/bin/cl430" -vmspx --abi=coffabi --code_model=small --data_model=small -O1 --opt_for_speed=0 -g --define=__CC430F6137__ --define=__LITTLE_ENDIAN__ --define=__CCSv5__ --define=__DEBUG__ --define=BOARD_EM430RF --diag_warning=225 --display_error_number --silicon_errata=CPU18 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal -z --stack_size=160 -m"sniffer.map" --heap_size=0 --use_hw_mpy=F5 -i"C:/ti/ccsv5/ccs_base/msp430/include" -i"C:/ti/ccsv5/tools/compiler/msp430_4.1.2/lib" -i"C:/ti/ccsv5/tools/compiler/msp430_4.1.2/include" --reread_libs --warn_sections --rom_model -o "sniffer.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(MSP430_EXECUTABLE_OUTPUTS__QUOTED) "sniffer.out"
	-$(RM) "otradio\cc430\radio_CC430.pp" "otplatform\cc430_lib\cc430_crc.pp" "otplatform\cc430_lib\cc430_dma.pp" "otplatform\cc430_lib\cc430_flash.pp" "otplatform\cc430_lib\cc430_gpio.pp" "otplatform\cc430_lib\cc430_map.pp" "otplatform\cc430_lib\cc430_pmm.pp" "otplatform\cc430_lib\cc430_rf.pp" "otplatform\cc430_lib\cc430_rtc.pp" "otplatform\cc430_lib\cc430_tim.pp" "otplatform\cc430_lib\cc430_ucs.pp" "otplatform\cc430\mpipe_CC430_chronoslcd.pp" "otplatform\cc430\mpipe_CC430_uart.pp" "otplatform\cc430\palfi_CC430_spi.pp" "otplatform\cc430\platform_CC430.pp" "otplatform\cc430\platform_isr_CC430.pp" "otplatform\cc430\veelite_core_X2_CC430.pp" "otlib\OTAPI_logger.pp" "otlib\OTAPI_tasker.pp" "otlib\OT_utils.pp" "otlib\alp_api_client.pp" "otlib\alp_api_server.pp" "otlib\alp_dashforth.pp" "otlib\alp_filedata.pp" "otlib\alp_logger.pp" "otlib\alp_main.pp" "otlib\alp_security.pp" "otlib\alp_sensor.pp" "otlib\auth.pp" "otlib\buffers.pp" "otlib\crc16.pp" "otlib\crypto_aes128.pp" "otlib\external.pp" "otlib\m2_dll_task.pp" 
	-$(RM) "otlib\m2_encode.pp" "otlib\m2_network.pp" "otlib\m2_transport.pp" "otlib\mpipe_task.pp" "otlib\queue.pp" "otlib\radio_task.pp" "otlib\session.pp" "otlib\veelite.pp" "otkernel\gulp\system_gulp.pp" "applets\dll_sig_rfinit.pp" "applets\dll_sig_rfterminate_2.pp" "applets\sys_sig_panic.pp" "applets\sys_sig_powerdown.pp" "app_code\main.pp" 
	-$(RM) "otradio\cc430\radio_CC430.obj" "otplatform\cc430_lib\cc430_crc.obj" "otplatform\cc430_lib\cc430_dma.obj" "otplatform\cc430_lib\cc430_flash.obj" "otplatform\cc430_lib\cc430_gpio.obj" "otplatform\cc430_lib\cc430_map.obj" "otplatform\cc430_lib\cc430_pmm.obj" "otplatform\cc430_lib\cc430_rf.obj" "otplatform\cc430_lib\cc430_rtc.obj" "otplatform\cc430_lib\cc430_tim.obj" "otplatform\cc430_lib\cc430_ucs.obj" "otplatform\cc430\mpipe_CC430_chronoslcd.obj" "otplatform\cc430\mpipe_CC430_uart.obj" "otplatform\cc430\palfi_CC430_spi.obj" "otplatform\cc430\platform_CC430.obj" "otplatform\cc430\platform_isr_CC430.obj" "otplatform\cc430\veelite_core_X2_CC430.obj" "otlib\OTAPI_logger.obj" "otlib\OTAPI_tasker.obj" "otlib\OT_utils.obj" "otlib\alp_api_client.obj" "otlib\alp_api_server.obj" "otlib\alp_dashforth.obj" "otlib\alp_filedata.obj" "otlib\alp_logger.obj" "otlib\alp_main.obj" "otlib\alp_security.obj" "otlib\alp_sensor.obj" "otlib\auth.obj" "otlib\buffers.obj" "otlib\crc16.obj" "otlib\crypto_aes128.obj" 
	-$(RM) "otlib\external.obj" "otlib\m2_dll_task.obj" "otlib\m2_encode.obj" "otlib\m2_network.obj" "otlib\m2_transport.obj" "otlib\mpipe_task.obj" "otlib\queue.obj" "otlib\radio_task.obj" "otlib\session.obj" "otlib\veelite.obj" "otkernel\gulp\system_gulp.obj" "applets\dll_sig_rfinit.obj" "applets\dll_sig_rfterminate_2.obj" "applets\sys_sig_panic.obj" "applets\sys_sig_powerdown.obj" "app_code\main.obj" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
