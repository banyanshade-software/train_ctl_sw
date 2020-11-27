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

#define CANTON_TYPE_DUMMY    (0)
#define CANTON_TYPE_REMOTE   (0xFF)
#define CANTON_TYPE_PROTO1   (1)

#ifndef TRAIN_SIMU
extern TIM_HandleTypeDef *CantonTimerHandles[8];
#endif

typedef struct canton_config {
    uint8_t  canton_type;
    uint16_t volts[16]; // unit : 1/100 V, from 1000 to 0
    // V2 and V4 vers
    uint16_t volts_v2[16]; // unit : 1/100 V, from 1000 to 0
    uint16_t volts_v4[16]; // unit : 1/100 V, from 1000 to 0
#ifndef TRAIN_SIMU
	GPIO_TypeDef *volt_port_b0;
	GPIO_TypeDef *volt_port_b1;
	GPIO_TypeDef *volt_port_b2;
	GPIO_TypeDef *volt_port_b3;
#else
    void *dummy; // for structure initialisation
#endif
	uint16_t volt_b0;
	uint16_t volt_b1;
	uint16_t volt_b2;
	uint16_t volt_b3;
	uint8_t pwm_timer_num; // 1.. index in CantonTimerHandles
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
    // for stats / report
    int16_t selected_centivolt;
    int16_t von_centivolt;
    
	//canton_volt_policy_t volt_policy;
	canton_occupency_t status;
	uint8_t curtrainidx;
	uint8_t fix_bemf;

	uint16_t i_on;
	uint16_t i_off;
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
	vpolicy_pure_volt,
    vpolicy_v2,
    vpolicy_v4
} train_volt_policy_t;


int volt_index(uint16_t mili_power,
		const canton_config_t *c1, canton_vars_t *v1,
		const canton_config_t *c2, canton_vars_t *v2,
		int *pvi1, int *pvi2, train_volt_policy_t);

void canton_reset(const canton_config_t *c, canton_vars_t *v);

int canton_take(int numcanton, canton_occupency_t st,  int trainidx);
int canton_change_status(int numcanton, canton_occupency_t st,  int trainidx);
int canton_release(int numcanton, int trainidx);

void canton_set_pwm(const canton_config_t *c, canton_vars_t *v,  int dir, int duty);
void canton_set_volt(const canton_config_t *c, canton_vars_t *v,  int voltidx);

//void canton_bemf(const canton_config_t *c, canton_vars_t *v, uint16_t adc1, uint16_t adc2);
void canton_bemf(const canton_config_t *c, canton_vars_t *v, uint16_t adc1, uint16_t adc2, uint16_t von1, uint16_t von2);
void canton_intensity(const canton_config_t *c, canton_vars_t *v, uint16_t ioff, uint16_t ion);




void canton_reset_calib(const canton_config_t *c, canton_vars_t *v, int16_t spd);
void canton_end_calib(const canton_config_t *c,   canton_vars_t *v, int16_t spd, int num);


#endif /* CANTON_H_ */
