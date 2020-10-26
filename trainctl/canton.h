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

#ifndef CANTON_H_
#define CANTON_H_

#include <stdint.h>
#include "misc.h"

#ifndef TRAIN_SIMU
#include "stm32f1xx_hal.h"
#endif

#define CANTON_TYPE_DUMMY    (0)
#define CANTON_TYPE_REMOTE   (0xFF)
#define CANTON_TYPE_PROTO1   (1)

#ifndef TRAIN_SIMU
extern TIM_HandleTypeDef *CantonTimerHandles[8];
#endif

typedef struct canton_config {
	uint8_t  canton_type;
	uint16_t volts[16]; // unit : 1/10 V, from 1000 to 0
#ifndef TRAIN_SIMU
	GPIO_TypeDef *volt_port;
#else
    void *dummy; // for structure initialisation
#endif
	uint16_t volt_b0;
	uint16_t volt_b1;
	uint16_t volt_b2;
	uint16_t volt_b3;
	uint8_t pwm_timer_num; // 1..
	uint32_t ch0;
	uint32_t ch1;

	uint8_t notif_bemf:1;
} canton_config_t;


typedef enum canton_occupency {
	canton_free = 0,
	canton_next,
	canton_occupied_cars,
	canton_occupied_loco
} canton_occupency_t;

struct train;

typedef struct canton_vars {
	int8_t cur_dir;
	uint8_t cur_voltidx;
	uint16_t cur_pwm_duty;
	int32_t bemf_centivolt; //  1/100V
	//canton_volt_policy_t volt_policy;
	canton_occupency_t status;
	uint8_t curtrainidx;
	uint8_t fix_bemf;
} canton_vars_t;


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
	vpolicy_pure_volt
} train_volt_policy_t;


int volt_index(uint16_t mili_power,
		const canton_config_t *c1, canton_vars_t *v1,
		const canton_config_t *c2, canton_vars_t *v2,
		int *pvi1, int *pvi2, train_volt_policy_t);

void canton_reset(const canton_config_t *c, canton_vars_t *v);

int canton_take(const canton_config_t *c, canton_vars_t *v, canton_occupency_t st,  int trainidx);
int canton_change_status(const canton_config_t *c, canton_vars_t *v, canton_occupency_t st,  int trainidx);
int canton_release(const canton_config_t *c, canton_vars_t *v, int trainidx);

void canton_set_pwm(const canton_config_t *c, canton_vars_t *v,  int dir, int duty);
void canton_set_volt(const canton_config_t *c, canton_vars_t *v,  int voltidx);

//void canton_bemf(const canton_config_t *c, canton_vars_t *v, uint16_t adc1, uint16_t adc2);
void canton_bemf(const canton_config_t *c, canton_vars_t *v, uint16_t adc1, uint16_t adc2, uint16_t von1, uint16_t von2);




void canton_reset_calib(const canton_config_t *c, canton_vars_t *v, int16_t spd);
void canton_end_calib(const canton_config_t *c,   canton_vars_t *v, int16_t spd, int num);


#endif /* CANTON_H_ */
