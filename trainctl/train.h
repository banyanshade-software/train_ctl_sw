/*
 * train.h
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

/*
 * train.h : configuration and variables for trains
 * 			there is very little more than that here, most processing is in traincontrol.c
 *
 * 			TODO: structure here describe more a locomotive than a train
 * 			so struct should probably be renamed, and train structure make reference to locomotive + cars
 */

#ifndef TRAIN_H_
#define TRAIN_H_

#include "param.h"
#include "spdctl/inertia.h"
#include "spdctl/pidctl.h"
#include "low/canton.h"



typedef struct train_config {
	pidctl_config_t pidcnf;
	inertia_config_t inertiacnf;
	train_volt_policy_t volt_policy;

	uint8_t enable_inertia; // 0=none 1=before PID 2=after PID

    // not as bitfield due to param.h
	uint8_t enabled;
	uint8_t enable_pid;
	uint8_t notify_speed;
	uint8_t notify_pose;
	uint8_t bemfIIR;
    uint8_t postIIR;
	uint8_t fix_bemf;
	uint8_t en_spd2pow;
	uint8_t min_power;

	uint8_t reversed; // if train motor is mounted in the wrong way
    uint8_t slipping;
	uint16_t pose_per_cm;
    uint8_t trainlen_left_cm;
    uint8_t trainlen_right_cm;
} train_config_t;

//void train_reset(const train_config_t *c, train_vars_t *v);
int train_reset_pos_estimate(int tidx);


//extern const param_t train_params[];

/*
 * Units :
 * -------
 * speed are 0 (0%) to 1000 (100%), with direction
 * volts are 0 (0V) to 1000 (10V)
 *
 */
#endif /* TRAIN_H_ */
