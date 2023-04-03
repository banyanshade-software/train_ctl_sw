/*
 * ctrl.c
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */


// #define RECORD_MSG 1



#include "../misc.h"
#include "../msg/trainmsg.h"
#include "ctrl.h"

#include "../topology/topology.h"
//#include "../railconfig.h"
#include "../config/conf_train.h"

#include "../statval.h"

#include "../topology/occupency.h"

#include "ctrlLT.h"


#include "detectP.h"

#include "../leds/led.h"
#include "../config/conf_globparam.h"
#include "trace_train.h"
#include "../oam/oam.h"
#include "ctc_periodic_refresh.h"

#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif





//static train_oldctrl_t otrctl[NUM_TRAINS] = {0};
static train_ctrl_t trctl[NUM_TRAINS] = {0};


#if NUM_TRAINS == 0
#error ctrl is included in build, but NUM_TRAINS is zero. check train.cnf and other cnf files
#endif


//static lsblk_num_t snone = {-1};

// -----------------------------------------------------------

const stat_val_t statval_ctrl[] = {
		{ trctl, offsetof(train_ctrl_t, _sdir),             sizeof(uint8_t)         _P("T#_ctrl_dir")},
		{ trctl, offsetof(train_ctrl_t, _target_unisgned_speed),    sizeof(uint16_t)        _P("T#_ctrl_target_speed")},
        { trctl, offsetof(train_ctrl_t, _desired_signed_speed),    sizeof(uint16_t)        _P("T#_ctrl_desired_speed")},
        { trctl, offsetof(train_ctrl_t, _state),           sizeof(train_state_t)   _P("T#_ctrl_state")},

        { trctl, offsetof(train_ctrl_t, c1_sblk.n),        sizeof(uint8_t)         _P("T#_ctrl_canton1_lsb")},
        { trctl, offsetof(train_ctrl_t, _curposmm),        sizeof(int32_t)         _P("T#_ctrl_curposmm")},
        { trctl, offsetof(train_ctrl_t, beginposmm),       sizeof(int32_t)         _P("T#_ctrl_beginposmm")},

#ifndef REDUCE_STAT
        { trctl, offsetof(train_ctrl_t, can1_addr),        sizeof(uint8_t)         _P("T#_ctrl_canton1_addr")},
        { trctl, offsetof(train_ctrl_t, can2_addr),        sizeof(uint8_t)         _P("T#_ctrl_canton2_addr")},
        { trctl, offsetof(train_ctrl_t, spd_limit),        sizeof(uint16_t)        _P("T#_ctrl_spd_limit")},
#endif
        { NULL,  sizeof(train_ctrl_t), 0 _P(NULL)}
};


// ------------------------------------------------------

static void ctrl_init(void);
static void _ctrl_init(int normalmode);
static void ctrl_enter_runmode(runmode_t m);
static void ctrl_tick(uint32_t tick, _UNUSED_ uint32_t dt);

static msg_handler_t msg_handler_selector(runmode_t);

static const tasklet_def_t ctrl_tdef = {
		.init 				= ctrl_init,
		.poll_divisor		= NULL,
		.emergency_stop 	= ctrl_init,
		.enter_runmode		= ctrl_enter_runmode,
		.pre_tick_handler	= check_block_delayed,
		.default_msg_handler = NULL,
		.default_tick_handler = ctrl_tick,
		.msg_handler_for	=  msg_handler_selector,
		.tick_handler_for 	= NULL,

		.recordmsg			= RECORD_MSG,

};
tasklet_t ctrl_tasklet = { .def = &ctrl_tdef, .init_done = 0, .queue=&to_ctrl};

// ------------------------------------------------------
static void normal_process_msg(msg_64_t *m);


msg_handler_t msg_handler_selector(runmode_t m)
{
    switch (m) {
        case runmode_detect2:
            return detect2_process_msg;
            break;
        case runmode_normal:
            return normal_process_msg;
            break;
        default:
            return NULL;
            break;
    }
}

// ------------------------------------------------------


static void ctrl_enter_runmode(runmode_t m)
{
    if (m != runmode_normal) {
        for (int i=0; i<NUM_TRAINS; i++) {
            ctrl_set_mode(i, train_notrunning);
        }
    }
    switch (m) {
        default:
            break;
        case runmode_normal:
            _ctrl_init(1);
            break;
        case runmode_detect2:
            detect2_init();
            break;
            
    }
}
// ------------------------------------------------------

