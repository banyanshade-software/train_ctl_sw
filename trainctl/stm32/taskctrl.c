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
#include "main.h"
#include "task.h"
#include "taskctrl.h"

#include "../msg/trainmsg.h"
#include "../low/canton.h"
#include "../low/canton_bemf.h"
#include "../low/presence_detection.h"
#include "../low/turnout.h"
#include "../spdctl/spdctl.h"
#include "../ctrl/ctrl.h"

#if 0
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
#endif
//#include "../../../stm32dev/ina3221/ina3221.h"
#include "trainctl_config.h"
#include "low/canton_bemf.h"


#define NUM_VAL_PER_CANTON (sizeof(adc_buffer_t)/sizeof(uint16_t))
#define ADC_HALF_BUFFER (NUM_LOCAL_CANTONS_HW * NUM_VAL_PER_CANTON)
#define NUM_ADC_SAMPLES (2*ADC_HALF_BUFFER)



static void run_task_ctrl(void);

void StartCtrlTask(void *argument)
{

	if (NUM_VAL_PER_CANTON != 4) Error_Handler();
	if (ADC_HALF_BUFFER != 10*2) Error_Handler();

	  set_pwm_freq(400);
	  CantonTimerHandles[1]=&htim1;
	  CantonTimerHandles[2]=&htim2;
	  CantonTimerHandles[3]=&htim3;
	  //CantonTimerHandles[3]=&htim3;
	  //XXX railconfig_setup_default();



	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	  // XX

	  HAL_TIM_Base_Start_IT(&htim8);


	  HAL_ADC_Start_DMA(&hadc1,(uint32_t *)train_adc_buffer, NUM_ADC_SAMPLES);

	  run_task_ctrl();
}

int cur_freqhz = 350;
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
	__HAL_TIM_SET_PRESCALER(&htim8, ps);
}




static void run_task_ctrl(void)
{

	//if ((0))   calibrate_bemf(); //XXXX
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

		if ((1)) {
			itm_debug2("ctick", notif, dt);
			//continue;
		}
		/*
		if (dt>67) {
			itm_debug2("ctick", notif, dt);
		}
		continue;
		*/
		/*
		void bemf_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);
		void canton_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);
		void turnout_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);
		ina3221
		void spdctl_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);
		void msgsrv_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);
		*/

		bemf_tick(notif, t, dt);
		msgsrv_tick(notif, t, dt);
		spdctl_run_tick(notif, t, dt);
		//msgsrv_tick(notif, t, dt);
		canton_tick(notif, t, dt);
		presdect_tick(notif, t, dt);
		turnout_tick(notif, t, dt);
		ctrl_run_tick(notif, t, dt);
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
	if ((0)) itm_debug1("conv/f", HAL_GetTick());
	xTaskNotifyFromISR(ctrlTaskHandle, NOTIF_NEW_ADC_2, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
	nhalf++;
	BaseType_t higher=0;
	if ((0)) itm_debug1("conv/h", HAL_GetTick());
	xTaskNotifyFromISR(ctrlTaskHandle, NOTIF_NEW_ADC_1, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{

}
void  HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{

}

