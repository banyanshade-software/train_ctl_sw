/*
 * canton.c
 *
 *  Created on: Oct 2, 2020
 *      Author: danielbraun
 */


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */



/*
 * canton.c : control of a block
 *            and thus of a block control board
 *            - set main voltage (3.5-10V)
 *            - set PWM and duty
 *            - process BEMF measure
 *
 *  		  preliminary (but unused for now) support for
 *  		  - block occupency / free
 *  		  - remote block for multiple MCU system
 */


#include <stddef.h>
#include <memory.h>
#include "misc.h"
#include "trainctl_iface.h"
#include "canton.h"
#include "canton_bemf.h"
#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#else
#include "train_simu.h"
#endif
#include "railconfig.h"


#ifndef TRAIN_SIMU
TIM_HandleTypeDef *CantonTimerHandles[8] = {NULL};
#endif


typedef struct canton_vars {
	int8_t cur_dir;
	uint8_t cur_voltidx;
	uint16_t cur_pwm_duty;
	int32_t selected_centivolt;
} canton_vars_t;

static canton_vars_t canton_vars[NUM_LOCAL_CANTONS];

#define USE_CANTON(_idx) \
		const canton_config_t *cconf = get_canton_cnf(_idx); \
		canton_vars_t         *cvars = &canton_vars[_idx];



static void canton_set_pwm(const canton_config_t *c, canton_vars_t *v,  int dir, int duty);
void canton_set_volt(const canton_config_t *c, canton_vars_t *v, int voltidx);

static void canton_reset(void)
{
	for (int i = 0; i<NUM_LOCAL_CANTONS; i++) {
		USE_CANTON(i)
		canton_set_pwm(cconf, cvars, 0, 0);
		canton_set_volt(cconf, cvars,  7);
	}
}

void canton_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	static int first=1;
	if (first) {
		first = 0;
		canton_reset();
		bemf_reset();
	}
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_canton(&m);
		if (rc) break;
		if (IS_CANTON(m.to)) {
			if (m.cmd & 0x40) {
				bemf_msg(&m);
				continue;
			}
			int cidx = m.to & 0x07;
			USE_CANTON(cidx)
			switch (m.cmd) {
			case CMD_STOP:
				canton_set_pwm(cconf, cvars, 0, 0);
				canton_set_volt(cconf, cvars,  7);
				break;
			case CMD_SETVPWM:
				canton_set_pwm(cconf, cvars, 0, 0);
				canton_set_volt(cconf, cvars,  7);
				break;
			}
		} else {
			switch (m.cmd) {
			case CMD_RESET: // FALLTHRU
			case CMD_EMERGENCY_STOP:
				canton_reset();
				bemf_reset();
				break;
			}
		}
	}
}

// ------------------------------------------------------------

/*
int canton_take(int numcanton, canton_occupency_t st,  int trainidx)
{
	if (trainidx<0 || trainidx>32) return canton_error(ERR_BAD_PARAM, "bad params 1");
	if (st < canton_next) return canton_error(ERR_BAD_PARAM, "bad params 2");
	USE_CANTON(numcanton) // cconf cvars
	//if (v->curtrain) return canton_error(ERR_CANTON_USED, "canton alreday in use");
	if (cvars->status != canton_free) return canton_error(ERR_CANTON_USED, "canton already in use");

	cvars->status = st;
	cvars->curtrainidx = trainidx;
	(void) cconf; // unused
	return 0;
}

int canton_change_status(int numcanton, canton_occupency_t st,  int trainidx)
{
	if (trainidx<0 || trainidx>32) return canton_error(ERR_BAD_PARAM, "bad params 3");
	if (st < canton_next) return canton_error(ERR_BAD_PARAM, "bad params 4");
	USE_CANTON(numcanton) // cconf cvars
	if (cvars->curtrainidx != trainidx) return canton_error(ERR_CANTON_USED, "canton already in use 2");
	cvars->status = st;
	(void)cconf; // unused
	return 0;
}

int canton_release(int numcanton, int trainidx)
{
	USE_CANTON(numcanton) // cconf cvars
	if (cvars->curtrainidx != trainidx) return canton_error(ERR_CANTON_USED, "canton already in use 3");
	cvars->curtrainidx = 0xFF;
	cvars->status = canton_free;
	(void)cconf; // unused
	return 0;
}
*/


