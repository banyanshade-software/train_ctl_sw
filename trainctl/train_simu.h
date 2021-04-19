#ifndef TRAIN_SIMU_H
#define TRAIN_SIMU_H


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */

void train_simu_canton_volt(int numcanton, int voltidx, int vlt100);
void train_simu_canton_set_pwm(int numcanton, int dir, int duty);

#define HAL_GetTick() (0)

#define NUM_LOCAL_CANTONS 8

#define USE_INA3221

#include "low/canton_bemf.h"
#include "low/canton.h"
#include "low/turnout.h"
#include "spdctl/traincontrol.h"
#include "msg/trainmsg.h"
#endif
