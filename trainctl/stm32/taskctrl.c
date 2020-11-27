/*
 * taskctrl.c
 *
 *  Created on: Oct 16, 2020
 *      Author: danielbraun
 */


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

#include "cmsis_os.h"
#include "../misc.h"
#include "../trainctl_iface.h"
#include "taskauto.h"
#include "taskctrl.h"
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#include "misc.h"
//#include "../traincontrol.h"





int cur_freqhz = 50;
extern TIM_HandleTypeDef htim1;

// #define __HAL_TIM_SET_PRESCALER(__HANDLE__, __PRESC__)       ((__HANDLE__)->Instance->PSC = (__PRESC__))
void set_pwm_freq(int freqhz)
{
	// 12MHz / 200 -> 60000
	// 50Hz = 1200
	int ps = (60000/freqhz)-1;
	if ((ps<1) || (ps>0xFFFF)) ps = 1200;
	ps = ps-1;
	cur_freqhz = 60000/(ps+1);
	__HAL_TIM_SET_PRESCALER(&htim1, ps);

}




void run_task_ctrl(void)
{
	if ((0))   calibrate_bemf(); //XXXX
	for (;;) {
		uint32_t notif;
		xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
		//debug_info('G', 0, "HOP", 0, 0, 0);
		static uint32_t oldt = 0;
		static uint32_t t0 = 0;
		uint32_t t = HAL_GetTick();
		// XXX we should have a global t0
		if (!t0) t0 = t;
		int32_t dt = (oldt) ? (t-oldt) : 1;
		oldt = t;

		train_run_tick(notif, t, dt);
		if ((0)) { // XXX
			static int s=0;
			if (!s && (t-t0>5000)) {
				train_set_target_speed(0, 50);
				s = 1;
			}
		}
		task_auto_tick();
	}

}

// ---------------------------------------------------------------
// ADC DMA callbacks
// ---------------------------------------------------------------

extern osThreadId_t ctrlTaskHandle;


static int nhalf=0;
static int nfull=0;


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	nfull++;
	BaseType_t higher=0;
	xTaskNotifyFromISR(ctrlTaskHandle, NOTIF_NEW_ADC_2, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
	nhalf++;
	BaseType_t higher=0;
	xTaskNotifyFromISR(ctrlTaskHandle, NOTIF_NEW_ADC_1, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{

}
void  HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{

}

