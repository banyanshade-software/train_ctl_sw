/*
 * canton_config.h
 *
 *  Created on: Apr 18, 2021
 *      Author: danielbraun
 */

#ifndef LOW_CANTON_CONFIG_H_
#define LOW_CANTON_CONFIG_H_

#include "../misc.h"

#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#endif



#define VOLT_SEL_3BITS 1	// otherwise, legacy on 4 bits

#ifdef VOLT_SEL_3BITS
#define NUM_VOLTS_VAL 8
#else
#define NUM_VOLTS_VAL 16
#endif


typedef struct canton_config {
    //uint8_t  canton_type;
    uint16_t volts_cv[NUM_VOLTS_VAL]; // unit : 1/100 V, from 1000 to 0
    // V2 and V4 vers
    //uint16_t volts_v2[16]; // unit : 1/100 V, from 1000 to 0
    //uint16_t volts_v4[16]; // unit : 1/100 V, from 1000 to 0
#ifndef TRAIN_SIMU
	GPIO_TypeDef *volt_port_b0;
	GPIO_TypeDef *volt_port_b1;
	GPIO_TypeDef *volt_port_b2;
#ifndef VOLT_SEL_3BITS
	GPIO_TypeDef *volt_port_b3;
#endif
#else
    void *dummy0; // for structure initialisation
    void *dummy1; // for structure initialisation
    void *dummy2; // for structure initialisation
#ifndef VOLT_SEL_3BITS
    void *dummy3; // for structure initialisation
#endif
#endif
	uint16_t volt_b0;
	uint16_t volt_b1;
	uint16_t volt_b2;
#ifndef VOLT_SEL_3BITS
	uint16_t volt_b3;
#endif
	uint8_t pwm_timer_num; // 1.. index in CantonTimerHandles
	uint32_t ch0;
	uint32_t ch1;

	uint8_t notif_bemf:1;
	uint8_t reverse_bemf:1;
} canton_config_t;

#endif /* LOW_CANTON_CONFIG_H_ */
