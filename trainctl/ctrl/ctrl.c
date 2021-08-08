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


// for test/debug
static uint8_t ignore_bemf_presence = 0;
static uint8_t ignore_ina_presence = 1;


#define SCEN_TWOTRAIN 	0
#define EOT_SPD_LIMIT	50   //20

// timers number
#define TLEAVE_C1  	0
#define TBEHAVE		1
#define NUM_TIMERS 	2

// timers values in tick (ms)
#define TLEAVE_C1_VALUE 20
#define TGUARD_C1_VALUE 100


// -----------------------------------------------------------
//per train stucture


typedef struct {
	train_mode_t   _mode;
	train_state_t  _state;

	uint16_t _target_speed;
	int8_t   _dir;

	uint8_t  canton1_addr;
	uint8_t  canton2_addr;


	uint16_t spd_limit;
	uint16_t desired_speed;
	//uint8_t limited:1;

	uint16_t behaviour_flags;
	uint32_t timertick[NUM_TIMERS];
} train_ctrl_t;


static train_ctrl_t trctl[NUM_TRAINS] = {0};

// -----------------------------------------------------------

const stat_val_t statval_ctrl[] = {
        { trctl, offsetof(train_ctrl_t, _state),           sizeof(train_state_t)   _P("T#_ctrl_state")},
        { trctl, offsetof(train_ctrl_t, _dir),             sizeof(uint8_t)         _P("T#_ctrl_dir")},
        { trctl, offsetof(train_ctrl_t, _target_speed),    sizeof(uint16_t)        _P("T#_ctrl_target_speed")},
        { trctl, offsetof(train_ctrl_t, canton1_addr),     sizeof(uint8_t)         _P("T#_ctrl_canton1_addr")},
        { trctl, offsetof(train_ctrl_t, canton2_addr),     sizeof(uint8_t)         _P("T#_ctrl_canton2_addr")},
        { trctl, offsetof(train_ctrl_t, desired_speed),    sizeof(uint16_t)        _P("T#_ctrl_desired_speed")},
        { trctl, offsetof(train_ctrl_t, spd_limit),        sizeof(uint16_t)        _P("T#_ctrl_spd_limit")},
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
static void evt_cmd_set_setdirspeed(int tidx, train_ctrl_t *tvars, int8_t dir, uint16_t spd, _UNUSED_ uint8_t generated);
static void evt_pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t c_adder);
static void evt_entered_c2(int tidx, train_ctrl_t *tvar, uint8_t from_bemf);
static void evt_leaved_c1(int tidx, train_ctrl_t *tvar);
static void evt_entered_c1(int tidx, train_ctrl_t *tvar, uint8_t from_bemf);
static void evt_leaved_c2(int tidx, train_ctrl_t *tvar);
static void evt_timer(int tidx, train_ctrl_t *tvar, int tnum);

typedef enum {
	upd_init,
	upd_change_dir,
	upd_c1c2,
	upd_pose_trig,
	upd_check,
} update_reason_t;

static void update_c2_state_limits(int tidx, train_ctrl_t *tvars, update_reason_t reason);
static void set_pose_trig(int numtrain, int32_t pose);
static int32_t pose_middle(int blknum, const train_config_t *tconf, int dir);

static void ctrl_set_tspeed(int trnum, train_ctrl_t *tvars, uint16_t tspd);
static void ctrl_set_dir(int trnum,  train_ctrl_t *tvars, int  dir, int force);


static void check_blk_tick(uint32_t tick);


// ----------------------------------------------------------------------------
// generic timer attached to train_ctrl_t struct

static void reset_timer(int tidx, train_ctrl_t *tvar, int numtimer);
static void set_timer(int tidx, train_ctrl_t *tvar, int numtimer, uint32_t tval);
static void check_timers(uint32_t tick);

// ----------------------------------------------------------------------------
// sub block presence handling

static void sub_presence_changed(uint32_t tick, uint8_t from_addr, uint8_t segnum, uint16_t v, int16_t ival);

// ----------------------------------------------------------------------------
//  block presence


#define USE_BLOCK_DELAY_FREE 1

static void set_block_num_occupency(int blknum, uint8_t v);
static void set_block_addr_occupency(uint8_t blkaddr, uint8_t v);
static uint8_t get_block_num_occupency(int blknum);
static void check_block_delayed(uint32_t tick);


// ----------------------------------------------------------------------------
// turnouts
static void set_turnout(int tn, int v);

