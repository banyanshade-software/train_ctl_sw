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
#include "trainctl_config.h"
#include "../misc.h"
#include "../msg/tasklet.h"

#ifndef TRAIN_SIMU


#if defined(STM32F4)
#include "stm32f4xx_hal.h"

#elif defined(STM32G4)
#include "stm32g4xx_hal.h"

#elif defined(STM32F1)
#include "stm32f1xx_hal.h"

#else
#error no board hal
#endif

#endif

//#include "canton_config.h"
#include "../config/conf_canton.h"

//#define CANTON_TYPE_DUMMY    (0)
//#define CANTON_TYPE_REMOTE   (0xFF)
//#define CANTON_TYPE_PROTO1   (1)



#ifndef BOARD_HAS_CANTON
#error BOARD_HAS_CANTON not defined, remove this file from build
#endif



#ifndef TRAIN_SIMU
extern TIM_HandleTypeDef *CantonTimerHandles[8];
#endif

//void canton_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);

extern tasklet_t canton_tasklet;

#define MAX_PWM 90


/*
 * find common voltage for 2 cantons, and calculate pwm duty cycle
 * according to policy of c1
 * returns volt_index of c1 and c2 in *pvi1 and *pvi2,
 * return value is pwm duty in 0-100 range
 */

#if 0
typedef enum train_volt_policy /*: uint8_t*/ {
	vpolicy_normal = 0,
	vpolicy_pure_pwm,
	vpolicy_pure_volt,
    //vpolicy_v2,
    //vpolicy_v4
} train_volt_policy_t;
#endif

#if 0
int volt_index(uint16_t mili_power,
		const conf_canton_t *c1, //canton_vars_t *v1,
		const conf_canton_t *c2, //canton_vars_t *v2,
		int *pvi1, int *pvi2, train_volt_policy_t);

#endif


#endif /* CANTON_H_ */
