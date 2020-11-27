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
#include "misc.h"
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

#define ST_IDLE		0
#define ST_SETA 	1
#define ST_RESETA 	2
#define ST_SETB 	3
#define ST_RESETB	4
// #define ST_TEST		10

void turnout_reset(int tidx)
{
	USE_TURNOUT(tidx) 	// aconf avars
	memset(avars, 0, sizeof(*avars));
	avars->value = 0;
	avars->st = ST_IDLE;
#ifndef TRAIN_SIMU
	HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinA, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinB, GPIO_PIN_RESET);
#endif
	debug_info('A', 0, "RESET", 0, 0,0);
    (void)aconf; // unused
}
int turnout_state(int tidx)
{
	turnout_vars_t *v = get_turnout_vars(tidx);
	if (!v) return turnout_error(ERR_BAD_PARAM, "bad idx");
	if (v->st != ST_IDLE) return 0; // change on progress
	return v->value; // XXX
}
int turnout_cmd(int tidx, int vab)
{
	USE_TURNOUT(tidx) 	// aconf avars

	if (!aconf || !avars) return turnout_error(ERR_BAD_PARAM, "bad idx");
	debug_info('A', 0, "CMD", tidx, vab, avars->value);

	//if (vab == v->value) return 0;
	avars->value = vab;
#ifndef TRAIN_SIMU
    HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinA, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(aconf->cmd_port, aconf->pinB, GPIO_PIN_RESET);
#endif
	if (-1==vab) {
		avars->st = ST_SETA;
	} else {
		avars->st = ST_SETB;
	}
	return 0;
}

int turnout_test(int tidx)
{
#if 0
	// for test, without the actual turnout plugged.. removed since potentially
	// dangerous for the turnout
	const turnout_config_t *c = get_turnout_cnf(tidx);
	turnout_vars_t *v = get_turnout_vars(tidx);

	debug_info('A', 0, "W", tidx, 0, v->value);
	if (!c || !v) return turnout_error(ERR_BAD_PARAM, "bad idx");
	/*
#ifndef TRAIN_SIMU
	HAL_GPIO_WritePin(c->cmd_port, c->pinA, GPIO_PIN_SET);
	HAL_GPIO_WritePin(c->cmd_port, c->pinB, GPIO_PIN_RESET);
#endif
	v->st = ST_TEST;
	*/
#endif
    return 0;
}




void turnout_tick(void)
{
	for (int i=0; i<NUM_TURNOUTS; i++) {
		USE_TURNOUT(i)		// aconf , avars
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
