################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include platform/stm32l0xx/_startup/subdir.mk
-include platform/stm32l0xx/subdir.mk
-include otsys/subdir.mk
-include otlib/subdir.mk
-include m2/subdir.mk
-include lib/STM32_USB_FS/subdir.mk
-include io/gnss_null/subdir.mk
-include hbuilder/ubxm8/subdir.mk
-include hbuilder/hbpatch/subdir.mk
-include hbuilder/hblib/subdir.mk
-include hbuilder/hbio/sx127x/subdir.mk
-include hbuilder/alp/subdir.mk
-include extensions/subdir.mk
-include app/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
ht_gnssdemo.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
ht_gnssdemo.list \

OBJCOPY_BIN += \
ht_gnssdemo.bin \


# All Target
all: ht_gnssdemo.elf secondary-outputs

# Tool invocations
ht_gnssdemo.elf: $(OBJS) $(USER_OBJS) /Users/jpnbook/Sync/Repository/OpenTag_HDO/apps/ht_gnssdemo/stm32cube/../../_common/linker/stm32l0xx_192_20_noeeprom.ld
	arm-none-eabi-gcc -o "ht_gnssdemo.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m0plus -T"/Users/jpnbook/Sync/Repository/OpenTag_HDO/apps/ht_gnssdemo/stm32cube/../../_common/linker/stm32l0xx_192_20_noeeprom.ld" --specs=nosys.specs -Wl,-Map="ht_gnssdemo.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

ht_gnssdemo.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "ht_gnssdemo.list"
	@echo 'Finished building: $@'
	@echo ' '

ht_gnssdemo.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "ht_gnssdemo.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
