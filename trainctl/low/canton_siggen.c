/*
 * canton_siggen.c
 *
 *  Created on: Aug 4, 2023
 *      Author: danielbraun
 */


#include <stddef.h>
#include <memory.h>
#include "../misc.h"
#include "../trainctl_iface.h"

#include "canton_siggen.h"

#define NUM_SAMPLES 32
static uint16_t signal[2*NUM_SAMPLES];
static int ngen = 0;

/*
 * timer clock is 48 MHz
 * 
 * normal config : 
 *   prescalair: 1200 (1199)
 *   period 200
 *  --> 200 Hz (100Hz x 2 due to center mode)
 *
 * signal generation :
 *   prescalair: 11
 *   period : 266 (will use 0-256)
 *  --> 16404 Hz -> 8202Hz
 *  assume 8200Hz
 */


/*
 * DMA1_Stream1_IRQHandler()
 */
static void siggen_setup_stopped(uint8_t timnum, uint32_t ch)
{
	(void)timnum;
	(void)ch;
}
static void siggen_setup_timer(uint8_t timnum, uint32_t ch)
{
	(void)timnum;
	(void)ch;
}

static void gen_signal(int t, int nbuf)
{
	(void)t;
	(void)nbuf;
}


// -----------------------------------------------------------------------


static void do_frequencies(int cidx, _UNUSED_ int timernum,  const conf_canton_t *cconf, canton_vars_t *cvars)
{
	//TIM_HandleTypeDef *pwm_timer = CantonTimerHandles[timernum];
	//HAL_StatusTypeDef rc = HAL_TIM_OnePulse_Init(pwm_timer, TIM_OPMODE_SINGLE);

	int freqhz = get_pwm_freq();

#define TFREQ(_f) do { \
	set_pwm_freq(_f, 1); \
	canton_set_pwm(cidx, cconf, cvars, 1, 20); \
	osDelay(20); \
	canton_set_pwm(cidx, cconf, cvars, 0, 0); \
	osDelay(20); \
} while (0)

	//int ms = 1*1000/freqhz;
	itm_debug2(DBG_DETECT, "st-oneshot", cidx, 0);

	TFREQ(250);
	TFREQ(500);
	TFREQ(1000);
	TFREQ(2000);
	TFREQ(5000);
	TFREQ(10000);
	TFREQ(20000);

	canton_set_pwm(cidx, cconf, cvars, 0, 0); \

	itm_debug1(DBG_DETECT, "end-onesh", cidx);
	set_pwm_freq(freqhz, 0); \

#if 0
	HAL_TIM_Base_Stop(pwm_timer);

	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_DISABLE;
	sSlaveConfig.InputTrigger = TIM_TS_ITR0;
	HAL_TIM_SlaveConfigSynchro(pwm_timer, &sSlaveConfig);

	pwm_timer->Init.Prescaler = 1199;
	pwm_timer->Init.CounterMode = TIM_COUNTERMODE_UP;
	pwm_timer->Init.Period = 200;
	pwm_timer->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	pwm_timer->Init.RepetitionCounter = 0;
	pwm_timer->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	if (HAL_TIM_Base_Init(pwm_timer) != HAL_OK) {
		Error_Handler();
	}
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(pwm_timer, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
    if (HAL_TIM_PWM_Init(pwm_timer) != HAL_OK)  {
	     Error_Handler();
	}
	if (HAL_TIM_OnePulse_Init(pwm_timer, TIM_OPMODE_SINGLE) != HAL_OK) {
	      Error_Handler();
	}
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(pwm_timer, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	TIM_OC_InitTypeDef sConfigOC = {0};
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	if (HAL_TIM_PWM_ConfigChannel(pwm_timer, &sConfigOC, cconf->ch0) != HAL_OK) {
		Error_Handler();
	}

	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};
	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;

	if (HAL_TIMEx_ConfigBreakDeadTime(pwm_timer, &sBreakDeadTimeConfig) != HAL_OK) {
		Error_Handler();
	}
	HAL_TIM_MspPostInit(pwm_timer);
#if 0
	pwm_timer->Instance->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS);
	pwm_timer->Instance->CR1 |= TIM_COUNTERMODE_UP;
	/* Reset the OPM Bit */
	pwm_timer->Instance->CR1 &= ~TIM_CR1_OPM;

	  /* Configure the OPM Mode */
	pwm_timer->Instance->CR1 |= TIM_OPMODE_SINGLE;
#endif
#define CNT_OFF 0
#define CNT_ON(_v) (_v)

	pwm_timer->Instance->CCR1 = CNT_OFF;
	pwm_timer->Instance->CCR2 = CNT_OFF;
	pwm_timer->Instance->CCR3 = CNT_OFF;
	pwm_timer->Instance->CCR4 = CNT_OFF;
	int t=200;

	switch (cconf->ch0) {
	case TIM_CHANNEL_1:
		pwm_timer->Instance->CCR1 = CNT_ON(t);
		break;
	case TIM_CHANNEL_2:
		itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH2/CCR1", 0);
		pwm_timer->Instance->CCR2 = CNT_ON(t);
		break;
	case TIM_CHANNEL_3:
		itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH3/CCR1", 0);
		pwm_timer->Instance->CCR3 = CNT_ON(t);
		break;
	case TIM_CHANNEL_4:
		itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH4/CCR1", 0);
		pwm_timer->Instance->CCR4 = CNT_ON(t);
		break;
	default:
		break;
	}

	HAL_TIM_OnePulse_Start(pwm_timer, cconf->ch0);
	HAL_TIM_Base_Start(pwm_timer);

	//if (rc) FatalError("TIMd", "time dirac", Error_Abort);
#endif
}

void start_signal_freqsteps(int cidx, const conf_canton_t *cconf,  canton_vars_t *cvars)
{
	itm_debug1(DBG_DETECT, "C/oneshot", cidx);
	canton_set_volt(cidx, cconf, cvars, 7);
	do_frequencies(cidx, cconf->pwm_timer_num, cconf, cvars);
	//canton_set_pwm(cidx, cconf, cvars, 1 /*sdir*/, 10 /*duty*/);
}

// -----------------------------------------------------------------------

void start_signal_gen(int cidx, const conf_canton_t *cconf, _UNUSED_ canton_vars_t *cvars,  uint16_t p)
{
	if (p==1) {
		// dirac
		start_signal_freqsteps(cidx, cconf, cvars);
		return;
	}
	ngen=0;
	memset(signal, 0, sizeof(signal));
	gen_signal(ngen, ngen%2);
	siggen_setup_stopped(cconf->pwm_timer_num, cconf->ch0);
	siggen_setup_timer(cconf->pwm_timer_num, cconf->ch1);
}
