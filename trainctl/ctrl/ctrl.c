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

#define DUMMY_BEHAVIOUR 0	// simple/hardcoded behaviour for test

// for test/debug
static uint8_t ignore_bemf_presence = 0;
static uint8_t ignore_ina_presence = 1;

//per train stucture

typedef struct {
	train_mode_t   _mode;
	train_status_t _status;

	uint16_t _target_speed;
	int8_t   _dir;

	uint8_t  canton1_addr;
	uint8_t  canton2_addr;

	uint8_t c1toc2transition:1;
	uint32_t c1toc2transition_inv;

	uint16_t spd_limit;
	uint16_t desired_speed;
} train_ctrl_t;


static train_ctrl_t trctl[NUM_TRAINS] = {0};

// "ERR "  -101

static uint8_t test_mode = 0;
static uint8_t testerAddr;


static void ctrl_reset(void);
static void presence_changed(uint32_t tick, uint8_t from_addr, uint8_t segnum, uint16_t v, int16_t ival);
static void train_switched_to_c2(int tn, const train_config_t *tconf, train_ctrl_t *tvar, uint8_t fromBemf);
static void pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t blkaddr);

#if DUMMY_BEHAVIOUR
#else
static void hi_entered_block_num(int tn, int blknum, int dir, int prevblk, int nexblk);
static void hi_pose_triggered(int tidx, train_ctrl_t *tvar, int blknum);
static uint16_t check_speed_limit(int tn, uint16_t tspd, train_ctrl_t *tvar);
static void set_speed_limit(int tn, uint16_t tspd);
#endif


static void ctrl_set_mode(int trnum, train_mode_t mode)
{
	if (trctl[trnum]._mode == mode) return;
	trctl[trnum]._mode = mode;
	// notif UI
	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
	m.to = MA_UI(1); // fix me
	m.cmd = CMD_TRMODE_NOTIF;
	m.v1u = mode;
	mqf_write_from_ctrl(&m);
}

static void ctrl_set_status(int trnum, train_status_t status)
{
	if (trctl[trnum]._status == status) return;
	trctl[trnum]._status = status;
	// notif UI
	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
	m.to = MA_UI(1); // fix me
	m.cmd = CMD_TRSTATUS_NOTIF;
	m.v1u = status;
	mqf_write_from_ctrl(&m);
}

static void update_c2(int trnum)
{
	train_ctrl_t *tvar = &trctl[trnum];
	if (tvar->canton1_addr == 0xFF) {
		itm_debug1(DBG_ERR|DBG_CTRL, "*** NO C1", trnum);
		return;
	}
	int dir = trctl[trnum]._dir;
	if (!dir) {
		tvar->canton2_addr = 0;
	} else {
		tvar->canton2_addr = next_block_addr(trctl[trnum].canton1_addr, (tvar->_dir<0));
	}
	itm_debug3(DBG_CTRL, "updt_c2", trnum, tvar->canton1_addr, tvar->canton2_addr);
	const train_config_t *tconf = get_train_cnf(trnum);
	if (tconf->reversed) dir = -dir;

	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
	m.to =  MA_TRAIN_SC(0);
	m.cmd = CMD_SET_C1_C2;
	m.vbytes[0] = trctl[trnum].canton1_addr;
	m.vbytes[1] = dir;
	m.vbytes[2] = trctl[trnum].canton2_addr;
	m.vbytes[3] = dir; // 0;
	mqf_write_from_ctrl(&m);
}

static int8_t ctrl_set_dir(int trnum, int dir)
{
	if (trctl[trnum]._dir == dir) return 0;
	//if (trctl[trnum]._target_speed) return 0;

	itm_debug2(DBG_CTRL, "setdir", trnum, dir);


	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
	trctl[trnum]._dir = dir;

	update_c2(trnum);


	// notif UI
	m.to = MA_UI(1); // fix me
	m.cmd = CMD_TRDIR_NOTIF;
	m.v1 = dir;
	mqf_write_from_ctrl(&m);

	// TODO : change C2
	return 1;
}

static int8_t ctrl_set_tspeed(int trnum, uint16_t tspd)
{
	if (trctl[trnum]._target_speed == tspd) return 0;
	trctl[trnum]._target_speed = tspd;

	// notif UI
	itm_debug2(DBG_UI|DBG_CTRL, "tx tspd notif", trnum, tspd);
	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
	m.to = MA_UI(1); // TODO : fix me
	m.cmd = CMD_TRTSPD_NOTIF;
	m.v1u = tspd;
	m.v2 = trctl[trnum]._dir;
	mqf_write_from_ctrl(&m);

	m.to = MA_TRAIN_SC(trnum);
	m.cmd = CMD_SET_TARGET_SPEED;
	// direction already given by SET_C1_C2
	//m.v1 = trctl[trnum]._dir*trctl[trnum]._target_speed;
	m.v1u = trctl[trnum]._target_speed;
	mqf_write_from_ctrl(&m);

	return 1;
}