/*
static void fatal(void)
{
	itm_debug1(DBG_ERR, "fatal", 0);
#ifdef TRAIN_SIMU
    abort();
#else
	FatalError("ABRT", "sub_presence_changed, Error_NotImpl);
#endif
}
*/

// ----------------------------------------------------------------------------
// train FSM

//static void evt_timer(int tidx, train_oldctrl_t *tvar, int tnum);

// ----------------------------------------------------------------------------
// sub block presence handling

static void sub_presence_changed(uint8_t from_addr, uint8_t segnum, uint16_t v, int16_t ival);
static void notify_presence_changed(uint8_t from_addr, uint8_t segnum, uint16_t v, int16_t ival);


// ----------------------------------------------------------------------------
// turnouts
//static int ctrl_set_turnout(xtrnaddr_t tn, enum topo_turnout_state v, int train);
static void set_door_ack(xtrnaddr_t tn, enum topo_turnout_state v);



// ----------------------------------------------------------------------------

void ctrl_set_mode(int trnum, train_mode_t mode)
{
    ctrl3_set_mode(trnum, &trctl[trnum], mode);
    trctl[trnum]._mode = mode;
}

// ----------------------------------------------------------------------------



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

static void ctrl_init(void)
{
	_ctrl_init(0);
}

static void _ctrl_init(int normalmode)
{
	if ((1)) {
		// sanity check
		xblkaddr_t t;
		t.v = 0x24;
		if (t.board != 2) FatalError("chk1", "check xblkaddr_t", Error_Check);
		if (t.canton != 4) FatalError("chk2", "check xblkaddr_t", Error_Check);
		xtrnaddr_t b;
		b.v = 0x47;
		if (b.board != 4) FatalError("chk3", "check xblkaddr_t", Error_Check);
		if (b.turnout != 7) FatalError("chk4", "check xblkaddr_t", Error_Check);
	}

	//memset(otrctl, 0, sizeof(train_oldctrl_t)*NUM_TRAINS);
	occupency_clear();

	if (normalmode) {
		ctrl_set_mode(0, train_manual);
		//ctrl_set_mode(1, train_auto);
		xtrnaddr_t t0 = { .v = 0};
		xtrnaddr_t t1 = { .v = 1};
		ctrl_set_turnout(t0, topo_tn_straight, -1);
		ctrl_set_turnout(t1, topo_tn_straight, -1);
		const _UNUSED_ lsblk_num_t s0 = {0};
		const _UNUSED_ lsblk_num_t s2 = {2};
		const _UNUSED_ lsblk_num_t s5 = {5};
		const _UNUSED_ lsblk_num_t s3 = {3};
		const _UNUSED_ lsblk_num_t s7 = {7};
		const _UNUSED_ lsblk_num_t s8 = {8};
		if ((1)) {
#ifdef TRAIN_SIMU
            void simu_set_train(int tidx, int sblk, int posmm); // SimTrain.m

            // 172, 170, 160, 150
            // 175, 190, 220, 250, 260
            // ok : 350, 300, 280, 270, 265, 262, 261
            //ctrl3_init_train(0, &trctl[0], s5, 260, 1);
            ctrl3_init_train(0, &trctl[0], s0, 260, 1);
            simu_set_train(0, trctl[0].c1_sblk.n, trctl[0]._curposmm);
            ctrl_set_mode(0, train_manual);

            /*
            ctrl3_init_train(1, &trctl[1], s8, 400, 1);
            simu_set_train(1, trctl[1].c1_sblk.n, trctl[1]._curposmm);
            */
			//ctrl2_init_train(0, &otrctl[0], s2);
#else
            ctrl3_init_train(0, &trctl[0], s5, 300, 1);
            ctrl_set_mode(0, train_manual);
            ctrl3_init_train(1, &trctl[1], s8, 400, 1);
            ctrl_set_mode(1, train_manual);
            ctrl3_init_train(2, &trctl[2], s7, 200, 1);
            ctrl_set_mode(2, train_manual);
#endif

			


		} else {
            //ctrl2_init_train(0, &otrctl[0], s0);
            //ctrl2_init_train(0, &otrctl[0], s0);
            ctrl3_init_train(0, &trctl[0], s0, 300, 1);
            //ctrl3_init_train(0, &trctl[0], s0, 1);
			//ctrl2_init_train(1, &trctl[1], s2);
			ctrl_set_mode(0, train_manual);
			ctrl_set_mode(1, train_notrunning);
		}
	}
}