// ----------------------------------------------------------------------------
// behaviour

// behaviour_flags
#define BEHAVE_STOPPED		(1<<1)		// 2
#define BEHAVE_EOT1			(1<<2)		// 4
#define BEHAVE_EOT2			(1<<3)		// 8
#define BEHAVE_BLKW			(1<<4)		// 16
#define BEHAVE_PTRIG		(1<<5)		// 32
#define BEHAVE_RESTARTBLK 	(1<<6)		// 64
#define BEHAVE_TBEHAVE		(1<<7)		// 128
#define BEHAVE_CHBKLK		(1<<8)		// 256

static void check_behaviour(uint32_t tick);


static void ctrl_reset(void)
{
	//TODO
}
// ----------------------------------------------------------------------------


static inline void set_state(int tidx, train_ctrl_t *tvar, train_state_t ns)
{
	switch (ns) {
	case train_off: 			itm_debug1(DBG_CTRL, "ST->OFF", tidx); break;
	case train_running_c1: 		itm_debug1(DBG_CTRL, "ST->RC1", tidx); break;
	case train_running_c1c2: 	itm_debug1(DBG_CTRL, "ST->C1C2", tidx); break;
	case train_station:			itm_debug1(DBG_CTRL, "ST->STA", tidx); break;
	case train_blk_wait:	 	itm_debug1(DBG_CTRL, "ST->BLKW", tidx); break;
	case train_end_of_track:	itm_debug1(DBG_CTRL, "ST->EOT", tidx); break;
	default: 					itm_debug2(DBG_CTRL, "ST->?", tidx, ns); break;
	}
	tvar->_state = ns;
	// notif UI
	msg_64_t m;
	m.from = MA_CONTROL_T(tidx);
	m.to = MA_UI(UISUB_TFT);
	m.cmd = CMD_TRSTATE_NOTIF;
	m.v1u = ns;
	mqf_write_from_ctrl(&m);
}


static void ctrl_set_mode(int trnum, train_mode_t mode)
{
	itm_debug2(DBG_CTRL, "set mode", trnum, mode);
	if (trctl[trnum]._mode == mode) return;
	trctl[trnum]._mode = mode;
	// notif UI
	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
	m.to = MA_UI(UISUB_TFT);
	m.cmd = CMD_TRMODE_NOTIF;
	m.v1u = mode;
	mqf_write_from_ctrl(&m);
}

/*
static void ctrl_set_status(int trnum, train_status_t status)
{
	if (trctl[trnum]._status == status) return;
	trctl[trnum]._status = status;
	// notif UI
	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
	m.to = MA_UI(UISUB_TFT);
	m.cmd = CMD_TRSTATUS_NOTIF;
	m.v1u = status;
	mqf_write_from_ctrl(&m);
}
*/


static void ctrl_init(void)
{
	memset(trctl, 0, sizeof(train_ctrl_t)*NUM_TRAINS);
	ctrl_set_mode(0, train_manual);
	ctrl_set_mode(1, train_auto);
	set_turnout(0, 0);
	if ((1)) {
		trctl[0].canton1_addr = MA_CANTON(0, 1);//MA_CANTON(0, 1); // initial blk
		trctl[0].canton2_addr = 0xFF;
		trctl[0]._dir = 0;
		trctl[0].desired_speed = 0;
		trctl[0]._target_speed = 0;
		set_state(0, &trctl[0], train_station);
		set_block_addr_occupency(trctl[0].canton1_addr, BLK_OCC_STOP);

		if ((SCEN_TWOTRAIN)) {
			trctl[1].canton1_addr = MA_CANTON(0, 2); // initial blk
			trctl[1].canton2_addr = 0xFF;
			trctl[1]._dir = 1;
			trctl[1]._target_speed = 0;
			trctl[1].desired_speed = 12;
			set_state(1, &trctl[1], train_station);
			set_block_addr_occupency(trctl[1].canton1_addr, BLK_OCC_STOP);

			update_c2_state_limits(0, &trctl[0], upd_init);
			update_c2_state_limits(1, &trctl[1], upd_init);

			if ((1)) {
				evt_cmd_set_setdirspeed(1, &trctl[1], 1, 30, 1);
			}
		} else {
			trctl[1].canton1_addr = 0xFF;
			trctl[1].canton2_addr = 0xFF;
			ctrl_set_mode(1, train_notrunning);
			set_state(1, &trctl[1], train_off);
			//trctl[1].enabled = 0;
			update_c2_state_limits(0, &trctl[0], upd_init);

		}
	}
}

