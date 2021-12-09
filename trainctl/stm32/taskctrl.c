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
#include "../low/canton_bemf.h"
#include "../leds/ledtask.h"
#include "canmsg.h"

/*
#define NUM_VAL_PER_CANTON (sizeof(adc_buffer_t)/sizeof(uint16_t))
#define ADC_HALF_BUFFER (NUM_LOCAL_CANTONS_HW * NUM_VAL_PER_CANTON)
#define NUM_ADC_SAMPLES (2*ADC_HALF_BUFFER)
*/


static void run_task_ctrl(void);
extern DMA_HandleTypeDef hdma_i2c3_rx;
extern DMA_HandleTypeDef hdma_i2c3_tx;

void StartCtrlTask(_UNUSED_ void *argument)
{
	int nsmpl = sizeof(train_adc_buf)/sizeof(uint16_t);

	if (sizeof(train_adc_buf) != sizeof(uint16_t)*NUM_LOCAL_CANTONS_HW*8) Error_Handler();
	if (nsmpl != NUM_LOCAL_CANTONS_HW*2*4) Error_Handler();
	if (NUM_LOCAL_CANTONS_HW != 6) Error_Handler();
	//__HAL_DMA_ENABLE_IT(&hdma_i2c3_rx, DMA_IT_TC);
	//__HAL_DMA_ENABLE_IT(&hdma_i2c3_tx, DMA_IT_TC);

	//if (NUM_VAL_PER_CANTON != 4) Error_Handler();
	//if (ADC_HALF_BUFFER != 10*2) Error_Handler();

	if ((1)) set_pwm_freq(100);
	CantonTimerHandles[1]=&htim1;
	CantonTimerHandles[2]=&htim2;
	CantonTimerHandles[3]=&htim3;
	CantonTimerHandles[4]=&htim12;
	//CantonTimerHandles[3]=&htim3;
	//XXX railconfig_setup_default();



	//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	//HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
	HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);

	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);

	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);

	HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);

	//HAL_TIM_Base_Start_IT(&htim8);
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_Base_Start(&htim12);

	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)train_adc_buf, nsmpl);
	//HAL_ADC_Start_DMA(&hadc1,(uint32_t *)train_adc_buffer, NUM_ADC_SAMPLES);

	startCycleCounter();
	/*
	uint64_t p = GetCycleCount64();
	uint64_t k = p;
	for (int i=0; i<50; i++) {
		uint64_t t = GetCycleCount64();
		uint32_t d = (uint32_t)(t - p);
		p = t;
		//itm_debug1(DBG_ERR, "cycl", d);
	}
	itm_debug1(DBG_ERR, "tcycl", GetCycleCount64()-k);
	*/

	if ((0)) {
		void CanTest(void);
		CanTest();
	}
	run_task_ctrl();
}

int cur_freqhz = 350;
extern TIM_HandleTypeDef htim1;

// #define __HAL_TIM_SET_PRESCALER(__HANDLE__, __PRESC__)       ((__HANDLE__)->Instance->PSC = (__PRESC__))
void set_pwm_freq(int freqhz)
{
	// 12MHz / 200 -> 60000
	// 50Hz = 1200
	int ps = (60000/freqhz); //-1;
	if ((ps<1) || (ps>0xFFFF)) ps = 1200;
	ps = ps-1;
	cur_freqhz = 60000/(ps+1);
	// not an error but we want it in the log
	itm_debug3(DBG_ERR|DBG_CTRL, "FREQ", freqhz, ps, cur_freqhz);
	__HAL_TIM_SET_PRESCALER(&htim1, ps);
	__HAL_TIM_SET_PRESCALER(&htim2, ps);
	__HAL_TIM_SET_PRESCALER(&htim3, ps);
	__HAL_TIM_SET_PRESCALER(&htim8, ps);
}

int get_pwm_freq(void)
{
	return cur_freqhz;
}


#define USE_NOTIF_TIM 0

