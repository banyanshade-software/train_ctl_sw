# stm32 subdir

This contains all (mostly) code that is specific to stm32 and/or FreeRTOS

mostly :

- task main function, which define which tasklet are attached to which RTOS task

- ina3221 code

- w25qxx external serial (SPI) flash code

- CAN bus handling

code for ssd1306 is outside trainctl, because it is usable in other projects

TODO : synchronisation of PWM through CAN messages