// ----------------------------------------------------------------------------
// timers


static void reset_timer(int tidx, train_ctrl_t *tvar, int numtimer)
{
	itm_debug2(DBG_CTRL, "reset_timer", tidx, numtimer);
	if (numtimer<0 || numtimer>=NUM_TIMERS) fatal();
	tvar->timertick[numtimer] = 0;
}
static void set_timer(int tidx, train_ctrl_t *tvar, int numtimer, uint32_t tval)
{
	itm_debug3(DBG_CTRL, "set_timer", tidx, numtimer, tval);
	if (numtimer<0 || numtimer>=NUM_TIMERS) fatal();
	tvar->timertick[numtimer] = HAL_GetTick() + tval;
}

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




static void sub_presence_changed(_UNUSED_ uint32_t tick, uint8_t from_addr, uint8_t lsegnum, uint16_t p, int16_t ival)
{
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
        }
        // -----------------------------------------
		if (IS_CONTROL_T(m.to)) {
			//if (test_mode) continue;
			int tidx = m.to & 0x7;
			train_ctrl_t *tvar = &trctl[tidx];

			switch (m.cmd) {
			case CMD_PRESENCE_CHANGE:{
				if (ignore_ina_presence) break;
				sub_presence_changed(tick, m.from, m.sub, m.v1u, m.v2);
				break;
			}
			case CMD_BEMF_DETECT_ON_C2: {
				itm_debug2(DBG_CTRL,"BEMF/C2", tidx,  m.v1u);
				train_ctrl_t *tvar = &trctl[tidx];
				if (m.v1u != tvar->canton2_addr) {
					// typ. because we already switch to c2 (msg SET_C1_C2 and CMD_BEMF_DETECT_ON_C2 cross over
					itm_debug3(DBG_CTRL, "not c2", tidx, m.v1u, tvar->canton2_addr);
					break;
				}
				evt_entered_c2(tidx, tvar, 1);
				// -----xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
				//if (ignore_bemf_presence) break;
				//const train_config_t *tconf = get_train_cnf(tidx);
				//train_switched_to_c2(tidx, tconf, tvar, 1);
				break;
			}
			case CMD_MDRIVE_SPEED_DIR:
				evt_cmd_set_setdirspeed(tidx, tvar, m.v2, m.v1u, 0);
				break;

			case CMD_POSE_TRIGGERED:
				itm_debug2(DBG_POSE, "Trig", m.v1u, m.v2u);
				evt_pose_triggered(tidx, tvar, m.v1u);
				break;
			default:
				break;

			}
		} else {
			itm_debug1(DBG_MSG|DBG_CTRL, "bad msg", m.to);
		}
	}
	check_timers(tick);
	check_blk_tick(tick);
	check_behaviour(tick);
	//hi_tick(notif_flags, tick, dt);

	if ((1)) {
		void txframe_send_stat(void);
		txframe_send_stat();
	}
}

// ---------------------------------------------------------------


static uint8_t blk_occup[NUM_CANTONS] = {0}; // TODO 32
static uint8_t occupency_changed = 0;


static uint8_t notif_blk_reset = 1;

static void notif_blk_occup_chg(int blknum, uint8_t val)
{
    msg_64_t m;
    m.from = MA_CONTROL();
    m.to = MA_UI(UISUB_TRACK);
    m.cmd = CMD_BLK_CHANGE;
    m.vbytes[0] = blknum;
    m.vbytes[1] = val;
    m.vbytes[2] = notif_blk_reset ? 1 : 0;
    notif_blk_reset = 0;
    mqf_write_from_ctrl(&m);
}

static void set_block_num_occupency(int blknum, uint8_t v)
{
	if (-1 == blknum) fatal();
	if (blk_occup[blknum] != v) {
		if (USE_BLOCK_DELAY_FREE && (v==BLK_OCC_FREE)) {
			if (blk_occup[blknum] >= BLK_OCC_DELAY1) fatal();
			blk_occup[blknum] = BLK_OCC_DELAYM;
			itm_debug1(DBG_CTRL, "delay free", blknum);
		} else {
			blk_occup[blknum] = v;
			occupency_changed = 1;
		}
	}
	if ((1)) {
		itm_debug3(DBG_CTRL, "BO123:", blk_occup[0], blk_occup[1], blk_occup[2]);
	}
    notif_blk_occup_chg(blknum, blk_occup[blknum]);
}