/*
train_oldctrl_t *ctrl_get_tvar(int trnum)
{
    abort();
    return NULL;  //&otrctl[trnum];
}
 */

// ----------------------------------------------------------------------------
// timers

/*
static void check_timers(uint32_t tick)
{
	//uint32_t t = HAL_GetTick();
	for (int tidx = 0; tidx<NUM_TRAINS; tidx++) {
		train_oldctrl_t *tvar = &otrctl[tidx];
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
*/

static void ctrl_tick(uint32_t tick, _UNUSED_ uint32_t dt)
{
    //check_timers(tick);
    
    
    uint16_t occ = _topology_or_occupency_changed;
    _topology_or_occupency_changed = 0;
    
    if (occ) {
        itm_debug1(DBG_CTRL, "ct/occ", 0);
    }
    if (occ) {
        
        for (int tidx = 0; tidx<NUM_TRAINS; tidx++) {
            uint16_t v = ~(1<<tidx);
            if (0==(occ & v)) continue;
            train_ctrl_t *tvars = &trctl[tidx];
            trace_train_postick(tick, tidx, tvars);
            if (occ) ctrl3_occupency_updated(tidx, tvars);
            
            /*
             train_oldctrl_t *otvars = &otrctl[tidx];
             const conf_train_t *tconf = conf_train_get(tidx);
             if (!tconf->enabled) continue;
             if (tvars->_mode == train_notrunning) continue;
             if ((0)) ctrl2_tick_process(tidx, otvars, tconf, occ); // xxxxxx
             */
        }
        
    }
    if (Oam_Ready) ctc_periodic_refresh(tick);
}
// ----------------------------------------------------------------------------



static void notify_presence_changed(_UNUSED_ uint8_t from_addr,  uint8_t lsegnum,  uint16_t p, _UNUSED_ int16_t ival)
{
	// TODO : from_addr should be used for board number
	static uint32_t pres = 0;
	int n = topology_num_sblkd();
	for (int i=0; i<n; i++) {
		lsblk_num_t ln;
		ln.n = i;
		int li = get_lsblk_ina3221(ln);
		if (li != lsegnum) continue;
		if (i>31) break;
		if (p) pres |= (1<<i);
		else   pres &= ~(1<<i);
	}
	msg_64_t m = {0};
	m.cmd = CMD_NOTIF_PRES;
	m.v32u = pres;
	m.to = MA3_UI_CTC;
	m.from = MA1_CONTROL();
	mqf_write_from_ctrl(&m);
}

static void bh(void)
{

}

