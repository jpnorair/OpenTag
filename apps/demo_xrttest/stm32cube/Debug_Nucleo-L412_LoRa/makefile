################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include platform/stm32l4xx/stm32l4xx_hal_1.14.0/subdir.mk
-include platform/stm32l4xx/_startup/subdir.mk
-include platform/stm32l4xx/subdir.mk
-include otsys/subdir.mk
-include otlib/subdir.mk
-include m2/subdir.mk
-include lib/STM32_USB_FS/subdir.mk
-include io/gnss_null/subdir.mk
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
ht_xrttest.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
ht_xrttest.list \

OBJCOPY_BIN += \
ht_xrttest.bin \


# All Target
all: ht_xrttest.elf secondary-outputs

# Tool invocations
ht_xrttest.elf: $(OBJS) $(USER_OBJS) /Users/jpnbook/Sync/Repository/OpenTag_HDO/apps/ht_xrttest/stm32cube/../../_common/linker/stm32l41x-42x_128_32.ld
	arm-none-eabi-gcc -o "ht_xrttest.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/Users/jpnbook/Sync/Repository/OpenTag_HDO/apps/ht_xrttest/stm32cube/../../_common/linker/stm32l41x-42x_128_32.ld" --specs=nosys.specs -Wl,-Map="ht_xrttest.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

ht_xrttest.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "ht_xrttest.list"
	@echo 'Finished building: $@'
	@echo ' '

ht_xrttest.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "ht_xrttest.bin"
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
