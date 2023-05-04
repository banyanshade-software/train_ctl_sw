################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/stm32/canmsg.c \
/Users/danielbraun/devel/train/sw/trainctl/stm32/oamtask.c \
/Users/danielbraun/devel/train/sw/trainctl/stm32/spi1_swo_trace.c \
/Users/danielbraun/devel/train/sw/trainctl/stm32/taskctrl.c \
/Users/danielbraun/devel/train/sw/trainctl/stm32/taskdisp.c \
/Users/danielbraun/devel/train/sw/trainctl/stm32/taskled.c \
/Users/danielbraun/devel/train/sw/trainctl/stm32/usbtask.c 

OBJS += \
./trainctl/stm32/canmsg.o \
./trainctl/stm32/oamtask.o \
./trainctl/stm32/spi1_swo_trace.o \
./trainctl/stm32/taskctrl.o \
./trainctl/stm32/taskdisp.o \
./trainctl/stm32/taskled.o \
./trainctl/stm32/usbtask.o 

C_DEPS += \
./trainctl/stm32/canmsg.d \
./trainctl/stm32/oamtask.d \
./trainctl/stm32/spi1_swo_trace.d \
./trainctl/stm32/taskctrl.d \
./trainctl/stm32/taskdisp.d \
./trainctl/stm32/taskled.d \
./trainctl/stm32/usbtask.d 


# Each subdirectory must supply rules for building sources it contributes
trainctl/stm32/canmsg.o: /Users/danielbraun/devel/train/sw/trainctl/stm32/canmsg.c trainctl/stm32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/stm32/oamtask.o: /Users/danielbraun/devel/train/sw/trainctl/stm32/oamtask.c trainctl/stm32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/stm32/spi1_swo_trace.o: /Users/danielbraun/devel/train/sw/trainctl/stm32/spi1_swo_trace.c trainctl/stm32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/stm32/taskctrl.o: /Users/danielbraun/devel/train/sw/trainctl/stm32/taskctrl.c trainctl/stm32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/stm32/taskdisp.o: /Users/danielbraun/devel/train/sw/trainctl/stm32/taskdisp.c trainctl/stm32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/stm32/taskled.o: /Users/danielbraun/devel/train/sw/trainctl/stm32/taskled.c trainctl/stm32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/stm32/usbtask.o: /Users/danielbraun/devel/train/sw/trainctl/stm32/usbtask.c trainctl/stm32/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-trainctl-2f-stm32

clean-trainctl-2f-stm32:
	-$(RM) ./trainctl/stm32/canmsg.d ./trainctl/stm32/canmsg.o ./trainctl/stm32/canmsg.su ./trainctl/stm32/oamtask.d ./trainctl/stm32/oamtask.o ./trainctl/stm32/oamtask.su ./trainctl/stm32/spi1_swo_trace.d ./trainctl/stm32/spi1_swo_trace.o ./trainctl/stm32/spi1_swo_trace.su ./trainctl/stm32/taskctrl.d ./trainctl/stm32/taskctrl.o ./trainctl/stm32/taskctrl.su ./trainctl/stm32/taskdisp.d ./trainctl/stm32/taskdisp.o ./trainctl/stm32/taskdisp.su ./trainctl/stm32/taskled.d ./trainctl/stm32/taskled.o ./trainctl/stm32/taskled.su ./trainctl/stm32/usbtask.d ./trainctl/stm32/usbtask.o ./trainctl/stm32/usbtask.su

.PHONY: clean-trainctl-2f-stm32