static void sub_presence_changed(_UNUSED_ uint8_t from_addr,  uint8_t lsegnum,  uint16_t p, _UNUSED_ int16_t ival)
{
	if (p && (4==lsegnum)) {
		bh();
	}
	itm_debug3(DBG_PRES|DBG_CTRL, "PrsChg-", lsegnum, p, ival);
	// FatalError("ABRT", "sub_presence_changed", Error_Abort);

    // TODO : from_addr should be used for board number
	for (int tidx=0; tidx < NUM_TRAINS; tidx++) {
		train_ctrl_t *tvar = &trctl[tidx];
        if (tvar->_mode == train_notrunning) continue;
        uint8_t is_s1 = 0;
        uint8_t is_s2 = 0;
        uint8_t is_c1 = 0;
        uint8_t is_c2 = 0;

        if ((tvar->c1_sblk.n != -1) && (lsegnum == get_lsblk_ina3221(tvar->c1_sblk))) {
            is_s1 = 1;
        }
    	xblkaddr_t cl = get_canton_for_ina(lsegnum);
        if (tvar->can1_xaddr.v != 0xFF) {
        	if (cl.v == tvar->can1_xaddr.v) is_c1 = 1;
        }
        if (tvar->can2_xaddr.v != 0xFF) {
        	if (cl.v == tvar->can2_xaddr.v) is_c2 = 1;
        }
        lsblk_num_t n2 = next_lsblk(tvar->c1_sblk, tvar->_sdir<0, NULL);
        if ((n2.n>=0) && (lsegnum == get_lsblk_ina3221(n2))) {
        	is_s2 = 1;
        }
        if (is_s1 || is_c1 || is_s2 || is_c2) {
        	itm_debug3(DBG_PRES|DBG_CTRL, "PrsChg ", tidx, p, is_s1);
        	itm_debug3(DBG_PRES|DBG_CTRL, "PrsChg.", is_s2, is_c1, is_c2);
        } else {
            continue;
        }
        if (p) {
            trace_train_ina3221(ctrl_tasklet.last_tick, tidx, lsegnum, 1);
            if ((0)) {
            } else if (is_s2 && !is_s1 && !is_c2) {
                ctrl3_evt_entered_new_lsblk_same_canton(tidx, tvar, n2, 0);
            } else if (is_s2 && !is_s1 && is_c2) {
                ctrl3_evt_entered_new_lsblk_c2_canton(tidx, tvar, n2);
            } else if (is_c1 && is_s1) {
                // normal condition, just a intensity toggled
            } else if (is_c1 && !is_s1 && !is_s2) {
            	// could be next(next(s1), if next(s1) is not ina controlled
                // and if pose is not correctly adjusted
                lsblk_num_t b = n2;
                for (;;) {
                    if (b.n == -1) break; // for first one, n2 might be -1
                    b = next_lsblk(b, tvar->_sdir<0, NULL);
                    if (b.n == -1) break;
                    if (canton_for_lsblk(b).v != tvar->can1_xaddr.v) break; // other canton, stop searching
                    if (get_lsblk_ina3221(b) == lsegnum) {
                        // found
                        ctrl3_evt_entered_new_lsblk_same_canton(tidx, tvar, b, 1);
                        return;
                    }
                }
            	itm_debug3(DBG_ERR|DBG_CTRL, "badSub2", tidx, lsegnum, tvar->can1_xaddr.v);
            	if ((1)) {
            		msg_64_t m = {0};
            		m.cmd = CMD_USB_TRACETRAIN;
            		m.from = MA1_CONTROL();
            	    m.to = MA2_USB_LOCAL;
            	    m.v1 = tidx;
                    mqf_write_from_ctrl(&m);
            	} else {
            		FatalError("bSub2", "bad subc2", Error_Abort);
            	}
            } else if (is_c2) {
                itm_debug2(DBG_ERR|DBG_CTRL, "sub/c2", tidx, lsegnum);
            } else {
            	FatalError("bSubc", "bad subc", Error_Abort);
            }
        
        } else {
            trace_train_ina3221(ctrl_tasklet.last_tick, tidx, lsegnum, 0);
        	/*if ((0)) { // XXX
        		if (is_c2) {
        			ctrl3_evt_leaved_c2(tidx, tvar);
        		} else if (is_c1 && is_s1) {
        			ctrl3_evt_leaved_c1(tidx, tvar);
        		}
        	}*/
        }

	}
}

// ----------------------------------------------------------------------------

/*
static void posecm_measured(int tidx, int32_t pose, lsblk_num_t blk1, lsblk_num_t blk2)
{
	int cm = get_lsblk_len_cm(blk1, NULL);
	if (blk2.n >= 0) cm += get_lsblk_len_cm(blk2, NULL);
	int32_t ppcm = pose / cm;
	itm_debug2(DBG_POSEC, "ppcm", tidx, ppcm);
	debug_info('P', tidx, "PPCM", ppcm, 0,0);

	if (abs(ppcm)<250) {
		itm_debug2(DBG_ERR, "sucp PPCM", tidx, ppcm);
		return;
	}
	const conf_train_t *tconf = conf_train_get(tidx);
	conf_train_t *wconf = (conf_train_t *)tconf; // writable
	const int alpha = 80; //0.80
	ppcm = abs(ppcm);
	wconf->pose_per_cm = (tconf->pose_per_cm * alpha + (100-alpha) * ppcm)/100;
	itm_debug2(DBG_CTRL|DBG_POSEC, "PPCM updated", tidx, wconf->pose_per_cm);

}
*/

