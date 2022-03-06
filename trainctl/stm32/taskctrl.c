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

#include "../utils/adc_mean.h"

/*
#define NUM_VAL_PER_CANTON (sizeof(adc_buffer_t)/sizeof(uint16_t))
#define ADC_HALF_BUFFER (NUM_LOCAL_CANTONS_HW * NUM_VAL_PER_CANTON)
#define NUM_ADC_SAMPLES (2*ADC_HALF_BUFFER)
*/

#define GUARD_SAMPLING 2
//static volatile adc_buf_t train_adc_buf[MAX_NUM_SAMPLING+GUARD_SAMPLING] __attribute__ ((aligned(32))); // double buffer


static void run_task_ctrl(void);
//extern DMA_HandleTypeDef hdma_i2c3_rx;
//extern DMA_HandleTypeDef hdma_i2c3_tx;


// ADC/Vof   NOTIF_  conv/f2/  "osc evtadc "tim"  unk b3 "tx trunc" pwmfreq


int cur_freqhz = 100;
static int numsampling = 0;
static int bemf_divisor = 1;

/*
 *  max pwm (MAX_PWM) is 90% : min off time in µs = (1000000/pwmhz)*.1
 *  	pwmhz       offtime
 *  	 50			2000 µs
 *  	100			1000 µs (1ms)
 *  	200 		 500 µs
 *  	300		 	 333 µs
 *
 * one ADC sampling and conversiont (12 bits):
 * 		12+3 = 15 ADCCLK
 * sampling a full adc_buf_t (12 conversions), at 12 bits :
 * 		15*12 ADCCLK = 180 ADCCLK
 * with ABP2 at 12MHz, PCLK2=12MHz (timers at 24MHz)
 * prescale : 2 -> 6MHz, 1 ADCCLK = 0.16 µs
 * one full adc_buf_t : 30 µS
 *
 * 		pwmhz		offtime		max NUM_SAMPLING
 * 		 50			2000 µs		66
 * 		100			1000		33
 * 		200			 500		16.6
 * 		300			 333		11.1
 *
 * 	https://electronics.stackexchange.com/questions/311326/stm32f20x-adc-sampling-time-rate
 */





static void TIM_ResetCounter(int tn, TIM_HandleTypeDef *htim)
{
	//if ((1)) return;
	TIM_TypeDef* TIMx = htim->Instance;
	/* Check the parameters */
	assert_param(IS_TIM_ALL_PERIPH(TIMx));

	/* Reset the Counter Register value */
	TIMx->CNT = 0;
	if (tn==8) return;
	if (tn==5) return;
	// #define __HAL_TIM_IS_TIM_COUNTING_DOWN(__HANDLE__)    (((__HANDLE__)->Instance->CR1 &(TIM_CR1_DIR)) == (TIM_CR1_DIR))

	//itm_debug2(DBG_TIM, "CR1 : ", tn, (TIMx->CR1 & TIM_CR1_DIR) ? 1 : 0);
	//TIMx->CR1 &= ~(TIM_CR1_DIR);
	//itm_debug2(DBG_TIM, "CR1b : ", tn, (TIMx->CR1 & TIM_CR1_DIR) ? 1 : 0);
}

static int adc_nsmpl = 0;

void StartCtrlTask(_UNUSED_ void *argument)
{

	adc_nsmpl = sizeof(train_adc_buf)/sizeof(uint16_t);

	if (sizeof(train_adc_buf) != sizeof(uint16_t)*NUM_LOCAL_CANTONS_HW*8) Error_Handler();
	if (adc_nsmpl != NUM_LOCAL_CANTONS_HW*2*4) Error_Handler();
	if (NUM_LOCAL_CANTONS_HW != 6) Error_Handler();



	CantonTimerHandles[1]=&htim1;
	CantonTimerHandles[2]=&htim2;
	CantonTimerHandles[3]=&htim3;
	CantonTimerHandles[4]=&htim12;
	//CantonTimerHandles[3]=&htim3;
	//XXX railconfig_setup_default();

	//osDelay(2000);	// 1000 ok, 500 ko, 100 ko


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

	set_pwm_freq(100, 1);

	/*
	portENTER_CRITICAL();
	//HAL_TIM_Base_Start_IT(&htim8);
	TIM_ResetCounter(8, &htim8);
	TIM_ResetCounter(2, &htim2);
	TIM_ResetCounter(3, &htim3);
	TIM_ResetCounter(1, &htim1);

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_Base_Start(&htim8);
	HAL_TIM_Base_Start(&htim12);
	HAL_TIM_Base_Start_IT(&htim1);
	//HAL_TIM_Base_Start(&htim1);

	//HAL_TIM_OC_Start(&htim8, TIM_CHANNEL_1);
	portEXIT_CRITICAL();
	*/



	// XXX XXX XXX XXX
	memset((void *)train_adc_buf, 0, sizeof(train_adc_buf));
	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)train_adc_buf, adc_nsmpl);



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


