/*
 * spdctl.c
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

#include <stdint.h>
#include <memory.h>

#include "../misc.h"
#include "../msg/trainmsg.h"


#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif

#include "../low/canton.h"
#include "inertia.h"
#include "pidctl.h"
//#include "train.h"
#include "../low/turnout.h"
#include "spdctl.h"
//#include "railconfig.h"
//#include "auto1.h"
//#include "txrxcmd.h"
#include "statval.h"

#include "../config/conf_train.h"
#include "../config/conf_canton.h"


static int volt_index(uint16_t mili_power,
		const conf_canton_t *c1, //canton_vars_t *v1,
		const conf_canton_t *c2, //canton_vars_t *v2,
		int *pvi1, int *pvi2, train_volt_policy_t pol);

// ----------------------------------------------------------------------------------
// global run mode, each tasklet implement this
static runmode_t run_mode = 0;
static uint8_t testerAddr;


// ----------------------------------------------------------------------------------


//static void process_adc(volatile adc_buffer_t *buf, int32_t ticks);
static void train_periodic_control(int numtrain, uint32_t dt);

static volatile int stop_all = 0;
static int calibrating=0;
void calibrate_periodic(uint32_t tick, uint32_t dt, uint32_t notif_Flags);

//static void highlevel_tick(void);

uint32_t train_tick_last_dt = 0;
uint32_t train_ntick = 0;



// ------------------------------------------------------


typedef struct train_vars {
	int16_t target_speed;	// always >= 0

	int32_t bemf_mv;		// millivolt
	pidctl_vars_t pidvars;
	inertia_vars_t inertiavars;

    uint8_t C1x;	// current canton adress
	uint8_t C2x; // next canton address
	// TODO add C2alt, alternative next canton (manual turnout / detect defect in turnout)
	int8_t  C1_dir; // -1 or +1
	int8_t  C2_dir;

	int16_t last_speed;
	int16_t prev_last_speed;

	uint16_t C1_cur_volt_idx;
	uint16_t C2_cur_volt_idx;

	int32_t position_estimate;
    int32_t pose_trig0;
    int32_t pose_trigU1;
	int32_t bemfiir;
    int16_t v_iir;

    uint8_t	c2hicnt:4;	// number of bemf_c2>bemf_c1, before setting c2bemf and notify higher layer
    uint8_t c2bemf:1; 	// 1=bemf shall be taken on C2
} train_vars_t;


static train_vars_t trspc_vars[NUM_TRAINS]={0};

// ------------------------------------------------------


const stat_val_t statval_spdctrl[] = {
    { trspc_vars, offsetof(train_vars_t, target_speed), 2       _P("T#_spd_target_speed")},
    { trspc_vars, offsetof(train_vars_t, bemf_mv), 4            _P("T#_bemf_mv")},
    { trspc_vars, offsetof(train_vars_t, pidvars.target_v), 4   _P("T#_pid_target_v")},
    { trspc_vars, offsetof(train_vars_t, pidvars.last_err), 4   _P("T#_pid_last_err")},
    { trspc_vars, offsetof(train_vars_t, pidvars.sume), 4       _P("T#_pid_sum_e")},
    { trspc_vars, offsetof(train_vars_t, pidvars.target_v), 4   _P("T#_pid_target")},
#ifndef REDUCE_STAT
    { trspc_vars, offsetof(train_vars_t, inertiavars.target), 2 _P("T#_ine_t")},
    { trspc_vars, offsetof(train_vars_t, inertiavars.cur100), 2    _P("T#_ine_c")},
    { trspc_vars, offsetof(train_vars_t, last_speed), 2         _P("T#_spd_curspeed")},
    { trspc_vars, offsetof(train_vars_t, position_estimate), 4  _P("T#_pose")},
    { trspc_vars, offsetof(train_vars_t, pose_trig0), 4          _P("T#_pose_trig1")},
    { trspc_vars, offsetof(train_vars_t, pose_trigU1), 4          _P("T#_pose_trig2")},
#endif
    { NULL, sizeof(train_vars_t), 0 _P(NULL)}
};


// ------------------------------------------------------


#define USE_TRAIN(_idx) \
		_UNUSED_ const conf_train_t *tconf = conf_train_get(_idx); \
		train_vars_t                *tvars = &trspc_vars[_idx];


static void train_periodic_control(int numtrain, uint32_t dt);
static void _set_speed(int tidx, const conf_train_t *cnf, train_vars_t *vars);
static void spdctl_reset(void);
static void set_c1_c2(int num, train_vars_t *tvars, uint8_t c1, int8_t dir1, uint8_t c2, int8_t dir2);

static void pose_check_trig(int numtrain, train_vars_t *tvars, int32_t lastincr);


static void spdctl_reset(void)
{
	memset(trspc_vars, 0, sizeof(trspc_vars));
	for (int  i = 0; i<NUM_TRAINS; i++) {
		trspc_vars[i].C1x = 0xFF;
		trspc_vars[i].C2x = 0xFF;
	}
}

volatile int16_t oscillo_t0bemf = 0;
volatile int16_t oscillo_t1bemf = 0;
extern volatile int oscillo_trigger_start;

void spdctl_run_tick(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, uint32_t dt)
{
	train_tick_last_dt = dt;
	train_ntick++;

	itm_debug1(DBG_SPDCTL ,"------- tk", (int) notif_flags);
	static int first=1;
	if (first) {
		first = 0;
		spdctl_reset();
       
	}
	/* process messages */
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_spdctl(&m);
		if (rc) break;
        
        switch (m.cmd) {
        case CMD_TRIG_OSCILLO:
        	oscillo_trigger_start = 1;
        	break;
        case CMD_RESET:
        	// FALLTHRU
        case CMD_EMERGENCY_STOP:
            spdctl_reset();
            break;
        case CMD_SETRUN_MODE:
        	if (m.v1u != run_mode) {
        		run_mode = (runmode_t) m.v1u;
        		testerAddr = m.from;
        		first = 1;
        	}
            break;
        default:
        	break;
        }

        switch (run_mode) {
        case runmode_normal: break;

        default: // FALLTHRU
        case runmode_off:
        	continue;
        }

        // mode normal
        if (MA1_ADDR_IS_TRAIN_ADDR(m.to)) {
            int tidx = MA1_TRAIN(m.to);
            USE_TRAIN(tidx)
            int cfrom = FROM_CANTON(m);
            switch (m.cmd) {
                case CMD_BEMF_NOTIF:
                    if (cfrom == tvars->C1x) {
                    	if (!tidx) oscillo_t0bemf = m.v1;
                    	else if (1==tidx) oscillo_t1bemf = m.v1;

                        itm_debug3(DBG_PID, "st bemf", tidx, m.v1, m.from);
                        if (!tvars->c2bemf) tvars->bemf_mv = m.v1;
                        break;
                    } else if (cfrom == tvars->C2x) {
                        itm_debug3(DBG_PID|DBG_CTRL, "c2 bemf", tidx, m.v1, m.from);
                        if (tvars->c2bemf) {
                        	tvars->bemf_mv = m.v1;
                        	if (!tidx) oscillo_t0bemf = m.v1;
                        	else if (1==tidx) oscillo_t1bemf = m.v1;
                        }
                        else if (abs(m.v1) > abs(tvars->bemf_mv)+300) {
                        	itm_debug3(DBG_PRES|DBG_PRES|DBG_CTRL, "c2_hi", tidx, m.v1, tvars->bemf_mv);
                        	if (tvars->c2hicnt >= 1) {
                        		msg_64_t m = {0};
                            	m.from = MA1_SPDCTL(tidx);
                            	m.to = MA1_CTRL(tidx);
                        		m.cmd = CMD_BEMF_DETECT_ON_C2;
                            	m.v1u = tvars->C2x;
                            	int32_t p = tvars->position_estimate / 100;
                            	if (abs(p)>0x7FFF) {
                            		// TODO: problem here pose is > 16bits
                            		itm_debug1(DBG_POSEC|DBG_ERR, "L pose", p);
                            		p = SIGNOF(p)*0x7FFF;
                            	}
                            	m.v2 = (int16_t) p;
                        		mqf_write_from_spdctl(&m);

                        		tvars->c2hicnt = 0;
                        		tvars->c2bemf = 1;
                        	} else {
                        		tvars->c2hicnt++;
                        	}
                        } else {
                        	tvars->c2hicnt = 0;
                        }
                        // check it ?
                    } else {
                        itm_debug2(DBG_ERR|DBG_PID, "unk bemf", m.v1, m.from);
                        // error
                    }
                    break;
                case CMD_SET_TARGET_SPEED:
                    itm_debug1(DBG_SPDCTL, "set_t_spd", m.v1u);
                    if (!tvars->target_speed && (m.v1u > 10)) {
                    	oscillo_trigger_start = 1;
                    }
                    tvars->target_speed = (int16_t) m.v1u;
                    break;
                case CMD_SET_C1_C2:
                    itm_debug3(DBG_SPDCTL|DBG_CTRL, "set_c1_c2", tidx, m.vbytes[0], m.vbytes[2]);
                    //static void set_c1_c2(int tidx, train_vars_t *tvars, uint8_t c1, int8_t dir1, uint8_t c2, int8_t dir2)
                    set_c1_c2(tidx, tvars, m.vbytes[0], m.vbytes[1], m.vbytes[2], m.vbytes[3]);
                    break;
                case CMD_POSE_SET_TRIG0:
                	itm_debug2(DBG_POSEC, "POSE set0", tidx, m.v32);
                	tvars->pose_trig0 = m.v32*10;
                	// check if already trigg
                	pose_check_trig(tidx, tvars, 0);
                	break;
                case CMD_POSE_SET_TRIG_U1:
                    itm_debug2(DBG_POSEC, "POSE setU1", tidx, m.v32);
                    tvars->pose_trigU1 = m.v32*10;
                    // check if already trigg
                    pose_check_trig(tidx, tvars, 0);
                default:
                    break;
            }
        }
	}
	/* process trains */
	for (int i=0; i<NUM_TRAINS; i++) {
		//itm_debug1(DBG_SPDCTL, "------ pc", i);
		train_periodic_control(i, dt);
	}
}
    

