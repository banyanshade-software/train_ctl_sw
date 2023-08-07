/*
 * cantonP.h
 *
 *  Created on: Aug 4, 2023
 *      Author: danielbraun
 */

#ifndef LOW_CANTONP_H_
#define LOW_CANTONP_H_


typedef struct canton_vars {
	int8_t cur_dir;
	uint8_t cur_voltidx;
	uint16_t cur_pwm_duty;
	int32_t selected_centivolt;
} canton_vars_t;


void canton_set_pwm(int cidx, const conf_canton_t *c, canton_vars_t *v,  int8_t sdir, int duty);
void canton_set_volt(int cn, const conf_canton_t *c, canton_vars_t *v, int voltidx);

#endif /* LOW_CANTONP_H_ */
