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

//#define RECORD_MSG 1

#include <stddef.h>
#include <memory.h>
#include "../misc.h"
#include "../trainctl_iface.h"
#include "canton.h"
#include "canton_bemf.h"
#include "../msg/tasklet.h"
#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#else
#include "train_simu.h"
#endif
//#include "../railconfig.h"
#include "../statval.h"


#ifndef BOARD_HAS_CANTON
#error BOARD_HAS_CANTON not defined, remove this file from build
#endif


#if (NUM_CANTONS == 0)
#error canton code included but NUM_CANTONS iz zero
#endif

#ifndef TRAIN_SIMU
TIM_HandleTypeDef *CantonTimerHandles[8] = {NULL};
#endif


// ------------------------------------------------------

static void canton_init(void);
static void handle_msg_normal(msg_64_t *m);
static msg_handler_t msg_handler_selector(runmode_t);
static void canton_enter_runmode(runmode_t m);

static const tasklet_def_t canton_tdef = {
		.init 				= canton_init,
		.poll_divisor		= NULL,
		.emergency_stop 	= canton_init,
		.enter_runmode		= canton_enter_runmode,
		.pre_tick_handler	= NULL,
		.default_msg_handler = handle_msg_normal,
		.default_tick_handler = NULL,
		.msg_handler_for	= msg_handler_selector,
		.tick_handler_for 	= NULL,

		.recordmsg			= RECORD_MSG,

};
tasklet_t canton_tasklet = { .def = &canton_tdef, .init_done = 0, .queue=&to_canton};




static void handle_msg_cantontest(msg_64_t *m);
static void handle_msg_detect1(msg_64_t *m);
static void handle_msg_detect2(msg_64_t *m);
static void handle_msg_off(msg_64_t *m);

static msg_handler_t msg_handler_selector(runmode_t m)
{
	switch (m) {
	default:  			return NULL;
	case runmode_off:	return handle_msg_off;
	case runmode_normal:return NULL; //default is normal
	case runmode_detect_experiment: return handle_msg_detect1;
	case runmode_detect2: return handle_msg_detect2;
	case runmode_testcanton: return handle_msg_cantontest;
	}
}


// ------------------------------------------------------


// ------------------------------------------------------

typedef struct canton_vars {
	int8_t cur_dir;
	uint8_t cur_voltidx;
	uint16_t cur_pwm_duty;
	int32_t selected_centivolt;
} canton_vars_t;

static canton_vars_t canton_vars[NUM_CANTONS]={0};

// ------------------------------------------------------

const stat_val_t statval_canton[] = {
#ifndef REDUCE_STAT
        { canton_vars, offsetof(canton_vars_t, cur_dir) ,       1     _P("C#_dir")},
        { canton_vars, offsetof(canton_vars_t, cur_voltidx) ,   1     _P("C#_vidx")},
#endif
        { canton_vars, offsetof(canton_vars_t, cur_pwm_duty) ,  2     _P("C#_pwm")},
        
        { NULL, sizeof(canton_vars_t), 0  _P(NULL)}
};

// ------------------------------------------------------

#define USE_CANTON(_idx) \
		const conf_canton_t   *cconf = conf_canton_get(_idx); \
		canton_vars_t         *cvars = &canton_vars[_idx];



static void canton_set_pwm(int cn, const conf_canton_t *c, canton_vars_t *v,  int8_t dir, int duty);
void canton_set_volt(int cn, const conf_canton_t *c, canton_vars_t *v, int voltidx);



//--------------------------------------------
// global run mode, each tasklet implement this
//static runmode_t run_mode = 0;
//static uint8_t testerAddr;
//--------------------------------------------



static void canton_reset(void)
{
	for (int i = 0; i<NUM_CANTONS; i++) {
		USE_CANTON(i)
		cvars->cur_dir = 99;
		canton_set_pwm(i, cconf, cvars, 0, 0);
		canton_set_volt(i, cconf, cvars,  7);
	}
}

