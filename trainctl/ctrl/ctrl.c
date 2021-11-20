/*
 * ctrl.c
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */



#include "misc.h"
#include "../msg/trainmsg.h"
#include "ctrl.h"

#include "../topology/topology.h"
#include "railconfig.h"
#include "statval.h"

#include "../topology/occupency.h"

#include "ctrlP.h"
#include "cautoP.h"

#include "../leds/led.h"

#define SCEN_TWOTRAIN 	0 //1



static train_ctrl_t trctl[NUM_TRAINS] = {0};

//static lsblk_num_t snone = {-1};

// -----------------------------------------------------------

const stat_val_t statval_ctrl[] = {
        { trctl, offsetof(train_ctrl_t, _state),           sizeof(train_state_t)   _P("T#_ctrl_state")},
        { trctl, offsetof(train_ctrl_t, _dir),             sizeof(uint8_t)         _P("T#_ctrl_dir")},
        { trctl, offsetof(train_ctrl_t, _target_speed),    sizeof(uint16_t)        _P("T#_ctrl_target_speed")},
        { trctl, offsetof(train_ctrl_t, can1_addr),        sizeof(uint8_t)         _P("T#_ctrl_canton1_addr")},
        { trctl, offsetof(train_ctrl_t, c1_sblk.n),        sizeof(uint8_t)         _P("T#_ctrl_canton1_lsb")},
        { trctl, offsetof(train_ctrl_t, can2_addr),        sizeof(uint8_t)         _P("T#_ctrl_canton2_addr")},
        { trctl, offsetof(train_ctrl_t, desired_speed),    sizeof(uint16_t)        _P("T#_ctrl_desired_speed")},
        { trctl, offsetof(train_ctrl_t, spd_limit),        sizeof(uint16_t)        _P("T#_ctrl_spd_limit")},
        { trctl, offsetof(train_ctrl_t, curposmm),         sizeof(int32_t)         _P("T#_curposmm")},
        { trctl, offsetof(train_ctrl_t, beginposmm),         sizeof(int32_t)         _P("T#_beginposmm")},
        { NULL,  sizeof(train_ctrl_t), 0 _P(NULL)}
};



// global run mode, each tasklet implement this
static runmode_t run_mode = 0;
static uint8_t testerAddr;

static void ctrl_reset(void);


static void fatal(void)
{
	itm_debug1(DBG_ERR, "fatal", 0);
#ifdef TRAIN_SIMU
    abort();
#else
    for (;;) osDelay(1000);
#endif
}

// ----------------------------------------------------------------------------
// train FSM
static void evt_timer(int tidx, train_ctrl_t *tvar, int tnum);


// ----------------------------------------------------------------------------
// generic timer attached to train_ctrl_t struct

//static void ctrl_reset_timer(int tidx, train_ctrl_t *tvar, int numtimer);
//static void ctrl_set_timer(int tidx, train_ctrl_t *tvar, int numtimer, uint32_t tval);
static void check_timers(uint32_t tick);

// ----------------------------------------------------------------------------
// sub block presence handling

static void sub_presence_changed(uint32_t tick, uint8_t from_addr, uint8_t segnum, uint16_t v, int16_t ival);

// ----------------------------------------------------------------------------
//  block occupency


// ----------------------------------------------------------------------------
// turnouts
static void set_turnout(int tn, int v);

// ----------------------------------------------------------------------------
// behaviour

static void check_behaviour(uint32_t tick);


static void ctrl_reset(void)
{
	//TODO
}
// ----------------------------------------------------------------------------




static void ctrl_set_mode(int trnum, train_mode_t mode)
{
    ctrl2_set_mode(trnum, &trctl[trnum], mode);
}



