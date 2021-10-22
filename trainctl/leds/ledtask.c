/*
 * ledtask.c
 *
 *  Created on: Oct 22, 2021
 *      Author: danielbraun
 */



#include "misc.h"
#include "../msg/trainmsg.h"

#include "ledtask.h"
#include "led.h"

// ----------------------------------------------------------------------------------
// global run mode, each tasklet implement this
static runmode_t run_mode = 0;
static uint8_t testerAddr;



void led_run_tick(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	itm_debug1(DBG_LED ,"------- tk", (int) notif_flags);
	if ((1)) {
		uint32_t tm = tick % 10000;
		static int r=0;
		if ((tm<100) && !r) {
			msg_64_t m;
			m.cmd = CMD_LED_RUN;
			m.from = MA_BROADCAST; // dont care
			m.to = MA_LED_B(0);
			m.v1u = 0;
			m.v2u = LED_PRG_TST;
			mqf_write_from_nowhere(&m);
			r = 1;
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
