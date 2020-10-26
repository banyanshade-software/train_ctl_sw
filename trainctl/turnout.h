/*
 * turnout.h
 *
 *  Created on: Oct 24, 2020
 *      Author: danielbraun
 */

#ifndef TURNOUT_H_
#define TURNOUT_H_

#include "misc.h"

#ifndef TRAIN_SIMU
#include "main.h"
#endif


typedef struct turnout_config {
#ifndef TRAIN_SIMU
	GPIO_TypeDef *cmd_port;
#else
    void *dummy; // for structure initialisation
#endif
	uint16_t pinA;
	uint16_t pinB;
} turnout_config_t;

typedef struct turnout_vars {
	uint8_t value;	// -1 = A, 1 = B, 0 = unknown
	uint8_t st;
} turnout_vars_t;


void turnout_reset(int tidx);


int turnout_state(int tidx);
int turnout_cmd(int tidx, int vab); // -1 or 1

void turnout_tick(void);


int turnout_test(int tidx);

#endif /* TURNOUT_H_ */
