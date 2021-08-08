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
	train_notrunning=0,	// train is not running, mode not yet defined
	train_manual,		// manual drive with anti collision
	train_fullmanual,	// full manual mode
	train_auto,			//	auto mode
} train_mode_t;


typedef enum {
	train_off=0,
	train_running_c1,	// running (auto or manual)
	train_running_c1c2, // transition c1/c2 on progress
	train_station,		// waiting at station
	train_blk_wait,		// stopped (block control)
	train_end_of_track,	// idem but can only be changed by a direction change
} train_state_t;


#define BLK_OCC_FREE    0x00
#define BLK_OCC_STOP    0x01
#define BLK_OCC_LEFT    0x02
#define BLK_OCC_RIGHT    0x03
#define BLK_OCC_C2        0x04

#define BLK_OCC_DELAY1    0x10
#define BLK_OCC_DELAYM    0x16


#endif /* CTRL_CTRL_H_ */