static void ctrl_init(void)
{
	memset(trctl, 0, sizeof(train_ctrl_t)*NUM_TRAINS);
	ctrl_set_mode(0, train_manual);
	//ctrl_set_mode(1, train_auto);
    set_turnout(0, 0);
    set_turnout(1, 0);
    lsblk_num_t s1 = {1};
    lsblk_num_t s2 = {2};
    lsblk_num_t _UNUSED_ s3 = {3};
	if ((1)) {
        ctrl2_init_train(0, &trctl[0], s1);
        ctrl2_init_train(1, &trctl[1], s2);
        if ((0)) {
            static uint8_t route[] = {
                _AR_SPD(-40), 0, _AR_WSTOP,
                _AR_SPD(28), 1, 3, _AR_LED, 0, LED_PRG_NEONON,
                _AR_TRGEVENT(0), _AR_WSTOP, _AR_SPD(0),
                _AR_SPD(-30), 4, _AR_LED, 0, LED_PRG_DIMOFF, 5, _AR_WSTOP,
                _AR_SPD(30),4, 3, _AR_LED, 0, LED_PRG_NEONON, _AR_WSTOP,
                _AR_SPD(-60), 1, _AR_LED, 0, LED_PRG_DIMOFF, 2, _AR_WSTOP,
				_AR_SPD(60), 1, 3,  _AR_LED, 0, LED_PRG_NEONON, _AR_TRGEVENT(1), _AR_WSTOP, _AR_SPD(0),
                _AR_WEVENT(1),
                _AR_SPD(-40), 1, _AR_STPHALF, _AR_LED, 0, LED_PRG_DIMOFF, _AR_WSTOP,
				_AR_TIMER(8), _AR_WTIMER,
				_AR_TIMER(8), _AR_WTIMER,
				_AR_TIMER(8), _AR_WTIMER,
				_AR_LOOP};
            trctl[0].routeidx = 0;
            trctl[0].route = route;
            // ctrl_set_mode(0, train_auto);
            
            static uint8_t route2[] = {_AR_WEVENT(0),
            		_AR_SPD(60), 1, _AR_STPHALF, _AR_WSTOP,
            		_AR_SPD(-60) , 0, _AR_WSTOP,
					_AR_WEVENT(1),
					_AR_SPD(40), 1, _AR_STPHALF,  _AR_WSTOP,
					_AR_SPD(-40), 2, _AR_TRGEVENT(1), _AR_WSTOP,
					_AR_LOOP};
            trctl[1].routeidx = 0;
            trctl[1].route = route2;
        }
#define SON  _AR_LED, 0, LED_PRG_NEONON
#define SOFF  _AR_LED, 0, LED_PRG_DIMOFF
        
        if ((1)) {
            static uint8_t route[] = {
                // T0 starts on 1
                // 1->0->2
                _AR_SPD(-30), 0, _AR_WSTOP, _AR_TRGEVENT(0),
                _AR_WEVENT(1),
                _AR_SPD(15), 1, /*_AR_STPHALF */ _AR_WSTOP, //_AR_SPD(0),
                _AR_TIMER(3), _AR_WTIMER,
                _AR_SPD(-35),  2, _AR_WSTOP,
                // wait and 2-1-3 sleep 3-4-5
                _AR_WEVENT(2),
                _AR_SPD(60), 1, SON, 3, _AR_WSTOP,  _AR_TRGEVENT(3),
                _AR_TIMER(2), _AR_WTIMER,
                _AR_SPD(-20), 4, SOFF, 5, _AR_WSTOP,
                _AR_TIMER(0), _AR_WTIMER,
                // 4, 3 sleep 1
                _AR_SPD(20), 4, 3, SON, _AR_WSTOP,
                _AR_TIMER(2), _AR_WTIMER,
                _AR_WEVENT(4), SOFF,
                _AR_SPD(-30), 1, _AR_STPHALF, _AR_WSTOP, _AR_SPD(0),
                _AR_TIMER(7), _AR_WTIMER,
                _AR_LOOP};
            
            trctl[0].routeidx = 0;
            trctl[0].route = route;
            // ctrl_set_mode(0, train_auto);
            
            static uint8_t route2[] = {
                    _AR_WEVENT(0),
                    // 2-1-3
                    _AR_SPD(60), 1, SON, 3, _AR_TRGEVENT(1), _AR_WSTOP,
                    // 4-5
                    _AR_SPD(-40) , 4, 5, _AR_WSTOP,
                    _AR_TIMER(0), _AR_WTIMER,
                    // 5-6
                    _AR_SPD(40), 6, _AR_WSTOP,
                    _AR_TIMER(0), _AR_WTIMER,
                    _AR_SPD(-40), 5, _AR_WSTOP,
                    _AR_SPD(40), 4, 3, SON, _AR_WSTOP,
                    _AR_SPD(-50), 1, SOFF, 0, _AR_WSTOP, _AR_TRGEVENT(2),
                    // 0-1-2
                    _AR_WEVENT(3),
                    _AR_SPD(40), 1, /*_AR_STPHALF,*/ _AR_WSTOP,
                    _AR_SPD(-40), 2, _AR_WSTOP, _AR_TRGEVENT(4), _AR_SPD(0),
                    _AR_LOOP};
            trctl[1].routeidx = 0;
            trctl[1].route = route2;
        }
        
		if ((SCEN_TWOTRAIN)) {
            ctrl2_init_train(1, &trctl[1], s2);

			if ((1)) {
                ctrl2_upcmd_set_desired_speed(1, &trctl[1], 30);
			}
		} else {
			ctrl_set_mode(1, train_notrunning);
		}
	}
}

