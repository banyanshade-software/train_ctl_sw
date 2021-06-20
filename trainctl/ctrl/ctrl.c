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

static void ctrl_reset(void);
static void presence_changed(int segboard, int segnum, int v);

//per train stucture

typedef struct {
	train_mode_t   _mode;
	train_status_t _status;

	uint8_t  canton1_addr;
	uint8_t  canton2_addr;
	int8_t   _dir;
	uint16_t _target_speed;

} train_ctrl_t;


static train_ctrl_t trctl[NUM_TRAINS];

// "ERR "  -101

static uint8_t test_mode = 0;
static uint8_t testerAddr;


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

static int8_t ctrl_set_dir(int trnum, int dir)
{
	if (trctl[trnum]._dir == dir) return 0;
	if (trctl[trnum]._target_speed) return 0;
	trctl[trnum]._dir = dir;
	// notif UI
	msg_64_t m;
	m.from = MA_CONTROL_T(trnum);
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
	m.v1 = trctl[trnum]._dir*trctl[trnum]._target_speed;
	mqf_write_from_ctrl(&m);

	return 1;
}


static void ctrl_init(void)
{
	memset(trctl, 0, sizeof(train_ctrl_t)*NUM_TRAINS);
	ctrl_set_mode(0, train_fullmanual);
	ctrl_set_tspeed(0, 0);
	ctrl_set_dir(0, 1);
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

        if ((1)) { // TODO remove
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
                break;
            case CMD_TEST_MODE:
                test_mode = m.v1u;
                testerAddr = m.from;
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
				int segboard = MA_2_BOARD(m.from);
				int segnum = m.sub;
				int v = m.v1;
				// 0.0
				presence_changed(segboard, segnum, v);
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
			default:
				break;

			}
/*
 * msg_64_t m;
    	m.from = MA_CANTON(localBoardNum, 0);
    	m.to = MA_CONTROL();
    	m.cmd = CMD_PRESENCE_CHANGE;
    	m.sub = i;
    	m.v1u = p;
 */
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

}

static void presence_changed(int segboard, int segnum, int v)
{
	static int t[3]={0};
	itm_debug3(DBG_PRES|DBG_CTRL, "PRC", segboard, segnum, v);
	if ((segnum<0) || (segnum>2)) return;
	t[segnum]=v;
	itm_debug3(DBG_PRES|DBG_CTRL, "PRS", t[0], t[1], t[2]);
}
