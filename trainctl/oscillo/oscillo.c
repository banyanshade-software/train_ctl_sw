/*
 * oscillo.c
 *
 *  Created on: Jan 24, 2022
 *      Author: danielbraun
 */



#include "cmsis_os.h"
#include "main.h"
#include "task.h"
//#include "oscillo.h"
#include "misc.h"

#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#else
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_tim.h"
#endif

#include "trainctl_iface.h"

#include "../msg/trainmsg.h"

#include "txrxcmd.h"
#include "railconfig.h"
#include <oscillo/oscillo.h>


extern ADC_HandleTypeDef hadc2;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim8;


//#define OSC_NUM_SAMPLES 32

static osc_values_t oscillo_buf[OSC_NUM_SAMPLES] __attribute__ ((aligned(32)));
static int oscilo_index = 0;

static volatile int oscilo_run = 0;
static volatile int oscilo_did_end = 0;
static volatile int oscilo_postprocess = 0;
static volatile int adc_in_progress = 0;

static void oscilo_start(void);
static void oscilo_end(void);

int oscilo_running(void)
{
	if (oscilo_run) return 1;
	if (oscilo_did_end) return 1;
	if (oscilo_postprocess) return 1;
	return 0;
}

volatile int ocillo_enable = 0;
volatile int oscillo_trigger_start = 0;

void StartOscilo(_UNUSED_ void *argument)
{

	//HAL_TIM_Base_Start_IT(&htim5);

	//HAL_ADC_Start_DMA(&hadc2,(uint32_t *)oscilo_buf, 4*2);
	//HAL_ADC_Start_IT(&hadc2);
    //__HAL_ADC_ENABLE_IT(&hadc2, ADC_IT_EOC);


	if ((0)) oscilo_start();
	for(;;) {
		osDelay(200);
		if (oscilo_did_end) {
			oscilo_postprocess = 1;
			itm_debug1(DBG_TIM, "proc osc", 0);
			frame_msg_t m;
			m.t = TXFRAME_TYPE_OSCILO;
			txframe_send(&m, 0);
			oscilo_did_end = 0;
			// TODO
		}
		if (ocillo_enable &&  oscillo_trigger_start) {
			oscillo_trigger_start = 0;
			oscilo_start();
		}
	}
}





void frame_send_oscilo(void(*cb)(uint8_t *d, int l))
{
	for (int i=0; i<OSC_NUM_SAMPLES; i++) {
		uint8_t buf[32];
		int l = txrx_frm_escape2(buf, (uint8_t *)&oscillo_buf[i], sizeof(osc_values_t), sizeof(buf));
		cb(buf, l);
	}
	oscilo_postprocess = 0;
}

static void oscilo_start(void)
{
	if (oscilo_postprocess || oscilo_did_end) return;
	memset(oscillo_buf, 0, sizeof(oscillo_buf));
	oscilo_index = 0;
	oscilo_run = 1;
	itm_debug1(DBG_TIM, "osc start", 0);
	HAL_TIM_Base_Start_IT(&htim5);
}

void oscilo_end(void)
{
	oscilo_run = 0;
	oscilo_index = 0;
	HAL_TIM_Base_Stop(&htim5);
	HAL_ADC_Stop_DMA(&hadc2);
	itm_debug1(DBG_TIM, "osc end", 0);
	oscilo_did_end = 1;
	adc_in_progress = 0; // XXX should already been 0, except if failure
}

static uint16_t adcbuf[16];