static void set_block_addr_occupency(uint8_t blkaddr, uint8_t v)
{
	set_block_num_occupency(_blk_addr_to_blk_num(blkaddr), v);
}

static uint8_t get_block_num_occupency(int blknum)
{
	if (-1 == blknum) fatal();
	return blk_occup[blknum];
}

_UNUSED_ static uint8_t get_block_addr_occupency(uint8_t blkaddr)
{
	return get_block_num_occupency(_blk_addr_to_blk_num(blkaddr));
}

static uint8_t occupied(int dir)
{
	if (dir<0) return BLK_OCC_LEFT;
	if (dir>0) return BLK_OCC_RIGHT;
	return BLK_OCC_STOP;
}

static void check_block_delayed(_UNUSED_ uint32_t tick)
{
	if (!USE_BLOCK_DELAY_FREE) return;
#if 0
	static int cnt = 0;
	cnt++;
	if (cnt % 10) return;
#else
	static uint32_t lastcheck = 0;
	if (tick<lastcheck+100) return;
	lastcheck = tick;
#endif
	for (int i=0; i<NUM_CANTONS; i++) {
		if (blk_occup[i] == BLK_OCC_DELAY1) {
			itm_debug1(DBG_CTRL, "FREE(d)", i);
			blk_occup[i] = BLK_OCC_FREE;
			occupency_changed = 1;
            notif_blk_occup_chg(i, blk_occup[i]);
		} else if (blk_occup[i] > BLK_OCC_DELAY1) {
			blk_occup[i]--;
		}
	}
}

// ---------------------------------------------------------------




static void evt_entered_c2(int tidx, train_ctrl_t *tvar, uint8_t from_bemf)
{
	if (from_bemf && ignore_bemf_presence) return;
	switch (tvar->_state) {
	case train_running_c1:
		if (from_bemf && ignore_ina_presence) {
			set_timer(tidx, tvar, TLEAVE_C1, TLEAVE_C1_VALUE);
		} else {
			set_timer(tidx, tvar, TLEAVE_C1, TGUARD_C1_VALUE);
		}
		set_block_addr_occupency(tvar->canton2_addr, occupied(tvar->_dir));
		set_state(tidx, tvar, train_running_c1c2);
		break;
	case train_running_c1c2:
		break;
	default:
		itm_debug2(DBG_ERR|DBG_CTRL, "bad st/1",tidx, tvar->_state);
		break;
	}
}

static void evt_leaved_c1(int tidx, train_ctrl_t *tvars)
{
	itm_debug2(DBG_CTRL, "evt_left_c1", tidx, tvars->_state);
	switch (tvars->_state) {
	case train_running_c1c2:
		reset_timer(tidx, tvars, TLEAVE_C1);
		set_block_addr_occupency(tvars->canton1_addr, BLK_OCC_FREE);
		tvars->canton1_addr = tvars->canton2_addr;
		tvars->canton2_addr = 0xFF;
		set_state(tidx, tvars, train_running_c1);
		update_c2_state_limits(tidx, tvars, upd_c1c2);
		tvars->behaviour_flags |= BEHAVE_CHBKLK;
		break;
	default:
		itm_debug2(DBG_ERR|DBG_CTRL, "bad st/2",tidx, tvars->_state);
		break;
	}
}

static void evt_entered_c1(int tidx, train_ctrl_t *tvars, _UNUSED_ uint8_t from_bemf)
{
	itm_debug2(DBG_CTRL, "enter C1", tidx, tvars->_state);
}
static void evt_leaved_c2(int tidx, train_ctrl_t *tvar)
{
	itm_debug2(DBG_CTRL, "leave C2", tidx, tvar->_state);
}

static void evt_tleave(int tidx, train_ctrl_t *tvars)
{
	if (ignore_ina_presence) {
		itm_debug2(DBG_ERR|DBG_CTRL, "TLeave", tidx, tvars->_state);
		evt_leaved_c1(tidx, tvars);
	} else if (tvars->_state == train_running_c1c2){
		// this is TGUARD
		itm_debug2(DBG_ERR|DBG_CTRL, "TGuard", tidx, tvars->_state);
		// for now we do the same, but more to do for long trains
		evt_leaved_c1(tidx, tvars);
	} else {
		itm_debug2(DBG_ERR|DBG_CTRL, "TGurd/bdst", tidx, tvars->_state);
	}
}

