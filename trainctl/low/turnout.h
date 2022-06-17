/*
 * turnout.h
 *
 *  Created on: Oct 24, 2020
 *      Author: danielbraun
 */


/*
 * (c) Daniel Braun 2020
 * ---------------------
 * available under GPLv3 http://www.gnu.org/licenses/gpl-3.0.html
 *
 */


/* turnout.h : control of turnouts (switches)
 *    	turnout_tick() is supposed to be invoked every 20ms and will handle the impulse shape
 *    	TODO: make sure GPIO are reset in HardFault handler (and other), otherwise a turnout
 *    	may receive continuous current
 *    	TODO: relyability of marklin switch/turnouts : several impulses might be better ?
 *
 */


#ifndef TURNOUT_H_
#define TURNOUT_H_

#include "../misc.h"
#include "../msg/tasklet.h"

#ifndef TRAIN_SIMU
#include "main.h"
#endif

//void turnout_tasklet(uint32_t notif_flags, uint32_t tick, uint32_t dt);
extern tasklet_t turnout_tasklet;


#endif /* TURNOUT_H_ */
