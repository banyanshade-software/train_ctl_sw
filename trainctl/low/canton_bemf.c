/*
 * canton_bemf.c
 *
 *  Created on: Apr 18, 2021
 *      Author: danielbraun
 */


#include <stdint.h>
#include "trainctl_config.h"
#include "canton_bemf.h"
//#include "canton_config.h"
#include "../msg/trainmsg.h"
//#include "../railconfig.h"
#include "../config/conf_canton.h"

#ifndef BOARD_HAS_CANTON
#error BOARD_HAS_CANTON not defined, remove this file from build
#endif


#if NEW_ADC_AVG

volatile adc_result_t adc_result[2]; // double buffer

#else

uint8_t adc_is_reversed = 0;
volatile adc_buf_t train_adc_buf[2]; // double buffer

#endif



runmode_t bemf_run_mode = runmode_off;
uint8_t bemf_test_all = 0;

#define NUM_TRIGS 4
typedef struct {
    int32_t posval;
    int8_t dir;
    uint8_t postag;
    uint8_t sender;
    //uint8_t postag2;
} bemf_trig_t;

typedef struct{
    uint8_t bemf_to;
    int32_t pose;
    bemf_trig_t trigs[NUM_TRIGS];
} canton_bemf_t;

static canton_bemf_t cbvars[NUM_CANTONS] = {0};

static void process_adc(volatile adc_result_t *buf, uint32_t deltaticks);
static void add_trig(canton_bemf_t *, uint8_t from, int32_t posval, uint8_t posetag,  int8_t dir);


#define USE_CANTON(_idx) \
		const conf_canton_t     *cconf = conf_canton_get(_idx); \
		canton_bemf_t           *cvars = &cbvars[_idx];

void bemf_reset(void)
{
	for (int i=0; i<NUM_CANTONS; i++) {
		cbvars[i].bemf_to = 0xFF;
        cbvars[i].pose = 0;
        memset(cbvars[i].trigs, 0, sizeof(bemf_trig_t)*NUM_TRIGS);
	}
}

void bemf_msg(msg_64_t *m)
{
	if (!MA0_IS_CANTON(m->to)) {
		// error
		itm_debug1(DBG_ERR, "bad bemf c", m->to);
		return;
	}
	int idx = m->subc; // MA_GET_CANTON_NUM(m->to);
	switch(m->cmd) {
	case CMD_BEMF_OFF:
		itm_debug1(DBG_SPDCTL|DBG_CTRL|DBG_DETECT, "BEMF OFF", idx);
		cbvars[idx].bemf_to = 0xFF;
        cbvars[idx].pose = 0;
        memset(cbvars[idx].trigs, 0, sizeof(bemf_trig_t)*NUM_TRIGS);
		break;
	case CMD_BEMF_ON:
		itm_debug2(DBG_SPDCTL|DBG_CTRL|DBG_DETECT, "BEMF ON", idx, m->from);
		cbvars[idx].bemf_to = m->from;
        cbvars[idx].pose = 0;
		break;
            
    case CMD_POSE_SET_TRIG:
            if ((m->from & 0x7) != (cbvars[idx].bemf_to &  0x7)) { // XXXX
                // sanity check, bemf_to is spdctl(numtrain) and sender should be ctrl(numtrain)
                itm_debug2(DBG_ERR, "st/bad", m->from, cbvars[idx].bemf_to);
                break;
            }
            add_trig(&cbvars[idx], m->from, m->va16*100, m->vcu8, m->vb8);
            break;
	default:
		itm_debug1(DBG_ERR, "bad bemf c", m->to);
		break;
	}
}

void bemf_tick(uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	switch (bemf_run_mode) {
	default: //FALLTHRU
	case runmode_testcan: // FALLTHRU
	case runmode_off:
		// dont handle bemf in these modes
		return;
		break;

	case runmode_testcanton : //FALLTHRU
	case runmode_detect_experiment:	// FALLTHRU
	case runmode_detect2: // FALLTHRU
	case runmode_normal :
		// do handle bemf in these modes
		break;

	}

	//itm_debug1(DBG_ADC, "------- btk", (int) notif_flags);
	if (notif_flags & NOTIF_NEW_ADC_1) {
		if (notif_flags & NOTIF_NEW_ADC_2) {
			itm_debug1(DBG_ERR|DBG_LOWCTRL|DBG_TIM, "both", (int) notif_flags);
			runtime_error(ERR_DMA, "both NEW_ADC1 and NEW_ADC2");
		}
		process_adc(&adc_result[0], dt);
	}
	if (notif_flags & NOTIF_NEW_ADC_2) {
		process_adc(&adc_result[1], dt);
	}
}



