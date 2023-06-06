################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/stm32/ina3221/ina3221.c 

OBJS += \
./trainctl/stm32/ina3221/ina3221.o 

C_DEPS += \
./trainctl/stm32/ina3221/ina3221.d 


# Each subdirectory must supply rules for building sources it contributes
trainctl/stm32/ina3221/ina3221.o: /Users/danielbraun/devel/train/sw/trainctl/stm32/ina3221/ina3221.c trainctl/stm32/ina3221/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-trainctl-2f-stm32-2f-ina3221

clean-trainctl-2f-stm32-2f-ina3221:
	-$(RM) ./trainctl/stm32/ina3221/ina3221.d ./trainctl/stm32/ina3221/ina3221.o ./trainctl/stm32/ina3221/ina3221.su

.PHONY: clean-trainctl-2f-stm32-2f-ina3221