// #pragma mark -

#ifndef TRAIN_SIMU

static void canton_set_pwm(const canton_config_t *c, canton_vars_t *v,  int dir, int duty)
{
	int t = 2*duty; // with centered pwm (or normal)

	if ((v->cur_dir == dir) && (v->cur_pwm_duty==duty)) return;

	TIM_HandleTypeDef *pwm_timer = CantonTimerHandles[c->pwm_timer_num];
	if (!pwm_timer) return;
	if (v->cur_dir != dir) {
		v->cur_dir = dir;
		if (dir>0) {
			HAL_TIM_PWM_Stop(pwm_timer, c->ch1);
			HAL_TIM_PWM_Start(pwm_timer, c->ch0);
		} else {
			HAL_TIM_PWM_Stop(pwm_timer, c->ch0);
			HAL_TIM_PWM_Start(pwm_timer, c->ch1);
		}
	}
	v->cur_pwm_duty = duty;
	uint32_t ch;
	if (dir>0) {
		ch = c->ch0;
		//TIM1->CCR1 = t;
	} else {
		ch = c->ch1;
		//TIM1->CCR2 = t;
	}
	switch (ch) {
	case TIM_CHANNEL_1:
		pwm_timer->Instance->CCR1 = t;
		break;
	case TIM_CHANNEL_2:
		pwm_timer->Instance->CCR2 = t;
		break;
	case TIM_CHANNEL_3:
		pwm_timer->Instance->CCR3 = t;
		break;
	case TIM_CHANNEL_4:
		pwm_timer->Instance->CCR4 = t;
		break;
	default:
		canton_error(ERR_BAD_PARAM_TIM, "bad timer channel");
		break;
	}
}
void canton_set_volt(const canton_config_t *c, canton_vars_t *v, int voltidx)
{
	v->cur_voltidx = voltidx;
    v->selected_centivolt =  (c->volts_cv[v->cur_voltidx]);

    if ((0)) debug_info('C', 0, "SET VLT ", voltidx,  v->selected_centivolt,0);
    if ((0)) debug_info('C', 0, "VLT BIT ", (voltidx & 0x03) ? 1 : 0,
    				(voltidx & 0x02) ? 1 : 0,
    				(voltidx & 0x01) ? 1 : 0);

	HAL_GPIO_WritePin(c->volt_port_b0, c->volt_b0, (voltidx & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(c->volt_port_b1, c->volt_b1, (voltidx & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(c->volt_port_b2, c->volt_b2, (voltidx & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#ifndef VOLT_SEL_3BITS
	HAL_GPIO_WritePin(c->volt_port_b3, c->volt_b3, (voltidx & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
#if 0
	uint16_t s = 0;
	uint16_t r = 0;
	if (voltidx & 0x01) s |= VOLT_0_SEL0_Pin; else r |=  c->volt_b0;
	if (voltidx & 0x02) s |= VOLT_0_SEL1_Pin; else r |=  c->volt_b1;
	if (voltidx & 0x04) s |= VOLT_0_SEL2_Pin; else r |=  c->volt_b2;
	if (voltidx & 0x08) s |= VOLT_0_SEL3_Pin; else r |=  c->volt_b3;
	HAL_GPIO_WritePin(c->volt_port, s, GPIO_PIN_SET);
	HAL_GPIO_WritePin(c->volt_port, r, GPIO_PIN_RESET);
#endif
}

#else  // TRAIN_SIMU
void canton_set_volt(const canton_config_t *c, canton_vars_t *v, int voltidx)
{
    int vlt = c->volts_cv[voltidx];
    int n = canton_idx(v);
    v->cur_voltidx = voltidx;
    v->selected_centivolt =  (c->volts_cv[v->cur_voltidx]);
    train_simu_canton_volt(n, voltidx, vlt);
}
void canton_set_pwm(const canton_config_t *c, canton_vars_t *v,  int dir, int duty)
{
    int n = canton_idx(v);
    v->cur_pwm_duty = duty;
    v->cur_dir = dir;
    train_simu_canton_set_pwm(n, dir, duty);
}

 void __attribute__((weak)) train_simu_canton_volt(int numcanton, int voltidx, int vlt100)
{
    
}

void __attribute__((weak)) train_simu_canton_set_pwm(int numcanton, int dir, int duty)
{
}


#endif


#define MAX_PVI (NUM_VOLTS_VAL-1)

int volt_index(uint16_t mili_power,
		const canton_config_t *c1, //canton_vars_t *v1,
		const canton_config_t *c2, //canton_vars_t *v2,
		int *pvi1, int *pvi2,
		train_volt_policy_t pol)
{
	int duty=0;
	*pvi1 = MAX_PVI;
	*pvi2 = MAX_PVI;

	if (mili_power <0)    return canton_error_rc(0, ERR_BAD_PARAM_MPOW, "negative milipower");
	if (mili_power >1000) return canton_error_rc(0, ERR_BAD_PARAM_MPOW, "milipower should be 0-999");
	switch (pol) {
	default :
        duty = 0;
		return canton_error_rc(0, ERR_BAD_PARAM_VPOL, "bad volt policy");
		break;
	case vpolicy_pure_pwm:
		*pvi1 = 0;
		*pvi2 = 0;
		duty = mili_power / 10;
		break;
    case vpolicy_normal:
            // fall back to full volt +  pwm
            *pvi1 = *pvi2 = 0;
            duty = mili_power / 10;
            for (int i=MAX_PVI; i>=0; i--) {
                if (!c1->volts_cv[i]) continue;
                // c1->volts in 0.01V unit
                int d = 100*mili_power / c1->volts_cv[i];
                if (d>MAX_PWM) {
                    continue;
                }
                // XXX for now we assume all canton have same board with same voltage level
                *pvi1 = *pvi2 = i;
                duty = d;
                break;
            }
		break;
#if 0
    case vpolicy_v2:
    	*pvi1 = *pvi2 = 0;
    	duty = mili_power / 10;
    	for (int i=15; i>=0; i--) {
    		if (!c1->volts_v2[i]) continue;
    		// c1->volts in 0.01V unit
			int d = 100*mili_power / c1->volts_v2[i];
			if (d>MAX_PWM) {
				continue;
			}
			// XXX for now we assume all canton have same board with same voltage level
			*pvi1 = *pvi2 = i;
			duty = d;
			break;
    	}
    	break;
    case vpolicy_v4:
    	*pvi1 = *pvi2 = 0;
    	duty = mili_power / 10;
    	for (int i=15; i>=0; i--) {
    		if (!c1->volts_v4[i]) continue;
    		// c1->volts in 0.01V unit
			int d = 100*mili_power / c1->volts_v4[i];
			if (d>MAX_PWM) {
				continue;
			}
			// XXX for now we assume all canton have same board with same voltage level
			*pvi1 = *pvi2 = i;
			duty = d;
			break;
    	}
    	break;
#endif
	case vpolicy_pure_volt:
		duty = MAX_PWM;
        int s = 0;
		for (int i=MAX_PVI; i>=0; i--) {
			if (!c1->volts_cv[i]) continue;
			// c1->volts in 0.01V unit. 10V = 1000
			int p = c1->volts_cv[i]*MAX_PWM/100;  // 0.01V * % , ex : 345*90
			if (p <= mili_power) {
                s = 1;
				*pvi1 = i;
				*pvi2 = i;
			} else {
                if (!s) {
                    // lower than minimal power
                    *pvi1 = i;
                    *pvi2 = i;
                    duty = 0;
                }
                // ok
				break;
			}
		}
		break;
	}
    if (duty>MAX_PWM) {
        duty = MAX_PWM;
        //canton_error(ERR_BAD_PARAM_MPOW, "test msg");
    }
	return duty;
}


#if 0


/*
=783
von=2088, voff=781
von=2088, voff=781
von=2086, voff=778
von=2086, voff=778
von=2086, voff=779
von=2086, voff=779
von=2091, voff=776
von=2091, voff=776
von=2093, voff=774
von=2093, voff=774
 *
 */

void canton_intensity(const canton_config_t *c, canton_vars_t *v, uint16_t ioff, uint16_t ion)
{
	v->i_off = ioff;
	v->i_on = ion;
	// convert to mA
	/*
	 *  R=0.1, gain=60, Vout=60*(I*0.1) = 6*I
	 *  200mA -> 6*.2 -> 1,2V -> ADC=4096*1.2/3.3 = 1490
	 *  adc = 4096*6*i/3.3,  adc=7447*i
	 *  mA = 1000*adc/7447
	 */
	v->prev_occupency = v->occupency;
	if (v->cur_pwm_duty>0) {
		uint32_t mA = 1000*ion/7447;
		// evaluate R
		// R=U/I
		uint32_t Rohm = mA ? ((uint32_t)(c->volts_cv[v->cur_voltidx]*10))/mA : 9999999;
		if (Rohm>100000) {
			v->occupency = CANTON_OCCUPENCY_FREE;
		} else if (Rohm>100) {
			const int Rwagon = 10000;
			int nwag = Rwagon / Rohm;
			if (nwag>10) nwag = 10;
			v->occupency = CANTON_OCCUPENCY_WAGON+nwag;
		} else {
			v->occupency = CANTON_OCCUPENCY_LOCO;
		}
	} else {
		v->occupency = CANTON_OCCUPENCY_UNKNOWN;
	}
}


void canton_bemf(const canton_config_t *c, canton_vars_t *v, uint16_t adc1, uint16_t adc2, uint16_t von1, uint16_t von2)
{
	num_canton_bemf++;
	int32_t val[4];
	int32_t volt2= bemf_convert_to_centivolt(c,v, adc2-adc1); // unit 1/100 V
    
    v->bemf_centivolt = volt2;
    v->von_centivolt = bemf_convert_to_centivolt_for_display(c,v, von2-von1);
    
	if (c->notif_bemf) {
		int32_t volt = bemf_convert_to_centivolt_for_display(c,v, adc2-adc1); // unit 1/100 V
		val[0] = volt;
		val[1] = bemf_convert_to_centivolt_for_display(c,v, von2-von1);
		val[2] = (int32_t) (c->volts_cv[v->cur_voltidx]);
		val[3] = (int32_t) v->cur_pwm_duty;
        uint8_t cmd = 'B';
#ifdef TRAIN_SIMU
        cmd = 'b';
#endif
		canton_notif(canton_idx(v), cmd, (uint8_t*)val, sizeof(int32_t)*4);
	}
	// TODO :
	// - fix high duty (90%) since measure is influenced by on state
	// - bemf to speed
	// send to train
	if (calibrating) {
		calib_store_bemf(adc2-adc1);
	}
}


/* ======================================================================================== */

#if INCLUDE_CALIB
#error ohla
static int calib_spd_to_index(int spd)
{
	if ((spd<-MAX_PWM) || (spd>MAX_PWM)) return 0;
	int x = spd+MAX_PWM;
	return x;
}
int calib_index_to_spd(int x)
{
	return x+MAX_PWM;
}
static int32_t calib_bemf[MAX_PWM*2+1];
static int32_t calsum;

void canton_reset_calib(const canton_config_t *c, canton_vars_t *v, int16_t spd)
{
	calib_bemf[calib_spd_to_index(spd)] = 0;
	calsum = 0;
	calibrating = 1;
}
void canton_end_calib(const canton_config_t *c,   canton_vars_t *v, int16_t spd, int num)
{
	calib_bemf[calib_spd_to_index(spd)]  = calsum/num;
	calibrating = 0;
}

static void calib_store_bemf(int32_t vraw)
{
	calsum += vraw;
}

#else
static void calib_store_bemf(int32_t vraw)
{

}
#endif

#endif

