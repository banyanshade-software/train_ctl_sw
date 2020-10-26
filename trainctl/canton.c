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


#include "misc.h"
#include "trainctl_iface.h"
#include "canton.h"
#ifndef TRAIN_SIMU
#include "stm32f1xx_hal.h"
#else
#include "train_simu.h"
#endif
#include "railconfig.h"


#ifndef TRAIN_SIMU
TIM_HandleTypeDef *CantonTimerHandles[8] = {NULL};
#endif

void canton_reset(const canton_config_t *c, canton_vars_t *v)
{
	v->curtrainidx = 0xFF;
	v->status = canton_free;
}

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

// #pragma mark -

#ifndef TRAIN_SIMU

void canton_set_pwm(const canton_config_t *c, canton_vars_t *v,  int dir, int duty)
{
	if (c->canton_type == CANTON_TYPE_DUMMY) return;

	if (c->canton_type == CANTON_TYPE_REMOTE ) {
		return;
	}
	int t = 2*duty; // with centered pwm (or normal)

	if ((v->cur_dir == dir) && (v->cur_pwm_duty==duty)) return;

	TIM_HandleTypeDef *pwm_timer = CantonTimerHandles[c->pwm_timer_num];
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
	if (c->canton_type == CANTON_TYPE_DUMMY) return;

	if (c->canton_type == CANTON_TYPE_REMOTE ) {
		return;
	}
	v->cur_voltidx = voltidx;

	uint16_t s = 0;
	uint16_t r = 0;
	if (voltidx & 0x01) s |= VOLT_SEL0_Pin; else r |=  c->volt_b0;
	if (voltidx & 0x02) s |= VOLT_SEL1_Pin; else r |=  c->volt_b1;
	if (voltidx & 0x04) s |= VOLT_SEL2_Pin; else r |=  c->volt_b2;
	if (voltidx & 0x08) s |= VOLT_SEL3_Pin; else r |=  c->volt_b3;
	HAL_GPIO_WritePin(c->volt_port, s, GPIO_PIN_SET);
	HAL_GPIO_WritePin(c->volt_port, r, GPIO_PIN_RESET);
}

#else  // TRAIN_SIMU
void canton_set_volt(const canton_config_t *c, canton_vars_t *v, int voltidx)
{
    int vlt = c->volts[voltidx];
    int n = canton_idx(v);
    train_simu_canton_volt(n, voltidx, vlt);
}
void canton_set_pwm(const canton_config_t *c, canton_vars_t *v,  int dir, int duty)
{
    int n = canton_idx(v);
    train_simu_canton_set_pwm(n, dir, duty);
}

 void __attribute__((weak)) train_simu_canton_volt(int numcanton, int voltidx, int vlt100)
{
    
}

void __attribute__((weak)) train_simu_canton_set_pwm(int numcanton, int dir, int duty)
{
}
#endif


