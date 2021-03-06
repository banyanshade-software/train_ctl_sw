/*
 * canton_bemf.c
 *
 *  Created on: Apr 18, 2021
 *      Author: danielbraun
 */


#include <stdint.h>
#include "canton_bemf.h"
#include "canton_config.h"
#include "../msg/trainmsg.h"
#include "railconfig.h"


volatile adc_buf_t train_adc_buf[2]; // double buffer

uint8_t bemf_test_mode = 0;
uint8_t bemf_test_all = 0;

static uint8_t bemf_to[NUM_LOCAL_CANTONS_SW] = {0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF};
static void process_adc(volatile adc_buf_t *buf, int32_t ticks);


#define USE_CANTON(_idx) \
		const canton_config_t *cconf = get_canton_cnf(_idx); \
		//canton_vars_t         *cvars = &canton_vars[_idx];

void bemf_reset(void)
{
	for (int i=0; i<NUM_LOCAL_CANTONS_SW; i++) {
		bemf_to[i]=0xFF;
	}
}

void bemf_msg(msg_64_t *m)
{
	if (!IS_CANTON(m->to)) {
		// error
		return;
	}
	int idx = m->to & 0x07;
	switch(m->cmd) {
	case CMD_BEMF_OFF:
		bemf_to[idx] = 0xFF;
		break;
	case CMD_BEMF_ON:
		bemf_to[idx] = m->from;
		break;
	}
}

void bemf_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	itm_debug1(DBG_ADC, "------- btk", notif_flags);
	if (notif_flags & NOTIF_NEW_ADC_1) {
		if (notif_flags & NOTIF_NEW_ADC_2) {
			itm_debug1(DBG_ERR|DBG_LOWCTRL|DBG_TIM, "both", notif_flags);
			runtime_error(ERR_DMA, "both NEW_ADC1 and NEW_ADC2");
		}
		process_adc(&train_adc_buf[0], dt);
	}
	if (notif_flags & NOTIF_NEW_ADC_2) {
		process_adc(&train_adc_buf[1], dt);
	}
}




// volt_measured * 4.545 = volt_real (10k 2.2k bridge)
// ADC 12bits: 0x3FF = 3.3V
// v = meas/1024 * 3.3 * 4.55

//static int num_canton_bemf = 0;

// adc1=0, adc2=2163, von1=0, von2=172
//static void calib_store_bemf(int32_t vraw);
//static int calibrating = 0;

#define INCLUDE_FIXBEMF 0
#define BEMF_RAW 0

#if INCLUDE_FIXBEMF
static const int32_t bemf_zero_values[MAX_PWM*2+1] = {-92, -72, -49, -41, -31, -24, -19, -15, -12, -9, -8, -80, -64, -47, -30, -23, -18, -14, -12, -7, -74, -60, -45, -35, -22, -18, -12, -71, -42, -32, -21, -18, -11, -11, -65, -40, -24, -18, -47, -37, -22, -13, -44, -28, -56, -34, -21, -14, -9, -49, -31, -47, -24, -35, -44, -20, -30, -31, -38, -30, -15, -8, -7, -4, -2, -1, -1, -1, 0, 0, -2, -1, -1, -1, -1, -2, -1, -1, -1, 0, -1, 0, -1, -1, -1, 0, 0, 0, -1, -2, 0, 0, 0, 0, 0, -3, -1, 0, 0, 0, 0, 0, 3, 0, 0, -1, -3, -2, 0, 0, 0, 4, 0, -1, 2, 3, 0, 1, 1, 4, 10, 22, 33, 25, 28, 17, 38, 29, 19, 43, 26, 44, 7, 9, 18, 31, 53, 25, 42, 13, 20, 35, 46, 17, 22, 37, 65, 9, 10, 15, 20, 29, 36, 70, 12, 14, 19, 33, 43, 55, 72, 9, 9, 16, 20, 25, 32, 51, 66, 87, 11, 11, 14, 17, 19, 27, 34, 43, 57, 72, 92};
#endif


static _UNUSED_ inline int32_t bemf_convert_to_centivolt_for_display(const canton_config_t *c,  int32_t m) // unit 0.01v
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
	if (v->fix_bemf && (v->curtrainidx!=0xFF)) {
		// XXX calibration
		// negative value measured on Von are different for >0 than <0
		// (-2280 vs 2160) probably due to different resistors in division
		// bridge
		if (m<0) {
			m = 2200*m/2000;
		}
	}
#endif
    if (BEMF_RAW) return m;
	return ((m * 4545 * 33) / (4096*100));
}


static inline int32_t bemf_convert_to_centivolt(const canton_config_t *c, int32_t m)
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
	if (v->fix_bemf && (v->curtrainidx!=0xFF)) {
		// XXX calibration

		if (m<0) {
			m = 2200*m/2000;
		}
	}
#endif

	return ((m * 4545 * 33) / (4096*100));
}

/// ---------------------------------------------------------------------------------------

static void process_adc(volatile adc_buf_t *buf, int32_t ticks)
{
	for (int i=0; i<NUM_LOCAL_CANTONS_HW; i++) {
		USE_CANTON(i)

		// process intensity / presence
		// process BEMF
#ifndef USE_INA3221
	    canton_intensity(cconf, cvars, buf[i].intOff, buf[i].intOn);
#error ohla
#endif
		int skp = 0;
		if (bemf_test_mode && (bemf_test_all)&& (i<3)) {
			skp = 1;
		} else if (0xFF == bemf_to[i]) {
			continue;
		}

		const canton_config_t *c = get_canton_cnf(i);
		/*
		int32_t voffa = bemf_convert_to_centivolt(c, buf[i].voffA);
		int32_t voffb = bemf_convert_to_centivolt(c, buf[i].voffB);
		int32_t vona = bemf_convert_to_centivolt(c, buf[i].vonA);
		int32_t vonb = bemf_convert_to_centivolt(c, buf[i].vonB);
		 */
		int32_t voffa = bemf_convert_to_centivolt(c, buf->off[i].vA);
		int32_t voffb = bemf_convert_to_centivolt(c, buf->off[i].vB);
		int32_t vona =  bemf_convert_to_centivolt(c, buf->on[i].vA);
		int32_t vonb =  bemf_convert_to_centivolt(c, buf->on[i].vB);

		int16_t voff = (int16_t)(voffb-voffa);
		int16_t von  = (int16_t)(vonb-vona);

		if (cconf->reverse_bemf) {
			voff = -voff;
			von = -von;
		}
		if ((1)) {
			itm_debug3(DBG_ADC|DBG_LOWCTRL, "ADC/Voff", i, voffa, voffb);
			itm_debug3(DBG_ADC|DBG_LOWCTRL, "ADC/Von",  i, vona, vonb);
			itm_debug3(DBG_ADC|DBG_LOWCTRL, "ADCoi", i,  voff, von);
		}
		if (skp) continue;
		if ((i==0) && NOTIF_VOFF) {
			static int cnt = 0;
			cnt++;
			if ((cnt%32)==0) {
				msg_64_t m;
				m.from = MA_CANTON(localBoardNum, i);
				m.to = MA_UI(1);
				m.cmd = CMD_VOFF_NOTIF;
				m.v1 = voff;
				m.v2 = von;
				mqf_write(&from_canton, &m);
			}
		}
		msg_64_t m;
		m.from = MA_CANTON(localBoardNum, i);
		m.to = bemf_to[i];
		m.cmd = CMD_BEMF_NOTIF;
		m.v1 = voff;
		m.v2 = von;
		mqf_write(&from_canton, &m);

	}
}

