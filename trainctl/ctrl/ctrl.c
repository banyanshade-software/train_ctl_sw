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

//per train stucture

typedef struct {
	train_mode_t   _mode;
	train_status_t _status;

	uint16_t _target_speed;
	int8_t   _dir;

	uint8_t  canton1_addr;
	uint8_t  canton2_addr;

	uint8_t c1toc2transition:1;

} train_ctrl_t;


static train_ctrl_t trctl[NUM_TRAINS];

// "ERR "  -101

static uint8_t test_mode = 0;
static uint8_t testerAddr;


static void ctrl_reset(void);
static void presence_changed(uint8_t from_addr, uint8_t segnum, uint16_t v, int16_t ival);
static void train_switched_to_c2(int tn, const train_config_t *tconf, train_ctrl_t *tvar, uint8_t fromBemf);
static void pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t blkaddr);


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
		itm_debug1(DBG_ERR|DBG_CTRL, "no c1", trnum);
		return;
	}
	int dir = trctl[trnum]._dir;
	if (!dir) {
		tvar->canton2_addr = 0;
	} else {
		tvar->canton2_addr = next_block_addr(trctl[trnum].canton1_addr, (tvar->_dir<0));
	}
	itm_debug3(DBG_CTRL, "updt_c2", trnum, tvar->canton1_addr, tvar->canton2_addr);
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
	itm_debug2(DBG_CTRL, "setdir", trnum, dir);
	if (trctl[trnum]._dir == dir) return 0;
	//if (trctl[trnum]._target_speed) return 0;

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


