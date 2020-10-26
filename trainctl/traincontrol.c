/*
 * traincontrol.c
 *
 *  Created on: Oct 3, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

/*
#ifndef TRAIN_SIMU
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#else
#include "train_simu.h"
#endif
*/


#include "misc.h"
#include "canton.h"
#include "inertia.h"
#include "pidctl.h"
#include "train.h"
#include "turnout.h"
#include "traincontrol.h"
#include "railconfig.h"
#include "auto1.h"


static void _set_speed(const train_config_t *cnf, train_vars_t *vars, int16_t v);

volatile adc_buffer_t train_adc_buffer[2*NUM_LOCAL_CANTONS];

static void process_adc(volatile adc_buffer_t *buf, int32_t ticks);
static void train_periodic_control(const train_config_t *cnf, train_vars_t *vars, int32_t dt);

static volatile int stop_all = 0;
static int calibrating=0;
void calibrate_periodic(uint32_t tick, uint32_t dt, uint32_t notif_Flags);


void train_run_tick( uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	if (notif_flags & NOTIF_STARTUP) {
		static int n=0;
		if (n) {
			runtime_error(ERR_STRANGE, "notif startup twice");
		}
		n++;
	}
	if (notif_flags & NOTIF_NEW_ADC_1) {
		if (notif_flags & NOTIF_NEW_ADC_2) {
			runtime_error(ERR_DMA, "both NEW_ADC1 and NEW_ADC2");
		}
		process_adc(train_adc_buffer, dt);
	}
	if (notif_flags & NOTIF_NEW_ADC_2) {
		process_adc(train_adc_buffer+NUM_LOCAL_CANTONS, dt);
	}

	turnout_tick();

	if (calibrating) {
		calibrate_periodic(tick, dt, notif_flags);
		return;
	}


	/* per train proces */
    //debug_info(0, "TRAIN", tick, dt);
	for (int i=0; i<NUM_TRAINS; i++) {
		if (stop_all) break;
		const train_config_t *c = get_train_cnf(i);
		train_vars_t   *v = get_train_vars(i);
		train_periodic_control(c, v, dt);
	}
}

static void process_adc(volatile adc_buffer_t *buf, int32_t ticks)
{
	for (int i=0; i<NUM_LOCAL_CANTONS; i++) {
		if (stop_all) break;
		// process intensity / presence
		// process BEMF
		const canton_config_t *cnf  = get_canton_cnf(i);
		canton_vars_t   *vars = get_canton_vars(i);
		if ((vars->status > canton_free) || calibrating) {
			canton_bemf(cnf, vars, buf[i].voffB , buf[i].voffA, buf[i].vonB , buf[i].vonA);
		}
	}
}

static int num_train_periodic_control = 0;
static int num_set_speed = 0;
void __attribute__((weak))  notif_target_bemf(const train_config_t *cnf, train_vars_t *vars, int32_t val)
{
    
}

static void train_periodic_control(const train_config_t *cnf, train_vars_t *vars, int32_t dt)
{
	if (stop_all) return;

	num_train_periodic_control++;


	int16_t v = vars->target_speed;

	if (cnf->enable_inertia) {
		int changed;
		vars->inertiavars.target = vars->target_speed;
		// int16_t inertia_value(inertia_config_t *cnf, inertia_vars_t *var, uint16_t elapsed_ticks, int *pchanged);
		v = inertia_value(&cnf->inertiacnf, &vars->inertiavars, dt, &changed);
	}
    
    if (cnf->enable_pid) {
        // corresponding BEMF target
        // 100% = 1V => ((1 / 4.54) / 3.3) * 4096 = 273
        int32_t tbemf = 280*v/100;
        pidctl_set_target(&cnf->pidcnf, &vars->pidvars, tbemf);
        notif_target_bemf(cnf, vars, tbemf);
    }

    if (cnf->enable_pid) {
        canton_vars_t *cv = get_canton_vars(vars->current_canton);
        if ((vars->target_speed == 0) && (abs(cv->bemf_centivolt)<15)) {
        	//debug_info('T', 0, "ZERO", cv->bemf_centivolt,0, 0);
			pidctl_reset(&cnf->pidcnf, &vars->pidvars);
        	v = 0;
        } else {
        	//const canton_config_t *cc = get_canton_cnf(vars->current_canton);
        	int32_t bemf = cv->bemf_centivolt;
        	if (cnf->bemfIIR) {
        		vars->bemfiir = (80*vars->bemfiir + 20*bemf)/100;
        		bemf = vars->bemfiir;
        	}

        	if ((0)) dt=50; // XXX

        	if (bemf>MAX_PID_VALUE)  bemf=MAX_PID_VALUE; // XXX
        	if (bemf<-MAX_PID_VALUE) bemf=-MAX_PID_VALUE;

        	int32_t v2 = pidctl_value(&cnf->pidcnf, &vars->pidvars, bemf, dt);

        	v2 = (v2>100) ? 100 : v2;
        	v2 = (v2<-100) ? -100: v2;
        	v = (int16_t)v2;

        }


    }
    int changed = (vars->last_speed != v);
    vars->last_speed = v;

    if (changed) {
    	if (cnf->en_spd2pow) {
    		// [0-100] -> [min_pwm .. MAX_PWM]
    		int s = SIGNOF(v);
    		int a = abs(v);
    		int v2 = (a>0.1) ? a * (MAX_PWM-cnf->min_power)/100 + cnf->min_power : 0;
    		v = s * v2;
    	}
        _set_speed(cnf, vars, v);
    }
    if (cnf->notify_speed) {
    		struct spd_notif n;
    		n.sv100 = v;
    		n.pid_target = vars->pidvars.target_v;
    		canton_vars_t *cv1 = get_canton_vars(vars->current_canton);
    		n.bemf_centivolt = cv1->bemf_centivolt;
    		train_notif(train_idx(vars), 'V', (void *)&n, sizeof(n));
    	}

    /* estimate speed/position with bemf */
    if ((1)) {
        //const canton_config_t cc =  get_canton_cnf(vars->current_canton);
    	canton_vars_t *cv = get_canton_vars(vars->current_canton);
        int32_t b = cv->bemf_centivolt;
        if (abs(b)<25) b = 0;
        // TODO: BEMF to speed
        vars->position_estimate += b;
        if (cnf->notify_pose) {
    		train_notif(train_idx(vars), 'i', (void *)&vars->position_estimate, sizeof(int32_t));
        }
    }
}

