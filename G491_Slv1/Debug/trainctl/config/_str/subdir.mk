################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_boards.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_canton.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_globparam.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_led.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_locomotive.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_servo.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_topology.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_train.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_turnout.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_utest.fields.c \
/Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_utestloc.fields.c 

OBJS += \
./trainctl/config/_str/conf_boards.fields.o \
./trainctl/config/_str/conf_canton.fields.o \
./trainctl/config/_str/conf_globparam.fields.o \
./trainctl/config/_str/conf_led.fields.o \
./trainctl/config/_str/conf_locomotive.fields.o \
./trainctl/config/_str/conf_servo.fields.o \
./trainctl/config/_str/conf_topology.fields.o \
./trainctl/config/_str/conf_train.fields.o \
./trainctl/config/_str/conf_turnout.fields.o \
./trainctl/config/_str/conf_utest.fields.o \
./trainctl/config/_str/conf_utestloc.fields.o 

C_DEPS += \
./trainctl/config/_str/conf_boards.fields.d \
./trainctl/config/_str/conf_canton.fields.d \
./trainctl/config/_str/conf_globparam.fields.d \
./trainctl/config/_str/conf_led.fields.d \
./trainctl/config/_str/conf_locomotive.fields.d \
./trainctl/config/_str/conf_servo.fields.d \
./trainctl/config/_str/conf_topology.fields.d \
./trainctl/config/_str/conf_train.fields.d \
./trainctl/config/_str/conf_turnout.fields.d \
./trainctl/config/_str/conf_utest.fields.d \
./trainctl/config/_str/conf_utestloc.fields.d 


# Each subdirectory must supply rules for building sources it contributes
trainctl/config/_str/conf_boards.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_boards.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_canton.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_canton.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_globparam.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_globparam.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_led.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_led.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_locomotive.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_locomotive.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_servo.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_servo.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_topology.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_topology.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_train.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_train.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_turnout.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_turnout.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_utest.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_utest.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
trainctl/config/_str/conf_utestloc.fields.o: /Users/danielbraun/devel/train/sw/trainctl/config/_str/conf_utestloc.fields.c trainctl/config/_str/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wextra -Werror -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-trainctl-2f-config-2f-_str

clean-trainctl-2f-config-2f-_str:
	-$(RM) ./trainctl/config/_str/conf_boards.fields.cyclo ./trainctl/config/_str/conf_boards.fields.d ./trainctl/config/_str/conf_boards.fields.o ./trainctl/config/_str/conf_boards.fields.su ./trainctl/config/_str/conf_canton.fields.cyclo ./trainctl/config/_str/conf_canton.fields.d ./trainctl/config/_str/conf_canton.fields.o ./trainctl/config/_str/conf_canton.fields.su ./trainctl/config/_str/conf_globparam.fields.cyclo ./trainctl/config/_str/conf_globparam.fields.d ./trainctl/config/_str/conf_globparam.fields.o ./trainctl/config/_str/conf_globparam.fields.su ./trainctl/config/_str/conf_led.fields.cyclo ./trainctl/config/_str/conf_led.fields.d ./trainctl/config/_str/conf_led.fields.o ./trainctl/config/_str/conf_led.fields.su ./trainctl/config/_str/conf_locomotive.fields.cyclo ./trainctl/config/_str/conf_locomotive.fields.d ./trainctl/config/_str/conf_locomotive.fields.o ./trainctl/config/_str/conf_locomotive.fields.su ./trainctl/config/_str/conf_servo.fields.cyclo ./trainctl/config/_str/conf_servo.fields.d ./trainctl/config/_str/conf_servo.fields.o ./trainctl/config/_str/conf_servo.fields.su ./trainctl/config/_str/conf_topology.fields.cyclo ./trainctl/config/_str/conf_topology.fields.d ./trainctl/config/_str/conf_topology.fields.o ./trainctl/config/_str/conf_topology.fields.su ./trainctl/config/_str/conf_train.fields.cyclo ./trainctl/config/_str/conf_train.fields.d ./trainctl/config/_str/conf_train.fields.o ./trainctl/config/_str/conf_train.fields.su ./trainctl/config/_str/conf_turnout.fields.cyclo ./trainctl/config/_str/conf_turnout.fields.d ./trainctl/config/_str/conf_turnout.fields.o ./trainctl/config/_str/conf_turnout.fields.su ./trainctl/config/_str/conf_utest.fields.cyclo ./trainctl/config/_str/conf_utest.fields.d ./trainctl/config/_str/conf_utest.fields.o ./trainctl/config/_str/conf_utest.fields.su ./trainctl/config/_str/conf_utestloc.fields.cyclo ./trainctl/config/_str/conf_utestloc.fields.d ./trainctl/config/_str/conf_utestloc.fields.o ./trainctl/config/_str/conf_utestloc.fields.su

.PHONY: clean-trainctl-2f-config-2f-_str