static void canton_init(void)
{
	canton_reset();
	bemf_reset();
}

static void canton_enter_runmode(runmode_t m)
{
	bemf_run_mode = m;
}

//--------------------------------------------

static void handle_canton_cmd(int cidx, msg_64_t *m)
{
	if ((m->cmd == CMD_BEMF_OFF) || (m->cmd==CMD_BEMF_ON)) {
		itm_debug1(DBG_LOWCTRL, "msg-bemf", m->to);
		bemf_msg(m);
		return;
	}


	USE_CANTON(cidx)
	if (!cvars) {
		itm_debug1(DBG_LOWCTRL|DBG_ERR, "no cvars", cidx);
		return;
	}
	switch (m->cmd) {
	case CMD_STOP:
		itm_debug1(DBG_LOWCTRL, "CMD STOP", 0);
		canton_set_pwm(cidx, cconf, cvars, 0, 0);
		canton_set_volt(cidx, cconf, cvars,  7);
		break;
	case CMD_SETVPWM:
		itm_debug3(DBG_LOWCTRL, "SETVPWM", cidx, m->v1u, m->v2);
		canton_set_pwm(cidx, cconf, cvars, SIGNOF0(m->v2), abs(m->v2));
		canton_set_volt(cidx, cconf, cvars,  m->v1u);
		break;
	default:
		itm_debug1(DBG_LOWCTRL, "not handled msg", m->cmd);
		break;
	}
}

static void handle_msg_off(_UNUSED_ msg_64_t *m)
{
	// no handling
}



static void handle_msg_normal(msg_64_t *m)
{
    int cidx = -1;
    if (!MA0_IS_CANTON(m->to)) return;
    cidx = m->subc; // MA_GET_CANTON_NUM(m->to);
    handle_canton_cmd(cidx, m);
}

static void handle_msg_detect1(msg_64_t *m)
{
    handle_msg_normal(m);
}

static void handle_msg_detect2(msg_64_t *m)
{
    if (IS_BROADCAST(m->to)) {
        return;
    }
    if (!MA0_IS_CANTON(m->to)) {
        return;
    }
    int cidx = m->subc; // MA_GET_CANTON_NUM(m->to);
    const conf_canton_t *cconf = conf_canton_get(cidx);
    canton_vars_t *cvars = &canton_vars[cidx];
    
    switch (m->cmd) {
        case CMD_START_DETECT_TRAIN:
            canton_set_volt(cidx, cconf, cvars,  7);
            m->cmd = CMD_BEMF_ON;
            bemf_msg(m);
            canton_set_pwm(cidx, cconf, cvars, 1, m->v1u); // % PWM
            break;
        case CMD_STOP_DETECT_TRAIN:
            m->cmd = CMD_BEMF_OFF;
            bemf_msg(m);
            canton_set_volt(cidx, cconf, cvars,  7);
            canton_set_pwm(cidx, cconf, cvars, 0, 0);
        default:
            break;
    }
   
}

static void handle_msg_cantontest(msg_64_t *m)
{
	int cidx = -1;
	if (IS_BROADCAST(m->to)) {
		cidx = -1;
	} else if (MA0_IS_CANTON(m->to)) {
		cidx = m->subc; // MA_GET_CANTON_NUM(m->to);
	} else {
		itm_debug1(DBG_LOWCTRL, "not handled msg", m->cmd);
		return;
	}
    if (CMD_SETVPWM == m->cmd) {
    	// in test mode, forward CMD_SETVPWM to UI for display
    	// handling actually done in handle_canton_cmd()
    	msg_64_t m2 = *m;
    	m2.from = m2.to;
    	m2.to = MA3_UI_GEN;
    	mqf_write_from_canton(&m2);
    }
    if (cidx>=0) handle_canton_cmd(cidx, m);
    else {
    	// broadcast
    	for (int i=0; i<NUM_CANTONS; i++) {
    		handle_canton_cmd(i, m);
    	}
    }
}


