/*
 * spdctl.h
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

#ifndef SPDCONTROL_H_
#define SPDCONTROL_H_

//#include "railconfig.h"
#include "trainctl_iface.h"


void spdctl_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);


extern volatile uint8_t trainctl_test_mode;



//void train_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);


int train_set_target_speed(int numtrain, int16_t target);
//void train_stop_all(void);

//void calibrate_bemf(void);




#endif /* TRAINCONTROL_H_ */
