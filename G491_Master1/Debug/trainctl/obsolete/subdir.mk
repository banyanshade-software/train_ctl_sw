################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/obsolete/auto1.c \
/Users/danielbraun/devel/train/sw/trainctl/obsolete/block_canton.c \
/Users/danielbraun/devel/train/sw/trainctl/obsolete/railconfig.c \
/Users/danielbraun/devel/train/sw/trainctl/obsolete/taskauto.c \
/Users/danielbraun/devel/train/sw/trainctl/obsolete/txrxcmd.c \
/Users/danielbraun/devel/train/sw/trainctl/obsolete/txrxtask.c 

OBJS += \
./trainctl/obsolete/auto1.o \
./trainctl/obsolete/block_canton.o \
./trainctl/obsolete/railconfig.o \
./trainctl/obsolete/taskauto.o \
./trainctl/obsolete/txrxcmd.o \
./trainctl/obsolete/txrxtask.o 

C_DEPS += \
./trainctl/obsolete/auto1.d \
./trainctl/obsolete/block_canton.d \
./trainctl/obsolete/railconfig.d \
./trainctl/obsolete/taskauto.d \
./trainctl/obsolete/txrxcmd.d \
./trainctl/obsolete/txrxtask.d 


# Each subdirectory must supply rules for building sources it contributes
trainctl/obsolete/auto1.o: /Users/danielbraun/devel/train/sw/trainctl/obsolete/auto1.c trainctl/obsolete/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/obsolete/block_canton.o: /Users/danielbraun/devel/train/sw/trainctl/obsolete/block_canton.c trainctl/obsolete/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/obsolete/railconfig.o: /Users/danielbraun/devel/train/sw/trainctl/obsolete/railconfig.c trainctl/obsolete/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/obsolete/taskauto.o: /Users/danielbraun/devel/train/sw/trainctl/obsolete/taskauto.c trainctl/obsolete/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/obsolete/txrxcmd.o: /Users/danielbraun/devel/train/sw/trainctl/obsolete/txrxcmd.c trainctl/obsolete/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/obsolete/txrxtask.o: /Users/danielbraun/devel/train/sw/trainctl/obsolete/txrxtask.c trainctl/obsolete/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-trainctl-2f-obsolete

clean-trainctl-2f-obsolete:
	-$(RM) ./trainctl/obsolete/auto1.d ./trainctl/obsolete/auto1.o ./trainctl/obsolete/auto1.su ./trainctl/obsolete/block_canton.d ./trainctl/obsolete/block_canton.o ./trainctl/obsolete/block_canton.su ./trainctl/obsolete/railconfig.d ./trainctl/obsolete/railconfig.o ./trainctl/obsolete/railconfig.su ./trainctl/obsolete/taskauto.d ./trainctl/obsolete/taskauto.o ./trainctl/obsolete/taskauto.su ./trainctl/obsolete/txrxcmd.d ./trainctl/obsolete/txrxcmd.o ./trainctl/obsolete/txrxcmd.su ./trainctl/obsolete/txrxtask.d ./trainctl/obsolete/txrxtask.o ./trainctl/obsolete/txrxtask.su

.PHONY: clean-trainctl-2f-obsolete