// ----------------------------------------------------------------------------
// timers


static void check_timers(uint32_t tick)
{
	//uint32_t t = HAL_GetTick();
	for (int tidx = 0; tidx<NUM_TRAINS; tidx++) {
		train_ctrl_t *tvar = &trctl[tidx];
		for (int j=0; j<NUM_TIMERS; j++) {
			uint32_t tv = tvar->timertick[j];
			if (!tv) continue;
			if (tv <= tick) {
				itm_debug3(DBG_CTRL, "tim", tidx, j, tv);
				tvar->timertick[j] = 0;
				evt_timer(tidx, tvar, j);
			}
		}
	}
}

// ----------------------------------------------------------------------------



static void sub_presence_changed(_UNUSED_ uint32_t tick, _UNUSED_ uint8_t from_addr, _UNUSED_ uint8_t lsegnum, _UNUSED_ uint16_t p, _UNUSED_ int16_t ival)
{
    abort();
#if 0
    xxxx
    
	int segnum = _sub_addr_to_sub_num(from_addr, lsegnum);
	itm_debug3(DBG_PRES|DBG_CTRL, "PRC",  p, lsegnum, ival);
	if ((segnum<0) || (segnum>11)) return;

	uint8_t canton = blk_addr_for_sub_addr(from_addr, segnum);
	if (0xFF == canton) {
		itm_debug2(DBG_ERR|DBG_CTRL, "blk??", from_addr, segnum);
		return;
	}
	itm_debug3(DBG_PRES|DBG_CTRL, "PRBLK", p, segnum, canton);

	int f = 0;

	for (int tn = 0; tn < NUM_TRAINS; tn++) {
		train_ctrl_t *tvar = &trctl[tn];
		const train_config_t *tconf = get_train_cnf(tn);
		// check enabled
		if (!tconf->enabled) continue;
		itm_debug3(DBG_PRES|DBG_CTRL, "prblk?", tn, tvar->canton1_addr, tvar->canton2_addr);
		if (tvar->canton1_addr == canton) {
			if (p) {
				itm_debug2(DBG_PRES, "?enter c1", tn, segnum);
				evt_entered_c1(tn, tvar, 0);
			} else {
				evt_leaved_c1(tn, tvar);
			}
			f = 1;
		} else if (tvar->canton2_addr == canton) {
			if (p) {
				evt_entered_c2(tn, tvar, 0);
			} else {
				evt_leaved_c2(tn, tvar);
			}
			f = 1;
		}
	}
	if (!f) {
		// presence on unexpected canton
		itm_debug2(DBG_ERR|DBG_PRES, "?unexp", segnum, canton);
	}
#endif
}

