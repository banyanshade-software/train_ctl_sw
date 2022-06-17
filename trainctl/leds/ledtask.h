/*
 * ledtask.h
 *
 *  Created on: Oct 22, 2021
 *      Author: danielbraun
 */

#ifndef LEDS_LEDTASK_H_
#define LEDS_LEDTASK_H_

#include "../msg/trainmsg.h"
#include "../msg/tasklet.h"

extern tasklet_t led_tasklet;

//oid led_tasklet(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, uint32_t dt);


#endif /* LEDS_LEDTASK_H_ */