void send_train_stopped(int numtrain, train_vars_t *tvars)
{
    msg_64_t m = {0};
    m.from = MA1_SPDCTL(numtrain);
    m.to = MA1_CTRL(numtrain);
    m.cmd = CMD_STOP_DETECTED;
    m.v32 = tvars->position_estimate / 10; // XXX TODO scale ?
    mqf_write_from_spdctl(&m);
}


static void train_periodic_control(int numtrain, uint32_t dt)
{
	if (stop_all) return;


	USE_TRAIN(numtrain)	// tconf tvars
    if (!tconf) {
        if ((0)) itm_debug1(DBG_SPDCTL, "unconf tr", numtrain);
        return;
    }
	if (!tconf->enabled) {
		//itm_debug1(DBG_SPDCTL, "disabled", numtrain);
		return;
	}
	int16_t v = tvars->target_speed;

	itm_debug2(DBG_SPDCTL, "target", numtrain, v);


    // inertia before PID
	if (1==tconf->enable_inertia) {
		int changed;
		inertia_set_target(numtrain, &tconf->inertia, &tvars->inertiavars, tvars->target_speed);
		//tvars->inertiavars.target = tvars->target_speed;
		v = inertia_value(numtrain, &tconf->inertia, &tvars->inertiavars, &changed);
		itm_debug3(DBG_INERTIA, "inertia", numtrain, tvars->target_speed, v);
	}
    

    if (tconf->enable_pid) {
        // corresponding BEMF target
        // 100% = 1.5V
        int32_t tbemf = 1800*v/100 * tvars->C1_dir;
        //tbemf = tbemf / 4; //XXX why ?? new cables (more capacitance ?)
        // TODO make this divisor a parameter
        pidctl_set_target(&tconf->pidctl, &tvars->pidvars, tbemf);
        // XXXX notif_target_bemf(tconf, tvars, tbemf);
    }

    int32_t bemf_mv = tvars->bemf_mv;
    if (tconf->bemfIIR) {
    	tvars->bemfiir = (tconf->bemfIIR*tvars->bemfiir + (100-tconf->bemfIIR)*bemf_mv)/100;
    	bemf_mv = tvars->bemfiir;
    }
    if (tconf->enable_pid) {
    	if (tvars->target_speed) {
    		tvars->pidvars.stopped = 0;
    	}
        if (!tvars->pidvars.stopped && (tvars->target_speed == 0) && (abs(tvars->bemf_mv)<100)) {
    		itm_debug1(DBG_PID, "stop", 0);
			pidctl_reset(&tconf->pidctl, &tvars->pidvars);
			debug_info('T', numtrain, "STOP_PID", 0,0, 0);
			tvars->pidvars.stopped = 1;
            send_train_stopped(numtrain, tvars);
            
        	v = 0;
        } else if (tvars->pidvars.stopped) {
    		itm_debug2(DBG_PID, "stopped", numtrain, v);
        	v = 0;
        } else {
        	itm_debug3(DBG_PID, "pid", numtrain, bemf_mv, v);
        	if (bemf_mv>MAX_PID_VALUE)  {
        		itm_debug3(DBG_PID|DBG_SPDCTL, "MAX_PID", numtrain, bemf_mv, MAX_PID_VALUE);
        		bemf_mv = MAX_PID_VALUE; // XXX
        	}
        	if (bemf_mv<-MAX_PID_VALUE) {
        		itm_debug3(DBG_PID|DBG_SPDCTL, "MAX_PID", numtrain, bemf_mv, MAX_PID_VALUE);
        		bemf_mv = -MAX_PID_VALUE;
        	}

        	int32_t v2 = pidctl_value(&tconf->pidctl, &tvars->pidvars, bemf_mv)/10; //XXX
        	int32_t v3;
        	v3 = (v2>100) ? 100 : v2;
        	v3 = (v3<-100) ? -100: v3;
        	itm_debug3(DBG_PID, "pid/r", numtrain, v3, v2);
        	v = (int16_t)v3 * tvars->C1_dir; // because it will be multiplied again when setting pwm
        }
    }
    if (tconf->postIIR) {
        tvars->v_iir = (tconf->postIIR*tvars->v_iir+(100-tconf->postIIR)*v)/100;
        v = tvars->v_iir;
    }
    // or inertia after PID
    if (2==tconf->enable_inertia) {
		inertia_set_target(numtrain, &tconf->inertia, &tvars->inertiavars, v);
        //tvars->inertiavars.target = v;
        v = inertia_value(numtrain, &tconf->inertia, &tvars->inertiavars, NULL);
    }

    if (tconf->en_spd2pow) {
    	// [0-100] -> [min_pwm .. MAX_PWM]
    	int s = SIGNOF(v);
    	int a = abs(v);
    	int v2 = (a>1) ? a * (MAX_PWM-tconf->min_power)/100 + tconf->min_power : 0;
    	v = s * v2;
    }

    if (!tconf->enable_pid) {
        // stop detection without PID - but we still use pidvars.stopped, this is a little bit ugly
        if (!tvars->pidvars.stopped) {
            if (!tvars->target_speed && !v) {
                tvars->pidvars.stopped = 1;
                send_train_stopped(numtrain, tvars);
            }
        } else if (tvars->target_speed) {
            tvars->pidvars.stopped = 0;
        }
    }
    
    
    int changed = (tvars->last_speed != v);
    tvars->last_speed = v;

    itm_debug3(DBG_PID|DBG_SPDCTL, "spd", numtrain, v, changed);

    if (changed) {
    	_set_speed(numtrain, tconf, tvars);
        if ((0)) { // TODO remove
            msg_64_t m;
            m.from = MA1_SPDCTL(numtrain);
            m.to = MA3_UI_GEN;
            m.cmd = CMD_NOTIF_SPEED;
            m.v1 = v;
            mqf_write_from_spdctl(&m);
        }
    }
    /*
    if (tconf->notify_speed) { // to be removed
    	struct spd_notif n;
    	n.sv100 = v;
    	n.pid_target = tvars->pidvars.target_v;
    	//canton_vars_t *cv1 = get_canton_vars(tvars->current_canton);
    	n.bemf_centivolt = tvars->bemf_mv/10; //cv1->bemf_centivolt;
    	train_notif(numtrain, 'V', (void *)&n, sizeof(n));
    }
    */

    /* estimate speed/position with bemf */
    if ((1)) {
        int32_t b = tvars->bemf_mv;
        if (abs(b)<100) b = 0; // XXX XXXX

        // TODO: BEMF to speed. currently part of it is done in convert_to_centivolt
        //       but we assume speed is really proportional to BEMF

        //  dt is not precise enough
        int32_t pi = (b*100)/tsktick_freqhz;
        tvars->position_estimate += pi;
        itm_debug3(DBG_POSE, "pose", numtrain, tvars->position_estimate, b);
        itm_debug3(DBG_POSE, "pi", b, dt,  pi);

        pose_check_trig(numtrain, tvars, b);
        if (tconf->notify_pose) {
    		train_notif(numtrain, 'i', (void *)&tvars->position_estimate, sizeof(int32_t));
        }
    }
}


