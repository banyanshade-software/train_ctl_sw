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

//#define RECORD_MSG 1

#include <stdint.h>
#include <memory.h>

#include "../misc.h"
#include "../msg/trainmsg.h"


#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif

#include "../low/canton.h"
#include "../low/canton_bemf.h"
#include "inertia.h"
#include "pidctl.h"
//#include "train.h"
#include "../low/turnout.h"
#include "spdctl.h"
//#include "railconfig.h"
//#include "auto1.h"
//#include "txrxcmd.h"
#include "statval.h"
#include "spdcxdir.h"

#include "../config/conf_train.h"
#include "../config/conf_locomotive.h"
#include "../config/conf_canton.h"
#include "../ctrl/train2loco.h"


// ----------------------------------------------------------------------------------
static void spdctl_reset(void);
static void spdctl_handle_msg(msg_64_t *m);
static void spdctrl_handle_tick(uint32_t tick, uint32_t dt);

static void spdctl_enter_runmode(_UNUSED_ runmode_t m)
{
	spdctl_reset();
}
static const tasklet_def_t spdctl_tdef = {
		.init 				= spdctl_reset,
		.poll_divisor		= NULL,
		.emergency_stop 	= spdctl_reset,
		.enter_runmode		= spdctl_enter_runmode,
		.pre_tick_handler	= NULL,
		.default_msg_handler = spdctl_handle_msg,
		.default_tick_handler = spdctrl_handle_tick,
		.msg_handler_for	= NULL,
		.tick_handler_for 	= NULL,

		.recordmsg			= RECORD_MSG,

};
tasklet_t spdctl_tasklet = { .def = &spdctl_tdef, .init_done = 0, .queue=&to_spdctl};

// ----------------------------------------------------------------------------------


// ----------------------------------------------------------------------------------

static int volt_index(uint16_t mili_power,
		const conf_canton_t *c1, //canton_vars_t *v1,
		const conf_canton_t *c2, //canton_vars_t *v2,
		int *pvi1, int *pvi2, train_volt_policy_t pol);

// ----------------------------------------------------------------------------------
// global run mode, each tasklet implement this
//static runmode_t run_mode = 0;
//static uint8_t testerAddr;



// ----------------------------------------------------------------------------------


//static void process_adc(volatile adc_buffer_t *buf, int32_t ticks);
static void train_periodic_control(int numtrain, uint32_t dt);

static volatile int stop_all = 0;
//static int calibrating=0;
//void calibrate_periodic(uint32_t tick, uint32_t dt, uint32_t notif_Flags);

//static void highlevel_tick(void);

uint32_t train_tick_last_dt = 0;
uint32_t train_ntick = 0;


// -----------------------------------------------------------------

// -----------------------------------------------------------------


// ------------------------------------------------------

#define MAX_TRIG 4

typedef struct train_vars {
	int16_t target_speed;	// always >= 0

	int32_t bemf_mv;		// millivolt
	pidctl_vars_t pidvars;
	inertia_vars_t inertiavars;

    uint8_t     dirbits;
    uint8_t     canton_added;
    xblkaddr_t  Cx[4];
    
	//xblkaddr_t C1x;	// current canton adress
	//xblkaddr_t C2x; // next canton address
	// TODO add C2alt, alternative next canton (manual turnout / detect defect in turnout)
	//int8_t  C1_dir; // -1 or +1
	//int8_t  C2_dir;

	int16_t last_speed;         // last spd value sent to canton
	int16_t prev_last_speed;    // previous value of last_speed (to detect direction change)
    int16_t last_trgbrk_spd;    // last spd value before PID (after brake and inertia)
    
	uint16_t C1_cur_volt_idx;
	uint16_t C2_cur_volt_idx;

    int16_t lastposed10;
    int16_t stopposed10;
    int16_t startbreakd10;
    int16_t spdbrake;
	//int32_t position_estimate;
    //int32_t pose_trig[MAX_TRIG];
    //uint8_t pose_trig_tag[MAX_TRIG];
    //int32_t pose_trigU1;
	int32_t bemfiir;
    int16_t v_iir;

    uint8_t	c2hicnt:4;	// number of bemf_c2>bemf_c1, before setting c2bemf and notify higher layer
    uint8_t c2bemf:1; 	// 1=bemf shall be taken on C2
    uint8_t brake:1;
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
    //{ trspc_vars, offsetof(train_vars_t, pose_trig0), 4          _P("T#_pose_trig1")},
    //{ trspc_vars, offsetof(train_vars_t, pose_trigU1), 4          _P("T#_pose_trig2")},
#endif
    { NULL, sizeof(train_vars_t), 0 _P(NULL)}
};


