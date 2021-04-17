/*
 * misc.c
 *
 *  Created on: Sep 14, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include "misc.h"
#ifndef TRAIN_SIMU
#include "main.h"
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#endif
#include <string.h>
#include <memory.h>


void flash_led(void)
{
#ifndef TRAIN_SIMU
	HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
	//HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
#endif
}




int trainctl_error(char l, int code, const char *msg)
{
	int16_t c = code;
	// add debug msg or notification here
	if (c >=0) c=-1;
	trainctl_notif('G', 0, 'E', (void *)&c, sizeof(c));
	return code;
}



// -------------------------------------------------------------------------

void __attribute__((weak)) trainctl_notif(uint8_t sel, uint8_t num, uint8_t cmd, uint8_t *dta, int dtalen) 
{

}
void __attribute__((weak)) trainctl_notif2(uint8_t sel, uint8_t num, uint8_t cmd, char *msg, int32_t v1, int32_t v2, int32_t v3)
{

}