static void set_c1_c2(int tidx, train_vars_t *tvars, uint8_t c1, int8_t dir1, uint8_t c2, int8_t dir2)
{
	msg_64_t m;
	m.from = MA1_SPDCTL(tidx);

	itm_debug3(DBG_SPDCTL, "s-c1", tidx, c1, dir1);
	itm_debug3(DBG_SPDCTL, "s-c2", tidx, c2, dir2);

	tvars->c2bemf = 0;

	if ((tvars->C1x != 0xFF) && (tvars->C1x != c1)  && (tvars->C1x != c2)) {
		TO_CANTON(m, tvars->C1x);
		itm_debug1(DBG_SPDCTL, "stp c1", tidx);
		m.cmd = CMD_STOP;
		mqf_write_from_spdctl(&m);
		m.cmd = CMD_BEMF_OFF;
		mqf_write_from_spdctl(&m);
	}
	if ((tvars->C2x != 0xFF) && (tvars->C2x != c1)  && (tvars->C2x != c2)) {
		TO_CANTON(m, tvars->C2x);
		itm_debug1(DBG_SPDCTL, "stp c2", tidx);
		m.cmd = CMD_STOP;
		mqf_write_from_spdctl(&m);
		m.cmd = CMD_BEMF_OFF;
		mqf_write_from_spdctl(&m);
	}
	if ((c1 != 0xFF) && (c1 != tvars->C1x) && (c1 != tvars->C2x)) {
		TO_CANTON(m, c1);
		m.cmd = CMD_BEMF_ON;
		mqf_write_from_spdctl(&m);
	}
	if ((c2 != 0xFF) && (c2 != tvars->C1x) && (c2 != tvars->C2x)) {
		TO_CANTON(m, c2);
		//m.to = c2;
		m.cmd = CMD_BEMF_ON;
		mqf_write_from_spdctl(&m);
	}
    if (c1 != tvars->C1x) {
        tvars->position_estimate = 0; // reset POSE
    }
	tvars->C1x = c1;
	tvars->C1_dir = dir1;
	tvars->C2x = c2;
	tvars->C2_dir = dir2;
	tvars->last_speed = 9000; // make sure cmd is sent
	itm_debug2(DBG_POSEC, "POS reset", tidx, tvars->position_estimate);
}



