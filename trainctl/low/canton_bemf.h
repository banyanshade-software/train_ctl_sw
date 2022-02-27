/*
 * canton_bemf.h
 *
 *  Created on: Apr 18, 2021
 *      Author: danielbraun
 */

#ifndef LOW_CANTON_BEMF_H_
#define LOW_CANTON_BEMF_H_


#include <stdint.h>
#include "../misc.h"

#ifndef TRAIN_SIMU
#include "main.h"
#else
#include "train_simu.h"
#endif

#include "canton_bemf.h"
#include "canton_config.h"
#include "../msg/trainmsg.h"


void bemf_reset(void);
void bemf_msg(msg_64_t *m);
void bemf_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);


typedef struct {
	// uint16_t I;
	uint16_t vA;
	uint16_t vB;
} adc_per_blk_t;

typedef struct {
#if 0
	adc_per_blk_t off[NUM_LOCAL_CANTONS_HW];
	adc_per_blk_t on[NUM_LOCAL_CANTONS_HW];
#else
	adc_per_blk_t on[NUM_LOCAL_CANTONS_HW];
	adc_per_blk_t off[NUM_LOCAL_CANTONS_HW];
#endif
} adc_buf_t;


typedef adc_buf_t adc_result_t;


//extern volatile adc_result_t *adc_result; // double buffer

extern volatile adc_buf_t train_adc_buf[2]; // double buffer
#define adc_result train_adc_buf

extern runmode_t bemf_run_mode;



#endif /* LOW_CANTON_BEMF_H_ */
