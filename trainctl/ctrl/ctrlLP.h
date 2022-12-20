//
//  ctrlLP.h
//  train_throttle
//
//  Created by Daniel Braun on 19/12/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef ctrlLP_h
#define ctrlLP_h

#include "../config/conf_train.h"
//#include "ctrlP.h"

#include "trig_tags.h"


typedef enum {
    train_state_off=0,
    train_state_running,         // running (auto or manual)
    train_state_station,         // waiting at station
    train_state_blkwait,         // stopped (block control)
    train_state_blkwait0,        // stopped (block control)
    train_state_end_of_track,    // stopped at end of track
    train_state_end_of_track0,   // idem, waiting for stop confirm
} train_state_t;


typedef struct {
    train_mode_t    _mode;
    train_state_t   _state;
    int16_t         _desired_speed;
    uint16_t        _target_speed;
    int8_t          _sdir;      // -1 or 1, 0 if stopped
    uint8_t         _spd_limit;
} train_ctrl_t;




// ----------------------------------------------------------------


/// ctrl3_init_train
/// initialize train on a given sblk
/// @param tidx train index
/// @param tvars train ctrl vars
/// @param sblk initial train position
void ctrl3_init_train(int tidx, train_ctrl_t *tvars, lsblk_num_t sblk);


/// ctrl3_upcmd_set_desired_speed
/// called when user / auto set a non null target speed
/// @param tidx train index
/// @param tvars train ctrl vars
/// @param desired_speed  desired speed (non null)
void ctrl3_upcmd_set_desired_speed(int tidx, train_ctrl_t *tvars, int16_t desired_speed);


/// ctrl3_upcmd_set_desired_speed_zero
/// called when user / auto set a null target speed
/// @param tidx train index
/// @param tvars train ctrl vars
void ctrl3_upcmd_set_desired_speed_zero(int tidx, train_ctrl_t *tvars);



/// ctrl3_stop_detected
/// called when stop is detected (effective speed is 0)
/// by spdctl
/// @param tidx trian index
/// @param tvars train ctrl vars
void ctrl3_stop_detected(int tidx, train_ctrl_t *tvars);



/// ctrl3_pose_triggered
/// called when spdctl detect trigger condition
/// @param tidx train index
/// @param tvars train ctrl vars
/// @param trigtag  trigger tag
void ctrl3_pose_triggered(int tidx, train_ctrl_t *tvars, pose_trig_tag_t trigtag);

#endif /* ctrlLP_h */
