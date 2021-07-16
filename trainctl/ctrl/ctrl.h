/*
 * ctrl.h
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */

#ifndef CTRL_CTRL_H_
#define CTRL_CTRL_H_

void ctrl_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);

typedef enum {
	train_notrrunning=0,	// train is not running, mode not yet defined
	train_manual,		// manual drive with anti collision
	train_fullmanual,	// full manual mode
	train_auto,			//	auto mode
} train_mode_t;


#endif /* CTRL_CTRL_H_ */
