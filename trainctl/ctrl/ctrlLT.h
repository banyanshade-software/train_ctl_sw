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
} __attribute__((packed)) train_state_t;


extern uint8_t ctrl_flag_notify_speed ;

typedef struct {
    uint8_t num;
    uint8_t postag;
    lsblk_num_t sblk;
    uint8_t futc2;
    uint8_t done:1;
} trig_pend_t;

#define NUM_PEND_TRIGS 6

typedef struct {
    train_mode_t    _mode;
    train_state_t   _state;
    int16_t         _desired_signed_speed;
    uint16_t        _target_unisgned_speed;
    int8_t          _sdir;      // -1 or 1, 0 if stopped
    uint8_t         _spd_limit;
    //
    lsblk_num_t c1_sblk;
    xblkaddr_t  canOld_xaddr;
    xblkaddr_t  can1_xaddr;
    xblkaddr_t  can2_xaddr;
    
    //xblkaddr_t  tmp_c2_future; // temporary c2, will be store in either pow_c2_future or res_c2_future
    xblkaddr_t  pow_c2_future;
    xblkaddr_t  res_c2_future;
    xblkaddr_t freecanton;
    lsblk_num_t sblkfreed;
    uint8_t c1c2dir_changed:1;
    //uint8_t measure_pose_percm:1;
    uint8_t c1c2:1;
    uint8_t off_requested:1;
    uint8_t has_delayed_spd:1;
    uint8_t purgeTrigs:1;
    
    uint8_t brake_for_eot:1;
    uint8_t brake_for_blkwait:1;
    uint8_t brake_for_user:1;
    int8_t delayed_spd;
    //
    // #longtrain
    uint8_t brake:1;
    uint8_t canMeasureOnSblk:1;
    uint8_t canMeasureOnCanton:1;
    
    uint8_t num_pos_adjust;
    
    //
    int32_t _curposmm;
    int32_t beginposmm; // left side ofl sblk,  either 0, or -len
    
    trig_pend_t pendTrigs[NUM_PEND_TRIGS];
    uint8_t trigNs;
    uint8_t trigSlt;
    
    // BRKFREE
    lsblk_num_t brake_on_free;
} train_ctrl_t;


#define POSE_UNKNOWN 9999999


extern int ignore_ina_pres(void);


// ----------------------------------------------------------------


/// ctrl3_init_train
/// initialize train on a given sblk
/// @param tidx train index
/// @param tvars train ctrl vars
/// @param sblk initial train sblk position
/// @param posmm initial train position
/// @param on if true turn train on an put it in station state
void ctrl3_init_train(int tidx, train_ctrl_t *tvars, lsblk_num_t sblk, int posmm, int on);


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
/// @param posed10 position of stop in pose/10 units
/// @param frombrake true if stopped was caused from braking
void ctrl3_stop_detected(int tidx, train_ctrl_t *tvars, int32_t posed10, int frombrake);



/// ctrl3_pose_triggered
/// called when spdctl detect trigger condition
/// @param tidx train index
/// @param tvars train ctrl vars
/// @param trigsn  trigger sequence number
/// @param ca_addr canton triggering action (trigger will be ignored if not current canton)
/// @param cposd10 current position,  in pose unit divided by 10
void ctrl3_pose_triggered(int tidx, train_ctrl_t *tvars, uint8_t trigsn, xblkaddr_t ca_addr, int16_t cposd10);


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


void ctrl3_evt_entered_c2(int tidx, train_ctrl_t *tvars, uint8_t from_bemf);

void ctrl3_evt_entered_new_lsblk_same_canton(int tidx, train_ctrl_t *tvars, lsblk_num_t sblk, int jumped);
void ctrl3_evt_entered_new_lsblk_c2_canton(int tidx, train_ctrl_t *tvars,lsblk_num_t sblk);


void ctrl3_set_mode(int tidx, train_ctrl_t *tvar, train_mode_t mode);

extern int ignore_ina_pres(void);
extern int ignore_bemf_pres(void);

int32_t ctrl3_getcurpossmm(train_ctrl_t *tvars, const conf_train_t *tconf, int left);

// called by c3auto (thru ctrl_set_brake_on_back)
void ctrl3_set_brake_on_back(int tidx, train_ctrl_t *tvars, lsblk_num_t ns);

#endif /* ctrlLT_h */
