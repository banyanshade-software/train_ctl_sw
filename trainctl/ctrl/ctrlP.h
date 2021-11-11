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
#define TGUARD_C1_VALUE 1

typedef struct {
    train_mode_t   _mode;
    train_state_t  _state;

    uint16_t tick_flags;

    uint16_t _target_speed;
    int8_t   _dir;
    
    uint8_t  c1c2:1;
    uint8_t  pose2_set:1;
    uint8_t  pose2_is_blk_wait:1;
    
    uint8_t     can1_addr;
    lsblk_num_t c1_sblk;
    uint8_t     can2_addr;

    uint16_t spd_limit;
    int16_t desired_speed;
    //int16_t desired_speed2;
    //uint8_t limited:1;

    uint16_t behaviour_flags;
    uint32_t timertick[NUM_TIMERS];
    
    int32_t curposmm;
    int32_t beginposmm; // left side is 0, mm of right side
} train_ctrl_t;

/*
 tick_flags values
 */
//#define _TFLAG_C1_CHANGED    (1<<0) obsolete
//#define _TFLAG_POSE_TRIG1  (1<<1)
#define _TFLAG_NEED_C2       (1<<1)
#define _TFLAG_C1LSB_CHANGED (1<<2)
#define _TFLAG_POSE_TRIG2    (1<<3)
#define _TFLAG_STOP_DETECTED (1<<4)
#define _TFLAG_OCC_CHANGED   (1<<5)
#define _TFLAG_STATE_CHANGED (1<<6)
#define _TFLAG_DIR_CHANGED   (1<<7)
#define _TFLAG_TSPD_CHANGED  (1<<8)
#define _TFLAG_LIMIT_CHANGED (1<<9)
#define _TFLAG_DSPD_CHANGED  (1<<10)
#define _TFLAG_C1_CHANGED    (1<<11)
#define _TFLAG_C2_CHANGED    (1<<11)


int ctrl2_tick_process(int tidx, train_ctrl_t *tvars, const train_config_t *tconf, int8_t occupency_changed);
void ctrl2_init_train(int tidx, train_ctrl_t *tvars,
                      lsblk_num_t sblk);

void ctrl2_upcmd_set_desired_speed(int tidx, train_ctrl_t *tvars, int16_t desired_speed);


void ctrl2_evt_entered_c2(int tidx, train_ctrl_t *tvar, uint8_t from_bemf);
void ctrl2_evt_leaved_c1(int tidx, train_ctrl_t *tvars);
void ctrl2_evt_entered_c1(int tidx, train_ctrl_t *tvars, _UNUSED_ uint8_t from_bemf);
void ctrl2_evt_leaved_c2(int tidx, train_ctrl_t *tvar);


void ctrl2_set_state(int tidx, train_ctrl_t *tvar, train_state_t ns);
void ctrl2_stop_detected(int tidx, train_ctrl_t *tvars);
void ctrl2_set_dir(int tidx, train_ctrl_t *tvar, int8_t dir);
void ctrl2_set_tspeed(int tidx, train_ctrl_t *tvar, uint16_t tspeed);
void ctrl2_check_alreadystopped(int tidx, train_ctrl_t *tvar);
void ctrl2_check_checkstart(int tidx, train_ctrl_t *tvar);
void ctrl2_check_stop(int tidx, train_ctrl_t *tvar);
void ctrl2_apply_speed_limit(int tidx, train_ctrl_t *tvar);
void ctrl2_update_topo(int tidx, train_ctrl_t *tvar, const train_config_t *tconf, int32_t *ppose1);
void ctrl2_update_c2(int tidx, train_ctrl_t *tvar, const train_config_t *tconf, int32_t *ppose0);
void ctrl2_notify_state(int tidx, train_ctrl_t *tvar);
void ctrl2_sendlow_tspd(int tidx, train_ctrl_t *tvar);
void ctrl2_sendlow_c1c2(int tidx, train_ctrl_t *tvar);
void ctrl2_evt_pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t ca_addr, uint8_t trigbits, int16_t cposd10);
void ctrl2_evt_stop_detected(int tidx, train_ctrl_t *tvar, int32_t pose);

void ctrl_set_pose_trig(int numtrain, int32_t pose, int n);

#define ignore_bemf_presence 0
#define ignore_ina_presence  1


void ctrl_reset_timer(int tidx, train_ctrl_t *tvar, int numtimer);
void ctrl_set_timer(int tidx, train_ctrl_t *tvar, int numtimer, uint32_t tval);


// behaviour_flags
#define BEHAVE_STOPPED        (1<<1)        // 2
#define BEHAVE_EOT1            (1<<2)        // 4
#define BEHAVE_EOT2            (1<<3)        // 8
#define BEHAVE_BLKW            (1<<4)        // 16
#define BEHAVE_PTRIG            (1<<5)        // 32
#define BEHAVE_RESTARTBLK     (1<<6)        // 64
#define BEHAVE_TBEHAVE        (1<<7)        // 128
#define BEHAVE_CHBKLK        (1<<8)        // 256

#ifdef OLD_CTRL
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
    msg_64_t m = {0};
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

#endif

#endif /* ctrlP_h */
