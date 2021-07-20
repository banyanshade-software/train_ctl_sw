/*
 * canton.h
 *
 *  Created on: Oct 2, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */


/*
 * canton.h : control of a block
 *            and thus of a block control board
 *            - set main voltage (3.5-10V)
 *            - set PWM and duty
 *            - process BEMF measure
 *
 *  		  preliminary (but unused for now) support for
 *  		  - block occupency / free
 *  		  - remote block for multiple MCU system
 */

#ifndef CANTON_H_
#define CANTON_H_

#include <stdint.h>
#include "misc.h"

#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#endif

#include "canton_config.h"

//#define CANTON_TYPE_DUMMY    (0)
//#define CANTON_TYPE_REMOTE   (0xFF)
//#define CANTON_TYPE_PROTO1   (1)

#ifndef TRAIN_SIMU
extern TIM_HandleTypeDef *CantonTimerHandles[8];
#endif

void canton_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);



#define MAX_PWM 90


/*
 * find common voltage for 2 cantons, and calculate pwm duty cycle
 * according to policy of c1
 * returns volt_index of c1 and c2 in *pvi1 and *pvi2,
 * return value is pwm duty in 0-100 range
 */


typedef enum train_volt_policy /*: uint8_t*/ {
	vpolicy_normal = 0,
	vpolicy_pure_pwm,
	vpolicy_pure_volt,
    //vpolicy_v2,
    //vpolicy_v4
} train_volt_policy_t;


int volt_index(uint16_t mili_power,
		const canton_config_t *c1, //canton_vars_t *v1,
		const canton_config_t *c2, //canton_vars_t *v2,
		int *pvi1, int *pvi2, train_volt_policy_t);



#endif /* CANTON_H_ */
