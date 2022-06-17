/*
 * ledtask.c
 *
 *  Created on: Oct 22, 2021
 *      Author: danielbraun
 */



#include "../misc.h"
#include "../msg/trainmsg.h"

#include "ledtask.h"
#include "led.h"

#ifndef BOARD_HAS_LED
#error BOARD_HAS_LED not defined, remove this file from build
#endif

// ----------------------------------------------------------------------------------

static void led_enter_runmode(_UNUSED_ runmode_t m)
{
	led_reset_all();
}
static void handle_led_msg(msg_64_t *m);
static void led_tick(_UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt);

static const tasklet_def_t led_tdef = {
		.init 				= led_reset_all,
		.poll_divisor		= NULL,
		.emergency_stop 	= led_reset_all,
		.enter_runmode		= led_enter_runmode,
		.pre_tick_handler	= NULL,
		.default_msg_handler = handle_led_msg,
		.default_tick_handler = led_tick,
		.msg_handler_for	= NULL,
		.tick_handler_for 	= NULL

};
tasklet_t led_tasklet = { .def = &led_tdef, .init_done = 0, .queue=&to_led};



// ----------------------------------------------------------------------------------
// global run mode, each tasklet implement this
//static runmode_t run_mode = 0;
//static uint8_t testerAddr;

static void handle_led_msg(msg_64_t *m)
{
	if (led_tasklet.runmode == runmode_off) {
		return;
	}
	switch (m->cmd) {
	case CMD_LED_RUN:
		led_start_prog(m->v1u, m->v2u);
		break;
	 default:
		 break;
	}
}

static void led_tick(_UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	if (led_tasklet.runmode == runmode_off) {
		return;
	}
	led_run_all();
}

#if 0
void led_tasklet(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	itm_debug1(DBG_LED ,"------- tk", (int) notif_flags);
	if ((0)) {
		uint32_t tm = tick % 10000;
		static int r=0;
		static int prog = 0;
		if ((tm<100) && !r) {
			msg_64_t m = {0};
			m.cmd = CMD_LED_RUN;
			m.from = MA3_BROADCAST; // dont care
			m.to = MA0_LED(0);
			m.v1u = 0;
			m.v2u = prog;
			mqf_write_from_nowhere(&m);
			r = 1;
			prog++;
			if (prog>LED_PRG_TST) {
				prog = 0;
			}
		}
		if (tm>5000) r=0;
	}

	static int first=1;
	if (first) {
		first = 0;
		led_reset_all();

	}
	/* process messages */
	led_run_all();

	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_led(&m);
		if (rc) break;

        switch (m.cmd) {
        case CMD_RESET:
        	// FALLTHRU
        case CMD_EMERGENCY_STOP:
        	led_reset_all();
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

        switch (m.cmd) {
        case CMD_LED_RUN:
            led_start_prog(m.v1u, m.v2u);
        	break;
        default:
        	break;
        }
	}
}
#endif
