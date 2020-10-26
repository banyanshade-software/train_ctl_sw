/*
 * turnout.c
 *
 *  Created on: Oct 24, 2020
 *      Author: danielbraun
 */

#include "turnout.h"
#include "misc.h"
#include "railconfig.h"
#include "trainctl_iface.h"
#ifndef TRAIN_SIMU
#include "stm32f1xx_hal.h"
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
	const turnout_config_t *c = get_turnout_cnf(tidx);
	turnout_vars_t *v = get_turnout_vars(tidx);
	v->value = 0;
	v->st = ST_IDLE;
#ifndef TRAIN_SIMU
	HAL_GPIO_WritePin(c->cmd_port, c->pinA, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(c->cmd_port, c->pinB, GPIO_PIN_RESET);
#endif
	debug_info('A', 0, "RESET", 0, 0,0);
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
	const turnout_config_t *c = get_turnout_cnf(tidx);
	turnout_vars_t *v = get_turnout_vars(tidx);

	debug_info('A', 0, "CMD", tidx, vab, v->value);
	if (!c || !v) return turnout_error(ERR_BAD_PARAM, "bad idx");
	//if (vab == v->value) return 0;
	v->value = vab;
#ifndef TRAIN_SIMU
    HAL_GPIO_WritePin(c->cmd_port, c->pinA, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(c->cmd_port, c->pinB, GPIO_PIN_RESET);
#endif
	if (-1==vab) {
		v->st = ST_SETA;
	} else {
		v->st = ST_SETB;
	}
	return 0;
}

int turnout_test(int tidx)
{
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
    return 0;
}




void turnout_tick(void)
{
	for (int i=0; i<NUM_TURNOUTS; i++) {
		const turnout_config_t *c = get_turnout_cnf(i);
		turnout_vars_t *v = get_turnout_vars(i);
		switch (v->st) {
		case ST_IDLE:
			break;
		case ST_SETA:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(c->cmd_port, c->pinA, GPIO_PIN_SET);
#endif
			v->st = ST_RESETA;
			debug_info('A', 0, "A0/SETA", 0, 0,0);
			break;
		case ST_SETB:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(c->cmd_port, c->pinB, GPIO_PIN_SET);
#endif
			v->st = ST_RESETB;
			debug_info('A', 0, "A0/SETB", 0, 0,0);
			break;
		case ST_RESETA:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(c->cmd_port, c->pinA, GPIO_PIN_RESET);
#endif
			v->st = ST_IDLE;
			debug_info('A', 0, "A0/RESETA", 0, 0,0);
			break;
		case ST_RESETB:
#ifndef TRAIN_SIMU
			HAL_GPIO_WritePin(c->cmd_port, c->pinB, GPIO_PIN_RESET);
#endif
			v->st = ST_IDLE;
			debug_info('A', 0, "A0/RESETB", 0, 0,0);
			break;
			/*
		case ST_TEST:
#ifndef TRAIN_SIMU
			HAL_GPIO_TogglePin(c->cmd_port, c->pinA);
			HAL_GPIO_TogglePin(c->cmd_port, c->pinB);
#endif
			break;
			*/
		default:
			turnout_error(ERR_BAD_STATE, "bad state");
			break;
		}
	}
}