// ----------------------------------------------------------------------------

/* to be refactor
static uint8_t auto_code[8][64];

uint8_t *ctrl_get_autocode(int numtrain)
{
	train_oldctrl_t *tvar = ctrl_get_tvar(numtrain);
	if (!tvar->route) {
		tvar->route = auto_code[numtrain];
	}
	return tvar->route;
}
 */
void ctrl_delayed_set_desired_spd(int tidx, int spd)
{
    train_ctrl_t *tvars = &trctl[tidx];
    if (tvars->has_delayed_spd) {
        FatalError("dely", "has_delayed_spd already set", Error_Abort);
    }
    tvars->has_delayed_spd = 1;
    tvars->delayed_spd = spd;
    /*
    if (spd) {
        ctrl3_upcmd_set_desired_speed(tidx, tvars, spd);
    } else {
        ctrl3_upcmd_set_desired_speed_zero(tidx, tvars);
    }
     */
}

static void normal_process_msg(msg_64_t *m)
{
    // -----------------------------------------
    xtrnaddr_t turnout = {0};
    switch (m->cmd) {
        default:
            break;
        case CMD_TURNOUT_HI_A:
        	// TODO : board
            turnout.v = m->v1u;
            ctrl_set_turnout(turnout, topo_tn_straight, -1);
            break;
        case CMD_TURNOUT_HI_B:
            turnout.v = m->v1u;
            ctrl_set_turnout(turnout, topo_tn_turn, -1);
            break;
        case CMD_TURNOUT_HI_TOG:
            turnout.v = m->v1u;
            enum topo_turnout_state s = topology_get_turnout(turnout);
            if (s != topo_tn_moving) {
                ctrl_set_turnout(turnout, s ? topo_tn_straight : topo_tn_turn, -1);
            }
            break;
        case CMD_SERVO_ACK:
            itm_debug2(DBG_CTRL, "servAck", m->subc, m->v1u);
            break;
        case CMD_SERVODOOR_ACK:
            itm_debug2(DBG_CTRL|DBG_SERVO, "servDAck", m->subc, m->v1u);
            turnout.turnout = m->subc;
            turnout.board = MA0_BOARD(m->from);
            turnout.isdoor = 1;
            set_door_ack(turnout, (enum topo_turnout_state)m->v1u);
            break;
    }
    // -----------------------------------------
    if (MA1_IS_CTRL(m->to)) {
        //if (test_mode) continue;
        int tidx = MA1_TRAIN(m->to);
        //train_oldctrl_t *otvar = &otrctl[tidx];
        train_ctrl_t *tvars = &trctl[tidx];
        //extern uint8_t Auto1ByteCode[]; // XXX temp hack
        switch (m->cmd) {
            case CMD_MDRIVE_SPEED_DIR: {
                int16_t spd = m->v2*m->v1u;
                if (spd) {
                    ctrl3_upcmd_set_desired_speed(tidx, tvars, spd);
                } else {
                    ctrl3_upcmd_set_desired_speed_zero(tidx, tvars);
                }
            }
            break;

                
                
        case CMD_SET_TRAIN_MODE:
            ctrl_set_mode(m->v1u, m->v2u);
            break;
        case CMD_START_AUTO:
            switch (m->v1u) {
                case 1:
                    ctrl_set_mode(tidx, train_manual); // make sure it is restarted if already auto
                    ctrl_set_mode(tidx, train_auto);
                    break;
                default:
                    itm_debug2(DBG_CTRL|DBG_ERR, "bad cauto", tidx, m->v1u);
                    break;
            }
            break;
        case CMD_PRESENCE_SUB_CHANGE:
            if ((1)) {
                debug_info('I', m->subc, "INA", m->v1u, m->v2, 0);
            }
            notify_presence_changed(m->from, m->subc, m->v1u, m->v2u);
            if (ignore_ina_pres()) break;
            sub_presence_changed(m->from, m->subc, m->v1u, m->v2);
            break;
    
        case CMD_BEMF_DETECT_ON_C2: {
            itm_debug2(DBG_CTRL,"BEMF/C2", tidx,  m->v1u);
            if (m->v1u != tvars->can2_xaddr.v) {
                itm_debug3(DBG_CTRL, "not c2", tidx, m->v1u, tvars->can2_xaddr.v);
                break;
            }
            /*if (tvars->measure_pose_percm) {
                tvars->measure_pose_percm = 0;
                lsblk_num_t s1 = {1};
                lsblk_num_t s4 = {4};
                posecm_measured(tidx, m->v2*10, s1, s4);
            }*/
            ctrl3_evt_entered_c2(tidx, tvars, 1);
            //ctrl2_evt_entered_c2(tidx, otvar, 1);
            break;
        }
           
        case CMD_POSE_TRIGGERED:
            itm_debug3(DBG_POSEC, "Trig", m->v1u, m->v2u, m->subc);
            //trace_train_trig(ctrl_tasklet.last_tick, tidx, tvars, (pose_trig_tag_t)m->vcu8, m->va16);
            xblkaddr_t tb = FROM_CANTON(*m);
            // int ctrl2_evt_pose_triggered(int tidx, train_oldctrl_t *tvar, const conf_train_t *tconf, xblkaddr_t ca_addr, uint8_t tag, int16_t cposd10)
            //ctrl2_evt_pose_triggered(tidx, otvar, NULL, tb, m->vcu8, m->va16);
            ctrl3_pose_triggered(tidx, tvars, m->vcu8, tb, m->va16);
            break;
        case CMD_STOP_DETECTED:
            //ctrl2_evt_stop_detected(tidx, otvar, NULL, m->v32);
            ctrl3_stop_detected(tidx, tvars);
            break;
        default:
            break;

        }
        if (tvars->c1c2dir_changed) {
        	FatalError("c1c2", "unhandled c1c2", Error_Abort);
        }
        if (tvars->has_delayed_spd) {
            tvars->has_delayed_spd = 0;
            int8_t spd = tvars->delayed_spd;
            if (spd) {
                ctrl3_upcmd_set_desired_speed(tidx, tvars, spd);
            } else {
                ctrl3_upcmd_set_desired_speed_zero(tidx, tvars);
            }
        }
    } else {
        itm_debug1(DBG_MSG|DBG_CTRL, "bad msg", m->to);
    }
}