static void evt_cmd_set_setdirspeed(int tidx, train_ctrl_t *tvars, int8_t dir, uint16_t tspd, _UNUSED_ uint8_t generated)
{
	itm_debug3(DBG_CTRL, "dirspd", tidx, dir, tspd);

	if (tvars->_state == train_off) {
		itm_debug1(DBG_ERR|DBG_CTRL, "dir ch off", tidx);
		return;
	}
	int8_t odir = tvars->_dir;
	uint16_t otspd = tvars->_target_speed;

	if (!dir && tspd) {
		itm_debug2(DBG_ERR|DBG_CTRL, "dir0spd", tidx, tspd);
		tspd = 0;
	}
	if (!tspd && dir) {
		itm_debug2(DBG_ERR|DBG_CTRL, "spd0dir", tidx, dir);
		dir = 0;
	}
	tvars->desired_speed = tspd;

	if ((tspd == otspd) && (dir == odir)) {
		// no change
		itm_debug3(DBG_CTRL, "dirspd/=", tidx, dir, tspd);
		return;
	}
	if ((tvars->_target_speed != 0) && (tvars->_dir != dir)) {
		itm_debug3(DBG_ERR|DBG_CTRL, "dir ch mov", tidx, dir, tvars->_target_speed);
		set_state(tidx, tvars, train_station); // say it did stopped
		// change dir while not stopped... what do we do here ?
	}
	if ((tvars->_state == train_station) && dir && tspd) {
		itm_debug1(DBG_CTRL, "quit stop", tidx);
		odir = 0;
		set_state(tidx, tvars, train_running_c1);
		set_block_addr_occupency(tvars->canton1_addr, (dir>0)? BLK_OCC_RIGHT:BLK_OCC_LEFT);
	}
	if (tvars->_state == train_running_c1c2 && (odir != dir) && dir) {
		// special care here TODO when reversing change while in c1 to c2 transition
		// TODO
		itm_debug1(DBG_ERR|DBG_CTRL, "c1c2 rev!", tidx);
	}

	if (dir != odir) {
		tvars->_dir = dir;
		if (!dir) {
			itm_debug1(DBG_CTRL, "stopping", tidx);
			set_block_addr_occupency(tvars->canton1_addr, BLK_OCC_STOP);
		} else {
			set_block_addr_occupency(tvars->canton1_addr, (dir>0)? BLK_OCC_RIGHT:BLK_OCC_LEFT);
		}
		update_c2_state_limits(tidx, tvars, upd_change_dir);
	}

	ctrl_set_dir(tidx, tvars, dir, 0);

	if (tvars->_mode != train_fullmanual) {
		tspd = MIN(tvars->spd_limit, tspd);
	}
	//ctrl_set_status(tidx, tspd ? train_running : train_station);
	ctrl_set_tspeed(tidx, tvars, tspd);
}


static void evt_pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t c_addr)
{
	itm_debug3(DBG_CTRL, "pose trgd", tidx, c_addr, tvar->_state);
	if (0==tidx) {
		itm_debug2(DBG_CTRL, "----trg0", c_addr, tvar->_state);
	}
	switch (tvar->_state) {
	case train_running_c1:
		if (c_addr == tvar->canton1_addr) {
			update_c2_state_limits(tidx, tvar, upd_pose_trig);
			//hi_pose_triggered(tidx, tvar, _blk_addr_to_blk_num(c_addr));
			// TODO
		} else {
			itm_debug3(DBG_ERR|DBG_POSE|DBG_CTRL, "ptrg bad", tidx, c_addr, tvar->canton1_addr);
		}
		break;
	default:
		itm_debug2(DBG_ERR|DBG_CTRL, "bad st/3",tidx, tvar->_state);
	}
}




static void evt_timer(int tidx, train_ctrl_t *tvar, int tnum)
{
	itm_debug2(DBG_CTRL, "timer evt", tidx, tnum);
	switch (tnum) {
	case TLEAVE_C1:
		evt_tleave(tidx, tvar);
		break;
	case TBEHAVE:
		tvar->behaviour_flags |= BEHAVE_TBEHAVE;
		break;
	default:
		itm_debug2(DBG_ERR|DBG_CTRL, "?TIM", tidx, tnum);
		fatal();
		break;
	}
}


// ---------------------------------------------------------------




