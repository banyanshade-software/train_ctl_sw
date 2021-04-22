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
#include "turnout_config.h"

#include "misc.h"
#include "../msg/trainmsg.h"


#include "railconfig.h"
#include "trainctl_iface.h"
#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#else
#include "train_simu.h"
#endif



static void turnout_reset(void);
static void process_turnout_timers(uint32_t tick, uint32_t dt);
static void process_turnout_cmd(msg_64_t *m, uint32_t tick, uint32_t dt);


void turnout_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt)
{
	static int first=1;
	if (first) {
		first = 0;
		turnout_reset();
	}
	process_turnout_timers(tick, dt);
	for (;;) {
		msg_64_t m;
		int rc = mqf_read_to_turnout(&m);
		if (rc) break;
		if (IS_TURNOUT(m.to)) {
			process_turnout_cmd(&m, tick, dt);
		} else if (IS_BROADCAST(m.to)) {
			switch (m.cmd) {
			case CMD_RESET: // FALLTHRU
			case CMD_EMERGENCY_STOP:
				turnout_reset();
				break;
			}
		} else {
			// error
		}
	}
}

// ----------------------------------------------------------------------------------------------

typedef struct turnout_vars {
	int8_t value;	// -1 = A, 1 = B, 0 = unknown
	uint8_t st;
} turnout_vars_t;

static turnout_vars_t tvars[NUM_TURNOUT];


#define ST_IDLE		0
#define ST_SETA 	1
#define ST_RESETA 	2
#define ST_SETB 	3
#define ST_RESETB	4
// #define ST_TEST		10


#define USE_TURNOUT(_idx) \
		const turnout_config_t *aconf = get_turnout_cnf(_idx); \
		turnout_vars_t         *avars = &tvars[_idx];


static void process_turnout_cmd(msg_64_t *m, uint32_t tick, uint32_t dt)
{
	uint8_t tidx = m->to & 0x07;
	USE_TURNOUT(tidx)
	if (!aconf || !avars) {
		turnout_error(ERR_BAD_PARAM, "bad idx");
		return;
	}
	debug_info('A', 0, "CMD", tidx, m->cmd, avars->value);
#ifndef TRAIN_SIMU
	if (!aconf->cmd_port) return;
#endif
	switch (m->cmd) {
	case CMD_TURNOUT_A:
		avars->value = -1;
#ifndef TRAIN_SIMU
	    HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinA, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinB, GPIO_PIN_RESET);
#endif
		avars->st = ST_SETA;
		break;
	case CMD_TURNOUT_B:
		avars->value = -1;
#ifndef TRAIN_SIMU
	    HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinA, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinB, GPIO_PIN_RESET);
#endif
		avars->st = ST_SETB;
		break;
	}
}


static void turnout_reset(void)
{
	for (int tidx=0; tidx<NUM_TURNOUT; tidx++) {
		USE_TURNOUT(tidx) 	// aconf avars
		memset(avars, 0, sizeof(*avars));
		avars->value = 0;
		avars->st = ST_IDLE;
		if (!aconf) {
			itm_debug1("tn skip", tidx);
			continue;
		}
#ifndef TRAIN_SIMU
		if (!aconf->cmd_port) return;

		HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinA, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinB, GPIO_PIN_RESET);
#endif
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




static void process_turnout_timers(uint32_t tick, uint32_t dt)
{
	for (int i=0; i<NUM_TURNOUTS; i++) {
		USE_TURNOUT(i)		// aconf , avars
#ifndef TRAIN_SIMU
        if (!aconf->cmd_port) continue;
#endif
		switch (avars->st) {
		case ST_IDLE:
			break;
		case ST_SETA:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinA, GPIO_PIN_SET);
#else
			(void)aconf; // unused in SIMU
#endif
			avars->st = ST_RESETA;
			debug_info('A', 0, "A0/SETA", 0, 0,0);
			break;
		case ST_SETB:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinB, GPIO_PIN_SET);
#endif
			avars->st = ST_RESETB;
			debug_info('A', 0, "A0/SETB", 0, 0,0);
			break;
		case ST_RESETA:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinA, GPIO_PIN_RESET);
#endif
			avars->st = ST_IDLE;
			debug_info('A', 0, "A0/RESETA", 0, 0,0);
			break;
		case ST_RESETB:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinB, GPIO_PIN_RESET);
#endif
			avars->st = ST_IDLE;
			debug_info('A', 0, "A0/RESETB", 0, 0,0);
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
			turnout_error(ERR_BAD_STATE, "bad state");
			break;
		}
	}
}
