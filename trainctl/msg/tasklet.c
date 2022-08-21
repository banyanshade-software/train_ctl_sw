/*
 * tasklet.c
 *
 *  Created on: Jun 15, 2022
 *      Author: danielbraun
 */

#include "../misc.h"
#include "tasklet.h"
#include "../msg/msgrecord.h"

static int handle_common_msg(tasklet_t *tasklet, const tasklet_def_t *td, msg_64_t *m)
{
	switch (m->cmd) {
	default : return 0;
	case CMD_SETRUN_MODE:
		if (tasklet->runmode != m->v1u) {
			tasklet->runmode = m->v1u;
			if (td->enter_runmode) td->enter_runmode(tasklet->runmode);
		}
		return 1;
		break;
	case CMD_EMERGENCY_STOP:
		if (td->emergency_stop) td->emergency_stop();
		return 1;
		break;
		//CMD_RESET
	}
}

void tasklet_run(tasklet_t *tasklet, uint32_t tick)
{
	const tasklet_def_t *td = tasklet->def;
	if (!tasklet->init_done) {
		tasklet->init_done = 1;
		if (td->init) td->init();
	}
	uint32_t dt = tick - tasklet->last_tick;
	if (td->poll_divisor) {
		if (td->poll_divisor(tick, dt)) return;
	}
	tasklet->last_tick = tick;

	if (td->pre_tick_handler) td->pre_tick_handler(tick, dt);

	msg_handler_t mh = NULL;
	if (td->msg_handler_for) {
		mh = td->msg_handler_for(tasklet->runmode);
	}
	if (!mh) mh = td->default_msg_handler;

	tick_handler_t th = NULL;
	if (td->tick_handler_for) {
		th = td->tick_handler_for(tasklet->runmode);
	}
	if (!th) th = td->default_tick_handler;

	for (;;) {
		msg_64_t msg;
		int rc = mqf_read(tasklet->queue, &msg);
		if (rc) break;
		if (td->recordmsg) {
			record_msg_read(&msg);
		}
		if (handle_common_msg(tasklet, td, &msg)) continue;
		if (mh) mh(&msg);
	}

	if (th) th(tick, dt);
}


void tasklet_run_all(tasklet_t *tasklets[])
{
	uint32_t tick = HAL_GetTick();
	for (int i=0; ;i++) {
		tasklet_t *t = tasklets[i];
		if (!t) break;
		tasklet_run(t, tick);
	}
}
