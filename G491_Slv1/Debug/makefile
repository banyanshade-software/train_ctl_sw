################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include trainctl/utils/subdir.mk
-include trainctl/trkpln/subdir.mk
-include trainctl/topology/subdir.mk
-include trainctl/stm32/ina3221/subdir.mk
-include trainctl/stm32/subdir.mk
-include trainctl/spdctl/subdir.mk
-include trainctl/oscillo/subdir.mk
-include trainctl/obsolete/subdir.mk
-include trainctl/oam/subdir.mk
-include trainctl/msg/subdir.mk
-include trainctl/low/subdir.mk
-include trainctl/leds/subdir.mk
-include trainctl/gen/msgdef_gen/subdir.mk
-include trainctl/gen/config_gen/subdir.mk
-include trainctl/ctrl/subdir.mk
-include trainctl/config/_str/subdir.mk
-include trainctl/config/subdir.mk
-include trainctl/IHM/subdir.mk
-include trainctl/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/subdir.mk
-include Middlewares/Third_Party/FreeRTOS/Source/subdir.mk
-include Drivers/STM32G4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
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


BUILD_ARTIFACT_NAME := G491_Slv1
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
G491_Slv1.elf \

MAP_FILES += \
G491_Slv1.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
G491_Slv1.list \


# All Target
all: main-build

# Main-build Target
main-build: G491_Slv1.elf secondary-outputs

# Tool invocations
G491_Slv1.elf G491_Slv1.map: $(OBJS) $(USER_OBJS) /Users/danielbraun/devel/train/sw/G491_Slv1/STM32G491RETX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "G491_Slv1.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"/Users/danielbraun/devel/train/sw/G491_Slv1/STM32G491RETX_FLASH.ld" --specs=nosys.specs -Wl,-Map="G491_Slv1.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

G491_Slv1.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "G491_Slv1.list"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) G491_Slv1.elf G491_Slv1.list G491_Slv1.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