static void _set_speed(int tidx, const conf_train_t *cnf, train_vars_t *vars)
{
    const conf_canton_t *c1;
    const conf_canton_t *c2;


	int16_t sv100 = vars->last_speed;

    c1 =  conf_canton_template(); //conf_canton_get(vars->C1);
    c2 =  c1; // conf_canton_template(); // conf_canton_get(vars->C2);
    
    if (vars->C1x == 0xFF) c1 = NULL;
    if (vars->C2x == 0xFF) c2 = NULL;
    
    if (!c1) {
    	itm_debug1(DBG_ERR|DBG_SPDCTL, "no canton", sv100);
        train_error(ERR_CANTON_NONE, "no canton");
        return;
    }

    int pvi1, pvi2;
    int sig = SIGNOF(sv100);
    uint16_t v = abs(sv100);
    uint16_t pwm_duty = volt_index(v*10 /* mili*/,
                                   c1, c2,
                                   &pvi1, &pvi2, cnf->volt_policy);

	int dir1 = sig * vars->C1_dir;
	int dir2 = sig * vars->C2_dir;


    msg_64_t m = {0};
    m.from = MA1_SPDCTL(tidx);
    TO_CANTON(m, vars->C1x);
    if (7==m.subc) Error_Handler(); // XXX
    //m.to = vars->C1;
    m.cmd = CMD_SETVPWM;
    m.v1u = pvi1;
    m.v2 = dir1*pwm_duty;
    if (7==m.subc) Error_Handler(); // XXX
	itm_debug3(DBG_SPDCTL, "setvpwm", m.v1u, m.v2, m.to);
    mqf_write_from_spdctl(&m);

    if (c2) {
    	itm_debug3(DBG_SPDCTL, "setvpwm/c2", m.v1u, m.v2, m.to);
    	m.v1u = pvi2;
    	m.v2 = dir2*pwm_duty;
    	TO_CANTON(m, vars->C2x);
    	//m.to = vars->C2;
    	mqf_write_from_spdctl(&m);
    }
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


static int _pose_check_trig(int numtrain, train_vars_t *tvars, int32_t lastincr, int32_t pose)
{
	if (!pose) return 0;
#if 0
	if (pose > 0) {
		if (lastincr<0) itm_debug3(DBG_ERR|DBG_POSEC, "wrong incr", numtrain, lastincr, pose);
		if (tvars->position_estimate >= pose) {
            return 1;
		}
	} else { // pose_trig < 0
		if (lastincr>0) itm_debug3(DBG_ERR|DBG_POSEC, "wrong incr", numtrain, lastincr, pose);
		if (tvars->position_estimate <= pose) {
            return 1;
		}
    }
#endif
    int dir = tvars->C1_dir;
    if (!dir) return 0;
    if ((SIGNOF0(lastincr)*dir) < 0) {
        itm_debug3(DBG_ERR|DBG_POSEC, "wrong incr", numtrain, lastincr, pose);
    }
    if (dir >0) {
        if (tvars->position_estimate >= pose) return 1;
    } else {
        if (tvars->position_estimate <= pose) return 1;
    }
	return 0;
}

static void pose_check_trig(int numtrain, train_vars_t *tvars, int32_t lastincr)
{
    int rc1 = _pose_check_trig(numtrain, tvars, lastincr, tvars->pose_trig0);
    int rc2 = _pose_check_trig(numtrain, tvars, lastincr, tvars->pose_trigU1);

    int r = 0;
    if (rc1>0) {
        r |= (1<<0);
        tvars->pose_trig0 = 0; // trig only once
        itm_debug3(DBG_POSEC, "POSE trg1", numtrain, tvars->position_estimate, tvars->pose_trig0);
    }
    if (rc2>0) {
        r |= (1<<1);
        tvars->pose_trigU1 = 0; // trig only once
        itm_debug3(DBG_POSEC, "POSE trg2", numtrain, tvars->position_estimate, tvars->pose_trigU1);
    }
    if (!r) return;

    msg_64_t m;
    m.from = MA1_SPDCTL(numtrain);
    m.to = MA1_CTRL(numtrain);
    m.cmd = CMD_POSE_TRIGGERED;
    m.subc = r;
    m.v1u = tvars->C1x;
    int32_t p = tvars->position_estimate / 100;
    if (abs(p)>0x7FFF) {
        // TODO: problem here pose is > 16bits
        itm_debug1(DBG_POSEC|DBG_ERR, "L pose", p);
        p = SIGNOF(p)*0x7FFF;
    }
    m.v2 = (int16_t) p;
    mqf_write_from_spdctl(&m);
}
/*
void train_stop_all(void)
{
	stop_all = 1;
	auto1_reset();
	railconfig_setup_default();
	for (int i=0; i<NUM_CANTONS; i++) {
		USE_CANTON(i) // cconf cvars
		// setup default has put dir=0 + pwm=0 and a canton_set_pwm(0,0)
		// would be ignored.
		canton_set_pwm(cconf, cvars,  1, 0);
		canton_set_pwm(cconf, cvars,  0, 0);
		canton_set_volt(cconf, cvars, 15);
	}
	railconfig_setup_default();
	debug_info('G', 0, "STOPALL", 0,0, 0);
	stop_all = 0;
}
*/

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
#if 0
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
		const conf_canton_t *c = conf_canton_get(i);
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
		const conf_canton_t *c = conf_canton_get(i);
		canton_vars_t   *v       = get_canton_vars(i);
		canton_end_calib(c, v, calib.spd, CALIB_NUM_VAL);
	}
}
#else
void calibrate_periodic(uint32_t tick, uint32_t dt, uint32_t notif_Flags)
{

}
#endif
#endif

