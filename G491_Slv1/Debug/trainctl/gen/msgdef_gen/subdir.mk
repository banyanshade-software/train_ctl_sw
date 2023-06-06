################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/gen.c \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/genmsg.c \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/main.c \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/system.c \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/utility.c 

O_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/gen.o \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/genmsg.o \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/main.o \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/system.o \
/Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/utility.o 

OBJS += \
./trainctl/gen/msgdef_gen/gen.o \
./trainctl/gen/msgdef_gen/genmsg.o \
./trainctl/gen/msgdef_gen/main.o \
./trainctl/gen/msgdef_gen/system.o \
./trainctl/gen/msgdef_gen/utility.o 

C_DEPS += \
./trainctl/gen/msgdef_gen/gen.d \
./trainctl/gen/msgdef_gen/genmsg.d \
./trainctl/gen/msgdef_gen/main.d \
./trainctl/gen/msgdef_gen/system.d \
./trainctl/gen/msgdef_gen/utility.d 


# Each subdirectory must supply rules for building sources it contributes
trainctl/gen/msgdef_gen/gen.o: /Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/gen.c trainctl/gen/msgdef_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/gen/msgdef_gen/genmsg.o: /Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/genmsg.c trainctl/gen/msgdef_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/gen/msgdef_gen/main.o: /Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/main.c trainctl/gen/msgdef_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/gen/msgdef_gen/system.o: /Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/system.c trainctl/gen/msgdef_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/gen/msgdef_gen/utility.o: /Users/danielbraun/devel/train/sw/trainctl/gen/msgdef_gen/utility.c trainctl/gen/msgdef_gen/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -Os -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-trainctl-2f-gen-2f-msgdef_gen

clean-trainctl-2f-gen-2f-msgdef_gen:
	-$(RM) ./trainctl/gen/msgdef_gen/gen.d ./trainctl/gen/msgdef_gen/gen.o ./trainctl/gen/msgdef_gen/gen.su ./trainctl/gen/msgdef_gen/genmsg.d ./trainctl/gen/msgdef_gen/genmsg.o ./trainctl/gen/msgdef_gen/genmsg.su ./trainctl/gen/msgdef_gen/main.d ./trainctl/gen/msgdef_gen/main.o ./trainctl/gen/msgdef_gen/main.su ./trainctl/gen/msgdef_gen/system.d ./trainctl/gen/msgdef_gen/system.o ./trainctl/gen/msgdef_gen/system.su ./trainctl/gen/msgdef_gen/utility.d ./trainctl/gen/msgdef_gen/utility.o ./trainctl/gen/msgdef_gen/utility.su

.PHONY: clean-trainctl-2f-gen-2f-msgdef_gen

