/*
 * traincontrol.h
 *
 *  Created on: Oct 3, 2020
 *      Author: danielbraun
 */

/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

/* traincontrol.h : main control of train :
 * 			target_speed -> inertia -> BEMF feedback -> volt + pwm
 */

#ifndef TRAINCONTROL_H_
#define TRAINCONTROL_H_

#include "railconfig.h"
#include "trainctl_iface.h"


extern volatile uint8_t trainctl_test_mode;


#define NUM_VAL_PER_CANTON (sizeof(adc_buffer_t)/sizeof(uint16_t))
#define ADC_HALF_BUFFER (NUM_LOCAL_CANTONS*NUM_VAL_PER_CANTON)
#define NUM_ADC_SAMPLES (2*NUM_LOCAL_CANTONS*NUM_VAL_PER_CANTON)

extern volatile adc_buffer_t train_adc_buffer[2*NUM_LOCAL_CANTONS];


void train_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);


int train_set_target_speed(int numtrain, int16_t target);
void train_stop_all(void);

void calibrate_bemf(void);




#endif /* TRAINCONTROL_H_ */