/* ------------------------------------------------------------------------ */


#if 0

static void unexpected_canton_occupency(uint8_t numcanton);
static void unexpected_unknown_canton_occupency(uint8_t numtrain, uint8_t numcanton, uint8_t cur);
static void lost_train(uint8_t numtrain);
static void train_switching_canton(uint8_t numtrain);
static void train_did_switch_canton(uint8_t numtrain);

static void highlevel_tick(void)
{
	// check unexpected presence
	for (int i=0; i<NUM_CANTONS; i++) {
		USE_CANTON(i);
        (void) cconf; // unused;
		if (cvars->curtrainidx != 0xFF) continue;
		if (cvars->occupency == CANTON_OCCUPENCY_FREE) continue;
		if (cvars->occupency == CANTON_OCCUPENCY_UNKNOWN) continue;
		unexpected_canton_occupency(i);
	}
	// check for next
	for (int i=0; i<NUM_TRAINS; i++) {
		USE_TRAIN(i);
		if (!tconf->enabled) continue;
        //(void) tconf; // unused
        if ((signof0(tvars->prev_last_speed) != signof0(tvars->last_speed)) && signof0(tvars->last_speed)) {
            // change dir
            int oldc = tvars->next_canton;
            block_canton_get_next(tvars->current_canton, tvars->C1_dir*signof0(tvars->last_speed), &tvars->next_canton, &tvars->C2_dir);
            //printf("dir change");
            if (oldc != tvars->next_canton) {
                if (oldc != 0xFF) {
                    USE_CANTON(oldc)
                    canton_set_pwm(cconf, cvars, 0, 0);
                    canton_set_train(oldc, 0xFF);
                }
                if (tvars->next_canton != 0xFF) {
                    canton_set_train(tvars->next_canton, i);
                }
            }
        }
        tvars->prev_last_speed = tvars->last_speed;
        
		int c1 = tvars->current_canton;
		int c2 = tvars->next_canton;
		//conf_canton_t *cc1 =  get_canton_cnf(c1);
		//conf_canton_t *cc2 =  get_canton_cnf(c2);
		canton_vars_t   *cv1 = get_canton_vars(c1);
		canton_vars_t   *cv2 = get_canton_vars(c2);

		if (cv1->occupency == CANTON_OCCUPENCY_UNKNOWN) unexpected_unknown_canton_occupency(i, c1, 0);
		if (!cv2) {
			if (cv1->occupency == CANTON_OCCUPENCY_FREE) {
				lost_train(i);
			}
			return;
		}
		if (cv2->occupency == CANTON_OCCUPENCY_UNKNOWN) unexpected_unknown_canton_occupency(i, c2, 1);
		if ((cv1->occupency == CANTON_OCCUPENCY_FREE) && (cv2->occupency == CANTON_OCCUPENCY_FREE)) {
			lost_train(i);
		}
		if (cv2->occupency > CANTON_OCCUPENCY_FREE) {
			if (cv1->occupency > CANTON_OCCUPENCY_FREE) train_switching_canton(i);
			else train_did_switch_canton(i);
		}
	}
}