// ----------------------------------------------------------------------------


static void posecm_measured(int tidx, int32_t pose, lsblk_num_t blknum)
{
	int cm = get_lsblk_len(blknum, NULL);
	int32_t ppcm = pose / cm;
	itm_debug2(DBG_POSEC, "ppcm", tidx, ppcm);
	debug_info('P', tidx, "PPCM", ppcm, 0,0);

}

// ----------------------------------------------------------------------------


void ctrl_run_tick(_UNUSED_ uint32_t notif_flags, uint32_t tick, _UNUSED_ uint32_t dt)
{
	
    static int first=1;

    if (first) {
        first = 0;
        ctrl_init();
        ctrl_reset();
    }
    
	check_block_delayed(tick);

	/* process messages */
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ctrl(&m);
		if (rc) break;
        switch (m.cmd) {
            case CMD_RESET:
                //test_mode = 0;
                // FALLTHRU
            case CMD_EMERGENCY_STOP:
                ctrl_reset(); // untested
                continue;
                break;
            case CMD_SETRUN_MODE:
            	if (run_mode != m.v1u) {
            		run_mode = m.v1u;
            		testerAddr = m.from;
            		first = 1;
            	}
                continue;
                break;
            default:
            	break;
        }
        if (run_mode != runmode_normal) continue;

        
       
        
        // -----------------------------------------
        switch (m.cmd) {
            default:
                break;
            case CMD_TURNOUT_HI_A:
                set_turnout(m.v1u, 0);
                break;
            case CMD_TURNOUT_HI_B:
                set_turnout(m.v1u, 1);
                break;
            case CMD_TURNOUT_HI_TOG:
                set_turnout(m.v1u, topology_get_turnout(m.v1u) ? 0 : 1);
                break;
        }
        // -----------------------------------------
		if (IS_CONTROL_T(m.to)) {
			//if (test_mode) continue;
			int tidx = m.to & 0x7;
			train_ctrl_t *tvar = &trctl[tidx];

            switch (m.cmd) {
            case CMD_PRESENCE_SUB_CHANGE:
                if (ignore_ina_presence) break;
                sub_presence_changed(tick, m.from, m.subc, m.v1u, m.v2);
                break;
        
            case CMD_BEMF_DETECT_ON_C2: {
                itm_debug2(DBG_CTRL,"BEMF/C2", tidx,  m.v1u);
                train_ctrl_t *tvar = &trctl[tidx];
                if (m.v1u != tvar->can2_addr) {
                    // typ. because we already switch to c2 (msg SET_C1_C2 and CMD_BEMF_DETECT_ON_C2 cross over
                    itm_debug3(DBG_CTRL, "not c2", tidx, m.v1u, tvar->can2_addr);
                    break;
                }
                if (tvar->measure_pose_percm) {
                	tvar->measure_pose_percm = 0;
                	lsblk_num_t s1 = {1};
                	posecm_measured(tidx, m.v2*10, s1);
                }
                ctrl2_evt_entered_c2(tidx, tvar, 1);
                break;
            }
            case CMD_MDRIVE_SPEED_DIR:
                ctrl2_upcmd_set_desired_speed(tidx, tvar, m.v2*m.v1u);
                break;

            case CMD_POSE_TRIGGERED:
                itm_debug3(DBG_POSE, "Trig", m.v1u, m.v2u, m.subc);
                ctrl2_evt_pose_triggered(tidx, tvar, m.v1u, m.subc, m.v2);
                break;
            case CMD_STOP_DETECTED:
                ctrl2_evt_stop_detected(tidx, tvar, m.v32);
                break;
            default:
                break;

            }
		} else {
			itm_debug1(DBG_MSG|DBG_CTRL, "bad msg", m.to);
		}
	}
    
    static int nsk=0;
    nsk++;
    if ((trctl[0]._mode != train_auto) && (nsk==100)) {
        ctrl_set_mode(0, train_auto);
    }
    if ((trctl[1]._mode != train_auto) && (nsk==100)) {
        ctrl_set_mode(1, train_auto);
    }

	check_timers(tick);
    
    uint8_t occ = topology_or_occupency_changed;
    topology_or_occupency_changed = 0;
    
    if (occ) {
        itm_debug1(DBG_CTRL, "ct/occ", 0);
    }
    for (int tidx = 0; tidx<NUM_TRAINS; tidx++) {
        train_ctrl_t *tvars = &trctl[tidx];
        const train_config_t *tconf = get_train_cnf(tidx);
        if (!tconf->enabled) continue;
        if (tvars->_mode == train_notrunning) continue;
        ctrl2_tick_process(tidx, tvars, tconf, occ);
    }
    //occupency_changed = 0;
    
	//check_blk_tick(tick);
	check_behaviour(tick);
	//hi_tick(notif_flags, tick, dt);

	if ((1)) {
		void txframe_send_stat(void);
		txframe_send_stat();
	}
}