static int _add_trig(canton_bemf_t *cvars, uint8_t from, int32_t posval, uint8_t posetag, int8_t dir)
{
    itm_debug3(DBG_POSEC, "set trg", posetag, dir, posval);
    if (!posval) {
        itm_debug1(DBG_ERR|DBG_POSEC, "nul trig", posetag);
        posval = 1; // 0 means no trigger
    }
    for (int pi=0; pi<NUM_TRIGS; pi++) {
        if (cvars->trigs[pi].posval == posval) {
            if (cvars->trigs[pi].postag == posetag) {
                itm_debug2(DBG_ERR|DBG_POSEC, "dup trig", posval, posetag);
                cvars->trigs[pi].dir = dir;
                cvars->trigs[pi].sender = from;
                return -1; // ignore it
            }
            // same poseval but different tag ; not  an error here,
            // but for now it could be a bug in ctrlP.c
            itm_debug3(DBG_ERR|DBG_POSEC, "same trig", posval, cvars->trigs[pi].postag, posetag);
        }
        if (cvars->trigs[pi].posval) continue;
        cvars->trigs[pi].posval = posval;
        cvars->trigs[pi].dir = dir;
        cvars->trigs[pi].postag = posetag;
        cvars->trigs[pi].sender = from;
        return pi;
     }
     itm_debug1(DBG_ERR|DBG_POSEC, "NO TRIG", posetag);
     FatalError("NOTR", "no trig", Error_CtrlBadPose);
     return -1;
}

