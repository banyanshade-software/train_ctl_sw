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


#endif /* LOW_CANTONP_H_ */