// called by different thread (planner) ! WARNING
int ctrl_get_train_curlsblk(int numtrain)
{
    train_ctrl_t *tvars = &trctl[numtrain];
    // we assume uint8 load/store is atomic
    // XXX is it ????
    int n = tvars->c1_sblk.n;
    return n;
}



__weak int can_send_stat(void)
{
    static int cnt = 0;
    cnt++;
    if (0==(cnt % 300)) return 1;
    return 0;
}

// ---------------------------------------------------------------

int ignore_ina_pres(void)
{
	if (ignore_ina_presence) return 1;
	if (conf_globparam_get(0)->ignoreIna3221) return 1;
	return 0;
}

int ignore_bemf_pres(void)
{
    if (ignore_ina_presence) return 0;
    if (ignore_bemf_presence) return 1;
    return 0;
}

// ---------------------------------------------------------------

/*
static void evt_tleave(int tidx, train_oldctrl_t *tvars)
{
    if (ignore_ina_pres() || (get_lsblk_ina3221(tvars->c1_sblk) == 0xFF)) {
        itm_debug2(DBG_ERR|DBG_CTRL, "TLeave", tidx, tvars->_ostate);
        ctrl2_evt_leaved_c1(tidx, tvars);
    } else if (tvars->c1c2){
        // this is TGUARD
        itm_debug2(DBG_ERR|DBG_CTRL, "TGuard", tidx, tvars->_ostate);
        // for now we do the same, but more to do for long trains
        ctrl2_evt_leaved_c1(tidx, tvars);
    } else {
        itm_debug2(DBG_ERR|DBG_CTRL, "TGurd/bdst", tidx, tvars->_ostate);
    }
}

*/




/*
static void evt_timer(int tidx, train_oldctrl_t *tvar, int tnum)
{
	itm_debug2(DBG_CTRL, "timer evt", tidx, tnum);
	switch (tnum) {
	case TLEAVE_C1:
		if (!ignore_ina_pres()) {
			itm_debug1(DBG_CTRL|DBG_ERR, "TLEAVE", tidx);
		}
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
*/
    
    
// ---------------------------------------------------------------
//
// this is the MAIN turnout cmd : (thru CMD_TURNOUT_HI_A/B)
// - sends cmd to turnout tasklet
// - updates topology
// - sends info to UI (cto)


