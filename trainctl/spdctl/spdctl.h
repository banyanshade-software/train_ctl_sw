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
#include "../misc.h"
#include "../msg/tasklet.h"

extern tasklet_t spdctl_tasklet;

/*
 CAUTION : used by ctrlLT to update pos, but
 only ok because ctrl and spdctl are on same node (and same thread)
 */
int16_t spdctl_get_lastpose(int tidx, xblkaddr_t b);

#endif /* TRAINCONTROL_H_ */
