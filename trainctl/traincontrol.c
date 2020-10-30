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


/* traincontrol.h : main control of train :
 * 			target_speed -> inertia -> BEMF feedback -> volt + pwm
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
#include "txrxcmd.h"


static void _set_speed(const train_config_t *cnf, train_vars_t *vars, int16_t v);

volatile adc_buffer_t train_adc_buffer[2*NUM_LOCAL_CANTONS];

static void process_adc(volatile adc_buffer_t *buf, int32_t ticks);
static void train_periodic_control(int numtrain, int32_t dt);

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
		txframe_send_stat();
		return;
	}


	/* per train proces */
    //debug_info(0, "TRAIN", tick, dt);
	for (int i=0; i<NUM_TRAINS; i++) {
		if (stop_all) break;
		train_periodic_control(i, dt);
	}
	txframe_send_stat();
}

static void process_adc(volatile adc_buffer_t *buf, int32_t ticks)
{
	for (int i=0; i<NUM_LOCAL_CANTONS; i++) {
		if (stop_all) break;
		// process intensity / presence
		// process BEMF
		USE_CANTON(i)  // cconf cvars
		if ((cvars->status > canton_free) || calibrating) {
			canton_bemf(cconf, cvars, buf[i].voffB , buf[i].voffA, buf[i].vonB , buf[i].vonA);
		}
	}
}

static int num_train_periodic_control = 0;
static int num_set_speed = 0;
void __attribute__((weak))  notif_target_bemf(const train_config_t *cnf, train_vars_t *vars, int32_t val)
{
    
}

static void train_periodic_control(int numtrain, int32_t dt)
{
	if (stop_all) return;

	num_train_periodic_control++;

	USE_TRAIN(numtrain)	// tconf tvars

	int16_t v = tvars->target_speed;

    // inertia before PID
	if (1==tconf->enable_inertia) {
		int changed;
		tvars->inertiavars.target = tvars->target_speed;
		v = inertia_value(&tconf->inertiacnf, &tvars->inertiavars, dt, &changed);
	}
    
    if (tconf->enable_pid) {
        // corresponding BEMF target
        // 100% = 1.5V
        int32_t tbemf = 150*v/100;
        pidctl_set_target(&tconf->pidcnf, &tvars->pidvars, tbemf);
        notif_target_bemf(tconf, tvars, tbemf);
    }

    if (tconf->enable_pid) {
        canton_vars_t *cv = get_canton_vars(tvars->current_canton);
        if ((tvars->target_speed == 0) && (abs(cv->bemf_centivolt)<10)) {
        	//debug_info('T', 0, "ZERO", cv->bemf_centivolt,0, 0);
			pidctl_reset(&tconf->pidcnf, &tvars->pidvars);
        	v = 0;
        } else {
        	//const canton_config_t *cc = get_canton_cnf(vars->current_canton);
        	int32_t bemf = cv->bemf_centivolt;
        	if (tconf->bemfIIR) {
        		tvars->bemfiir = (80*tvars->bemfiir + 20*bemf)/100;
        		bemf = tvars->bemfiir;
        	}

        	if (bemf>MAX_PID_VALUE)  bemf=MAX_PID_VALUE; // XXX
        	if (bemf<-MAX_PID_VALUE) bemf=-MAX_PID_VALUE;

        	int32_t v2 = pidctl_value(&tconf->pidcnf, &tvars->pidvars, bemf, dt);

        	v2 = (v2>100) ? 100 : v2;
        	v2 = (v2<-100) ? -100: v2;
        	v = (int16_t)v2;

        }
    }
    if (tconf->postIIR) {
        tvars->v_iir = (80*tvars->v_iir+20*v)/100;
        v = tvars->v_iir;
    }
    // or inertia after PID
    if (2==tconf->enable_inertia) {
        tvars->inertiavars.target = v;
        v = inertia_value(&tconf->inertiacnf, &tvars->inertiavars, dt, NULL);
    }
    int changed = (tvars->last_speed != v);
    tvars->last_speed = v;

    if (changed) {
    	if (tconf->en_spd2pow) {
    		// [0-100] -> [min_pwm .. MAX_PWM]
    		int s = SIGNOF(v);
    		int a = abs(v);
    		int v2 = (a>1) ? a * (MAX_PWM-tconf->min_power)/100 + tconf->min_power : 0;
    		v = s * v2;
    	}
        _set_speed(tconf, tvars, v);
    }
    if (tconf->notify_speed) {
    		struct spd_notif n;
    		n.sv100 = v;
    		n.pid_target = tvars->pidvars.target_v;
    		canton_vars_t *cv1 = get_canton_vars(tvars->current_canton);
    		n.bemf_centivolt = cv1->bemf_centivolt;
    		train_notif(numtrain, 'V', (void *)&n, sizeof(n));
    	}

    /* estimate speed/position with bemf */
    if ((1)) {
    	canton_vars_t *cv = get_canton_vars(tvars->current_canton);
        int32_t b = cv->bemf_centivolt;
        if (abs(b)<25) b = 0;
        // TODO: BEMF to speed. currently part of it is done in convert_to_centivolt
        //       but we assume speed is really proportional to BEMF
        tvars->position_estimate += b;
        if (tconf->notify_pose) {
    		train_notif(numtrain, 'i', (void *)&tvars->position_estimate, sizeof(int32_t));
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
	USE_TRAIN(numtrain) // tconf tvars
	(void)tconf; // unused
	if (!tvars) return -1;
	tvars->target_speed = target;

	/* shall we do something special when target is 0 ?
	 * this is commented out for now, because I like the slow stop without it..
	 * if (0 == target) {
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
		USE_CANTON(i) // cconf cvars
		canton_set_pwm(cconf, cvars,  0, 0);
		canton_set_volt(cconf, cvars, 15);
	}
	railconfig_setup_default();
	debug_info('G', 0, "STOPALL", 0,0, 0);
	stop_all = 0;
}

/*
 * calibration of BEMF
 * first attempt is to measure BEMF when no train is on track - tracjs are capacitive, and thus power don't
 * go down to 0 during "off" time, though this is not much a problem as a 3.3K resistor is being added
 * between the rails. But behaviour is probably different with a train
 * The following should be measured / taken in account :
 * - capacitive behaviour of the rails
 * - difference between BEMF measurement on each rails (thus in each direction), due to different resistors
 * - non-linearity ??
 */
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