// ------------------------------------------------------------


// #pragma mark -

#ifndef TRAIN_SIMU

HAL_StatusTypeDef my_HAL_TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t Channel)
{
  /* Check the parameters */
  assert_param(IS_TIM_CCX_INSTANCE(htim->Instance, Channel));

  /* Disable the Capture compare channel */
  TIM_CCxChannelCmd(htim->Instance, Channel, TIM_CCx_DISABLE);

  if (IS_TIM_BREAK_INSTANCE(htim->Instance) != RESET)
  {
    /* Disable the Main Output */
    __HAL_TIM_MOE_DISABLE(htim);
  }

  /* Disable the Peripheral */
  //__HAL_TIM_DISABLE(htim);

  /* Change the htim state */
  htim->State = HAL_TIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}


#define USE_PWM_STOP 0
/*
 * it seems that output goes to high impedence when we stop pwm ????
 */

#if NEW_ADC_AVG
// with polarity low on channels
#define CNT_OFF 200
#define CNT_ON(_v) (200-(_v))
#else
#define CNT_OFF 0
#define CNT_ON(_v) (_v)
#endif

static void canton_set_pwm(int cidx, const conf_canton_t *c, canton_vars_t *v,  int8_t dir, int duty)
{
	if (c->reverse) {
		dir=-dir;
	}
	itm_debug3(DBG_LOWCTRL, "c/set_pwm", cidx, dir, duty);
	int t = 2*duty; // with centered pwm (or normal)

	if ((v->cur_dir == dir) && (v->cur_pwm_duty==duty)) {
		itm_debug1(DBG_LOWCTRL, "c/same", duty);
		return;
	}

	TIM_HandleTypeDef *pwm_timer = CantonTimerHandles[c->pwm_timer_num];
	if (!pwm_timer) {
		itm_debug2(DBG_LOWCTRL|DBG_ERR, "c/notim", cidx, c->pwm_timer_num);
		return;
	}
	if (v->cur_dir != dir) {
		v->cur_dir = dir;
		itm_debug3(DBG_LOWCTRL, "set dir", dir, c->ch0, c->ch1);
		if (dir>0) {
			if (USE_PWM_STOP) my_HAL_TIM_PWM_Stop(pwm_timer, c->ch1);
			else HAL_TIM_PWM_Start(pwm_timer, c->ch1);
			HAL_TIM_PWM_Start(pwm_timer, c->ch0);
		} else if (dir<0) {
			if (USE_PWM_STOP) my_HAL_TIM_PWM_Stop(pwm_timer, c->ch0);
			else HAL_TIM_PWM_Start(pwm_timer, c->ch0);
			HAL_TIM_PWM_Start(pwm_timer, c->ch1);
		} else {
			if (USE_PWM_STOP) {
				my_HAL_TIM_PWM_Stop(pwm_timer, c->ch0);
				my_HAL_TIM_PWM_Stop(pwm_timer, c->ch1);
			} else {
				HAL_TIM_PWM_Start(pwm_timer, c->ch0);
				HAL_TIM_PWM_Start(pwm_timer, c->ch1);
			}
		}
	}
	if (!dir) {
		duty = 0;
		if (USE_PWM_STOP) {
			v->cur_pwm_duty = 0;
			return;
		}
	}
	v->cur_pwm_duty = duty;
	uint32_t chon;
	uint32_t choff;
	if (dir>0) {
		chon = c->ch0;
		choff = c->ch1;
	} else {
		chon = c->ch1;
		choff = c->ch0;
	}

	if (!USE_PWM_STOP) {
		switch (choff) {
		case TIM_CHANNEL_1:
			itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH1/CCR1", 0);
			pwm_timer->Instance->CCR1 = CNT_OFF;
			break;
		case TIM_CHANNEL_2:
			itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH2/CCR1", 0);
			pwm_timer->Instance->CCR2 = CNT_OFF;
			break;
		case TIM_CHANNEL_3:
			itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH3/CCR1", 0);
			pwm_timer->Instance->CCR3 = CNT_OFF;
			break;
		case TIM_CHANNEL_4:
			itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH4/CCR1", 0);
			pwm_timer->Instance->CCR4 = CNT_OFF;
			break;
		default:
			canton_error(ERR_BAD_PARAM_TIM, "bad timer channel");
			break;
		}
	}
	switch (chon) {
	case TIM_CHANNEL_1:
		itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH1/CCR1", t);
		pwm_timer->Instance->CCR1 = CNT_ON(t);
		break;
	case TIM_CHANNEL_2:
		itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH2/CCR1", t);
		pwm_timer->Instance->CCR2 = CNT_ON(t);
		break;
	case TIM_CHANNEL_3:
		itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH3/CCR1", t);
		pwm_timer->Instance->CCR3 = CNT_ON(t);
		break;
	case TIM_CHANNEL_4:
		itm_debug1(DBG_LOWCTRL|DBG_TIM, "CH4/CCR1", t);
		pwm_timer->Instance->CCR4 = CNT_ON(t);
		break;
	default:
		canton_error(ERR_BAD_PARAM_TIM, "bad timer channel");
		break;
	}

}
void canton_set_volt(int cidx, const conf_canton_t *c, canton_vars_t *v, int voltidx)
{
	v->cur_voltidx = voltidx;
    v->selected_centivolt =  (c->volts_cv[v->cur_voltidx]);
	itm_debug3(DBG_LOWCTRL, "c/set_volt", cidx, voltidx, v->selected_centivolt);

    if ((0)) debug_info('C', 0, "SET VLT ", voltidx,  v->selected_centivolt,0);
    if ((0)) debug_info('C', 0, "VLT BIT ", (voltidx & 0x03) ? 1 : 0,
    				(voltidx & 0x02) ? 1 : 0,
    				(voltidx & 0x01) ? 1 : 0);

	HAL_GPIO_WritePin(c->volt_port_b0, c->volt_b0, (voltidx & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(c->volt_port_b1, c->volt_b1, (voltidx & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
	HAL_GPIO_WritePin(c->volt_port_b2, c->volt_b2, (voltidx & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

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
void __attribute__((weak)) train_simu_canton_volt(int numcanton, int voltidx, int vlt100)
{
    
}

void __attribute__((weak)) train_simu_canton_set_pwm(int numcanton, int8_t dir, int duty)
{
}
void canton_set_volt(int n, const conf_canton_t *c, canton_vars_t *v, int voltidx)
{
    int vlt = c->volts_cv[voltidx];
    // int n = canton_idx(v);
    v->cur_voltidx = voltidx;
    v->selected_centivolt =  (c->volts_cv[v->cur_voltidx]);
    train_simu_canton_volt(n, voltidx, vlt);
}
void canton_set_pwm(int n, const conf_canton_t *c, canton_vars_t *v,  int8_t dir, int duty)
{
    //int n = canton_idx(v);
    v->cur_pwm_duty = duty;
    v->cur_dir = dir;
    train_simu_canton_set_pwm(n, dir, duty);
}

 



#endif

#if 0
// moved to spdctl
#define MAX_PVI (NUM_VOLTS_VAL-1)

int volt_index(uint16_t mili_power,
		const canton_config_t *c1, //canton_vars_t *v1,
		_UNUSED_ const canton_config_t *c2, //canton_vars_t *v2,
		int *pvi1, int *pvi2,
		train_volt_policy_t pol)
{
	int duty=0;
	*pvi1 = MAX_PVI;
	*pvi2 = MAX_PVI;

	//if (mili_power <0)    return canton_error_rc(0, ERR_BAD_PARAM_MPOW, "negative milipower");
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
#endif

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