int volt_index(uint16_t mili_power,
		const canton_config_t *c1, canton_vars_t *v1,
		const canton_config_t *c2, canton_vars_t *v2,
		int *pvi1, int *pvi2,
		train_volt_policy_t pol)
{
	int duty=0;
	*pvi1 = 15;
	*pvi2 = 15;

	if (mili_power <0) return canton_error_rc(0, ERR_BAD_PARAM_MPOW, "negative milipower");
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
            for (int i=15; i>=0; i--) {
                if (!c1->volts[i]) continue;
                // c1->volts in 0.01V unit
                int d = 100*mili_power / c1->volts[i];
                if (d>MAX_PWM) {
                    continue;
                }
                // XXX for now we assume all canton have same board with same voltage level
                *pvi1 = *pvi2 = i;
                duty = d;
                break;
            }
		break;
	case vpolicy_pure_volt:
		duty = MAX_PWM;
        int s = 0;
		for (int i=15; i>=0; i--) {
			if (!c1->volts[i]) continue;
			// c1->volts in 0.01V unit. 10V = 1000
			int p = c1->volts[i]*MAX_PWM/100;  // 0.01V * % , ex : 345*90
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

// volt_measured * 4.545 = volt_real (10k 2.2k bridge)
// ADC 12bits: 0x3FF = 3.3V
// v = meas/1024 * 3.3 * 4.55

static int num_canton_bemf = 0;

// adc1=0, adc2=2163, von1=0, von2=172
static void calib_store_bemf(int32_t vraw);
static int calibrating = 0;

#define INCLUDE_FIXBEMF 0

#if INCLUDE_FIXBEMF
static const int32_t bemf_zero_values[MAX_PWM*2+1] = {-92, -72, -49, -41, -31, -24, -19, -15, -12, -9, -8, -80, -64, -47, -30, -23, -18, -14, -12, -7, -74, -60, -45, -35, -22, -18, -12, -71, -42, -32, -21, -18, -11, -11, -65, -40, -24, -18, -47, -37, -22, -13, -44, -28, -56, -34, -21, -14, -9, -49, -31, -47, -24, -35, -44, -20, -30, -31, -38, -30, -15, -8, -7, -4, -2, -1, -1, -1, 0, 0, -2, -1, -1, -1, -1, -2, -1, -1, -1, 0, -1, 0, -1, -1, -1, 0, 0, 0, -1, -2, 0, 0, 0, 0, 0, -3, -1, 0, 0, 0, 0, 0, 3, 0, 0, -1, -3, -2, 0, 0, 0, 4, 0, -1, 2, 3, 0, 1, 1, 4, 10, 22, 33, 25, 28, 17, 38, 29, 19, 43, 26, 44, 7, 9, 18, 31, 53, 25, 42, 13, 20, 35, 46, 17, 22, 37, 65, 9, 10, 15, 20, 29, 36, 70, 12, 14, 19, 33, 43, 55, 72, 9, 9, 16, 20, 25, 32, 51, 66, 87, 11, 11, 14, 17, 19, 27, 34, 43, 57, 72, 92};
#endif


static inline int32_t bemf_convert_to_centivolt_for_display(const canton_config_t *c, canton_vars_t *v, int32_t m) // unit 0.01v
{
#if INCLUDE_FIXBEMF
	if (v->fix_bemf && (v->curtrainidx!=0xFF)) {
		// int idx = v->cur_pwm_duty * v->cur_dir;
		// XXX
		//const train_config_t *tc = get_train_cnf(v->curtrainidx);
		train_vars_t *tv = get_train_vars(v->curtrainidx);
		int idx = tv->last_speed;  //tc->enable_pid ? tv->last_speed2 : tv->last_speed;
		if (idx<-MAX_PWM)     idx = -MAX_PWM;
		else if (idx>MAX_PWM) idx =  MAX_PWM;
		m -= bemf_zero_values[idx+MAX_PWM];
	}
#endif
	if (v->fix_bemf && (v->curtrainidx!=0xFF)) {
		// XXX calibration
		// negative value measured on Von are different for >0 than <0
		// (-2280 vs 2160) probably due to different resistors in division
		// bridge
		if (m<0) {
			m = 2200*m/2000;
		}
	}
    if (BEMF_RAW) return m;
	return ((m * 4545 * 33) / (4096*100));
}


static inline int32_t bemf_convert_to_centivolt(const canton_config_t *c, canton_vars_t *v, int32_t m)
{
#if INCLUDE_FIXBEMF
	if (v->fix_bemf && (v->curtrainidx!=0xFF)) {
		//const train_config_t *tc = get_train_cnf(v->curtrainidx);
		train_vars_t *tv = get_train_vars(v->curtrainidx);
		int idx = tv->last_speed; // tc->enable_pid ? tv->last_speed2 : tv->last_speed;
		if (idx<-MAX_PWM)     idx = -MAX_PWM;
		else if (idx>MAX_PWM) idx =  MAX_PWM;
		m -= bemf_zero_values[idx+MAX_PWM];
	}
#endif
	if (v->fix_bemf && (v->curtrainidx!=0xFF)) {
		// XXX calibration

		if (m<0) {
			m = 2200*m/2000;
		}
	}
	return ((m * 4545 * 33) / (4096*100));
}




void canton_bemf(const canton_config_t *c, canton_vars_t *v, uint16_t adc1, uint16_t adc2, uint16_t von1, uint16_t von2)
{
	num_canton_bemf++;
	int32_t val[4];
	int32_t volt2= bemf_convert_to_centivolt(c,v, adc2-adc1); // unit 1/100 V

	if (c->notif_bemf) {
		int32_t volt = bemf_convert_to_centivolt_for_display(c,v, adc2-adc1); // unit 1/100 V
		val[0] = volt;
		val[1] = bemf_convert_to_centivolt_for_display(c,v, von2-von1);
		val[2] = (int32_t) (c->volts[v->cur_voltidx]);
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
	v->bemf_centivolt = volt2;
	if (calibrating) {
		calib_store_bemf(adc2-adc1);
	}
}


/* ======================================================================================== */

#if INCLUDE_CALIB

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


