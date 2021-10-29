//
//  ctrlP.h
//  train_throttle
//
//  Created by Daniel BRAUN on 28/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef ctrlP_h
#define ctrlP_h

#include "railconfig.h"
// -----------------------------------------------------------
//per train stucture


// timers number
#define TLEAVE_C1      0
#define TBEHAVE        1
#define NUM_TIMERS     2

// timers values in tick (ms)
#define TLEAVE_C1_VALUE 20
#define TGUARD_C1_VALUE 100


typedef enum {
    train_notrunning=0,    // train is not running, mode not yet defined
    train_manual,        // manual drive with anti collision
    train_fullmanual,    // full manual mode
    train_auto,            //    auto mode
} train_mode_t;


typedef enum {
    train_off=0,
    train_running_c1,    // running (auto or manual)
    train_running_c1c2, // transition c1/c2 on progress
    train_station,        // waiting at station
    train_blk_wait,        // stopped (block control)
    train_end_of_track,    // idem but can only be changed by a direction change
} train_state_t;



typedef struct {
    train_mode_t   _mode;
    train_state_t  _state;

    uint16_t _target_speed;
    int8_t   _dir;

    uint8_t     can1_addr;
    lsblk_num_t c1_sblk;
    uint8_t     can2_addr;

    uint16_t spd_limit;
    uint16_t desired_speed;
    //uint8_t limited:1;

    uint16_t behaviour_flags;
    uint32_t timertick[NUM_TIMERS];
} train_ctrl_t;


void ctrl_evt_cmd_set_setdirspeed(int tidx, train_ctrl_t *tvars, int8_t dir, uint16_t spd, _UNUSED_ uint8_t generated);



#define ignore_bemf_presence 0
#define ignore_ina_presence  1


void ctrl_reset_timer(int tidx, train_ctrl_t *tvar, int numtimer);
void ctrl_set_timer(int tidx, train_ctrl_t *tvar, int numtimer, uint32_t tval);



static inline void ctrl_set_state(int tidx, train_ctrl_t *tvar, train_state_t ns)
{
    if (ns == tvar->_state) {
        return;
    }
    switch (ns) {
    case train_off:             itm_debug1(DBG_CTRL, "ST->OFF", tidx); break;
    case train_running_c1:         itm_debug1(DBG_CTRL, "ST->RC1", tidx); break;
    case train_running_c1c2:     itm_debug1(DBG_CTRL, "ST->C1C2", tidx); break;
    case train_station:            itm_debug1(DBG_CTRL, "ST->STA", tidx); break;
    case train_blk_wait:         itm_debug1(DBG_CTRL, "ST->BLKW", tidx); break;
    case train_end_of_track:    itm_debug1(DBG_CTRL, "ST->EOT", tidx); break;
    default:                     itm_debug2(DBG_CTRL, "ST->?", tidx, ns); break;
    }
    tvar->_state = ns;
    
    
    // notif UI
    msg_64_t m;
    m.from = MA_CONTROL_T(tidx);
    m.to = MA_UI(UISUB_TFT);
    m.cmd = CMD_TRSTATE_NOTIF;
    m.v1u = ns;
    mqf_write_from_ctrl(&m);
}


void ctrl_changed_lsblk(int tidx, train_ctrl_t *tvars, lsblk_num_t newsblk);

void ctrl_evt_entered_c2(int tidx, train_ctrl_t *tvar, uint8_t from_bemf);
void ctrl_evt_leaved_c1(int tidx, train_ctrl_t *tvar);


typedef enum {
    upd_init,
    upd_change_dir,
    upd_c1c2,
    upd_pose_trig,
    upd_check,
} ctrl_update_reason_t;


/// ctrl_update_c2_state_limits
///
///updates c2 info, and potentially speed limit and state
///
///should be called whenever changes are made in tvars :
/// change direction, train goes on new canton, or topology/occupency
/// changed
///
/// function is to be break into smaller part
///
/// @param tidx train number
/// @param tvars train tvars
/// @param tconf train config (used for cm to POSE conversion)
/// @param updreason reason for updating

void ctrl_update_c2_state_limits(int tidx, train_ctrl_t *tvars, const train_config_t *tconf, ctrl_update_reason_t updreason);


/// ctrl_set_pose_trig
///     order SPDCTL to set a trigger on POSE (POSition Estimate)
///
/// ctrl_set_pose_trig sends a CMD_POSE_SET_TRIG1 or CMD_POSE_SET_TRIG2 to SPDCTL
/// trigger 0 is used to detect logical sub block transition
///
/// trigger 1 is used to stop in the middle of lsblk e.g. when end of track
///
/// @param numtrain train number
/// @param pose pose value (internal unit, corresponding to BEMF integration)
/// @param trignum trigger num (0 or 1)

void ctrl_set_pose_trig(int numtrain, int32_t pose, int trignum);



// behaviour_flags
#define BEHAVE_STOPPED        (1<<1)        // 2
#define BEHAVE_EOT1            (1<<2)        // 4
#define BEHAVE_EOT2            (1<<3)        // 8
#define BEHAVE_BLKW            (1<<4)        // 16
#define BEHAVE_PTRIG            (1<<5)        // 32
#define BEHAVE_RESTARTBLK     (1<<6)        // 64
#define BEHAVE_TBEHAVE        (1<<7)        // 128
#define BEHAVE_CHBKLK        (1<<8)        // 256


// --------------------------------------
// main target-speed and direction command
// sent to SPEED_CONTROL
//

/// ctrl_set_tspeed
///     set target speed
///
///     target speed and direction are set independantly
///
///     UI is notified (unless no change are made) and
///     command CMD_SET_TARGET_SPEED is sent to SpeedCtrl
///
/// @param trnum train number
/// @param tvars train tvars
/// @param tspd target speed (>=0)

void ctrl_set_tspeed(int trnum, train_ctrl_t *tvars, uint16_t tspd);


/// ctrl_set_dir
///    set train diretion
///
///    note that train may have speed 0 and direvtion non-0
///    (meaning it is stopping but not yet stopped due to inertia)
///
///    UI is notified (unless no change are made)
///
///    Nothing is sent to SpeedCtr so it is assumed that ctrl_set_tspeed() will also be sent.
///
/// @param trnum train number
/// @param tvars train tvars
/// @param dir direction (-1/0/1)
/// @param force force sending msg even if no changed are made


void ctrl_set_dir(int trnum,  train_ctrl_t *tvars, int  dir, int force);


#endif /* ctrlP_h */
