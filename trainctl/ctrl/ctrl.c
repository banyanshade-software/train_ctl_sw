/*
 * ctrl.c
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */



#include "misc.h"
#include "../msg/trainmsg.h"
#include "ctrl.h"

static void ctrl_reset(void);
static void presence_changed(int segboard, int segnum, int v);

void ctrl_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	static int first=1;
	if (first) {
		first = 0;
		ctrl_reset();
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
			itm_debug1("bad msg", m.to);
		}
	}
	// xxx
}

// ---------------------------------------------------------------
static void ctrl_reset(void)
{

}

static void presence_changed(int segboard, int segnum, int v)
{

}
