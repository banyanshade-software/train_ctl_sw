//
//  ctrlLP.c
//  train_throttle
//
//  Created by Daniel Braun on 19/12/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#include <stdio.h>

#include <stdint.h>


#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

//#include "../railconfig.h"
#include "../config/conf_train.h"

#include "ctrl.h"
#include "ctrlLP.h"
#include "longtrain.h"
//#include "ctrlP.h"
//#include "cautoP.h"


#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif

static lsblk_num_t snone = {-1};

// -----------------------------------------------------------------

uint8_t ctrl_flag_notify_speed = 1;

// -----------------------------------------------------------------

static int _train_eot(int tidx, train_ctrl_t *tvars, int sdir);
static int _train_blkwait(int tidx, train_ctrl_t *tvars, int sdir);

// -----------------------------------------------------------------

static void _set_dir(int tidx, train_ctrl_t *tvars, int sdir);
static void _update_spd_limit(int tidx, train_ctrl_t *tvars, int sdir);
static void _set_speed(int tidx, train_ctrl_t *tvars, int signed_speed, int apply);
static void _set_state(int tidx, train_ctrl_t *tvars, train_state_t newstate);
static void _apply_speed(int tidx, train_ctrl_t *tvars);
// -----------------------------------------------------------------



void ctrl3_init_train(int tidx, train_ctrl_t *tvars, lsblk_num_t sblk)
{
    itm_debug1(DBG_CTRL, "INIT", tidx);
    memset(tvars, 0, sizeof(*tvars));
    tvars->_desired_speed = 0;
    tvars->_mode = train_manual;
    tvars->_sdir = 0;
    tvars->_spd_limit = 100;
    tvars->_state = train_state_station;
    tvars->_target_speed = 0;
    
    tvars->beginposmm = 0;
    tvars->_curposmm = POSE_UNKNOWN;
    tvars->c1_sblk = sblk;
    //TODO
}


void ctrl3_upcmd_set_desired_speed(int tidx, train_ctrl_t *tvars, int16_t desired_speed)
{
    if (!desired_speed) FatalError("DSpd", "FSM desspd",  Error_FSM_DSpd);
    int sdir = SIGNOF0(desired_speed);
    switch (tvars->_state) {
        case train_station:
station:
            if (_train_eot(tidx, tvars, sdir)) {
                return;
            }
            if (_train_blkwait(tidx, tvars, sdir)) {
                _set_dir(tidx, tvars, sdir);
                _set_state(tidx, tvars, train_state_blkwait);
                return;
            }
            // otherwise, start train
            _set_dir(tidx, tvars, sdir);
            _update_spd_limit(tidx, tvars, sdir);
            _set_speed(tidx, tvars, desired_speed, 1);
            return;
            break;
            
        case train_state_running:
            if (sdir == tvars->_sdir) {
                _update_spd_limit(tidx, tvars, sdir);
                _set_speed(tidx, tvars, desired_speed, 1);
                return;
            } else {
                // change direction
                ctrl3_upcmd_set_desired_speed_zero(tidx, tvars);
                return;
            }
            break;
            
        case train_state_end_of_track:
        case train_state_end_of_track0:
            if (sdir == tvars->_sdir) {
                // same dir, ignore
                return;
            }
            // handle as in station
            goto station;
            break;
            
        default:
            break;
    }
    FatalError("FSM_", "end fsm", Error_FSM_Nothandled);
}

void ctrl3_upcmd_set_desired_speed_zero(int tidx, train_ctrl_t *tvars)
{
    switch (tvars->_state) {
        case train_state_station:
            return;
            break;
            
        case train_state_running:
            _set_speed(tidx, tvars, 0, 1);
            return;
            
        case train_state_blkwait:
        case train_state_end_of_track:
            _set_dir(tidx, tvars, 0);
            _set_state(tidx, tvars, train_state_station);
            return;
            break;
            
        case train_state_blkwait0:
        case train_state_end_of_track0:
            _set_speed(tidx, tvars, 0, 0);
            return;
            break;
        default:
            break;
    }
    FatalError("FSM_", "end fsm", Error_FSM_Nothandled);
}