void tim5_elapsed(void)
{
	//itm_debug1(DBG_TIM, "tim5", 0);
	if (!oscilo_run) return;


	oscillo_buf[oscilo_index].tim1cnt =  __HAL_TIM_GET_COUNTER(&htim1);
	oscillo_buf[oscilo_index].tim2cnt =  __HAL_TIM_GET_COUNTER(&htim2);
	oscillo_buf[oscilo_index].tim8cnt =  __HAL_TIM_GET_COUNTER(&htim8);
	//TIM_TypeDef *T1 = htim1.Instance;
	//TIM_TypeDef *T2 = htim2.Instance;
	//uint16_t sr1 = T1->SR;
	//uint16_t sr2 = T2->SR;
	//oscilo_buf[oscilo_index].valt1ch1 = (sr1 & TIM_SR_CC1IF) ? 1 : 0;
	//oscilo_buf[oscilo_index].valt1ch2 = (sr1 & TIM_SR_CC2IF) ? 1 : 0;


	oscillo_buf[oscilo_index].valt1ch1 = HAL_GPIO_ReadPin(PWM_0_0_GPIO_Port, PWM_0_0_Pin);
	oscillo_buf[oscilo_index].valt1ch2 = HAL_GPIO_ReadPin(PWM_0_1_GPIO_Port, PWM_0_1_Pin);

	oscillo_buf[oscilo_index].valt1ch3 = HAL_GPIO_ReadPin(PWM_1_0_GPIO_Port, PWM_1_0_Pin);
	oscillo_buf[oscilo_index].valt1ch4 = HAL_GPIO_ReadPin(PWM_1_1_GPIO_Port, PWM_1_1_Pin);

	oscillo_buf[oscilo_index].valt2ch1 = HAL_GPIO_ReadPin(PWM_2_0_GPIO_Port, PWM_2_0_Pin);
	oscillo_buf[oscilo_index].valt2ch2 = HAL_GPIO_ReadPin(PWM_2_1_GPIO_Port, PWM_2_1_Pin);

	oscillo_buf[oscilo_index].valt2ch3 = HAL_GPIO_ReadPin(PWM_3_0_GPIO_Port, PWM_3_0_Pin);
	oscillo_buf[oscilo_index].valt2ch4 = HAL_GPIO_ReadPin(PWM_3_1_GPIO_Port, PWM_3_1_Pin);

	oscillo_buf[oscilo_index].t0bemf = oscillo_t0bemf;
	oscillo_buf[oscilo_index].t1bemf = oscillo_t1bemf;

	oscillo_buf[oscilo_index].ina0 = oscillo_ina0;
	oscillo_buf[oscilo_index].ina0 = oscillo_ina1;
	oscillo_buf[oscilo_index].ina0 = oscillo_ina2;
	//itm_debug3(DBG_TIM, "oscina", oscillo_ina0, oscillo_ina1, oscillo_ina2);

	if (oscillo_buf[oscilo_index].t0bemf > 7000) {
		//extern void bemf_hi(void);
		//bemf_hi();
	}
	oscillo_buf[oscilo_index].evtadc = oscillo_evtadc;
	if (oscillo_evtadc) {
		//itm_debug1(DBG_TIM, "osc evtadc", oscilo_evtadc);
		oscillo_evtadc = 0;
	}
 	oscilo_index++;
	if (oscilo_index >= OSC_NUM_SAMPLES) {
		oscilo_end();
	} else {
		if (adc_in_progress) {
			itm_debug1(DBG_TIM, "ADC  pgrs", adc_in_progress);
			return;
		}
		adc_in_progress = 1;
		if ((0)) {
			HAL_ADC_Start_DMA(&hadc2,(uint32_t *)(&oscillo_buf[oscilo_index-1].vadc[0]), 4);
		} else {
			memcpy(&oscillo_buf[oscilo_index-1].vadc[0],adcbuf, 2*4);
			memset(adcbuf, 0, sizeof(adcbuf));
			HAL_StatusTypeDef rc = HAL_ADC_Start_DMA(&hadc2,(uint32_t *)adcbuf, 4);
			if (rc != HAL_OK) {
				itm_debug1(DBG_ERR|DBG_TIM, "OSC ERR", rc);
				adc_in_progress = 0;
			}

		}
	}
}

static void conv_done(int f)
{
	if (f) adc_in_progress = 0;
	//itm_debug1(DBG_TIM, "CONV", f);
}

void HAL_ADC_ConvCpltCallback2(_UNUSED_ ADC_HandleTypeDef* hadc)
{
	conv_done(1);
}
void HAL_ADC_ConvHalfCpltCallback2(_UNUSED_ ADC_HandleTypeDef* hadc)
{
	conv_done(0);
}
void HAL_ADC_ErrorCallback2(_UNUSED_ ADC_HandleTypeDef* hadc)
{
	conv_done(-1);
}
