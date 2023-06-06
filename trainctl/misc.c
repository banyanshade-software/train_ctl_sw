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


#if defined(STM32F4)
#include "stm32f4xx_hal.h"

#elif defined(STM32G4)
#include "stm32g4xx_hal.h"

#elif defined(STM32F1)
#include "stm32f1xx_hal.h"

#else
#error no board hal
#endif


#include "cmsis_os.h"
#endif
#include <string.h>
#include <memory.h>


#ifndef BOARD_LED_GPIO_Port
#ifdef TURN4A_BOARD_LED_GPIO_Port
#define BOARD_LED_GPIO_Port TURN4A_BOARD_LED_GPIO_Port
#define BOARD_LED_Pin		TURN4A_BOARD_LED_Pin
#endif
#endif


void flash_led(void)
{
#ifndef TRAIN_SIMU

#ifndef BOARD_LED_GPIO_Port
#ifdef  LD2_GPIO_Port
#define BOARD_LED_GPIO_Port LD2_GPIO_Port
#endif
#endif


#ifndef BOARD_LED_Pin
#ifdef  LD2_Pin
#define BOARD_LED_Pin LD2_Pin
#endif
#endif

	HAL_GPIO_TogglePin(BOARD_LED_GPIO_Port,BOARD_LED_Pin);
	//HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
#endif
}




int trainctl_error(_UNUSED_ char l, int code, _UNUSED_ const char *msg)
{
	int16_t c = code;
	// add debug msg or notification here
	if (c >=0) c=-1;
	trainctl_notif('G', 0, 'E', (void *)&c, sizeof(c));
	return code;
}



// -------------------------------------------------------------------------

void __attribute__((weak)) trainctl_notif(_UNUSED_ uint8_t sel, _UNUSED_ uint8_t num, _UNUSED_ uint8_t cmd, _UNUSED_ uint8_t *dta, _UNUSED_ int dtalen)
{

}
void __attribute__((weak)) trainctl_notif2(_UNUSED_ uint8_t sel, _UNUSED_ uint8_t num, _UNUSED_ uint8_t cmd, _UNUSED_ char *msg, _UNUSED_ int32_t v1, _UNUSED_ int32_t v2, _UNUSED_ int32_t v3)
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




/*
 * CMSISv2 / DEBUG 		RELEASE		frameQ removed		CMSISv1
 * stack :
 * uiTask	304
 * ctrlTask 536
 * txrx     280
 * ina      328
 * led		136
 * oscillo  128
 * oam      380
 *
 * MEM
 * CCRAM 7.83K			 7.83			7.83			8.82 (oam stack in ccm)
 * RAM   53.95K			53.73			52.11			44.61
 * (tot)								(60)			(53.43)
 * FLASH 153.7K			93.48			93.22			90.62
 *
 * oscillo buffer uses 28K
 *
 *
 */

