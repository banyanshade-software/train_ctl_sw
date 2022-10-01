/*
 * ctrl.h
 *
 *  Created on: Apr 21, 2021
 *      Author: danielbraun
 */

#ifndef CTRL_CTRL_H_
#define CTRL_CTRL_H_

#include "../msg/tasklet.h"

extern tasklet_t ctrl_tasklet;
//void ctrl_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);





typedef enum {
    train_notrunning=0,    // train is not running/existing,
    train_manual,        // manual drive with anti collision
    train_fullmanual,    // full manual mode
    train_auto,            //    auto mode
} train_mode_t;


typedef enum {
    train_off=0,
    train_running_c1,    // running (auto or manual)
#ifdef OLD_CTRL
    train_running_c1c2, // transition c1/c2 on progress
#endif
    train_station,        // waiting at station
    train_blk_wait,        // stopped (block control)
    train_end_of_track,    // idem but can only be changed by a direction change
} train_state_t;


int ctrl_get_train_curlsblk(int numtrain);

#endif /* CTRL_CTRL_H_ */