void ctrl3_stop_detected(int tidx, train_ctrl_t *tvars)
{
    switch (tvars->_state) {
        case train_state_station:
        case train_state_blkwait:
        case train_state_end_of_track:
            //ignore
            return;
            break;
        
        case train_state_running:
            if (tvars->_target_speed == 0) {
                _set_dir(tidx, tvars, 0);
                _set_state(tidx, tvars, train_state_station);
            }
            return;
            break;
            
        case train_state_blkwait0:
            if (tvars->_desired_speed) {
                _set_state(tidx, tvars, train_state_blkwait);
            } else {
                _set_dir(tidx, tvars, 0);
                _set_state(tidx, tvars, train_state_station);
            }
            return;
            break;
            
        case train_state_end_of_track0:
            if (tvars->_desired_speed) {
                _set_state(tidx, tvars, train_state_end_of_track);
            } else {
                _set_dir(tidx, tvars, 0);
                _set_state(tidx, tvars, train_state_station);
            }
            return;
            break;
        default:
            break;
    }
    FatalError("FSM_", "end fsm", Error_FSM_Nothandled);

}


void ctrl3_pose_triggered(int tidx, train_ctrl_t *tvars, pose_trig_tag_t trigtag)
{
    switch (tvars->_state) {
        case train_state_blkwait0:
        case train_state_blkwait:
        case train_state_end_of_track0:
        case train_state_end_of_track:
        case train_state_station:
            //ignore
            return;
            break;
            
        case train_state_running:
            switch (trigtag) {
                case tag_end_lsblk:
                    // TODO
                    // update position, update trigs
                    return;
                    break;
                case tag_stop_blk_wait:
                    _set_speed(tidx, tvars, 0, 1);
                    _set_state(tidx, tvars, train_state_blkwait0);
                    return;
                    break;
                case tag_stop_eot:
                    _set_speed(tidx, tvars, 0, 1);
                    _set_state(tidx, tvars, train_state_end_of_track0);
                    return;
                    break;
                default:
                    break;
            }
        default:
            break;
    }
    FatalError("FSM_", "end fsm", Error_FSM_Nothandled);
}

// -----------------------------------------------------------------

static int _train_eot(int tidx, train_ctrl_t *tvars, int sdir)
{
    // TODO
    return 0;
}
static int _train_blkwait(int tidx, train_ctrl_t *tvars, int sdir)
{
    // TODO
    return 0;
}

// -----------------------------------------------------------------

static void _set_dir(int tidx, train_ctrl_t *tvars, int sdir)
{
    if (tvars->_sdir == sdir) return;
    tvars->_sdir = sdir;
}
static void _update_spd_limit(int tidx, train_ctrl_t *tvars, int sdir)
{
    // TODO
    tvars->_spd_limit = 70;
}
static void _set_speed(int tidx, train_ctrl_t *tvars, int signed_speed, int applyspd)
{
    if (tvars->_sdir != SIGNOF0(signed_speed)) {
        FatalError("DIRs", "bad spd sign", Error_FSM_SignDir);
    }
    tvars->_desired_speed = signed_speed;
    if (!applyspd) return;
    _apply_speed(tidx, tvars);
}
static void _apply_speed(int tidx, train_ctrl_t *tvars)
{
    int spd = tvars->_desired_speed;
    if (abs(tvars->_desired_speed) > tvars->_spd_limit) spd = SIGNOF0(spd)*tvars->_spd_limit;
    tvars->_target_speed = spd;
    // TODO : send to spdctl
    
}
static void _set_state(int tidx, train_ctrl_t *tvars, train_state_t newstate)
{
    if (tvars->_state == newstate) return;
    tvars->_state = newstate;
    //  sanity check
    switch (newstate) {
        case train_state_blkwait:
        case train_state_blkwait0:
        case train_state_end_of_track0:
        case train_state_end_of_track:
            if (!tvars->_sdir) {
                FatalError("FSMb", "FSM san check", Error_FSM_Sanity1);
            }
            if (!tvars->_desired_speed) {
                FatalError("FSMb", "FSM san check", Error_FSM_Sanity2);
            }
        case train_state_station:
            if (tvars->_sdir) {
                FatalError("FSMb", "FSM san check", Error_FSM_Sanity3);
            }
            if (tvars->_desired_speed) {
                FatalError("FSMb", "FSM san check", Error_FSM_Sanity4);
            }
            break;

        default:
            break;
    }
}
