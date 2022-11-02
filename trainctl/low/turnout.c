/*
 * turnout.c
 *
 *  Created on: Oct 24, 2020
 *      Author: danielbraun
 */


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

/* turnout.c : control of turnouts (switches)
 *    	turnout_tick() is supposed to be invoked every 20ms and will handle the impulse shape
 *    	TODO: make sure GPIO are reset in HardFault handler (and other), otherwise a turnout
 *    	may receive continuous current
 *
 */


#include <stdint.h>
#include <stddef.h>
#include <memory.h>

#include "turnout.h"
//#include "turnout_config.h"
#include "../config/conf_turnout.h"

#include "../misc.h"
#include "../msg/trainmsg.h"


//#include "../railconfig.h"
#include "../trainctl_iface.h"
#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#else
#include "train_simu.h"
#endif

#include "../statval.h"



#ifndef BOARD_HAS_TURNOUTS
#error BOARD_HAS_TURNOUTS not defined, remove this file from build
#endif


#if (NUM_TURNOUTS == 0)
#error turnouts code included but NUM_TURNOUTS is zero
#endif


// ------------------------------------------------------

static void turnout_init(void);
static void turnout_enter_runmode(runmode_t m);
static void process_turnout_timers(uint32_t tick, uint32_t dt);
static int turnout_poll_divisor(uint32_t tick, uint32_t dt);
static void process_turnout_cmd(msg_64_t *m); //, uint32_t tick, uint32_t dt);


static const tasklet_def_t turnout_tdef = {
		.init 				= turnout_init,
		.poll_divisor		= turnout_poll_divisor,
		.emergency_stop 	= turnout_init,
		.enter_runmode		= turnout_enter_runmode,
		.pre_tick_handler	= process_turnout_timers,
		.default_msg_handler = process_turnout_cmd,
		.default_tick_handler = NULL,
		.msg_handler_for	= NULL,
		.tick_handler_for 	= NULL

};
tasklet_t turnout_tasklet = { .def = &turnout_tdef, .init_done = 0, .queue=&to_turnout};


static void turnout_enter_runmode(_UNUSED_ runmode_t m)
{
	turnout_init();
}

static volatile int emerg_stop = 0;

void TurnoutEmergencyStop(void)
{
	emerg_stop = 1;
	turnout_init();
}

// ------------------------------------------------------


//static void process_turnout_timers(uint32_t tick, uint32_t dt);
//static void process_turnout_cmd(msg_64_t *m, uint32_t tick, uint32_t dt);

static int turnout_poll_divisor(_UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	// now use dt
	if (dt>=45) {
		return 0;
	}
	return 1;
	/*
	static int cnt = 0;
	cnt ++;
	itm_debug2(DBG_TURNOUT, "tdiv", cnt, dt);
	if (cnt%4) return 1; // skip
	return 0;
	*/
}

/*
void turnout_tasklet(_UNUSED_ uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	static int first=1;
	if (first) {
		first = 0;
		turnout_init();
	}
	static int cnt = 0;
	cnt ++;
	if (cnt%4) return; // divide freq
	// TODO we need a fixed freq for turnout

	process_turnout_timers(tick, dt);
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_turnout(&m);
		if (rc) break;
		if (MA0_IS_TURNOUT(m.to)) {
			process_turnout_cmd(&m, tick, dt);
		} else if (IS_BROADCAST(m.to)) {
			switch (m.cmd) {
			default:
				break;
			case CMD_RESET: // FALLTHRU
			case CMD_EMERGENCY_STOP:
				turnout_init();
				break;
			}
		} else {
			itm_debug1(DBG_TURNOUT|DBG_ERR, "turnout un/cmd", m.cmd);
		}
	}
}*/


// ----------------------------------------------------------------------------------------------


/*
 * state variables for each turnout
 */

typedef struct turnout_vars {
	int8_t value;	// -1 = A, 1 = B, 0 = unknown ; currently this is set but not used
	uint8_t st;
} turnout_vars_t;


/*
 * state info for all (local board) turnouts
 * NUM_TURNOUTS is defined by configuration generator (config/conf_turnout.h)
 */

static turnout_vars_t tvars[NUM_TURNOUTS]={0};

/* states
 * state machine handles sending only a short burst to turnout
 * (turnout would (probably) not handle a long continuous 12V)
 */

#define ST_IDLE		0
#define ST_SETA 	1
#define ST_RESETA 	2
#define ST_SETB 	3
#define ST_RESETB	4

// #define ST_TEST		10

#define DBG_MSG_TURNOUT 0


#define USE_TURNOUT(_idx) \
		const conf_turnout_t *aconf = conf_turnout_get(_idx); \
		turnout_vars_t       *avars = &tvars[_idx];