static void update_c2_state_limits(int tidx, train_ctrl_t *tvars, update_reason_t updreason)
{
	itm_debug3(DBG_CTRLHI, "UPDC2", tidx, tvars->canton1_addr, updreason);
	uint8_t c2addr = 0xFF;
	uint16_t olim = tvars->spd_limit;
	uint32_t posetval = 0;

	if (updreason == upd_pose_trig) tvars->behaviour_flags |= BEHAVE_PTRIG;

	if ((tidx==1) && (tvars->canton1_addr==0x02) && (tvars->canton2_addr==0x01)) {
		itm_debug1(DBG_CTRL, "hop", tidx);
	}
	switch (tvars->_state) {
	case train_off:
	case train_station:
		tvars->_dir = 0;
		tvars->_target_speed = 0;
		if (tvars->canton2_addr != 0xFF) set_block_addr_occupency(tvars->canton2_addr, BLK_OCC_FREE);
		tvars->canton2_addr = 0xFF;
		goto sendlow;
	default:
		break;
	}
	if (tvars->canton1_addr == 0xFF) {
		itm_debug1(DBG_ERR|DBG_CTRL, "*** NO C1", tidx);
		return;
	}
	if (!tvars->_dir) {
		set_state(tidx, tvars, train_station);
		tvars->behaviour_flags |= BEHAVE_STOPPED;
		tvars->_target_speed = 0;
		if (tvars->canton2_addr != 0xFF) set_block_addr_occupency(tvars->canton2_addr, BLK_OCC_FREE);
		tvars->canton2_addr = 0xFF;
		goto sendlow;
	}
	int c1num = _blk_addr_to_blk_num(tvars->canton1_addr);
	int c2num = _next_block_num(c1num, (tvars->_dir<0));

	itm_debug3(DBG_CTRL, "prev c1c2", tidx, tvars->canton1_addr, tvars->canton2_addr);
	itm_debug3(DBG_CTRL, "c1c2num", tidx, c1num, c2num);

	if (c2num < 0) {
		// end of track
		if (updreason == upd_c1c2) {
			itm_debug1(DBG_CTRL, "eot", tidx);
			tvars->spd_limit = EOT_SPD_LIMIT;//			set_speed_limit(tn, 20);
			const train_config_t *tconf = get_train_cnf(tidx);
			posetval = pose_middle(_blk_addr_to_blk_num(tvars->canton1_addr), tconf, tvars->_dir);
			tvars->behaviour_flags |= BEHAVE_EOT1;
		} else if (updreason == upd_pose_trig) {
			itm_debug1(DBG_CTRL, "eot2", tidx);
			set_state(tidx, tvars, train_end_of_track);
			tvars->spd_limit = 0;
			tvars->behaviour_flags |= BEHAVE_EOT2;
		}
	} else {
		switch (blk_occup[c2num]) {
			case BLK_OCC_FREE:
				itm_debug2(DBG_CTRL, "free", tidx, c2num);
				tvars->spd_limit = 100; //set_speed_limit(tidx, 100);
				switch (tvars->_state) {
				case train_running_c1:
					break;
				case train_blk_wait:
					set_state(tidx, tvars, train_running_c1);
					tvars->behaviour_flags |= BEHAVE_RESTARTBLK;
					break;
				default:
					itm_debug2(DBG_ERR|DBG_CTRL, "bad st/4", tidx, tvars->_state);
					break;
				}
				break;
			default:
			case BLK_OCC_RIGHT:
			case BLK_OCC_LEFT:
			case BLK_OCC_STOP:
				itm_debug3(DBG_CTRL, "occ", tidx, c2num, blk_occup[c2num]);
				set_state(tidx, tvars, train_blk_wait);
				tvars->behaviour_flags |= BEHAVE_BLKW;
				c2num = -1;
				tvars->spd_limit = 0;
				break;
			case BLK_OCC_C2: {
			    uint8_t c2addr = (c2num>=0) ? _blk_num_to_blk_addr(c2num) : 0xFF;
			    if (c2addr == tvars->canton2_addr) {
			    	// normal case, same C2
			    	break;
			    } else if (tvars->canton2_addr != 0xFF) {
			    	// change C2. Can this occur ? if turnout is changed
			    	// but turnout should not be changed if C2 already set
			    	itm_debug3(DBG_ERR|DBG_CTRL, "C2 change", tidx, tvars->canton2_addr, c2addr);
			    	set_block_addr_occupency(tvars->canton2_addr, BLK_OCC_FREE);
			    	tvars->canton2_addr = 0xFF;
			    }
			    // occupied
				itm_debug2(DBG_CTRL, "OCC C2", tidx, c2num);
				set_state(tidx, tvars, train_blk_wait);
				c2num = - 1;
				tvars->spd_limit = 0;
				break;
			}
		}
	}
	if (c2num>=0) {
		// sanity check, can be removed (TODO)
		if ((get_block_num_occupency(c2num) != BLK_OCC_FREE)
				&& (get_block_num_occupency(c2num) != BLK_OCC_C2))fatal();
		set_block_num_occupency(c2num, BLK_OCC_C2);
	}
    c2addr = (c2num>=0) ? _blk_num_to_blk_addr(c2num) : 0xFF;

sendlow:
	if ((c2addr != tvars->canton2_addr) || (updreason == upd_c1c2) || (updreason == upd_change_dir) ||(updreason==upd_init)) {
		itm_debug3(DBG_CTRL, "C1C2", tidx, tvars->canton1_addr, tvars->canton2_addr);
		tvars->canton2_addr = c2addr;

		int dir = tvars->_dir;
		const train_config_t *tconf = get_train_cnf(tidx);
		if (tconf->reversed) dir = -dir;

		msg_64_t m;
		m.from = MA_CONTROL_T(tidx);
		m.to =  MA_TRAIN_SC(tidx);
		m.cmd = CMD_SET_C1_C2;
		m.vbytes[0] = tvars->canton1_addr;
		m.vbytes[1] = dir;
		m.vbytes[2] = tvars->canton2_addr;
		m.vbytes[3] = dir; // 0;
		mqf_write_from_ctrl(&m);
	}
	if ((tvars->_mode != train_fullmanual) && (olim != tvars->spd_limit)) {
		itm_debug2(DBG_CTRL, "lim upd", tidx, tvars->spd_limit);
		uint16_t tspd = MIN(tvars->spd_limit, tvars->desired_speed);
		switch (updreason) {
		case upd_change_dir: // do nothing, ctrl_set_tspeed will be updated
			break;
		case upd_init:
			break;
		default:
			ctrl_set_tspeed(tidx, tvars, tspd);
			break;
		}
	}
	if (posetval) {
		//itm_debug2(DBG_CTRL, "set pose", tidx, posetval);
		// POSE trigger must be sent *after* CMD_SET_C1_C2
		set_pose_trig(tidx, posetval);
	}

}