int ctrl_set_turnout(xtrnaddr_t tn, enum topo_turnout_state v, int train)
{
	itm_debug2(DBG_CTRL|DBG_TURNOUT, "TURN", tn.v, v);
    if (tn.v == 0xFF) {
    	itm_debug1(DBG_ERR|DBG_CTRL, "bad tn", train);
    	FatalError("TNf", "bad tn", Error_Abort);
    }


    enum topo_turnout_state val = v;
    if (tn.isdoor) {
        val = topo_tn_moving;
    }
	int rc = topology_set_turnout(tn, val, train);
    if (rc) {
    	itm_debug3(DBG_CTRL|DBG_TURNOUT, "tn busy", train, tn.v, rc);
    	return rc;
    }
    
    msg_64_t m = {0};
    if (tn.isdoor) {
        // send to servo
        m.from = MA1_CONTROL();
        m.to = MA0_SERVO(tn.board);
        m.subc = tn.turnout;
        m.cmd = CMD_SERVODOOR_SET;
        m.v1u = v ? 1 : 0;
        mqf_write_from_ctrl(&m);
        
        if ((1)) {
            // forward to UI/CTO
            m.to = MA3_UI_CTC;
            m.subc = tn.v;
            m.cmd = CMD_TN_CHG_NOTIF;
            m.v1 = topo_tn_moving;
            mqf_write_from_ctrl(&m);
        }
    } else {
        m.from = MA1_CONTROL();
        m.to = MA0_TURNOUT(tn.board);
        m.subc = tn.turnout;
        m.cmd = v ? CMD_TURNOUT_B : CMD_TURNOUT_A;
        mqf_write_from_ctrl(&m);
        
        // forward to UI/CTO
        m.to = MA3_UI_CTC;
        m.subc = tn.v;
        m.cmd = CMD_TN_CHG_NOTIF;
        m.v1 = v ? 1 : 0;
        mqf_write_from_ctrl(&m);
    }
   
    return 0;
}


static void set_door_ack(xtrnaddr_t tn, enum topo_turnout_state v)
{
    itm_debug2(DBG_CTRL|DBG_TURNOUT, "DACK", tn.v, v);
    if (tn.v == 0xFF) {
    	itm_debug1(DBG_ERR|DBG_CTRL, "bad dack", 0);
    	FatalError("DACK", "bad tn", Error_Abort);
    }

    if (!tn.isdoor) {
        itm_debug1(DBG_ERR|DBG_CTRL, "dack no d", tn.v);
        return;
    }
    
    int rc = topology_set_turnout(tn, v, -1);
    if (rc) {
        itm_debug2(DBG_CTRL|DBG_TURNOUT, "tn bsy-d", tn.v, rc);
        return;
    }
    
    
    msg_64_t m = {0};
    
    m.from = MA1_CONTROL();
    
    
    // forward to UI/CTO
    m.to = MA3_UI_CTC;
    m.subc = tn.v;
    m.cmd = CMD_TN_CHG_NOTIF;
    m.v1 = v;
    mqf_write_from_ctrl(&m);
    
}

int ctrl2_set_turnout(xtrnaddr_t tn, enum topo_turnout_state v, int train)
{
    return ctrl_set_turnout(tn, v, train);
}

 // #longtrain
/*
int ctrl2_lock_turnout(xtrnaddr_t tn, int train)
{
    int rc = occupency_turnout_reserve(tn, train);
    if (rc) {
        itm_debug3(DBG_CTRL, "tn busy", train, tn.v, rc);
        return rc;
    }
    return rc;
}
 */

void ctrl2_unlock_turnout(xtrnaddr_t tn, int train) // #longtrain
{
    occupency_turnout_release(tn, train);
}


void ctrl2_send_led(uint8_t led_num, uint8_t prog_num)
{
    msg_64_t m = {0};
    m.from = MA1_CONTROL();
    m.to = MA0_LED(0); // TODO board num
    m.cmd = CMD_LED_RUN;
    m.v1u = led_num;
    m.v2u = prog_num;
    
    mqf_write_from_ctrl(&m);
}

// ---------------------------------------------------------------





