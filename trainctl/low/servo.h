/*
 * servo.h
 *
 *  Created on: Nov 2, 2022
 *      Author: danielbraun
 */

#ifndef LOW_SERVO_H_
#define LOW_SERVO_H_

#include "../misc.h"
#include "../msg/tasklet.h"


#ifndef TRAIN_SIMU
#include "main.h"
#endif

extern tasklet_t servo_tasklet;


#endif /* LOW_SERVO_H_ */