// ---------------------------------------------------------------

static void ctrl_set_tspeed(int trnum, train_ctrl_t *tvars, uint16_t tspd)
{
	if (tvars->_target_speed == tspd) return;
	tvars->_target_speed = tspd;

	// notif UI
	itm_debug2(DBG_UI|DBG_CTRL, "ctrl_set_tspeed", trnum, tspd);
	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
	m.to = MA_UI(UISUB_TFT);
	m.cmd = CMD_TRTSPD_NOTIF;
	m.v1u = tspd;
	m.v2 = trctl[trnum]._dir;
	mqf_write_from_ctrl(&m);

	m.to = MA_TRAIN_SC(trnum);
	m.cmd = CMD_SET_TARGET_SPEED;
	// direction already given by SET_C1_C2
	//m.v1 = trctl[trnum]._dir*trctl[trnum]._target_speed;
	m.v1u = tvars->_target_speed;
	mqf_write_from_ctrl(&m);


}

static void ctrl_set_dir(int trnum,  train_ctrl_t *tvars, int  dir, int force)
{
	if (!force && (tvars->_dir == dir)) return;

	itm_debug2(DBG_CTRL, "setdir", trnum, dir);


	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
	tvars->_dir = dir;

	// notif UI
	m.to = MA_UI(UISUB_TFT);
	m.cmd = CMD_TRDIR_NOTIF;
	m.v1 = dir;
	mqf_write_from_ctrl(&m);
}

// ---------------------------------------------------------------


static void set_pose_trig(int numtrain, int32_t pose)
{
	itm_debug2(DBG_CTRL, "set posetr", numtrain, pose);
	msg_64_t m;
	m.from = MA_CONTROL_T(numtrain);
	m.from = MA_CONTROL_T(numtrain);
	m.to =  MA_TRAIN_SC(numtrain);
	m.cmd = CMD_POSE_SET_TRIG;
	const train_config_t *tconf = get_train_cnf(numtrain);
	if (tconf->reversed)  m.v32 = -pose;
	else m.v32 = pose;
	mqf_write_from_ctrl(&m);
}