volatile uint32_t t0ctrl;
static void run_task_ctrl(void)
{
	int cnt = 0;
	//if ((0))   calibrate_bemf(); //XXX

	if ((1)) {
		msg_64_t m;
		m.from = MA_BROADCAST;
		m.to = MA_BROADCAST;
		m.cmd = CMD_SETRUN_MODE;
		//m.v1u = runmode_off;
		m.v1u = runmode_normal;
		//m.v1u = runmode_detect2;

		mqf_write_from_nowhere(&m); // XXX it wont be sent to ctl
	}

	for (;;) {
		uint32_t notif;
		xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
		if ((1)) {
			int n = 0;
			if (notif & NOTIF_NEW_ADC_1)  n = 1;
			if (notif & NOTIF_NEW_ADC_2)  n |= 2;
			itm_debug2(DBG_LOWCTRL, "-----", 0 /*(notif & NOTIF_TIM8) ? 1 : 0*/, n);
			if (n==3) {
				itm_debug1(DBG_LOWCTRL|DBG_ERR, "both", n);
				if ((1)) continue; // skip this tick
			}
		}
		cnt++;
		t0ctrl = HAL_GetTick();
#if USE_NOTIF_TIM
		if (notif & NOTIF_TIM8) {
			if (cnt>20) presdect_tick(notif, 0, 0);
		}
		if (0==(notif & (NOTIF_NEW_ADC_1|NOTIF_NEW_ADC_2))) continue;
#endif
		//debug_info('G', 0, "HOP", 0, 0, 0);
		static uint32_t oldt = 0;
		static uint32_t t0 = 0;
		uint32_t t = HAL_GetTick();
		// XXX we should have a global t0
		if (!t0) t0 = t;
		int32_t dt = (oldt) ? (t-oldt) : 1;
		oldt = t;

		if ((0)) {
			itm_debug2(DBG_LOWCTRL, "ctick", notif, dt);
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
		itm_debug1(DBG_LOWCTRL, "--msg", dt);
		msgsrv_tick(notif, t, dt);
		itm_debug1(DBG_LOWCTRL, "--spdctl", dt);
		spdctl_run_tick(notif, t, dt);
		itm_debug1(DBG_LOWCTRL, "--canton", dt);
		canton_tick(notif, t, dt);
		itm_debug1(DBG_LOWCTRL, "--trnout", dt);
		turnout_tick(notif, t, dt);
		itm_debug1(DBG_LOWCTRL, "--ctrl", dt);
		ctrl_run_tick(notif, t, dt);
		_UNUSED_ uint32_t e1 = HAL_GetTick() - t;
		if ((0)) CAN_Tasklet(notif, t, dt);
#if USE_NOTIF_TIM
#else
		//if (cnt>20) {
		itm_debug1(DBG_LOWCTRL, "--pres", dt);
		presdect_tick(notif, t, dt);
		//}
#endif
		//led_run_tick(notif, t, dt);

		itm_debug1(DBG_LOWCTRL, "--done", dt);
		uint32_t et = HAL_GetTick() - t;
		if ((1)) {
			//itm_debug2(DBG_ERR, "ctrl tick", e1, et);
			if (et>9) {
				itm_debug1(DBG_ERR, "long proc", et);
			}
		}
	}

}

// ---------------------------------------------------------------
// ADC DMA callbacks
// ---------------------------------------------------------------

extern osThreadId_t ctrlTaskHandle;


static int nhalf=0;
static int nfull=0;


void HAL_ADC_ConvCpltCallback(_UNUSED_ ADC_HandleTypeDef* AdcHandle)
{
	nfull++;
	BaseType_t higher=0;
	if ((0)) itm_debug1(DBG_TIM, "conv/f", HAL_GetTick());
	xTaskNotifyFromISR(ctrlTaskHandle, NOTIF_NEW_ADC_2, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
}

void HAL_ADC_ConvHalfCpltCallback(_UNUSED_ ADC_HandleTypeDef* hadc)
{
	nhalf++;
	BaseType_t higher=0;
	if ((0)) itm_debug1(DBG_TIM, "conv/h", HAL_GetTick());
	xTaskNotifyFromISR(ctrlTaskHandle, NOTIF_NEW_ADC_1, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
}

void HAL_ADC_LevelOutOfWindowCallback(_UNUSED_ ADC_HandleTypeDef* hadc)
{
	itm_debug1(DBG_ERR|DBG_TIM, "ADC ERR", 1);
}
void  HAL_ADC_ErrorCallback(_UNUSED_ ADC_HandleTypeDef *hadc)
{
	itm_debug1(DBG_ERR|DBG_TIM, "ADC ERR", 0);
}

