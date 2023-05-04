################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/uitrack/uitrack.c 

OBJS += \
./trainctl/uitrack/uitrack.o 

C_DEPS += \
./trainctl/uitrack/uitrack.d 


# Each subdirectory must supply rules for building sources it contributes
trainctl/uitrack/uitrack.o: /Users/danielbraun/devel/train/sw/trainctl/uitrack/uitrack.c trainctl/uitrack/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-trainctl-2f-uitrack

clean-trainctl-2f-uitrack:
	-$(RM) ./trainctl/uitrack/uitrack.d ./trainctl/uitrack/uitrack.o ./trainctl/uitrack/uitrack.su

.PHONY: clean-trainctl-2f-uitrack