static uint16_t check_speed_limit(int tn, uint16_t tspd, train_ctrl_t *tvar)
{
	if (tspd > tvar->spd_limit) {
		itm_debug3(DBG_CTRLHI, "spd lim", tn, tvar->spd_limit, tspd);
		return tvar->spd_limit;
	}
	return tspd;
}
static void set_speed_limit(int tn, uint16_t lim)
{
	itm_debug2(DBG_CTRLHI, "limit", tn, lim);
	train_ctrl_t *tvar = &trctl[tn];
	uint16_t oldlim = tvar->spd_limit;
	tvar->spd_limit = lim;

	if (lim < tvar->_target_speed) {
		ctrl_set_tspeed(tn, lim);
	} else if ((lim > oldlim) && (tvar->_target_speed < tvar->desired_speed)) {
		uint16_t ns = MIN(lim, tvar->desired_speed);
		ctrl_set_tspeed(tn, ns);
	}
}

static void ctrl_init(void)
{
	memset(trctl, 0, sizeof(train_ctrl_t)*NUM_TRAINS);
	ctrl_set_mode(0, train_manual);
	ctrl_set_tspeed(0, 0);
}


// ----------------------------------------------------------------------------

void ctrl_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	static int first=1;
	if (first) {
		first = 0;
		ctrl_init();
		ctrl_reset();

		msg_64_t m;
		ui_msg(1, IHMMSG_TRAINCTL_INIT, &m, MA_CONTROL());
		mqf_write_from_ctrl(&m);
		if ((1)) {
			trctl[0].canton1_addr = MA_CANTON(0, 1); // initial blk
			trctl[0].canton2_addr = 0xFF;
			ctrl_set_dir(0, 0);
			hi_entered_block_num(0, 1, 0, -1, -1);
		}

        if ((0)) { // test
            msg_64_t m;
        	m.from = MA_CONTROL();
        	m.to = MA_BROADCAST;
        	m.cmd = CMD_TEST_MODE;
        	m.v1u = 1;
            mqf_write_from_ctrl(&m);
        }

    }

	/* process messages */
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_ctrl(&m);
		if (rc) break;
        switch (m.cmd) {
            case CMD_RESET:
                test_mode = 0; // FALLTHRU
            case CMD_EMERGENCY_STOP:
                ctrl_reset();
                continue;
                break;
            case CMD_TEST_MODE:
                test_mode = m.v1u;
                testerAddr = m.from;
                continue;
                break;
        }
        if (test_mode & (m.from != testerAddr)) {
            continue;
        }
		if (IS_CONTROL_T(m.to)) {
			if (test_mode) continue;
			int tidx = m.to & 0x7;
			train_ctrl_t *tvar = &trctl[tidx];

			switch (m.cmd) {
			case CMD_PRESENCE_CHANGE:{
				if (ignore_ina_presence) break;
				presence_changed(tick, m.from, m.sub, m.v1u, m.v2);
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
				if (ignore_bemf_presence) break;

				const train_config_t *tconf = get_train_cnf(tidx);
				train_switched_to_c2(tidx, tconf, tvar, 1);
				break;
			}
			case CMD_MDRIVE_SPEED_DIR:
				if (ctrl_set_dir(tidx, m.v2)) {
					hi_entered_block_num(tidx, -1, m.v2, -1, -1);
				}
				//FALLTHRU
			case CMD_MDRIVE_SPEED:
				itm_debug3(DBG_CTRL, "M/spd", tidx, m.v1u, m.v2);
				int16_t tspd = m.v1u;
				// check speed is allowed
				// transmit to speedctl
				_UNUSED_ int spdchanged = 0;
				tvar->desired_speed = tspd;
				if ((tvar->_mode == train_fullmanual) || (tvar->_mode == train_manual)) {
					if (tvar->_mode != train_fullmanual) {
						tspd = check_speed_limit(tidx, tspd, tvar);
					}
					ctrl_set_status(tidx, tspd ? train_running : train_station);
					spdchanged = ctrl_set_tspeed(tidx, tspd);
				}
				break;
			case CMD_MDRIVE_DIR:
				itm_debug2(DBG_CTRL, "M/dir", tidx, m.v1);
				// TODO
				// check speed is 0, dir changed not allowed otherwised
				// check dir is allowed
				// store dir
				if (0==tvar->_target_speed) {
					if ((tvar->_mode == train_fullmanual) || (tvar->_mode == train_manual)) {
						if (tvar->_mode != train_fullmanual) {
							// check validity
						}
						ctrl_set_dir(tidx, m.v1);
					}
				}
				break;
			case CMD_POSE_TRIGGERED:
				itm_debug2(DBG_POSE, "Trig", m.v1u, m.v2u);
				pose_triggered(tidx, tvar, m.v1u);
				break;
			default:
				break;

			}
		} else {
			itm_debug1(DBG_MSG|DBG_CTRL, "bad msg", m.to);
		}
	}
	// xxx
}