// #define __HAL_TIM_SET_PRESCALER(__HANDLE__, __PRESC__)       ((__HANDLE__)->Instance->PSC = (__PRESC__))
void set_pwm_freq(int freqhz, int crit)
{

	//if ((1)) return;
	if (!freqhz) {
		return;
	}
	// 12MHz / 200 -> 60000
	// 50Hz = 1200
#define FRQ_MULT (2)							// 1 : for 24 MHz (ABP prescaler = /8) ; 2 : 48MHz, ABP prescaler = /4

	bemf_divisor = (freqhz)/100; 			// use to skip some ADC when using high frequencies
												// divisor must be 1 (no division) or even value
	if (bemf_divisor > 1) {
		bemf_divisor = (bemf_divisor/2)*2;
	}

	int ps = (FRQ_MULT*60000/freqhz); //-1;
	if ((ps<1) || (ps>0xFFFF)) ps = 1200;
	ps = ps-1;
	cur_freqhz = FRQ_MULT*60000/(ps+1);
	// not an error but we want it in the log
	itm_debug3(DBG_ERR|DBG_CTRL, "FREQ", freqhz, ps, cur_freqhz);

	if (crit) portENTER_CRITICAL();
	numsampling = 0;
	HAL_TIM_Base_Stop(&htim1);
	HAL_TIM_Base_Stop(&htim2);
	HAL_TIM_Base_Stop(&htim3);
	HAL_TIM_Base_Stop(&htim8);

	__HAL_TIM_SET_PRESCALER(&htim2, ps);
	__HAL_TIM_SET_PRESCALER(&htim3, ps);
	__HAL_TIM_SET_PRESCALER(&htim8, ps);
	__HAL_TIM_SET_PRESCALER(&htim1, ps);

	TIM_ResetCounter(8, &htim8);
	TIM_ResetCounter(2, &htim2);
	TIM_ResetCounter(3, &htim3);
	TIM_ResetCounter(1, &htim1);



	htim8.Instance->EGR = TIM_EGR_UG;
	htim2.Instance->EGR = TIM_EGR_UG;
	htim3.Instance->EGR = TIM_EGR_UG;
	htim1.Instance->EGR = TIM_EGR_UG;

	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_Base_Start(&htim8);
	HAL_TIM_Base_Start_IT(&htim1);
	if (crit) portEXIT_CRITICAL();
	itm_debug1(DBG_TIM|DBG_ERR, "freq ", cur_freqhz); // not an error but it is importanrt
}

int get_pwm_freq(void)
{
	return cur_freqhz;
}

void ADC_IRQ_UserHandler(void)
{
	itm_debug1(DBG_TIM, "ADC compl", 0);
}

#define USE_NOTIF_TIM 0

volatile uint32_t t0ctrl = 0;

/*static const int ckcpu = 1;
static int t_1;
static int t_2;
static int t_3;
static int t_4;
static int t_5;
*/
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

		mqf_write_from_nowhere(&m); // from_nowher, otherwise it wont be sent to ctl
	}

	for (;;) {
		if ((1)) { // measure actual frequency
			static int cnt = 0;
			static uint32_t t0 = 0;
			cnt ++;
			uint32_t t1 = HAL_GetTick();
			if (!t0) {
				t0 = t1;
				cnt = 0;
			} else  if (t1-t0 >= 10000) {
				itm_debug2(DBG_TIM | DBG_DETECT, "wk FREQ", cnt/10, cnt%10);
				cnt = 0;
				t0 = t1;
			}
		}

		uint32_t notif;
		xTaskNotifyWait(0, 0xFFFFFFFF, &notif, portMAX_DELAY);
		if ((1)) {
			int n = 0;
			if (notif & NOTIF_NEW_ADC_1)  n = 1;
			if (notif & NOTIF_NEW_ADC_2)  n |= 2;
			itm_debug2(DBG_LOWCTRL|DBG_TIM, "-----", 0 /*(notif & NOTIF_TIM8) ? 1 : 0*/, n);
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

		bemf_tick(notif, t, dt);
		//if (ckcpu) t_1 = HAL_GetTick();

		itm_debug1(DBG_LOWCTRL, "--msg", dt);
		msgsrv_tick(notif, t, dt);
		//if (ckcpu) t_2 = HAL_GetTick();

		itm_debug1(DBG_LOWCTRL, "--spdctl", dt);
		spdctl_run_tick(notif, t, dt);
		//if (ckcpu) t_3 = HAL_GetTick();

		itm_debug1(DBG_LOWCTRL, "--canton", dt);
		canton_tick(notif, t, dt);
		//if (ckcpu) t_4 = HAL_GetTick();

		itm_debug1(DBG_LOWCTRL, "--trnout", dt);
		turnout_tick(notif, t, dt);
		//if (ckcpu) t_5 = HAL_GetTick();

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
		uint32_t endtime = HAL_GetTick();
		uint32_t et = endtime - t;
		if ((1)) {
			//itm_debug2(DBG_ERR, "ctrl tick", e1, et);
			if (et>9) {
				itm_debug1(DBG_ERR, "long proc", et);
				/*itm_debug1(DBG_ERR, "t1", t_1 - t);
				itm_debug1(DBG_ERR, "t2", t_2 - t_1);
				itm_debug1(DBG_ERR, "t3", t_3 - t_2);
				itm_debug1(DBG_ERR, "t4", t_4 - t_3);
				itm_debug1(DBG_ERR, "t5", t_5 - t_4);
				itm_debug1(DBG_ERR, "t6", endtime - t_5);
				*/
			}
		}
	}

}

