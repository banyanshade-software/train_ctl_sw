################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/config.c \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/gen.c \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/main.c \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/nodeutil.c \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/system.c \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/utility.c 

O_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/config.o \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/gen.o \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/main.o \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/nodeutil.o \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/system.o \
/Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/utility.o 

OBJS += \
./trainctl/gen/config_gen/config.o \
./trainctl/gen/config_gen/gen.o \
./trainctl/gen/config_gen/main.o \
./trainctl/gen/config_gen/nodeutil.o \
./trainctl/gen/config_gen/system.o \
./trainctl/gen/config_gen/utility.o 

C_DEPS += \
./trainctl/gen/config_gen/config.d \
./trainctl/gen/config_gen/gen.d \
./trainctl/gen/config_gen/main.d \
./trainctl/gen/config_gen/nodeutil.d \
./trainctl/gen/config_gen/system.d \
./trainctl/gen/config_gen/utility.d 


# Each subdirectory must supply rules for building sources it contributes
trainctl/gen/config_gen/config.o: /Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/config.c trainctl/gen/config_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/gen/config_gen/gen.o: /Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/gen.c trainctl/gen/config_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/gen/config_gen/main.o: /Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/main.c trainctl/gen/config_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/gen/config_gen/nodeutil.o: /Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/nodeutil.c trainctl/gen/config_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/gen/config_gen/system.o: /Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/system.c trainctl/gen/config_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/gen/config_gen/utility.o: /Users/danielbraun/devel/train/sw/trainctl/gen/config_gen/utility.c trainctl/gen/config_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-trainctl-2f-gen-2f-config_gen

clean-trainctl-2f-gen-2f-config_gen:
	-$(RM) ./trainctl/gen/config_gen/config.d ./trainctl/gen/config_gen/config.o ./trainctl/gen/config_gen/config.su ./trainctl/gen/config_gen/gen.d ./trainctl/gen/config_gen/gen.o ./trainctl/gen/config_gen/gen.su ./trainctl/gen/config_gen/main.d ./trainctl/gen/config_gen/main.o ./trainctl/gen/config_gen/main.su ./trainctl/gen/config_gen/nodeutil.d ./trainctl/gen/config_gen/nodeutil.o ./trainctl/gen/config_gen/nodeutil.su ./trainctl/gen/config_gen/system.d ./trainctl/gen/config_gen/system.o ./trainctl/gen/config_gen/system.su ./trainctl/gen/config_gen/utility.d ./trainctl/gen/config_gen/utility.o ./trainctl/gen/config_gen/utility.su

.PHONY: clean-trainctl-2f-gen-2f-config_gen