// ------------------------------------------------------


#define USE_TRAIN(_idx) \
		train_vars_t                *tvars = &trspc_vars[_idx]; \
        _UNUSED_ const conf_train_t *trconf = conf_train_get(_idx); \
        _UNUSED_ const conf_locomotive_t *tconf = getloco(_idx);


static void train_periodic_control(int numtrain, uint32_t dt);
static void _set_speed(int tidx, const conf_locomotive_t *cnf, train_vars_t *vars);
static void spdctl_reset(void);
//static void set_c1_c2(int tidx, train_vars_t *tvars, xblkaddr_t c1, int8_t dir1, xblkaddr_t c2, int8_t dir2);

//static void pose_check_trig(int numtrain, train_vars_t *tvars, int32_t lastincr);

/*
static inline xblkaddr_t to_xblk(uint8_t val)
{
	xblkaddr_t x;
	x.v = val;
	return x;
}
*/


static void spdctl_reset(void)
{
	memset(trspc_vars, 0, sizeof(trspc_vars));
	for (int  i = 0; i<NUM_TRAINS; i++) {
        trspc_vars[i].dirbits = 0;
        for (int k=0; k<4; k++) trspc_vars[i].Cx[k].v = 0xFF;
        trspc_vars[i].pidvars.trstopped = 1;
        
        inertia_reset(i, &trspc_vars[i].inertiavars);
        pidctl_reset(&trspc_vars[i].pidvars);
		//trspc_vars[i].C1x.v = 0xFF;
		//trspc_vars[i].C2x.v = 0xFF;
	}
    // reset BEMF
    // XXX TODO should not be here since canton_bemf is on remote board
    //bemf_reset();
}

volatile int16_t oscillo_t0bemf = 0;
volatile int16_t oscillo_t1bemf = 0;

extern volatile int oscillo_trigger_start;
extern volatile int oscillo_canton_of_interest;

static void _start_canton(int tidx, uint8_t v);
static void _stop_canton(int tidx,  uint8_t v);

int16_t spdctl_get_lastpose(int tidx, xblkaddr_t b)
{
    USE_TRAIN(tidx);
    if (b.v != tvars->Cx[0].v) {
        FatalError("Spd C1", "bad C1 in spdctl_get_lastpose", Error_Spd_badc1);
        return 0;
    }
    int16_t v = tvars->lastposed10;
    return v;
}
    
