/*
 * canton_bemf.h
 *
 *  Created on: Apr 18, 2021
 *      Author: danielbraun
 */

#ifndef LOW_CANTON_BEMF_H_
#define LOW_CANTON_BEMF_H_


#include <stdint.h>
#include "misc.h"

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

/*

// notif_flags
#ifndef USE_INA3221
typedef struct {
	uint16_t intOff; // XXX
	uint16_t voffA;
	uint16_t voffB;
	uint16_t intOn; // XXX
	uint16_t vonA;
	uint16_t vonB;

} adc_buffer_t;
#error ohla
#else
typedef struct {
	//uint16_t intOff; // XXX
	uint16_t voffA;
	uint16_t voffB;
	//uint16_t intOn; // XXX
	uint16_t vonA;
	uint16_t vonB;

} adc_buffer_t;
#endif


extern volatile adc_buffer_t train_adc_buffer[2*NUM_LOCAL_CANTONS_HW];
*/
typedef struct {
	// uint16_t I;
	uint16_t vA;
	uint16_t vB;
} adc_per_blk_t;

typedef struct {
	adc_per_blk_t off[NUM_LOCAL_CANTONS_HW];
	adc_per_blk_t on[NUM_LOCAL_CANTONS_HW];
} adc_buf_t;


extern volatile adc_buf_t train_adc_buf[2]; // double buffer

extern uint8_t bemf_test_mode;
extern uint8_t bemf_test_all;

/*
#define NUM_VAL_PER_CANTON (sizeof(adc_buffer_t)/sizeof(uint16_t))
#define ADC_HALF_BUFFER (NUM_LOCAL_CANTONS_HW * NUM_VAL_PER_CANTON)
#define NUM_ADC_SAMPLES (2*ADC_HALF_BUFFER)
*/

#endif /* LOW_CANTON_BEMF_H_ */
