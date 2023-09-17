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

static void set_oneshot(int timernum, const conf_canton_t *cconf)
{
	TIM_HandleTypeDef *pwm_timer = CantonTimerHandles[timernum];
	//HAL_StatusTypeDef rc = HAL_TIM_OnePulse_Init(pwm_timer, TIM_OPMODE_SINGLE);

	TIM_SlaveConfigTypeDef sSlaveConfig = {0};
	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_DISABLE;
	sSlaveConfig.InputTrigger = TIM_TS_ITR0;
	HAL_TIM_SlaveConfigSynchro(pwm_timer, &sSlaveConfig);

	// CounterMode
	pwm_timer->Instance->CR1 &= ~(TIM_CR1_DIR | TIM_CR1_CMS);
	pwm_timer->Instance->CR1 |= TIM_COUNTERMODE_UP;
	/* Reset the OPM Bit */
	pwm_timer->Instance->CR1 &= ~TIM_CR1_OPM;

	  /* Configure the OPM Mode */
	pwm_timer->Instance->CR1 |= TIM_OPMODE_SINGLE;

	HAL_TIM_OnePulse_Start(pwm_timer, cconf->ch1);

	//if (rc) FatalError("TIMd", "time dirac", Error_Abort);
}

void start_signal_dirac(int cidx, const conf_canton_t *cconf,  canton_vars_t *cvars)
{
	canton_set_volt(cidx, cconf, cvars, 7);
	set_oneshot(cconf->pwm_timer_num, cconf);
	//canton_set_pwm(cidx, cconf, cvars, 1 /*sdir*/, 10 /*duty*/);
}

// -----------------------------------------------------------------------

void start_signal_gen(int cidx, const conf_canton_t *cconf, _UNUSED_ canton_vars_t *cvars,  uint16_t p)
{
	if (p==1) {
		// dirac
		start_signal_dirac(cidx, cconf, cvars);
		return;
	}
	ngen=0;
	memset(signal, 0, sizeof(signal));
	gen_signal(ngen, ngen%2);
	siggen_setup_stopped(cconf->pwm_timer_num, cconf->ch0);
	siggen_setup_timer(cconf->pwm_timer_num, cconf->ch1);
}
