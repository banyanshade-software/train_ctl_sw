/*
 * turnout_config.h
 *
 *  Created on: Apr 18, 2021
 *      Author: danielbraun
 */

#ifndef LOW_TURNOUT_CONFIG_H_
#define LOW_TURNOUT_CONFIG_H_

//#define NUM_TURNOUT 8

typedef struct turnout_config {
#ifndef TRAIN_SIMU
	GPIO_TypeDef *cmd_port;
#else
    void *dummy; // for structure initialisation
#endif
	uint16_t pinA;
	uint16_t pinB;
} turnout_config_t;


#endif /* LOW_TURNOUT_CONFIG_H_ */