static int32_t pose_middle(int blknum, const train_config_t *tconf, int dir)
{
	int cm = get_blk_len(blknum);
	uint32_t p = cm * tconf->pose_per_cm;
	uint32_t pm = p/2;
	if (dir<0) pm = -pm;
	return pm;
}

// ---------------------------------------------------------------


static void check_blk_tick(_UNUSED_ uint32_t tick)
{
	if ((0)) return;
	if (occupency_changed) {
		occupency_changed = 0;
		for (int tidx=0; tidx<NUM_TRAINS; tidx++) {
			train_ctrl_t *tvars = &trctl[tidx];
			const train_config_t *tconf = get_train_cnf(tidx);
			if (!tconf->enabled) continue;
			if (tvars->_state == train_off) continue;
			if ((tvars->_state == train_blk_wait) || (tvars->spd_limit <100)) {
				itm_debug1(DBG_CTRL, "chk", tidx);
				update_c2_state_limits(tidx, tvars, upd_check);
			}
		}
	}
}


// ---------------------------------------------------------------

static void set_turnout(int tn, int v)
{
	itm_debug2(DBG_CTRL, "TURN", tn, v);
	if (tn<0) fatal();
	if (tn>=NUM_TURNOUTS) fatal();
	if (tn>=NUM_LOCAL_TURNOUTS) fatal(); // TODO
	msg_64_t m;
	m.from = MA_CONTROL();
	m.to = MA_TURNOUT(0, tn); // TODO board num
	m.cmd = v ? CMD_TURNOUT_B : CMD_TURNOUT_A;

	mqf_write_from_ctrl(&m);
	topolgy_set_turnout(tn, v);

	occupency_changed = 1;
}

// ---------------------------------------------------------------

static void check_behaviour(_UNUSED_ uint32_t tick)
{
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
				set_timer(tidx, tvars, TBEHAVE, 500);
			}
			continue;
		}
		if (!flags) continue;

		tvars->behaviour_flags = 0;
		// ---- behave
		itm_debug3(DBG_CTRL, "hi f=", tidx, flags, tvars->canton1_addr);
		if (tidx == 1) {
			if ((flags & BEHAVE_RESTARTBLK) && (tvars->canton1_addr == MA_CANTON(0,2))) {
				set_turnout(0, 1);
				continue;
			}
			if ((flags & BEHAVE_EOT2) && (tvars->_dir > 0)) {
				set_timer(tidx, tvars, TBEHAVE, 3*60*1000);
				evt_cmd_set_setdirspeed(tidx, tvars, 0, 0, 1);
				return;
			}
			if ((flags & BEHAVE_EOT2) && (tvars->_dir < 0)) {
				set_timer(tidx, tvars, TBEHAVE, 300);
				evt_cmd_set_setdirspeed(tidx, tvars, 0, 0, 1);
				set_turnout(0, 0);
				continue;
			}
			if (flags & BEHAVE_EOT2) {
				itm_debug3(DBG_CTRL, "unex EOT2", tidx, tvars->_dir, tvars->canton1_addr);
				set_timer(tidx, tvars, TBEHAVE, 300);
				continue;
			}
			if (flags & BEHAVE_TBEHAVE) {
				if (tvars->canton1_addr == MA_CANTON(0,1)) {
					evt_cmd_set_setdirspeed(tidx, tvars, -1, 15, 1);
				} else if (tvars->canton1_addr == MA_CANTON(0,2)) {
					evt_cmd_set_setdirspeed(tidx, tvars, 1, 15, 1);
				} else {
					itm_debug3(DBG_CTRL, "unex TB", tidx, tvars->_dir, tvars->canton1_addr);
				}
				continue;
			}
		}
		if (tidx == 0) {
			if (flags & BEHAVE_TBEHAVE) {
				itm_debug2(DBG_CTRL, "TBehave", tidx, tvars->canton1_addr);
				if (tvars->canton1_addr == MA_CANTON(0,0)) {
					evt_cmd_set_setdirspeed(tidx, tvars, 1, 45, 1);
				} else if (tvars->canton1_addr == MA_CANTON(0,1)) {
					evt_cmd_set_setdirspeed(tidx, tvars, -1, 45, 1);
				} else {
					// should not happen
					ctrl_set_mode(tidx, train_manual);
				}
				continue;
			}
			if (flags & BEHAVE_EOT2)  {
				set_timer(tidx, tvars, TBEHAVE, 700);
				evt_cmd_set_setdirspeed(tidx, tvars, 0, 0, 1);
				continue;
			}
		}
	}
}






