################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/uitrack/uigen.c 

OBJS += \
./trainctl/uitrack/uigen.o 

C_DEPS += \
./trainctl/uitrack/uigen.d 


# Each subdirectory must supply rules for building sources it contributes
trainctl/uitrack/uigen.o: /Users/danielbraun/devel/train/sw/trainctl/uitrack/uigen.c trainctl/uitrack/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-trainctl-2f-uitrack

clean-trainctl-2f-uitrack:
	-$(RM) ./trainctl/uitrack/uigen.cyclo ./trainctl/uitrack/uigen.d ./trainctl/uitrack/uigen.o ./trainctl/uitrack/uigen.su

.PHONY: clean-trainctl-2f-uitrack