// ---------------------------------------------------------------
//static int spd0 = 30;

static void ctrl_reset(void)
{
	//TODO
}

static void presence_changed(uint32_t tick, uint8_t from_addr, uint8_t lsegnum, uint16_t p, int16_t ival)
{
	static int t[12]={0};
	int segnum = _sub_addr_to_sub_num(from_addr, lsegnum);
	itm_debug3(DBG_PRES|DBG_CTRL, "PRC",  p, lsegnum, ival);
	if ((segnum<0) || (segnum>11)) return;
	t[segnum]=p;
	itm_debug3(DBG_PRES|DBG_CTRL, "PRS", t[0], t[1], t[2]);

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
				tvar->c1toc2transition = 0;
			} else {
				if (tvar->c1toc2transition) {
					itm_debug2(DBG_PRES, "leave c1", tn, segnum);
					train_switched_to_c2(tn, tconf, tvar, 0);
				} else {
					tvar->c1toc2transition_inv = tick;
					itm_debug2(DBG_PRES, "?lv c1 no tr", tn, segnum);
				}

			}
			f = 1;
		} else if (tvar->canton2_addr == canton) {
			if (p) {
				if (tick - tvar->c1toc2transition_inv < 20) {
					itm_debug2(DBG_PRES, "enter c2b", tn, segnum);
					train_switched_to_c2(tn, tconf, tvar, 0);
				} else {
					itm_debug2(DBG_PRES, "enter c2", tn, segnum);
					tvar->c1toc2transition = 1;
				}
			} else {
				itm_debug2(DBG_PRES, "?leave c2", tn, segnum);
			}
			f = 1;
		}
	}
	if (!f) {
		// presence on unexpected canton
		itm_debug2(DBG_ERR|DBG_PRES, "?unexp", segnum, canton);
	}
}

static void set_pose_trig(int numtrain, int32_t pose);

static void train_switched_to_c2(int tn, const train_config_t *tconf, train_ctrl_t *tvar, uint8_t fromBemf)
{
	uint8_t c1 = tvar->canton1_addr;
	uint8_t c2 = tvar->canton2_addr;

	tvar->c1toc2transition_inv = 0;
	tvar->c1toc2transition = 0;

	itm_debug2(DBG_CTRL, "switch c2", tn, tvar->canton2_addr);
	if (c2 == 0xFF) {
		itm_debug2(DBG_ERR|DBG_CTRL, "swt no c2", tn, c1);
		return;
	}
	tvar->canton1_addr = tvar->canton2_addr;
	update_c2(tn);

#if DUMMY_BEHAVIOUR
	if ((1)) {
		if ((c1 == MA_CANTON(0,1)) &&
				(c2 == MA_CANTON(0,0))) {
			// 1->0
			itm_debug1(DBG_CTRL|DBG_POSE, "HI 1-0", 0);
			set_pose_trig(tn, -17000);
		} else if  ((c2 == MA_CANTON(0,1)) &&
				(c1 == MA_CANTON(0,0))) {
			// 0->1
			itm_debug1(DBG_CTRL|DBG_POSE, "HI °-1", 0);
			set_pose_trig(tn, 9000);
		}
	}
#else
	hi_entered_block_num(tn, _blk_addr_to_blk_num(c2), tvar->_dir, _blk_addr_to_blk_num(c1), _blk_addr_to_blk_num(tvar->canton2_addr));
#endif
}

static void set_pose_trig(int numtrain, int32_t pose)
{
	msg_64_t m;
	m.from = MA_CONTROL_T(numtrain);
	m.from = MA_CONTROL_T(numtrain);
	m.to =  MA_TRAIN_SC(0);
	m.cmd = CMD_POSE_SET_TRIG;
	const train_config_t *tconf = get_train_cnf(numtrain);
	if (tconf->reversed)  m.v32 = -pose;
	else m.v32 = pose;
	mqf_write_from_ctrl(&m);
}

