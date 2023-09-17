################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/Users/danielbraun/devel/train/sw/stm32dev/disp_tft/ssd1306.c \
/Users/danielbraun/devel/train/sw/stm32dev/disp_tft/ssd1306_fonts.c \
/Users/danielbraun/devel/train/sw/stm32dev/disp_tft/ssd1306_tests.c 

OBJS += \
./stm32dev/disp_tft/ssd1306.o \
./stm32dev/disp_tft/ssd1306_fonts.o \
./stm32dev/disp_tft/ssd1306_tests.o 

C_DEPS += \
./stm32dev/disp_tft/ssd1306.d \
./stm32dev/disp_tft/ssd1306_fonts.d \
./stm32dev/disp_tft/ssd1306_tests.d 


# Each subdirectory must supply rules for building sources it contributes
stm32dev/disp_tft/ssd1306.o: /Users/danielbraun/devel/train/sw/stm32dev/disp_tft/ssd1306.c stm32dev/disp_tft/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
stm32dev/disp_tft/ssd1306_fonts.o: /Users/danielbraun/devel/train/sw/stm32dev/disp_tft/ssd1306_fonts.c stm32dev/disp_tft/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
stm32dev/disp_tft/ssd1306_tests.o: /Users/danielbraun/devel/train/sw/stm32dev/disp_tft/ssd1306_tests.c stm32dev/disp_tft/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32G491xx -c -I"/Users/danielbraun/devel/train/sw/trainctl" -I../Core/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc -I../Drivers/STM32G4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32G4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -Wswitch-default -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-stm32dev-2f-disp_tft

clean-stm32dev-2f-disp_tft:
	-$(RM) ./stm32dev/disp_tft/ssd1306.cyclo ./stm32dev/disp_tft/ssd1306.d ./stm32dev/disp_tft/ssd1306.o ./stm32dev/disp_tft/ssd1306.su ./stm32dev/disp_tft/ssd1306_fonts.cyclo ./stm32dev/disp_tft/ssd1306_fonts.d ./stm32dev/disp_tft/ssd1306_fonts.o ./stm32dev/disp_tft/ssd1306_fonts.su ./stm32dev/disp_tft/ssd1306_tests.cyclo ./stm32dev/disp_tft/ssd1306_tests.d ./stm32dev/disp_tft/ssd1306_tests.o ./stm32dev/disp_tft/ssd1306_tests.su

.PHONY: clean-stm32dev-2f-disp_tft