// ---------------------------------------------------------------



// ---------------------------------------------------------------

static void evt_tleave(int tidx, train_ctrl_t *tvars)
{
    if (ignore_ina_presence) {
        itm_debug2(DBG_ERR|DBG_CTRL, "TLeave", tidx, tvars->_state);
        ctrl2_evt_leaved_c1(tidx, tvars);
    } else if (tvars->c1c2){
        // this is TGUARD
        itm_debug2(DBG_ERR|DBG_CTRL, "TGuard", tidx, tvars->_state);
        // for now we do the same, but more to do for long trains
        ctrl2_evt_leaved_c1(tidx, tvars);
    } else {
        itm_debug2(DBG_ERR|DBG_CTRL, "TGurd/bdst", tidx, tvars->_state);
    }
}







static void evt_timer(int tidx, train_ctrl_t *tvar, int tnum)
{
	itm_debug2(DBG_CTRL, "timer evt", tidx, tnum);
	switch (tnum) {
	case TLEAVE_C1:
		evt_tleave(tidx, tvar);
		break;
	case TAUTO:
		cauto_had_timer(tidx, tvar);
		break;
	default:
		itm_debug2(DBG_ERR|DBG_CTRL, "?TIM", tidx, tnum);
		fatal();
		break;
	}
}


// ---------------------------------------------------------------





// ---------------------------------------------------------------

// ---------------------------------------------------------------



// ---------------------------------------------------------------



// ---------------------------------------------------------------
//
// this is the MAIN turnout cmd : (thru CMD_TURNOUT_HI_A/B)
// - sends cmd to turnout tasklet
// - updates topology
// - sends info to UI (cto)

static void set_turnout(int tn, int v)
{
	itm_debug2(DBG_CTRL, "TURN", tn, v);
	if (tn<0) fatal();
	if (tn>=NUM_TURNOUTS) fatal();
	if (tn>=NUM_LOCAL_TURNOUTS) fatal(); // TODO
    msg_64_t m = {0};
	m.from = MA_CONTROL();
	m.to = MA_TURNOUT(0, tn); // TODO board num
	m.cmd = v ? CMD_TURNOUT_B : CMD_TURNOUT_A;

	mqf_write_from_ctrl(&m);
	topolgy_set_turnout(tn, v);
    
    // forward to CTO
    m.to = MA_UI(UISUB_TRACK);
    m.v2 = tn;
    mqf_write_from_ctrl(&m);
}

void ctrl2_set_turnout(int tn, int v)
{
    set_turnout(tn, v);
}


void ctrl2_send_led(uint8_t led_num, uint8_t prog_num)
{
    msg_64_t m = {0};
    m.from = MA_CONTROL();
    m.to = MA_LED_B(0); // TODO board num
    m.cmd = CMD_LED_RUN;
    m.v1u = led_num;
    m.v2u = prog_num;
    
    mqf_write_from_ctrl(&m);
}

// ---------------------------------------------------------------