static void unexpected(void)
{
	// insert breakpoint
}
static void unexpected_canton_occupency(uint8_t numcanton)
{
	unexpected();
}
static void unexpected_unknown_canton_occupency(uint8_t numtrain, uint8_t numcanton, uint8_t cur)
{
	unexpected();
}
static void lost_train(uint8_t numtrain)
{
    debug_info('T', numtrain, "LOST", 0, 0,0);
}
static void train_switching_canton(uint8_t numtrain)
{
    USE_TRAIN(numtrain)
    (void) tconf; // unused
    //debug_info('T', numtrain, "SWITCHING", tvars->current_canton, tvars->next_canton,0);
}

static void train_did_switch_canton(uint8_t numtrain)
{
	USE_TRAIN(numtrain);
    (void) tconf; // unused
    debug_info('T', numtrain, "SWT DONE", tvars->current_canton, tvars->next_canton, tvars->C2_dir);
	const conf_canton_t *c_old = conf_canton_get(tvars->current_canton);
	canton_vars_t *v_old = get_canton_vars(tvars->current_canton);
	canton_set_pwm(c_old, v_old, 0, 0);

	block_canton_exit(numtrain, tvars->current_canton);

	tvars->current_canton = tvars->next_canton;
	tvars->C1_dir = tvars->C2_dir;

	block_canton_enter(numtrain, tvars->current_canton);

	// find next canton and next canton dir
	block_canton_get_next(tvars->current_canton, tvars->C1_dir, &(tvars->next_canton), &(tvars->C2_dir));
    debug_info('T', numtrain, "NEXT", tvars->next_canton, tvars->C2_dir,0);

}
#endif





#define VOLT_SEL_3BITS 1	// otherwise, legacy on 4 bits

#ifdef VOLT_SEL_3BITS
#define NUM_VOLTS_VAL 8
#else
#define NUM_VOLTS_VAL 16
#endif
#define MAX_PVI (NUM_VOLTS_VAL-1)

static int volt_index(uint16_t mili_power,
		const conf_canton_t *c1, //canton_vars_t *v1,
		_UNUSED_ const conf_canton_t *c2, //canton_vars_t *v2,
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

