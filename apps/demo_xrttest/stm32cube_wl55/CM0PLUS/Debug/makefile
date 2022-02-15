################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include platform/stm32wl55_m0/stm32wlxx_hal_1.11.0/subdir.mk
-include platform/stm32wl55_m0/_startup/subdir.mk
-include platform/stm32wl55_m0/subdir.mk
-include otsys/subdir.mk
-include otlib/subdir.mk
-include m2/subdir.mk
-include io/stm32wl_lora/subdir.mk
-include extensions/subdir.mk
-include app/subdir.mk
-include alp/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := demo_xrttest_CM0PLUS
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
demo_xrttest_CM0PLUS.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
demo_xrttest_CM0PLUS.list \

OBJCOPY_BIN += \
demo_xrttest_CM0PLUS.bin \


# All Target
all: main-build

# Main-build Target
main-build: demo_xrttest_CM0PLUS.elf secondary-outputs

# Tool invocations
demo_xrttest_CM0PLUS.elf: $(OBJS) $(USER_OBJS) /Users/jpnbook/Sync/Repository/OpenTag/apps/demo_xrttest/stm32cube_wl55/CM0PLUS/../../../_common/linker/stm32wl55xc_64_16_CM0PLUS.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "demo_xrttest_CM0PLUS.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m0plus -T"/Users/jpnbook/Sync/Repository/OpenTag/apps/demo_xrttest/stm32cube_wl55/CM0PLUS/../../../_common/linker/stm32wl55xc_64_16_CM0PLUS.ld" --specs=nosys.specs -Wl,-Map="demo_xrttest_CM0PLUS.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

demo_xrttest_CM0PLUS.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "demo_xrttest_CM0PLUS.list"
	@echo 'Finished building: $@'
	@echo ' '

demo_xrttest_CM0PLUS.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "demo_xrttest_CM0PLUS.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout demo_xrttest_CM0PLUS.bin demo_xrttest_CM0PLUS.elf demo_xrttest_CM0PLUS.list
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
