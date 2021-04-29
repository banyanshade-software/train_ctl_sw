/*
 * ctrl.c
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */



#include "misc.h"
#include "../msg/trainmsg.h"
#include "ctrl.h"

#include "./topology/topology.h"

static void ctrl_reset(void);
static void presence_changed(int segboard, int segnum, int v);

// "ERR "  -101

void ctrl_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	static int first=1;
	if (first) {
		first = 0;
		ctrl_reset();
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
            m.v1 = 90;
            itm_debug1(DBG_SPDCTL|DBG_CTRL, "init/spd", m.v1);
            mqf_write_from_ctrl(&m); //
        }

    }
	if ((1)) { // test
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
			m.to = MA_CANTON(0, 0);
			m.cmd = CMD_SETVPWM;
			m.v1u = 0;
			m.v2 = md*30;
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
		if (IS_CONTROL_T(m.to)) {
			int tidx = m.to & 0x7;
			switch (m.cmd) {
			case CMD_PRESENCE_CHANGE: {
				int segboard = MA_2_BOARD(m.from);
				int segnum = m.sub;
				int v = m.v1;
				// 0.0
				presence_changed(segboard, segnum, v);
				break;
			}
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

		} else if (IS_BROADCAST(m.to)) {
			switch (m.cmd) {
			case CMD_RESET: // FALLTHRU
			case CMD_EMERGENCY_STOP:
				ctrl_reset();
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

}

static void presence_changed(int segboard, int segnum, int v)
{
	static int t[3]={0};
	itm_debug3(DBG_PRES|DBG_CTRL, "PRC", segboard, segnum, v);
	if ((segnum<0) || (segnum>2)) return;
	t[segnum]=v;
	itm_debug3(DBG_PRES|DBG_CTRL, "PRS", t[0], t[1], t[2]);
}