static void _set_speed(const train_config_t *cnf, train_vars_t *vars, int16_t sv100)
{
    const canton_config_t *c1;
    const canton_config_t *c2;
    canton_vars_t *cv1;
    canton_vars_t *cv2;
    
    num_set_speed++;
    c1 =  get_canton_cnf(vars->current_canton);
    c2 =  get_canton_cnf(vars->next_canton);
    cv1 = get_canton_vars(vars->current_canton);
    cv2 = get_canton_vars(vars->next_canton);
    
    if (!c1 || !cv1) {
        train_error(ERR_CANTON_NONE, "no canton");
        return;
    }
    int pvi1, pvi2;
    int sig = SIGNOF(sv100);
    uint16_t v = abs(sv100);
    uint16_t pwm_duty = volt_index(v*10 /* mili*/,
                                   c1, cv1, c2, cv2,
                                   &pvi1, &pvi2, cnf->volt_policy);
    if (pvi1 != vars->cur_c1_volt_idx) {
        vars->cur_c1_volt_idx = pvi1;
        canton_set_volt(c1, cv1, pvi1);
        cv1->fix_bemf = cnf->fix_bemf;
    }
    if (pvi2 != vars->cur_c2_volt_idx) {
		vars->cur_c2_volt_idx = pvi2;
		if (cv2) canton_set_volt(c2, cv2, pvi2);
        if (cv2) cv2->fix_bemf = cnf->fix_bemf;
	}
	int dir1 = sig * vars->current_canton_dir;
	int dir2 = sig * vars->next_canton_dir;
	canton_set_pwm(c1, cv1, dir1, pwm_duty);
	if (cv2) canton_set_pwm(c2, cv2, dir2, pwm_duty);


}


/* =========================================================================== */

int train_set_target_speed(int numtrain, int16_t target)
{
	if (calibrating) return 1;
	//const train_config_t *c = get_train_cnf(numtrain);
	train_vars_t *vars   = get_train_vars(numtrain);
	if (!vars) return -1;
	vars->target_speed = target;
	/*if (0 == target) {
		if (c->enable_pid) {
			pidctl_reset(&c->pidcnf, &vars->pidvars);
		}
	}*/
	return 0;
}

void train_stop_all(void)
{
	stop_all = 1;
	auto1_reset();
	railconfig_setup_default();
	for (int i=0; i<NUM_CANTONS; i++) {
		const canton_config_t *c = get_canton_cnf(i);
		canton_vars_t   *v = get_canton_vars(i);
		canton_set_pwm(c, v,  0, 0);
		canton_set_volt(c, v, 15);
	}
	railconfig_setup_default();
	debug_info('G', 0, "STOPALL", 0,0, 0);
	stop_all = 0;
}

typedef struct {
	int16_t spd;
	int n;
} calib_t;
#define CALIB_NUM_VAL 50

static calib_t calib;

void calibrate_bemf(void)
{
	if (calibrating) return;
    debug_info('G', 0, "CAL/START", 0, 0, 0);
	calibrating=1;
	calib.n = -1;
	calib.spd = -MAX_PWM-1; // -91;
}


#if INCLUDE_CALIB

static void cantons_start_calib(void);
static void cantons_stop_calib(void);


void calibrate_periodic(uint32_t tick, uint32_t dt, uint32_t notif_Flags)
{
	if (calib.n == CALIB_NUM_VAL) {
		cantons_stop_calib();
		calib.n = -1;
	}
	if (calib.n == -1) {
		calib.spd++;
	    debug_info('G', 0, "CAL", calib.spd, 0, 0);
		if (calib.spd>MAX_PWM) {
			// calib finished
			uint8_t c;
			global_notif('k', &c, 0);
			calibrating = 0;
			return;
		}
		calib.n = 0;
		cantons_start_calib();
	}
	calib.n++;
}

static void cantons_start_calib(void)
{
	train_volt_policy_t vpol = vpolicy_normal;

	for (int i=0; i<NUM_CANTONS; i++) {
		const canton_config_t *c = get_canton_cnf(i);
		canton_vars_t   *v       = get_canton_vars(i);
		canton_reset_calib(c, v, calib.spd);
		int vltidx; int dummy;
		uint16_t pwm_duty = volt_index(abs(calib.spd)*10, /* mili*/
		                                   c, v, NULL, NULL,
		                                   &vltidx, &dummy, vpol);
		canton_set_volt(c, v, vltidx);
		canton_set_pwm(c, v, SIGNOF(calib.spd), pwm_duty);
	}
}
static void cantons_stop_calib(void)
{
	for (int i=0; i<NUM_CANTONS; i++) {
		const canton_config_t *c = get_canton_cnf(i);
		canton_vars_t   *v       = get_canton_vars(i);
		canton_end_calib(c, v, calib.spd, CALIB_NUM_VAL);
	}
}
#else
void calibrate_periodic(uint32_t tick, uint32_t dt, uint32_t notif_Flags)
{

}
#endif