static void check_behaviour(_UNUSED_ uint32_t tick)
{
#ifdef OLD_CTRL
	for (int tidx = 0; tidx<NUM_TRAINS; tidx++) {
		const train_config_t *tconf = get_train_cnf(tidx);
		if (!tconf->enabled) continue;

		if (!SCEN_TWOTRAIN) return; // XXX

		train_ctrl_t *tvars = &trctl[tidx];

		uint16_t flags = tvars->behaviour_flags;

		//itm_debug3(DBG_CTRL, "(hi f)", tidx, flags, tvars->canton1_addr);

		if (tvars->_mode != train_auto) {
			if ((tidx == 0) && (flags & BEHAVE_EOT2) && (tvars->_dir<0)) {
				ctrl_set_mode(tidx, train_auto);
				ctrl_set_timer(tidx, tvars, TBEHAVE, 500);
			}
			continue;
		}
		if (!flags) continue;

		tvars->behaviour_flags = 0;
        
		// ---- behave
		itm_debug3(DBG_CTRL, "hi f=", tidx, flags, tvars->c1_sblk.n);
		if (tidx == 1) {
			if ((flags & BEHAVE_RESTARTBLK) && (tvars->c1_sblk.n == 2)) {
                set_turnout(0, 1);
				continue;
			}
			if ((flags & BEHAVE_EOT2) && (tvars->_dir > 0)) {
				ctrl_set_timer(tidx, tvars, TBEHAVE, 1*60*1000);
				ctrl_evt_cmd_set_setdirspeed(tidx, tvars, 0, 0, 1);
                //set_turnout(0, 1);
				return;
			}
			if ((flags & BEHAVE_EOT2) && (tvars->_dir < 0)) {
				ctrl_set_timer(tidx, tvars, TBEHAVE, 300);
				ctrl_evt_cmd_set_setdirspeed(tidx, tvars, 0, 0, 1);
				set_turnout(0, 0);
				continue;
			}
			if (flags & BEHAVE_EOT2) {
				itm_debug3(DBG_CTRL, "unex EOT2", tidx, tvars->_dir, tvars->c1_sblk.n);
				ctrl_set_timer(tidx, tvars, TBEHAVE, 300);
				continue;
			}
			if (flags & BEHAVE_TBEHAVE) {
				if (tvars->c1_sblk.n == 1) {
					ctrl_evt_cmd_set_setdirspeed(tidx, tvars, -1, 40, 1);
				} else if (tvars->c1_sblk.n == 2) {
					ctrl_evt_cmd_set_setdirspeed(tidx, tvars, 1, 40, 1);
				} else {
					itm_debug3(DBG_CTRL, "unex TB", tidx, tvars->_dir, tvars->can1_addr);
				}
				continue;
			}
		}
		if (tidx == 0) {
			if (flags & BEHAVE_TBEHAVE) {
				itm_debug2(DBG_CTRL, "TBehave", tidx, tvars->c1_sblk.n);
				if (tvars->c1_sblk.n == 0) {
					ctrl_evt_cmd_set_setdirspeed(tidx, tvars, 1, 95, 1);
				} else if (tvars->can1_addr == MA_CANTON(0,1)) {
					ctrl_evt_cmd_set_setdirspeed(tidx, tvars, -1, 95, 1);
				} else {
					// should not happen
					ctrl_set_mode(tidx, train_manual);
				}
				continue;
			}
			if (flags & BEHAVE_EOT2)  {
				ctrl_evt_cmd_set_setdirspeed(tidx, tvars, 0, 0, 1);
                if (tvars->c1_sblk.n == 0) {
                    ctrl_set_timer(tidx, tvars, TBEHAVE, 1000*5);
                    set_turnout(0, 1);
                } else if (tvars->c1_sblk.n == 1) {
                    ctrl_set_timer(tidx, tvars, TBEHAVE, 1000*60*1);
                }
				continue;
			}
		}
	}
#else
    // TODO abort();
#endif
}