// ---------------------------------------------------------------
// ADC DMA callbacks
// ---------------------------------------------------------------

extern osThreadId_t ctrlTaskHandle;


static uint32_t nhalf=0;
static uint32_t nfull=0;

__weak void HAL_ADC_ConvCpltCallback2(_UNUSED_ ADC_HandleTypeDef* hadc)
{
}
__weak void HAL_ADC_ConvHalfCpltCallback2(_UNUSED_ ADC_HandleTypeDef* hadc)
{
}
__weak void HAL_ADC_ErrorCallback2(_UNUSED_ ADC_HandleTypeDef* hadc)
{
}

/*
static int convert_to_mv(int m)
{
    return ((m * 4545 * 33) / (4096*10));
}


static int numresult = 0;

int dump_adc = 0; // for debug



static void write_num(uint8_t *buf, uint32_t v, int ndigit)
{
	for (;ndigit>0; ndigit--) {
		buf[ndigit-1] = '0'+ (v % 10);
		v = v/10;
	}
}
*/
volatile uint8_t oscillo_evtadc;


void HAL_ADC_ConvCpltCallback(_UNUSED_ ADC_HandleTypeDef* hadc)
{
	if (hadc != &hadc1) {
		HAL_ADC_ConvCpltCallback2(hadc);
		return;
	}
	nfull++;
	if ((bemf_divisor >= 4) && (nfull % (bemf_divisor/2))) {
		return;
	}
	oscillo_evtadc = 2;
	if ((0)) itm_debug1(DBG_TIM, "conv/f", HAL_GetTick());
	BaseType_t higher=0;
	xTaskNotifyFromISR(ctrlTaskHandle, NOTIF_NEW_ADC_2, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
}

void HAL_ADC_ConvHalfCpltCallback(_UNUSED_ ADC_HandleTypeDef* hadc)
{
	if (hadc != &hadc1) {
		HAL_ADC_ConvHalfCpltCallback2(hadc);
		return;
	}
	nhalf++;
	if (bemf_divisor > 1) {
		return;
	}
	oscillo_evtadc = 1;
	BaseType_t higher=0;
	if ((0)) itm_debug1(DBG_TIM, "conv/h", HAL_GetTick());
	xTaskNotifyFromISR(ctrlTaskHandle, NOTIF_NEW_ADC_1, eSetBits, &higher);
	portYIELD_FROM_ISR(higher);
}

static void adc_err(void)
{
	itm_debug1(DBG_TIM, "adc_err", 0);
}
void HAL_ADC_LevelOutOfWindowCallback(_UNUSED_ ADC_HandleTypeDef* hadc)
{
	adc_err();
	itm_debug1(DBG_ERR|DBG_TIM, "ADC out", 1);
}
void  HAL_ADC_ErrorCallback(_UNUSED_ ADC_HandleTypeDef *hadc)
{
	adc_err();
	if (hadc != &hadc1) {
		HAL_ADC_ErrorCallback2(hadc);
		return;
	}
	itm_debug1(DBG_ERR|DBG_TIM, "ADC ERR", 0);
}


void vApplicationStackOverflowHook(_UNUSED_ xTaskHandle xTask, _UNUSED_ signed char *pcTaskName)
{
	itm_debug1(DBG_ERR, "STK OVF", 1);
	for (;;) {

	}
}