static void process_turnout_cmd(msg_64_t *m)
{
	if (!MA0_IS_TURNOUT(m->to)) {
		return;
	}
	if (turnout_tasklet.runmode == runmode_off) {
		return;
	}
	uint8_t tidx = m->subc;
	USE_TURNOUT(tidx)
	if (!aconf || !avars) {
		turnout_error(ERR_BAD_PARAM, "bad idx");
		return;
	}
	if ((DBG_MSG_TURNOUT)) debug_info('A', 0, "CMD", tidx, m->cmd, avars->value);
#ifndef TRAIN_SIMU
	// unconfigured turnout
	if ((!aconf->cmd_portA) || (!aconf->cmd_portB)) {
		itm_debug2(DBG_ERR|DBG_TURNOUT, "unconf turn", tidx, m->cmd);
		return;
	}
#endif
	switch (m->cmd) {
	case CMD_TURNOUT_A:
		itm_debug2(DBG_TURNOUT, "TA", tidx, avars->value);
		avars->value = -1;
		// reset both, and set state. set will be done on timer (TODO: we could set it directly here ?)
#ifndef TRAIN_SIMU
	    HAL_GPIO_WritePin(aconf->cmd_portA, aconf->pinA, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(aconf->cmd_portB, aconf->pinB, GPIO_PIN_RESET);
#endif
		avars->st = aconf->reverse ? ST_SETB : ST_SETA;
		break;
	case CMD_TURNOUT_B:
		itm_debug2(DBG_TURNOUT, "TB", tidx, avars->value);
		avars->value = 1;
#ifndef TRAIN_SIMU
	    HAL_GPIO_WritePin(aconf->cmd_portA, aconf->pinA, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(aconf->cmd_portB, aconf->pinB, GPIO_PIN_RESET);
#endif
		avars->st = aconf->reverse ? ST_SETA : ST_SETB;
		break;
	default:
		itm_debug1(DBG_ERR|DBG_TURNOUT, "inv cmd", m->cmd);
		break;
	}
}


static void turnout_init(void)
{
	for (int tidx=0; tidx<NUM_TURNOUTS; tidx++) {
		USE_TURNOUT(tidx) 	// aconf avars
		memset(avars, 0, sizeof(*avars));
		avars->value = 0;
		avars->st = ST_IDLE;
		if (!aconf) {
			itm_debug1(DBG_TURNOUT, "tn skip", tidx);
			continue;
		}
#ifndef TRAIN_SIMU
		if (!aconf->cmd_portA) return;
		if (!aconf->cmd_portB) return;

		HAL_GPIO_WritePin(aconf->cmd_portA, aconf->pinA, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(aconf->cmd_portB, aconf->pinB, GPIO_PIN_RESET);
#endif
		itm_debug1(DBG_TURNOUT, "A/RESET", tidx);
		debug_info('A', 0, "RESET", 0, 0,0);
		(void)aconf; // unused
	}
}
/*
int turnout_state(int tidx)
{
	turnout_vars_t *v = get_turnout_vars(tidx);
	if (!v) return turnout_error(ERR_BAD_PARAM, "bad idx");
	if (v->st != ST_IDLE) return 0; // change on progress
	return v->value; // XXX
}
*/



static void process_turnout_timers(_UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
	for (int i=0; i<NUM_TURNOUTS; i++) {
		if (emerg_stop) return;
		USE_TURNOUT(i)		// aconf , avars
#ifndef TRAIN_SIMU
        if (!aconf->cmd_portA) continue;
		if (!aconf->cmd_portB) continue;
#endif
		switch (avars->st) {
		case ST_IDLE:
			break;
		case ST_SETA:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(aconf->cmd_portA, aconf->pinA, GPIO_PIN_SET);
#else
			(void)aconf; // unused in SIMU
#endif
			avars->st = ST_RESETA;
			itm_debug1(DBG_TURNOUT, "A/SETA", i);
			if ((DBG_MSG_TURNOUT)) debug_info('A', 0, "A0/SETA", 0, 0,0);
			break;
		case ST_SETB:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(aconf->cmd_portB, aconf->pinB, GPIO_PIN_SET);
#endif
			avars->st = ST_RESETB;
			itm_debug1(DBG_TURNOUT, "A/SETB", i);
			if ((DBG_MSG_TURNOUT)) debug_info('A', 0, "A0/SETB", 0, 0,0);
			break;
		case ST_RESETA:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(aconf->cmd_portA, aconf->pinA, GPIO_PIN_RESET);
#endif
			avars->st = ST_IDLE;
			itm_debug1(DBG_TURNOUT, "A/RESETA", i);
			if ((DBG_MSG_TURNOUT)) debug_info('A', 0, "A0/RESETA", 0, 0,0);
			break;
		case ST_RESETB:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(aconf->cmd_portB, aconf->pinB, GPIO_PIN_RESET);
#endif
			avars->st = ST_IDLE;
			itm_debug1(DBG_TURNOUT, "A/RESETB", i);
			if ((DBG_MSG_TURNOUT)) debug_info('A', 0, "A0/RESETB", 0, 0,0);
			break;
			/*
		case ST_TEST:
#ifndef TRAIN_SIMU
			HAL_GPIO_TogglePin(aconf->cmd_port, aconf->pinA);
			HAL_GPIO_TogglePin(aconf->cmd_port, aconf->pinB);
#endif
			break;
			 */
		default:
			itm_debug1(DBG_TURNOUT|DBG_ERR, "bad state", avars->st);
			turnout_error(ERR_BAD_STATE, "bad state");
			break;
		}
	}
}