static void spdctl_handle_msg(msg_64_t *m)
{
	// msg handled in any state / from & to conditions
	switch (m->cmd) {
	case CMD_TRIG_OSCILLO:
		oscillo_canton_of_interest = m->v1u;
		oscillo_trigger_start = 1;
		break;
	default:
		break;
	}
    int k=0;
	if (MA1_ADDR_IS_TRAIN_ADDR(m->to)) {
		int tidx = MA1_TRAIN(m->to);
		USE_TRAIN(tidx)
		xblkaddr_t cfrom = FROM_CANTON(*m);
		switch (m->cmd) {
		case CMD_BEMF_NOTIF:
                /*if ((1)) { // debug
                    if ((cfrom.v == MA0_CANTON(3)) && (m->v1 != 0))  {
                        printf("break here");
                    }
                }*/
                for (k=0; k<4; k++) {
                    if (tvars->Cx[k].v == cfrom.v) break;
                }
                if (k==4) {
                    itm_debug2(DBG_ERR|DBG_PID, "unk bemf", m->v1, m->from);
                } else if (k==0) {
                    // C1
                    if (!tidx) oscillo_t0bemf = m->v1;
                    else if (1==tidx) oscillo_t1bemf = m->v1;
                    itm_debug3(DBG_PID, "st bemf", tidx, m->v1, m->from);
                    itm_debug3(DBG_POSE, "lposd10", tidx, tvars->lastposed10, m->v2);
                    tvars->lastposed10 = m->v2;
                    if (!tvars->c2bemf) tvars->bemf_mv = m->v1;
                } else if ((k==1)||(k==2)) {
                    // C2
                    itm_debug3(DBG_PID, "c2 bemf", tidx, m->v1, m->from);
                    if (tvars->c2bemf) {
                        tvars->bemf_mv = m->v1;
                        //tvars->lastposed10 = m->v2;
                        if (!tidx) oscillo_t0bemf = m->v1;
                        else if (1==tidx) oscillo_t1bemf = m->v1;
                    } else if (abs(m->v1) > abs(tvars->bemf_mv)+300) {
                        itm_debug3(DBG_PRES|DBG_PRES|DBG_CTRL, "c2_hi", tidx, m->v1, tvars->bemf_mv);
                        if (tvars->c2hicnt >= 1) {
                            msg_64_t mdt = {0};
                            mdt.from = MA1_SPDCTL(tidx);
                            mdt.to = MA1_CTRL(tidx);
                            mdt.cmd = CMD_BEMF_DETECT_ON_C2;
                            mdt.v1u = tvars->Cx[k].v;
                            /* XXXXX int32_t p = tvars->position_estimate / 100;
                             if (abs(p)>0x7FFF) {
                             // TODO: problem here pose is > 16bits
                             itm_debug1(DBG_POSEC|DBG_ERR, "L pose", p);
                             p = SIGNOF(p)*0x7FFF;
                             }
                             m.v2 = (int16_t) p;*/
                            mqf_write_from_spdctl(&mdt);
                            
                            tvars->c2hicnt = 0;
                            tvars->c2bemf = 1;
                        } else {
                            tvars->c2hicnt++;
                        }
                    } 
                } else if (k==3) {
                    itm_debug2(DBG_PID, "Cold emf", m->v1, m->from);
                } else {
                    itm_debug2(DBG_ERR|DBG_PID, "Unk bemf", m->v1, m->from);
                }
                break;
#if 0
			if (cfrom.v == tvars->C1x.v) {
				if (!tidx) oscillo_t0bemf = m->v1;
				else if (1==tidx) oscillo_t1bemf = m->v1;
				itm_debug3(DBG_PID, "st bemf", tidx, m->v1, m->from);
                tvars->lastposed10 = m->v2;
				if (!tvars->c2bemf) tvars->bemf_mv = m->v1;
				break;
			} else if (cfrom.v == tvars->C2x.v) {
				itm_debug3(DBG_PID, "c2 bemf", tidx, m->v1, m->from);
				if (tvars->c2bemf) {
					tvars->bemf_mv = m->v1;
                    tvars->lastposed10 = m->v2;
					if (!tidx) oscillo_t0bemf = m->v1;
					else if (1==tidx) oscillo_t1bemf = m->v1;
				}
				else if (abs(m->v1) > abs(tvars->bemf_mv)+300) {
					itm_debug3(DBG_PRES|DBG_PRES|DBG_CTRL, "c2_hi", tidx, m->v1, tvars->bemf_mv);
					if (tvars->c2hicnt >= 1) {
						msg_64_t m = {0};
						m.from = MA1_SPDCTL(tidx);
						m.to = MA1_CTRL(tidx);
						m.cmd = CMD_BEMF_DETECT_ON_C2;
						m.v1u = tvars->C2x.v;
						/* XXXXX int32_t p = tvars->position_estimate / 100;
						if (abs(p)>0x7FFF) {
							// TODO: problem here pose is > 16bits
							itm_debug1(DBG_POSEC|DBG_ERR, "L pose", p);
							p = SIGNOF(p)*0x7FFF;
						}
						m.v2 = (int16_t) p;*/
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
				itm_debug2(DBG_ERR|DBG_PID, "unk bemf", m->v1, m->from);
				// error
			}
			break;
#endif
                
		case CMD_SET_TARGET_SPEED:
			itm_debug2(DBG_SPDCTL, "set_t_spd", tidx, m->v1u);
			if (!tvars->target_speed && (m->v1u > 10)) {
				oscillo_trigger_start = 1;
			}
			tvars->target_speed = (int16_t) m->v1u;
			break;
        case CMD_BRAKE:
                if (m->subc) {
                    /*
                     start brake
                     stopposed10 is the pose value where we re supposed to brake
                     startbreakd10 is the current pose value, start of brake
                     spdbrake is the current speed
                     |----(spdbrake)-\
                     |                 \
                     |___________________\____>t
                               (start)   |    |(stop)
                     */
                	if (tvars->brake) {
                		// this occurs if other events arrived while braking, such as leav_c1
                		// rc would require brake, but without handling here, it will restart braking at
                		itm_debug3(DBG_BRAKE, "BRAKbrak", tvars->target_speed,  tvars->startbreakd10, tvars->spdbrake);
                		/*
                		 * check if stoppoesed10 is identical ??
                		 */
                		if (abs(tvars->stopposed10 - ((int16_t) m->v32)) < 10) break;
                        tvars->stopposed10 = (int16_t) m->v32;
                        tvars->startbreakd10 = tvars->lastposed10;
                        tvars->spdbrake = tvars->last_trgbrk_spd;
                	} else {
                		tvars->brake = 1;
                		tvars->stopposed10 = (int16_t) m->v32;
                		tvars->startbreakd10 = tvars->lastposed10;
                		tvars->spdbrake = tvars->target_speed; // XXX or tvars->last_speed ?
                        //tvars->spdbrake = tvars->last_speed;

                	}
                    itm_debug3(DBG_BRAKE, "BRAK", tidx, tvars->startbreakd10, tvars->stopposed10);
                } else {
                    itm_debug1(DBG_BRAKE|DBG_INERTIA, "BRAK:off", tidx);
                    tvars->brake = 0;
                    tvars->stopposed10 = 0;
                }
                break;
        case CMD_SET_C4:
            itm_debug3(DBG_SPDCTL|DBG_CTRL, "set_c4", tidx, m->vbytes[0], m->vbytes[1]);
            itm_debug3(DBG_SPDCTL|DBG_CTRL, "set_c4.", tidx, m->vbytes[2], m->vbytes[3]);
                tvars->c2bemf = 0;
                tvars->dirbits = m->subc;
                for (int i=0; i<4; i++) {
                    uint8_t v = tvars->Cx[i].v;
                    if (v == 0xFF) continue;
                    int k;
                    for (k=0; k<4; k++) {
                        if (m->vbytes[k]==v) break;
                    }
                    if (k==4) {
                        _stop_canton(tidx, v);
                    }
                }
                for (int k=0; k<4; k++) {
                    uint8_t v = m->vbytes[k];
                    if (0xFF==v) continue;;
                    int i;
                    for (i=0; i<4; i++) {
                        if (tvars->Cx[i].v == v) break;
                    }
                    if (i==4) {
                        _start_canton(tidx, v);
                        tvars->canton_added = 1;
                    }
                }
                for (int i=0; i<4; i++) {
                    tvars->Cx[i].v = m->vbytes[i];
                }
                break;
                
#if 0
		case CMD_SET_C1_C2:
			itm_debug3(DBG_SPDCTL|DBG_CTRL, "set_c1_c2", tidx, m->vbytes[0], m->vbytes[2]);
			//set_c1_c2(int tidx, train_vars_t *tvars, xblkaddr_t c1, int8_t dir1, xblkaddr_t c2, int8_t dir2)
			set_c1_c2(tidx, tvars, to_xblk(m->vbytes[0]), m->vbytes[1], to_xblk(m->vbytes[2]), m->vbytes[3]);
			break;
#endif
		case CMD_POSE_SET_TRIG: //ctrl->canton     subc=tag v1=POSE/10   v2=dir now to CANTON
			itm_debug1(DBG_POSEC|DBG_ERR, "should be sent to canton_bemf", 0);
			break;
			// XXX itm_debug3(DBG_POSEC, "POSE set", tidx, m->v1, tvars->subc);
			/* XXX tvars->pose_trig0 = m->v1*10;
            if (m->subc) tvars->position_estimate = 0;
			// check if already trigg
			pose_check_trig(tidx, tvars, 0);
			break;
		case CMD_POSE_SET_TRIG_U1:
			itm_debug3(DBG_POSEC, "POSE setU1", tidx, m->v32, tvars->position_estimate);
			tvars->pose_trigU1 = m->v32*10;
            if (m->subc) tvars->position_estimate = 0;
			// check if already trigg
			pose_check_trig(tidx, tvars, 0);*/
		default:
			break;
		}
	}
}

static void spdctrl_handle_tick(_UNUSED_ uint32_t tick, uint32_t dt)
{
    /* process trains */
    for (int i=0; i<NUM_TRAINS; i++) {
        //itm_debug1(DBG_SPDCTL, "------ pc", i);
        train_periodic_control(i, dt);
    }
}

#if 0
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
            xblkaddr_t cfrom = FROM_CANTON(m);
            switch (m.cmd) {
                case CMD_BEMF_NOTIF:
                    
                    if (cfrom == tvars->C1x) {
                    	if (!tidx) oscillo_t0bemf = m.v1;
                    	else if (1==tidx) oscillo_t1bemf = m.v1;

                        itm_debug3(DBG_PID, "st bemf", tidx, m.v1, m.from);
                        if (!tvars->c2bemf) {
                            tvars->bemf_mv = m.v1;
                            tvars->lastpose = m.v2;
                        }
                        break;
                    } else if (cfrom.v == tvars->C2x.v) {
                        itm_debug3(DBG_PID|DBG_CTRL, "c2 bemf", tidx, m.v1, m.from);
                        if (tvars->c2bemf) {
                        	tvars->bemf_mv = m.v1;
                            tvars->lastpose = m.v2;
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
                            	m.v1u = tvars->C2x.v;
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
                        itm_debug3(DBG_ERR|DBG_PID, "unk bemf", tidx, m.v1, m.from);
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
                    //set_c1_c2(int tidx, train_vars_t *tvars, xblkaddr_t c1, int8_t dir1, xblkaddr_t c2, int8_t dir2)
                    set_c1_c2(tidx, tvars, to_xblk(m.vbytes[0]), m.vbytes[1], to_xblk(m.vbytes[2]), m.vbytes[3]);
                    break;
                case CMD_POSE_SET_TRIG:
                    // should be sent to canton/canton_bemf now
                    itm_debug1(DBG_ERR|DBG_POSEC, "ERR/cb", tidx);
                    break;
                    /*
                    itm_debug3(DBG_POSEC, "POSE set", tidx, m.v2, m.v1);
                    int pidx = pose_add_trig(tvars, m.v2, m.v1*100);
                    if (pidx>=0) pose_check_trig(tidx, tvars, pidx, 0);
                    break;*/
                /*
                case CMD_POSE_SET_TRIG0:
                	itm_debug2(DBG_POSEC, "POSE set0", tidx, m.v1);
                	tvars->pose_trig0 = m.v1*100;
                	// check if already trigg
                	pose_check_trig(tidx, tvars, 0);
                	break;
                case CMD_POSE_SET_TRIG_U1:
                    itm_debug2(DBG_POSEC, "POSE setU1", tidx, m.v1);
                    tvars->pose_trigU1 = m.v1*100;
                    // check if already trigg
                    pose_check_trig(tidx, tvars, 0);
                 */
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


#endif

void send_train_stopped(int numtrain, train_vars_t *tvars)
{
    
    msg_64_t m = {0};
    m.from = MA1_SPDCTL(numtrain);
    m.to = MA1_CTRL(numtrain);
    m.cmd = CMD_STOP_DETECTED;
    m.subc = tvars->brake;
    m.v32 = tvars->lastposed10; // XXX TODO scale ?
    mqf_write_from_spdctl(&m);
    
    tvars->brake = 0;
    tvars->target_speed = 0;
    //tvars->inertiavars.cur100 = 0;
}

#define punch_start 1

static inline int16_t comp_brake(int16_t startval, int32_t k1000, const conf_locomotive_t *lconf)
{
    // linear decrease from startval to minval
    //static const int16_t minval = 15;
    int16_t minval = lconf->min_power_dec;
    int16_t v = startval;
    if (startval > minval) {
        int16_t t = startval-minval;
        v = (int16_t)((t * k1000)/1000);
        v += minval;
    }
    if (k1000<=1) return 0;
    return v;
}

static void train_periodic_control(int numtrain, _UNUSED_ uint32_t dt)
{
	if (stop_all) return;


	USE_TRAIN(numtrain)	// tconf tvars
    if (!tconf) {
        if ((0)) itm_debug1(DBG_SPDCTL, "unconf tr", numtrain);
        return;
    }
	if (!trconf->enabled) {
		//itm_debug1(DBG_SPDCTL, "disabled", numtrain);
		return;
	}
	int16_t target_with_brake = tvars->target_speed;

    // ----- handling brake
	itm_debug2(DBG_SPDCTL, "target", numtrain, target_with_brake);
    if (tvars->brake) {
    	// XXX possible div0 here
    	int32_t k1000;
    	if (tvars->stopposed10 != tvars->startbreakd10) {
            itm_debug3(DBG_BRAKE, "trg:k1000", tvars->lastposed10, tvars->startbreakd10, tvars->stopposed10);
    		k1000 = (1000*(tvars->stopposed10-tvars->lastposed10))/(tvars->stopposed10 - tvars->startbreakd10);
    	} else {
    		k1000 = 0;
    	}
        if (k1000<0) {
        	k1000 = 0;
        }
        target_with_brake = comp_brake(tvars->spdbrake, k1000, tconf);
    	itm_debug3(DBG_BRAKE, "trg:brak", numtrain, k1000, target_with_brake);
    }

    int16_t target_processed = target_with_brake;
    
    // ----- handling inertia
    // inertia before PID
	if (1==tconf->enable_inertia) {
        /* dont apply inertia when brake is on progress
         * the inertia effect is performed by brake, because we
         * need precise stop position
         */
        if (!tvars->brake) {
            inertia_set_target(numtrain, &tconf->inertia, &tvars->inertiavars, target_processed);
            if (!tvars->pidvars.trstopped) {
                int changed;
                //tvars->inertiavars.target = tvars->target_speed;
                target_processed = inertia_value(numtrain, tconf, &tvars->inertiavars, &changed);
                itm_debug3(DBG_INERTIA, "inertia", numtrain, tvars->target_speed, target_processed);
            }
        } else if (tvars->brake) {
        	inertia_temporary_deactivated(numtrain, &tconf->inertia, &tvars->inertiavars, target_processed);
        }
	}
    
    tvars->last_trgbrk_spd = target_processed;
    
    // ----- PID set target

    if (tconf->enable_pid) {
        // corresponding BEMF target
        int8_t dir = __spdcx_dir(tvars->dirbits, 0);
        int32_t tbemf = 1800*target_processed/100 * dir;
        //tbemf = tbemf / 4; //XXX why ?? new cables (more capacitance ?)
        // TODO make this divisor a parameter
        pidctl_set_target(&tconf->pidctl, &tvars->pidvars, tbemf);
        // XXXX notif_target_bemf(tconf, tvars, tbemf);
    }

    // IIR before pid
    
    int32_t bemf_mv = tvars->bemf_mv;
    if (tconf->bemfIIR) {
    	tvars->bemfiir = (tconf->bemfIIR*tvars->bemfiir + (100-tconf->bemfIIR)*bemf_mv)/100;
    	bemf_mv = tvars->bemfiir;
    }
    
    // PID handling
    int punchit = 0;
    if (tconf->enable_pid) {
    	if (target_with_brake) {
    		if ((punch_start) && (tvars->pidvars.trstopped)) {
    			itm_debug3(DBG_PID|DBG_SPDCTL|DBG_INERTIA, "punch", numtrain, target_with_brake, tvars->brake);
                punchit = 1;
    		}
            tvars->pidvars.trstopped = 0;
    	}
        if (!tvars->pidvars.trstopped && (target_with_brake == 0) && (target_processed==0) && (abs(tvars->bemf_mv)<100)) {
    		itm_debug1(DBG_PID|DBG_SPDCTL|DBG_BRAKE, "stop", numtrain);
			pidctl_reset(&tvars->pidvars);
            inertia_reset(numtrain, &tvars->inertiavars);
			debug_info('T', numtrain, "STOP_PID", 0,0, 0);
			tvars->pidvars.trstopped = 1;
            send_train_stopped(numtrain, tvars);
            target_processed = 0;
            
        } else if (tvars->pidvars.trstopped) {
    		//itm_debug2(DBG_PID|DBG_BRAKE, "trstopped", numtrain, target_processed);
            target_processed = 0;
            
        } else {
        	itm_debug3(DBG_PID, "pid", numtrain, bemf_mv, target_processed);
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
            target_processed = (int16_t)v3 * __spdcx_dir(tvars->dirbits, 0); // because it will be multiplied again when setting pwm
        }
    }
    if (tconf->postIIR) {
        tvars->v_iir = (tconf->postIIR*tvars->v_iir+(100-tconf->postIIR)*target_processed)/100;
        target_processed = tvars->v_iir;
    }
    //  ---------   inertia after PID, obsolete, to be removed
    if (2==tconf->enable_inertia) {
        if (!tvars->brake && !tvars->pidvars.trstopped) {
            inertia_set_target(numtrain, &tconf->inertia, &tvars->inertiavars, target_processed);
            //tvars->inertiavars.target = v;
            target_processed = inertia_value(numtrain, tconf, &tvars->inertiavars, NULL);
        } else {
        	inertia_temporary_deactivated(numtrain, &tconf->inertia, &tvars->inertiavars, target_processed);
        }
    }

    if (tconf->en_spd2pow) {
    	// [0-100] -> [min_pwm .. MAX_PWM]
    	int s = SIGNOF(target_processed);
    	int a = abs(target_processed);
    	int v2 = (a>1) ? a * (MAX_PWM-tconf->min_power)/100 + tconf->min_power : 0;
        target_processed = s * v2;
    }

    if (!tconf->enable_pid) {
        // stop detection without PID - but we still use pidvars.stopped, this is a little bit ugly
        if (!tvars->pidvars.trstopped) {
            if (!tvars->target_speed && !target_with_brake && !target_processed) {
                tvars->pidvars.trstopped = 1;
                send_train_stopped(numtrain, tvars);
            }
        } else if (tvars->target_speed) {
            tvars->pidvars.trstopped = 0;
        }
    }
    
    
    int changed = (tvars->last_speed != target_processed);
    tvars->last_speed = target_processed;

    itm_debug3(DBG_PID|DBG_SPDCTL, "spd", numtrain, target_processed, changed);

    // force resending spd if cantons were added
    if (tvars->canton_added) {
    	tvars->canton_added = 0;
    	changed = 1;
    }

    if (changed) {
    	_set_speed(numtrain, tconf, tvars);
        if ((0)) { // TODO remove
            msg_64_t m;
            m.from = MA1_SPDCTL(numtrain);
            m.to = MA3_UI_GEN;
            m.cmd = CMD_NOTIF_SPEED;
            m.v1 = target_processed;
            mqf_write_from_spdctl(&m);
        }
    }
   
}


static void _start_canton(int tidx, uint8_t v)
{
    msg_64_t m = {0};
    m.from = MA1_SPDCTL(tidx);
    xblkaddr_t c;
    c.v = v;
    TO_CANTON(m, c);
    //m.to = c2;
    m.cmd = CMD_BEMF_ON;
    mqf_write_from_spdctl(&m);
}
static void _stop_canton(int tidx, uint8_t v)
{
    msg_64_t m = {0};
    m.from = MA1_SPDCTL(tidx);
    xblkaddr_t c;
    c.v = v;
    TO_CANTON(m, c);
    itm_debug2(DBG_SPDCTL, "stp c2", tidx, v);
    m.cmd = CMD_STOP;
    mqf_write_from_spdctl(&m);
    m.cmd = CMD_BEMF_OFF;
    mqf_write_from_spdctl(&m);
}


#if 0
static void set_c1_c2(int tidx, train_vars_t *tvars, xblkaddr_t c1, int8_t dir1, xblkaddr_t c2, int8_t dir2)
{
    msg_64_t m = {0};
	m.from = MA1_SPDCTL(tidx);

	itm_debug3(DBG_SPDCTL, "s-c1", tidx, c1.v, dir1);
	itm_debug3(DBG_SPDCTL, "s-c2", tidx, c2.v, dir2);

	tvars->c2bemf = 0;
    
    if (tvars->C1x.v != c1.v) {
        itm_debug3(DBG_POSEC, "chg c1", tidx, tvars->C1x.v, c1.v);
    }

	if ((tvars->C1x.v != 0xFF) && (tvars->C1x.v != c1.v)  && (tvars->C1x.v != c2.v)) {
		TO_CANTON(m, tvars->C1x);
		itm_debug1(DBG_SPDCTL, "stp c1", tidx);
		m.cmd = CMD_STOP;
		mqf_write_from_spdctl(&m);
		m.cmd = CMD_BEMF_OFF;
		mqf_write_from_spdctl(&m);
	}
	if ((tvars->C2x.v != 0xFF) && (tvars->C2x.v != c1.v)  && (tvars->C2x.v != c2.v)) {
		TO_CANTON(m, tvars->C2x);
		itm_debug1(DBG_SPDCTL, "stp c2", tidx);
		m.cmd = CMD_STOP;
		mqf_write_from_spdctl(&m);
		m.cmd = CMD_BEMF_OFF;
		mqf_write_from_spdctl(&m);
	}
	if ((c1.v != 0xFF) && (c1.v != tvars->C1x.v) && (c1.v != tvars->C2x.v)) {
		TO_CANTON(m, c1);
		m.cmd = CMD_BEMF_ON;
		mqf_write_from_spdctl(&m);
	}
	if ((c2.v != 0xFF) && (c2.v != tvars->C1x.v) && (c2.v != tvars->C2x.v)) {
		TO_CANTON(m, c2);
		//m.to = c2;
		m.cmd = CMD_BEMF_ON;
		mqf_write_from_spdctl(&m);
	}

		/*
    if (c1.v != tvars->C1x.v) {
        tvars->position_estimate = 0; // reset POSE when C1 changes
    }
    */


	tvars->C1x = c1;
	tvars->C1_dir = dir1;
	tvars->C2x = c2;
	tvars->C2_dir = dir2;
	tvars->last_speed = 9000; // make sure cmd is sent
}
#endif


static void _set_speed(int tidx, const conf_locomotive_t *cnf, train_vars_t *vars)
{
    const conf_canton_t *c1;

	int16_t sv100 = vars->last_speed;
    itm_debug3(DBG_SPDCTL, "setspd ", tidx, sv100, vars->Cx[0].v);
    itm_debug3(DBG_SPDCTL, "setspd.", vars->Cx[1].v, vars->Cx[2].v, vars->Cx[3].v);


    c1 =  conf_canton_template(); //conf_canton_get(vars->C1);

    
    if (!c1) {
    	itm_debug1(DBG_ERR|DBG_SPDCTL, "no canton", sv100);
        train_error(ERR_CANTON_NONE, "no canton");
        return;
    }

    int pvi1, pvi2;
    int sig = SIGNOF(sv100);
    uint16_t v = abs(sv100);
    uint16_t pwm_duty = volt_index(v*10 /* mili*/,
                                   c1, c1,
                                   &pvi1, &pvi2, cnf->volt_policy);
    msg_64_t m = {0};
    m.from = MA1_SPDCTL(tidx);
    m.cmd = CMD_SETVPWM;
    m.vb0 = pvi1;

    for (int i=0; i<4; i++) {
        if (vars->Cx[i].v == 0xFF) continue;
        int dir = sig * __spdcx_dir(vars->dirbits, i);
        TO_CANTON(m, vars->Cx[i]);
        itm_debug3(DBG_SPDCTL, "setvpwm", tidx, vars->Cx[i].v, dir*pwm_duty);
        m.vb1 = dir*pwm_duty;
        mqf_write_from_spdctl(&m);
    }
}




/* =========================================================================== */



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

