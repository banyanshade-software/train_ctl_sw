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


void start_signal_gen(const conf_canton_t *cconf, _UNUSED_ canton_vars_t *cvars, _UNUSED_ uint16_t p)
{
	ngen=0;
	memset(signal, 0, sizeof(signal));
	gen_signal(ngen, ngen%2);
	siggen_setup_stopped(cconf->pwm_timer_num, cconf->ch0);
	siggen_setup_timer(cconf->pwm_timer_num, cconf->ch1);
}
