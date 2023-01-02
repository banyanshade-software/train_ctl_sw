//
//  ctrlLT.h
//  train_throttle
//
//  Created by Daniel Braun on 19/12/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef ctrlLT_h
#define ctrlLT_h

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


#define MAX_LSBLK_CARS 4
struct forwdsblk {
    lsblk_num_t r[MAX_LSBLK_CARS];
    int16_t     rlen_cm;            // caution value should be updated if curposmm changed
    int8_t      nr;
};

extern uint8_t ctrl_flag_notify_speed ;


typedef struct {
    train_mode_t    _mode;
    train_state_t   _state;
    int16_t         _desired_signed_speed;
    uint16_t        _target_unisgned_speed;
    int8_t          _sdir;      // -1 or 1, 0 if stopped
    uint8_t         _spd_limit;
    //
    lsblk_num_t c1_sblk;
    xblkaddr_t  can1_xaddr;
    xblkaddr_t  can2_xaddr;
    uint8_t c1c2dir_changed:1;

    //
    // #longtrain
    struct forwdsblk leftcars;
    struct forwdsblk rightcars;
    //
    int32_t _curposmm;
    int32_t stopposmm;  // when brake condition, position where train is expected to stop
    int32_t beginposmm; // left side ofl sblk,  either 0, or -len

    uint8_t brake:1;
    uint8_t off_requested:1;
} train_ctrl_t;


#define POSE_UNKNOWN 9999999


extern int ignore_ina_pres(void);


// ----------------------------------------------------------------


/// ctrl3_init_train
/// initialize train on a given sblk
/// @param tidx train index
/// @param tvars train ctrl vars
/// @param sblk initial train position
/// @param on if true turn train on an put it in station state
void ctrl3_init_train(int tidx, train_ctrl_t *tvars, lsblk_num_t sblk, int on);


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
/// @param ca_addr canton triggering action (trigger will be ignored if not current canton)
/// @param cposd10 current position,  in pose unit divided by 10
void ctrl3_pose_triggered(int tidx, train_ctrl_t *tvars, pose_trig_tag_t trigtag, xblkaddr_t ca_addr, int16_t cposd10);


/// ctrl3_occupency_updated
/// called when turnouts or occupency had been updated
/// by spdctl
/// @param tidx trian index
/// @param tvars train ctrl vars
void ctrl3_occupency_updated(int tidx, train_ctrl_t *tvars);


/// turn_train_on turn train on, and put it in station state
/// only possible if train is off
/// @param tidx train index
/// @param tvars train ctrl vars
void turn_train_on(int tidx, train_ctrl_t *tvars);

void turn_train_off(int tidx, train_ctrl_t *tvars);

#endif /* ctrlLT_h */