static void ctrl_init(void)
{
	memset(trctl, 0, sizeof(train_ctrl_t)*NUM_TRAINS);
	ctrl_set_mode(0, train_fullmanual);
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
		}
        if ((0)) { // TODO remove
            msg_64_t m;
            m.to = MA_TRAIN_SC(0);
            m.cmd = CMD_SET_C1_C2;
            m.vbytes[0] = MA_CANTON(0, 0);
            m.vbytes[1] = 1;
            m.vbytes[2] = MA_CANTON(0, 1); // 0xFF;
            m.vbytes[3] = 1; // 0;
            itm_debug1(DBG_SPDCTL|DBG_CTRL, "init/c1c2", 0);
            mqf_write_from_ctrl(&m);
            m.cmd = CMD_SET_TARGET_SPEED;
            //m.v1 = 90;
            //itm_debug1(DBG_SPDCTL|DBG_CTRL, "init/spd", m.v1);
            //mqf_write_from_ctrl(&m); //
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
	if ((0) && !test_mode) { // test
		static int cd = 0;
		int t = (tick / 3000);
		int tt = t % 3;
		int md = 0;
		switch (tt) {
		case 0: md = 1; break;
		case 1: md = 0; break;
		case 2: md = -1; break;
		}
		//md = 0; // XXX
		if (md != cd) {
			cd = md;

			msg_64_t m;
			uint8_t t = 0;
			switch (md) {
			case 0: t = IHMMSG_TRAINCTL_STOP; break;
			case 1: t = IHMMSG_TRAINCTL_FWD; break;
			case -1: t = IHMMSG_TRAINCTL_REV; break;
			}
			//ui_msg(1, t, &m, MA_CONTROL_T(0));
			mqf_write_from_ctrl(&m);

			m.from = MA_CONTROL_T(0);
			m.to = MA_CANTON(0, 0);
			m.cmd = CMD_SETVPWM;
			m.v1u = 0;
			m.v2 = md*70;
			itm_debug3(DBG_SPDCTL|DBG_CTRL, "init/c0", md, m.v2, t);
			mqf_write_from_ctrl(&m);
			m.to = MA_CANTON(0, 1);
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
			case CMD_PRESENCE_CHANGE: {
				presence_changed(m.from, m.sub, m.v1u, m.v2);
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
				const train_config_t *tconf = get_train_cnf(tidx);
				train_switched_to_c2(tidx, tconf, tvar, 1);
				break;
			}
			case CMD_MDRIVE_SPEED_DIR:
				ctrl_set_dir(tidx, m.v2);
				//FALLTHRU
			case CMD_MDRIVE_SPEED:
				itm_debug3(DBG_CTRL, "M/spd", tidx, m.v1u, m.v2);
				int16_t tspd = m.v1u;
				// check speed is allowed
				// transmit to speedctl
				_UNUSED_ int spdchanged = 0;
				if ((tvar->_mode == train_fullmanual) || (tvar->_mode == train_manual)) {
					if (tvar->_mode != train_fullmanual) {
						// check validity
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

static void presence_changed(uint8_t from_addr, uint8_t lsegnum, uint16_t p, int16_t ival)
{
	static int t[12]={0};
	int segnum = _sub_addr_to_sub_num(from_addr, lsegnum);
	itm_debug3(DBG_PRES|DBG_CTRL, "PRC", lsegnum, p, ival);
	if ((segnum<0) || (segnum>11)) return;
	t[segnum]=p;
	itm_debug3(DBG_PRES|DBG_CTRL, "PRS", t[0], t[1], t[2]);

	uint8_t canton = blk_addr_for_sub_addr(from_addr, segnum);
	if (0xFF == canton) {
		itm_debug2(DBG_ERR|DBG_CTRL, "blk??", from_addr, segnum);
		return;
	}
	int f = 0;

	for (int tn = 0; tn < NUM_TRAINS; tn++) {
		train_ctrl_t *tvar = &trctl[tn];
		const train_config_t *tconf = get_train_cnf(tn);
		// check enabled
		if (!tconf->enabled) continue;
		if (tvar->canton1_addr == canton) {
			if (p) {
				itm_debug2(DBG_PRES, "?enter c1", tn, segnum);
			} else {
				if (tvar->c1toc2transition) {
					itm_debug2(DBG_PRES, "leave c1", tn, segnum);
					train_switched_to_c2(tn, tconf, tvar, 0);
				} else {
					itm_debug2(DBG_PRES, "?lv c1 no tr", tn, segnum);
				}

			}
			f = 1;
		} else if (tvar->canton2_addr == canton) {
			if (p) {
				itm_debug2(DBG_PRES, "enter c2", tn, segnum);
				tvar->c1toc2transition = 1;
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

	itm_debug2(DBG_CTRL, "switch c2", tn, tvar->canton2_addr);

	tvar->canton1_addr = tvar->canton2_addr;
	update_c2(tn);

	if ((0)) {
		if ((c1 == MA_CANTON(0,1)) &&
				(c2 == MA_CANTON(0,0))) {
			// 1->0
			itm_debug1(DBG_CTRL|DBG_POSE, "HI 1-0", 0);
			int nd = 0;
			if (tvar->_dir>0) nd = -1;
			else if (tvar->_dir<0) nd = 1;
			ctrl_set_dir(tn, nd);
		} else if  ((c2 == MA_CANTON(0,1)) &&
				(c1 == MA_CANTON(0,0))) {
			// 1<-0
			itm_debug1(DBG_CTRL|DBG_POSE, "HI °-1", 0);
			int nd = 0;
			if (tvar->_dir>0) nd = -1;
			else if (tvar->_dir<0) nd = 1;
			ctrl_set_dir(tn, nd);
		}
	}
	if ((1)) {
		if ((c1 == MA_CANTON(0,1)) &&
				(c2 == MA_CANTON(0,0))) {
			// 1->0
			itm_debug1(DBG_CTRL|DBG_POSE, "HI 1-0", 0);
			set_pose_trig(tn, -9000);
		} else if  ((c2 == MA_CANTON(0,1)) &&
				(c1 == MA_CANTON(0,0))) {
			// 0->1
			itm_debug1(DBG_CTRL|DBG_POSE, "HI °-1", 0);
			set_pose_trig(tn, 6000);
		}
	}
}

static void set_pose_trig(int numtrain, int32_t pose)
{
	msg_64_t m;
	m.from = MA_CONTROL_T(numtrain);
	m.from = MA_CONTROL_T(numtrain);
	m.to =  MA_TRAIN_SC(0);
	m.cmd = CMD_POSE_SET_TRIG;
	m.v32 = pose;
	mqf_write_from_ctrl(&m);
}

static void pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t blkaddr)
{
	int nd = 0;
	if (tvar->_dir>0) nd = -1;
	else if (tvar->_dir<0) nd = 1;
	ctrl_set_dir(tidx, nd);
}