static void add_trig(canton_bemf_t *cvars, uint8_t from, int32_t posval, uint8_t posetag, int8_t dir)
{
    int pi = _add_trig(cvars, from, posval, posetag, dir);
    if (pi) {
        // check trig ?
        // nothing to do it will be checked at first bemf notif
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
//#define BEMF_RAW 0

#if INCLUDE_FIXBEMF
static const int32_t bemf_zero_values[MAX_PWM*2+1] = {-92, -72, -49, -41, -31, -24, -19, -15, -12, -9, -8, -80, -64, -47, -30, -23, -18, -14, -12, -7, -74, -60, -45, -35, -22, -18, -12, -71, -42, -32, -21, -18, -11, -11, -65, -40, -24, -18, -47, -37, -22, -13, -44, -28, -56, -34, -21, -14, -9, -49, -31, -47, -24, -35, -44, -20, -30, -31, -38, -30, -15, -8, -7, -4, -2, -1, -1, -1, 0, 0, -2, -1, -1, -1, -1, -2, -1, -1, -1, 0, -1, 0, -1, -1, -1, 0, 0, 0, -1, -2, 0, 0, 0, 0, 0, -3, -1, 0, 0, 0, 0, 0, 3, 0, 0, -1, -3, -2, 0, 0, 0, 4, 0, -1, 2, 3, 0, 1, 1, 4, 10, 22, 33, 25, 28, 17, 38, 29, 19, 43, 26, 44, 7, 9, 18, 31, 53, 25, 42, 13, 20, 35, 46, 17, 22, 37, 65, 9, 10, 15, 20, 29, 36, 70, 12, 14, 19, 33, 43, 55, 72, 9, 9, 16, 20, 25, 32, 51, 66, 87, 11, 11, 14, 17, 19, 27, 34, 43, 57, 72, 92};
#endif

#if 0
static _UNUSED_ inline int32_t bemf_convert_to_centivolt_for_display(_UNUSED_ const canton_config_t *c,  int32_t m) // unit 0.01v
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
#endif


static inline int32_t bemf_convert_to_millivolt(_UNUSED_ const conf_canton_t *c, int32_t m)
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

	return ((m * 4545 * 33) / (4096*10));
}

/// ---------------------------------------------------------------------------------------

static void process_adc(volatile adc_result_t *buf, _UNUSED_ uint32_t deltaticks)
{
	static int cnt=0;
	cnt++; // for debug, allow us to print msg every x adc conversiont

	for (int i=0; i<NUM_CANTONS; i++) {
		USE_CANTON(i)

		// process intensity / presence
		// process BEMF
#ifndef USE_INA3221
	    canton_intensity(cconf, cvars, buf[i].intOff, buf[i].intOn);
#error ohla
#endif
		int skp = 0;
		if ((bemf_run_mode == runmode_testcanton) && (i>5)) {
			skp = 1;
		}


#if NEW_ADC_AVG
		int32_t voff = bemf_convert_to_millivolt(cconf, buf->meas[i].vBA);
		//int32_t voffa = bemf_convert_to_millivolt(cconf, buf->meas[i].vA);
		//int32_t voffb = bemf_convert_to_millivolt(cconf, buf->meas[i].vB);
		//int32_t vona = 0; // not available with NEW_ADC_AVG
		int16_t von = 0;
#else
		int32_t voffa = bemf_convert_to_millivolt(cconf, buf->off[i].vA);
		int32_t voffb = bemf_convert_to_millivolt(cconf, buf->off[i].vB);
		int32_t vona =  bemf_convert_to_millivolt(cconf, buf->on[i].vA);
		int32_t vonb =  bemf_convert_to_millivolt(cconf, buf->on[i].vB);
		int16_t voff = (int16_t)(voffb-voffa);
		int16_t von  = (int16_t)(vonb-vona);
#endif

		if (adc_is_reversed) {
			// swap von/voff, depending on synchro between
			// ADC conversion (TIM8) and PWM (TIM1)
			// adc_is_reversed is set by taskctrl.c
			int32_t t = voff;
			voff = von;
			von = t;
		}
		if (cconf->reverse_bemf) {
			voff = -voff;
			von = -von;
		}
		if ((0)) {
			//if (!(cnt % 50)) {
				itm_debug3(DBG_ADC|DBG_LOWCTRL, "ADC/Vof", i, voffa, voffb);
				itm_debug3(DBG_ADC|DBG_LOWCTRL, "ADC/Von",  i, vona, vonb);
				itm_debug3(DBG_ADC|DBG_LOWCTRL, "ADC/V01", i,  voff, von);
			//}
		}

		/* only send bemf for canton that are active (expecting bemf)
		 * this could be (and was) done before millivolt conversion,
		 * but we want some debug
		 */
		if (0xFF == cvars->bemf_to) {
			continue;
		}
		if (skp) continue;


		if ((i==0) && NOTIF_VOFF) {
			static int cnt = 0;
			cnt++;
			if ((cnt%32)==0) {
				msg_64_t m;
				m.from = MA0_CANTON(oam_localBoardNum());
				m.subc = i;
				m.to = MA3_UI_GEN;
				m.cmd = CMD_VOFF_NOTIF;
				m.v1 = (int16_t) voff;
				m.v2 = von;
				mqf_write(&from_canton, &m);
			}
		}
        /* modif for #longtrain
         * we now compute pose and triggers here
         */
        int32_t b = voff;
        if (abs(b)<100) b = 0; // XXX XXXX
        // TODO: BEMF to speed. currently part of it is done in convert_to_centivolt
        //       but we assume speed is really proportional to BEMF

        //  dt is not precise enough
        int32_t pi = (b*100)/tsktick_freqhz;
        cvars->pose += pi;
        
        // check trigs
        int s = SIGNOF0(pi);
        uint8_t ptag = 0;
        bemf_trig_t *ptrig = NULL;
        if (s) {
            for (int ti=NUM_TRIGS-1; ti>=0; ti--) {
                // check in reverse order, so that oldest trig
                // is fired first
                if (!cvars->trigs[ti].posval) continue;
                if (cvars->trigs[ti].dir>0) {
                    // pose is incrementing
                    if (cvars->pose > cvars->trigs[ti].posval) {
                        ptag = cvars->trigs[ti].postag;
                        itm_debug3(DBG_POSEC|DBG_POSE, "TRIG>", cvars->pose, cvars->trigs[ti].posval, ptag);
                        cvars->trigs[ti].posval = 0;
                        ptrig = &cvars->trigs[ti];
                        break;
                    }
                } else {
                    // pose is decrementing
                    if (cvars->pose < cvars->trigs[ti].posval) {
                        ptag = cvars->trigs[ti].postag;
                        itm_debug3(DBG_POSEC|DBG_POSE, "TRIG<", cvars->pose, cvars->trigs[ti].posval, ptag);
                        cvars->trigs[ti].posval = 0;
                        ptrig = &cvars->trigs[ti];
                        break;
                    }
                }
                
            }
        }
        if (ptag) {
            itm_debug1(DBG_POSEC, "trig", ptag);
        }
		msg_64_t m = {0};
		m.from = MA0_CANTON(oam_localBoardNum());
		m.subc = i;
        //m.subc = ptag; // xxxx
		m.to = cvars->bemf_to;
        
		m.cmd = CMD_BEMF_NOTIF;
		m.v1 = voff;
        //m.v2 = von;
        m.v2 = cvars->pose/100;
		mqf_write(&from_canton, &m);
        
        if (ptrig) {
            m.to = ptrig->sender;
            m.cmd = CMD_POSE_TRIGGERED;
            m.va16 = cvars->pose/100;
            m.vcu8 = ptag;
            m.vb8 = ptrig->dir;
            mqf_write(&from_canton, &m);
        }
    }
}