static void pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t blkaddr)
{
#if DUMMY_BEHAVIOUR
	int nd = 0;
	if (tvar->_dir>0) nd = -1;
	else if (tvar->_dir<0) nd = 1;
	ctrl_set_dir(tidx, nd);
#else
	hi_pose_triggered(tidx, tvar, _blk_addr_to_blk_num(blkaddr));
#endif
}

static int32_t pose_middle(int blknum, const train_config_t *tconf, int dir)
{
	int cm = get_blk_len(blknum);
	uint32_t p = cm * tconf->pose_per_cm;
	uint32_t pm = p/2;
	if (dir<0) pm = -pm;
	return pm;
}

// ------------------------------------------------------------------------------

#define BLK_OCC_FREE	0x00
#define BLK_OCC_STOP	0x01
#define BLK_OCC_LEFT	0x10
#define BLK_OCC_RIGHT	0x11

static uint8_t blk_occup[32] = {0}; // TODO 32
static uint8_t occupency_changed = 0;

static int hi_update_speed_limit(int tn,  train_ctrl_t *tvar, int blknum);

static void hi_entered_block_num(int tn, int blknum, int dir, int prevblk, int _nexblk)
{
	itm_debug3(DBG_CTRLHI, "ENTER", tn, blknum, prevblk);
	// blknum = -1 for direction change
	if ((blknum>=0) && blk_occup[blknum]) {
		itm_debug3(DBG_ERR|DBG_CTRLHI, "OCCUP", tn, blknum, prevblk);
		set_speed_limit(tn, 0);
		return;
	}
	if (prevblk>=0) {
		blk_occup[prevblk] = BLK_OCC_FREE;
		occupency_changed = 1;
	}
	if (blknum < 0) {
		blknum = _blk_addr_to_blk_num(trctl[tn].canton1_addr);
	}
	if (dir<0) {
		blk_occup[blknum] = BLK_OCC_LEFT;
	} else if (dir>0) {
		blk_occup[blknum] = BLK_OCC_RIGHT;
	} else {
		blk_occup[blknum] = BLK_OCC_STOP;
	}
	occupency_changed = 1;

	// update speed limit
	train_ctrl_t *tvar = &trctl[tn];
	const train_config_t *tconf = get_train_cnf(tn);
	int rc = hi_update_speed_limit(tn, tvar, blknum);
	itm_debug2(DBG_CTRLHI, "lim rc", tn, rc);
	if (rc==1) {
		set_pose_trig(tn, pose_middle(blknum, tconf, tvar->_dir));
	}
}

static void hi_pose_triggered(int tidx, train_ctrl_t *tvar, int blknum)
{
	// retest and stop if still limit
	int rc = hi_update_speed_limit(tidx, tvar, blknum);
	if (1 == rc) {
		set_speed_limit(tidx, 0);
	}
}

static int hi_update_speed_limit(int tn,  train_ctrl_t *tvar, int blknum)
{
	switch (tvar->_mode) {
	case train_notrrunning:
		itm_debug1(DBG_CTRLHI, "not rung", tn);
		return 0;
	case train_fullmanual:
		itm_debug1(DBG_CTRLHI, "fmanual", tn);
		return 0;
	default:
		break;
	}
	if (!tvar->_dir) {
		itm_debug1(DBG_CTRLHI, "stopped", tn);
		return 0;
	}

	int nextblk = _next_block_num(blknum, (tvar->_dir<0));
	if (nextblk<0) {
		// end of track
		itm_debug1(DBG_CTRLHI, "eot", tn);
		set_speed_limit(tn, 20);
		return 1;
	}
	switch (blk_occup[nextblk]) {
	case BLK_OCC_FREE:
		itm_debug1(DBG_CTRLHI, "free", tn);
		set_speed_limit(tn, 100);
		return 0;
	case BLK_OCC_LEFT:
		if (tvar->_dir<0) {
			itm_debug1(DBG_CTRLHI, "occ lft", tn);
			set_speed_limit(tn, 0);
			return 2;
		} else {
			// same dir
			itm_debug1(DBG_CTRLHI, "occl same", tn);
			set_speed_limit(tn, 20);
			return 1;
		}
		break;
	case BLK_OCC_RIGHT:
		if (tvar->_dir>0) {
			itm_debug1(DBG_CTRLHI, "occ right", tn);
			set_speed_limit(tn, 0);
			return 2;
		} else {
			// same dir
			itm_debug1(DBG_CTRLHI, "occr same", tn);
			set_speed_limit(tn, 20);
			return 1;
		}
		break;
	}
	// not reached
	set_speed_limit(tn, 100);
	return 0;
}



