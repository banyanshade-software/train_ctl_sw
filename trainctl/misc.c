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




int trainctl_error(char l, int code, _UNUSED_ const char *msg)
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



void long_isr(uint32_t dur)
{
	itm_debug1(DBG_ERR, "long isr", dur);
	//for (int i=0; i<100; i++) {
	//	itm_debug1(DBG_ERR, "micro", GetCurrentMicro());
	//}
}

/*
 * https://electronics.stackexchange.com/questions/76098/high-resolution-system-timer-in-stm32
 */
volatile uint64_t last_cycle_count_64 = 0;

// Call at least every 2^32 cycles (every 59.6 seconds @ 72 MHz).
uint64_t GetCycleCount64(void)
{
#ifdef TRAIN_SIMU
    return 0;
#else
  uint32_t primask;
  asm volatile ("mrs %0, PRIMASK" : "=r"(primask));
  asm volatile ("cpsid i");  // Disable interrupts.
  int64_t r = last_cycle_count_64;
  r += DWT->CYCCNT - (uint32_t)(r);
  last_cycle_count_64 = r;
  asm volatile ("msr PRIMASK, %0" : : "r"(primask));  // Restore interrupts.
  return r;
#endif
}

void startCycleCounter(void)
{
#ifndef TRAIN_SIMU
	DWT->CYCCNT |= DWT_CTRL_CYCCNTENA_Msk;  // Set bit 0.
#endif
}


