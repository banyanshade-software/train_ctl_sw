//
//  ctrlP.c
//  train_throttle
//
//  Created by Daniel BRAUN on 28/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <stdint.h>


#include "misc.h"
#include "../msg/trainmsg.h"

#include "topology.h"
#include "occupency.h"

#include "ctrl.h"
#include "ctrlP.h"
#include "railconfig.h"

static lsblk_num_t snone = {-1};

#define EOT_SPD_LIMIT    70   //20


// for test/debug
//static uint8_t ignore_bemf_presence = 0;
//static uint8_t ignore_ina_presence = 1;

static void fatal(void)
{
    itm_debug1(DBG_ERR, "fatal", 0);
#ifdef TRAIN_SIMU
    abort();
#else
    for (;;) osDelay(1000);
#endif
}



static int32_t ctrl_pose_middle(lsblk_num_t lsb, const train_config_t *tconf, int dir)
{
    int cm = get_lsblk_len(lsb);
    uint32_t p = cm * tconf->pose_per_cm;
    uint32_t pm = p/2;
    if (dir<0) pm = -pm;
    return pm;
}


static int32_t ctrl_pose_end(lsblk_num_t lsb, const train_config_t *tconf, int dir)
{
    int cm = get_lsblk_len(lsb);
    uint32_t p = cm * tconf->pose_per_cm;
    if (dir<0) p = -p;
    return p;
}

void ctrl_set_pose_trig(int numtrain, int32_t pose, int n)
{
    itm_debug2(DBG_CTRL, "set posetr", numtrain, pose);
    msg_64_t m = {0};
    m.from = MA_CONTROL_T(numtrain);
    m.to =  MA_TRAIN_SC(numtrain);
    m.cmd = n ? CMD_POSE_SET_TRIG2 :  CMD_POSE_SET_TRIG1;
    const train_config_t *tconf = get_train_cnf(numtrain);
    if (tconf->reversed)  m.v32 = -pose;
    else m.v32 = pose;
    mqf_write_from_ctrl(&m);
}


void ctrl_reset_timer(int tidx, train_ctrl_t *tvar, int numtimer)
{
    itm_debug2(DBG_CTRL, "reset_timer", tidx, numtimer);
    if (numtimer<0 || numtimer>=NUM_TIMERS) fatal();
    tvar->timertick[numtimer] = 0;
}

void ctrl_set_timer(int tidx, train_ctrl_t *tvar, int numtimer, uint32_t tval)
{
    itm_debug3(DBG_CTRL, "set_timer", tidx, numtimer, tval);
    if (numtimer<0 || numtimer>=NUM_TIMERS) fatal();
    tvar->timertick[numtimer] = HAL_GetTick() + tval;
}



// -----

#ifdef OLD_CTRL
void ctrl_changed_lsblk(int tidx, train_ctrl_t *tvars, lsblk_num_t newsblk)
{
    if (newsblk.n == tvars->c1_sblk.n) {
        itm_debug2(DBG_ERR|DBG_CTRL, "sam lsbl", tidx, newsblk.n);
        return;
    }
    uint8_t ca = canton_for_lsblk(newsblk);
    if (ca == 0xFF) {
        itm_debug2(DBG_ERR|DBG_CTRL, "inval lsb", tidx, newsblk.n);
        return;
    }
    if (ca == tvars->can2_addr) {
        // TODO : switch canton
    }
    tvars->c1_sblk = newsblk;
    // TODO update POSE
    // TODO update occupency
}

void ctrl_evt_entered_c2(int tidx, train_ctrl_t *tvar, uint8_t from_bemf)
{
    if (from_bemf && ignore_bemf_presence) return;
    switch (tvar->_state) {
    case train_running_c1:
        if (from_bemf && ignore_ina_presence) {
            ctrl_set_timer(tidx, tvar, TLEAVE_C1, TLEAVE_C1_VALUE);
        } else {
            ctrl_set_timer(tidx, tvar, TLEAVE_C1, TGUARD_C1_VALUE);
        }
        set_block_addr_occupency(tvar->can2_addr, occupied(tvar->_dir), tidx,
                                 first_lsblk_with_canton(tvar->can2_addr, tvar->c1_sblk));
        ctrl_set_state(tidx, tvar, train_running_c1c2);
        break;
    case train_running_c1c2:
        break;
    default:
        itm_debug2(DBG_ERR|DBG_CTRL, "bad st/1",tidx, tvar->_state);
        break;
    }
}



void ctrl_evt_leaved_c1(int tidx, train_ctrl_t *tvars)
{
    itm_debug3(DBG_CTRL, "evt_left_c1", tidx, tvars->_state, tvars->can1_addr);
    switch (tvars->_state) {
    case train_running_c1c2:
        ctrl_reset_timer(tidx, tvars, TLEAVE_C1);
    set_block_addr_occupency(tvars->can1_addr, BLK_OCC_FREE, 0xFF, snone);
        tvars->can1_addr = tvars->can2_addr;
        tvars->c1_sblk = first_lsblk_with_canton(tvars->can2_addr, tvars->c1_sblk);
        tvars->can2_addr = 0xFF; // will be updated by update_c2
        ctrl_set_state(tidx, tvars, train_running_c1);
        ctrl_update_c2_state_limits(tidx, tvars, get_train_cnf(tidx), upd_c1c2);
        tvars->behaviour_flags |= BEHAVE_CHBKLK;
        break;
    default:
        itm_debug2(DBG_ERR|DBG_CTRL, "bad st/2",tidx, tvars->_state);
        break;
    }
}



void evt_entered_c1(int tidx, train_ctrl_t *tvars, _UNUSED_ uint8_t from_bemf)
{
    itm_debug2(DBG_CTRL, "enter C1", tidx, tvars->_state);
}
void evt_leaved_c2(int tidx, train_ctrl_t *tvar)
{
    itm_debug2(DBG_CTRL, "leave C2", tidx, tvar->_state);
}





void ctrl_update_c2_state_limits(int tidx, train_ctrl_t *tvars, const train_config_t *tconf, ctrl_update_reason_t updreason)
{
    itm_debug3(DBG_CTRL, "UPDC2", tidx, tvars->can1_addr, updreason);
    uint8_t c2 = 0xFF;
    uint16_t olim = tvars->spd_limit;
    uint32_t posetval = 0;
    if (updreason == upd_pose_trig) tvars->behaviour_flags |= BEHAVE_PTRIG;

    // -----------------
    // check stop state
    switch (tvars->_state) {
    case train_off:
    case train_station:
        tvars->_dir = 0;
        tvars->_target_speed = 0;
        if (tvars->can2_addr != 0xFF) set_block_addr_occupency(tvars->can2_addr, BLK_OCC_FREE, 0xFF, snone);
        tvars->can2_addr = 0xFF;
        goto sendlow;
    default:
        break;
    }
    if (tvars->can1_addr == 0xFF) {
        itm_debug1(DBG_ERR|DBG_CTRL, "*** NO C1", tidx);
        return;
    }
    if (!tvars->_dir) {
        ctrl_set_state(tidx, tvars, train_station);
        tvars->behaviour_flags |= BEHAVE_STOPPED;
        tvars->_target_speed = 0;
        if (tvars->can2_addr != 0xFF) set_block_addr_occupency(tvars->can2_addr, BLK_OCC_FREE, 0xFF, snone);
        tvars->can2_addr = 0xFF;
        goto sendlow;
    }
    
    
    //-----------
    // update c2
    // check eot, set pose
    itm_debug3(DBG_CTRL, "prev c1c2", tidx, tvars->can1_addr, tvars->can2_addr);
    uint8_t alternate = 0;
    lsblk_num_t ns = next_lsblk(tvars->c1_sblk, (tvars->_dir < 0), &alternate);
    c2 = canton_for_lsblk(ns);
    itm_debug3(DBG_CTRL, "c1c2addr", tidx, tvars->can2_addr, c2);

    if (ns.n < 0) {
        // end of track
        switch (updreason) {
            case upd_c1c2:
                itm_debug1(DBG_CTRL, "eot", tidx);
                tvars->spd_limit = EOT_SPD_LIMIT;
                //xxxx
                posetval = ctrl_pose_middle(tvars->c1_sblk, tconf, tvars->_dir);
                tvars->behaviour_flags |= BEHAVE_EOT1;
                break;
            case upd_change_dir:
                //FALLTHRU
            case upd_pose_trig:
                itm_debug1(DBG_CTRL, "eot2", tidx);
                ctrl_set_state(tidx, tvars, alternate ? train_blk_wait : train_end_of_track);
                tvars->spd_limit = 0;
                tvars->behaviour_flags |= BEHAVE_EOT2;
                break;
            default:
                break;
        }
    } else {
        switch (get_block_addr_occupency(c2)) {
            case BLK_OCC_FREE:
                itm_debug2(DBG_CTRL, "free", tidx, c2);
                tvars->spd_limit = 100; //set_speed_limit(tidx, 100);
                switch (tvars->_state) {
                case train_running_c1:
                    break;
                case train_blk_wait:
                    ctrl_set_state(tidx, tvars, train_running_c1);
                    tvars->behaviour_flags |= BEHAVE_RESTARTBLK;
                    break;
                default:
                    itm_debug2(DBG_ERR|DBG_CTRL, "bad st/4", tidx, tvars->_state);
                    break;
                }
                break;
            default:
            case BLK_OCC_RIGHT:
            case BLK_OCC_LEFT:
            case BLK_OCC_STOP:
                itm_debug3(DBG_CTRL, "occ", tidx, c2, get_block_addr_occupency(c2));
                ctrl_set_state(tidx, tvars, train_blk_wait);
                tvars->behaviour_flags |= BEHAVE_BLKW;
                c2 = 0xFF;
                tvars->spd_limit = 0;
                break;
            case BLK_OCC_C2: {
                if (c2 == tvars->can2_addr) {
                    // normal case, same C2
                    break;
                } else if (tvars->can2_addr != 0xFF) {
                    // change C2. Can this occur ? if turnout is changed
                    // but turnout should not be changed if C2 already set
                    itm_debug3(DBG_ERR|DBG_CTRL, "C2 change", tidx, tvars->can2_addr, c2);
                    set_block_addr_occupency(tvars->can2_addr, BLK_OCC_FREE, 0xFF, snone);
                    tvars->can2_addr = 0xFF;
                }
                // occupied
                itm_debug2(DBG_CTRL, "OCC C2", tidx, c2);
                ctrl_set_state(tidx, tvars, train_blk_wait);
                c2 = 0xFF;
                tvars->spd_limit = 0;
                break;
            }
        }
    }
    if (c2 != 0xFF) {
        // sanity check, can be removed (TODO)
        if ((get_block_addr_occupency(c2) != BLK_OCC_FREE)
                && (get_block_addr_occupency(c2) != BLK_OCC_C2)) fatal();
        set_block_addr_occupency(c2, BLK_OCC_C2, tidx, snone);
    }
    //-----------
    // send to spd ctrl
    // 
sendlow:
    if ((c2 != tvars->can2_addr) || (updreason == upd_c1c2) || (updreason == upd_change_dir) ||(updreason==upd_init)) {
        itm_debug3(DBG_CTRL, "C1C2", tidx, tvars->can1_addr, tvars->can2_addr);
        tvars->can2_addr = c2;

        int dir = tvars->_dir;
        const train_config_t *tconf = get_train_cnf(tidx);
        if (tconf->reversed) dir = -dir;

        msg_64_t m = {0};
        m.from = MA_CONTROL_T(tidx);
        m.to =  MA_TRAIN_SC(tidx);
        m.cmd = CMD_SET_C1_C2;
        m.vbytes[0] = tvars->can1_addr;
        m.vbytes[1] = dir;
        m.vbytes[2] = tvars->can2_addr;
        m.vbytes[3] = dir; // 0;
        mqf_write_from_ctrl(&m);
    }
    if ((tvars->_mode != train_fullmanual) && (olim != tvars->spd_limit)) {
        itm_debug2(DBG_CTRL, "lim upd", tidx, tvars->spd_limit);
        uint16_t tspd = MIN(tvars->spd_limit, tvars->desired_speed);
        switch (updreason) {
        case upd_change_dir: // do nothing, ctrl_set_tspeed will be updated
            break;
        case upd_init:
            break;
        default:
            ctrl_set_tspeed(tidx, tvars, tspd);
            break;
        }
    }
    if (posetval) {
        //itm_debug2(DBG_CTRL, "set pose", tidx, posetval);
        // POSE trigger must be sent *after* CMD_SET_C1_C2
        ctrl_set_pose_trig(tidx, posetval, 1);
    }

}



void ctrl_set_pose_trig(int numtrain, int32_t pose, int n)
{
    itm_debug2(DBG_CTRL, "set posetr", numtrain, pose);
    msg_64_t m = {0};
    m.from = MA_CONTROL_T(numtrain);
    m.to =  MA_TRAIN_SC(numtrain);
    m.cmd = n ? CMD_POSE_SET_TRIG2 :  CMD_POSE_SET_TRIG1;
    const train_config_t *tconf = get_train_cnf(numtrain);
    if (tconf->reversed)  m.v32 = -pose;
    else m.v32 = pose;
    mqf_write_from_ctrl(&m);
}




void ctrl_set_tspeed(int trnum, train_ctrl_t *tvars, uint16_t tspd)
{
    if (tvars->_target_speed == tspd) return;
    tvars->_target_speed = tspd;

    // notif UI
    itm_debug2(DBG_UI|DBG_CTRL, "ctrl_set_tspeed", trnum, tspd);
    msg_64_t m = {0};
    m.from = MA_CONTROL_T(trnum);
    m.to = MA_UI(UISUB_TFT);
    m.cmd = CMD_TRTSPD_NOTIF;
    m.v1u = tspd;
    m.v2 = tvars->_dir;
    mqf_write_from_ctrl(&m);

    m.to = MA_TRAIN_SC(trnum);
    m.cmd = CMD_SET_TARGET_SPEED;
    // direction already given by SET_C1_C2
    //m.v1 = trctl[trnum]._dir*trctl[trnum]._target_speed;
    m.v1u = tvars->_target_speed;
    m.v2 = 0;
    mqf_write_from_ctrl(&m);
}


void ctrl_set_dir(int trnum,  train_ctrl_t *tvars, int  dir, int force)
{
    if (!force && (tvars->_dir == dir)) return;

    itm_debug2(DBG_CTRL, "setdir", trnum, dir);

    tvars->_dir = dir;

    msg_64_t m = {0};
    m.from = MA_CONTROL_T(trnum);

    // notif UI
    m.to = MA_UI(UISUB_TFT);
    m.cmd = CMD_TRDIR_NOTIF;
    m.v1 = dir;
    m.v2 = 0;
    mqf_write_from_ctrl(&m);
}
#endif


// ----------------------


static void set_speed_limit(train_ctrl_t *tvar, uint16_t lim)
{
    if (lim == tvar->spd_limit) return;
    tvar->spd_limit = lim;
    tvar->tick_flags |= _TFLAG_LIMIT_CHANGED;
}


void ctrl2_init_train(int tidx, train_ctrl_t *tvars,
                      lsblk_num_t sblk)
{
    tvars->c1_sblk = sblk;
    tvars->_dir = 0;
    tvars->_target_speed = 0;
    tvars->_state = train_station;
    tvars->can1_addr = canton_for_lsblk(sblk);
    tvars->can2_addr = 0xFF;
    tvars->desired_speed = 0;
    tvars->spd_limit = 100;
    tvars->c1c2 = 0;
    tvars->pose2_set = 0;
    tvars->behaviour_flags = 0;
    tvars->tick_flags |= _TFLAG_C1_CHANGED |
        _TFLAG_C1LSB_CHANGED | _TFLAG_DIR_CHANGED |
        _TFLAG_DSPD_CHANGED | _TFLAG_TSPD_CHANGED |
        _TFLAG_STATE_CHANGED | _TFLAG_LIMIT_CHANGED;
}

void ctrl2_upcmd_set_desired_speed(int tidx, train_ctrl_t *tvars, int16_t desired_speed)
{
    if (tvars->desired_speed != desired_speed) {
        tvars->tick_flags |= _TFLAG_DSPD_CHANGED;
#if 0
        int s0 = SIGNOF0(desired_speed);
        if ((s0 && tvars->_dir) && (s0 != tvars->_dir)) {
            // first stop
            tvars->desired_speed = 0;
            tvars->desired_speed2 = desired_speed;
        }
#endif
        tvars->desired_speed = desired_speed;
        //tvars->desired_speed2 = 0;
    }
}

void ctrl2_set_state(int tidx, train_ctrl_t *tvar, train_state_t ns)
{
    if (ns == tvar->_state) {
        return;
    }
    tvar->_state = ns;
    tvar->tick_flags |= _TFLAG_STATE_CHANGED;
}

void ctrl2_stop_detected(int tidx, train_ctrl_t *tvars)
{
    switch (tvars->_state) {
        case train_running_c1:
            ctrl2_set_state(tidx, tvars, train_station);
            break;
        default:
            break;
    }
}


void ctrl2_set_dir(int tidx, train_ctrl_t *tvar, int8_t dir)
{
    if (tvar->_dir != dir) {
        tvar->tick_flags |= _TFLAG_DIR_CHANGED;
        tvar->_dir = dir;
    }
}

void ctrl2_set_tspeed(int tidx, train_ctrl_t *tvar, uint16_t tspeed)
{
    if (tvar->_target_speed != tspeed) {
        tvar->tick_flags |= _TFLAG_TSPD_CHANGED;
        tvar->_target_speed = tspeed;
    }
}

void ctrl2_check_alreadystopped(int tidx, train_ctrl_t *tvar)
{
    if (tvar->desired_speed) return;
    if (tvar->_state == train_running_c1) {
        if ((tvar->_dir == 0) | !tvar->_target_speed) {
            ctrl2_set_state(tidx, tvar, train_station);
        }
    }
}
void ctrl2_check_checkstart(int tidx, train_ctrl_t *tvar)
{
    if (!tvar->desired_speed) return;
    switch (tvar->_state) {
        case train_station:
            ctrl2_set_dir(tidx, tvar, SIGNOF0(tvar->desired_speed));
            ctrl2_set_state(tidx, tvar, train_running_c1);
            ctrl2_set_tspeed(tidx, tvar, SIGNOF0(tvar->desired_speed));
            break;
            
        default:
            break;
    }
}

void ctrl2_check_stop(int tidx, train_ctrl_t *tvar)
{
    switch (tvar->_state) {
        case train_off:
        case train_station:
            ctrl2_set_dir(tidx, tvar, 0);
            //FALLTHRU
        case train_blk_wait:
        case train_end_of_track:
            ctrl2_set_tspeed(tidx, tvar, 0);
            ctrl2_set_tspeed(tidx, tvar, 0);
            if (tvar->can2_addr != 0xFF) set_block_addr_occupency(tvar->can2_addr, BLK_OCC_FREE, 0xFF, snone);
            tvar->can2_addr = 0xFF;
            break;
            
            
        default:
            break;
    }
}

void ctrl2_apply_speed_limit(int tidx, train_ctrl_t *tvar)
{
    int16_t v;
     switch (tvar->_state) {
         default:
             ctrl2_set_tspeed(tidx, tvar, 0);
             break;
         //case train_running_c1c2:
         case train_running_c1:
             if (SIGNOF0(tvar->desired_speed) != tvar->_dir) {
                 ctrl2_set_tspeed(tidx, tvar, 0);
                 break;
             }
             v = abs(tvar->desired_speed);
             v = MIN(tvar->spd_limit, v);
             ctrl2_set_tspeed(tidx, tvar, v);
             break;
     }
}
static void ctrl2_had_trig2(int tidx, train_ctrl_t *tvar)
{
    switch (tvar->_state) {
        //case train_running_c1c2:
        case train_running_c1:
            ctrl2_set_state(tidx, tvar, train_end_of_track);
            break;
            
        default:
            break;
    }
}

void ctrl2_update_topo(int tidx, train_ctrl_t *tvar, const train_config_t *tconf, int32_t *ppose1)
{
    switch (tvar->_state) {
        case train_off:
        case train_station:
            return;
        default:
            break;
    }
    uint8_t alternate = 0;
    lsblk_num_t ns = next_lsblk(tvar->c1_sblk, (tvar->_dir < 0), &alternate);
    if (ns.n < 0) {
        switch (tvar->_state) {
            //case train_running_c1c2:
            case train_running_c1:
                if (tvar->pose2_set) {
                    ctrl2_set_state(tidx, tvar, alternate ? train_blk_wait : train_end_of_track);
                } else {
                    tvar->pose2_set = 1;
                    set_speed_limit(tvar, EOT_SPD_LIMIT);
                    //xxxx
                    int32_t posetval = ctrl_pose_middle(tvar->c1_sblk, tconf, tvar->_dir);
                    //ctrl_set_pose_trig(tidx, posetval, 1);
                    *ppose1 = posetval;
                }
                break;
                
            default:
                break;
        }
        return;
    }
    switch (tvar->_state) {
        case train_blk_wait:
            tvar->tick_flags |= _TFLAG_LIMIT_CHANGED;
            ctrl2_set_state(tidx, tvar, train_running_c1);
            break;
        default:
            break;
    }
}
    

void ctrl2_update_c2(int tidx, train_ctrl_t *tvar, const train_config_t *tconf, int32_t *ppose0)
{
    if (tvar->can1_addr == 0xFF) fatal();
    if (canton_for_lsblk(tvar->c1_sblk) != tvar->can1_addr) fatal();
    //uint8_t old_c2 = tvar->can2_addr;
    tvar->can2_addr = 0xFF;

    uint8_t alternate = 0;
    lsblk_num_t ns = next_lsblk(tvar->c1_sblk, (tvar->_dir < 0), &alternate);
    uint8_t c2 = canton_for_lsblk(ns);
    itm_debug3(DBG_CTRL, "c1c2addr", tidx, tvar->can2_addr, c2);
    if (c2 == tvar->can1_addr) {
        //set trig
        int32_t posetval = ctrl_pose_end(tvar->c1_sblk, tconf, tvar->_dir);
        //ctrl_set_pose_trig(tidx, posetval, 0);
        *ppose0 = posetval;
    } else {
        tvar->can2_addr = c2;
    }
    tvar->tick_flags |= _TFLAG_C1C2_CHANGED;
}

void ctrl2_notify_state(int tidx, train_ctrl_t *tvar)
{
    msg_64_t m = {0};
    m.from = MA_CONTROL_T(tidx);
    m.to = MA_UI(UISUB_TFT);
    m.cmd = CMD_TRSTATE_NOTIF;
    m.v1u = tvar->_state;
    mqf_write_from_ctrl(&m);
}

void ctrl2_sendlow_tspd(int tidx, train_ctrl_t *tvar)
{
    msg_64_t m = {0};
    m.from = MA_CONTROL_T(tidx);
    m.to =  MA_TRAIN_SC(tidx);
    m.to = MA_TRAIN_SC(tidx);
    m.cmd = CMD_SET_TARGET_SPEED;
    // direction already given by SET_C1_C2
    //m.v1 = trctl[trnum]._dir*trctl[trnum]._target_speed;
    m.v1u = tvar->_target_speed;
    m.v2 = 0;
    mqf_write_from_ctrl(&m);
}

void ctrl2_sendlow_c1c2(int tidx, train_ctrl_t *tvar)
{
    msg_64_t m = {0};
    m.from = MA_CONTROL_T(tidx);
    m.to =  MA_TRAIN_SC(tidx);
    m.cmd = CMD_SET_C1_C2;
    int dir = tvar->_dir;
    m.vbytes[0] = tvar->can1_addr;
    m.vbytes[1] = dir;
    m.vbytes[2] = tvar->can2_addr;
    m.vbytes[3] = dir; // 0;
    mqf_write_from_ctrl(&m);
}

void ctrl2_evt_entered_c2(int tidx, train_ctrl_t *tvar, uint8_t from_bemf)
{
    if (from_bemf && ignore_bemf_presence) return;
    if (tvar->_state != train_running_c1) {
        itm_debug3(DBG_CTRL|DBG_ERR, "ent C2/badst", tidx, tvar->_state, from_bemf);
        return;
    }
    if (tvar->c1c2) {
        itm_debug3(DBG_CTRL|DBG_ERR, "ent C2/c1c2", tidx, tvar->_state, from_bemf);
    }
    tvar->c1c2 = 1;
}


void ctrl2_evt_leaved_c1(int tidx, train_ctrl_t *tvars)
{
    itm_debug3(DBG_CTRL, "evt_left_c1", tidx, tvars->_state, tvars->can1_addr);
    if (tvars->_state != train_running_c1) {
        itm_debug2(DBG_CTRL|DBG_ERR, "leav_c2/bs", tidx, tvars->_state);
        return;
    }
    if (!tvars->c1c2) {
        itm_debug2(DBG_CTRL|DBG_ERR, "leav_c2/nc1c2", tidx, tvars->_state);
        return;
    }
    
    ctrl_reset_timer(tidx, tvars, TLEAVE_C1);
    tvars->c1c2 = 0;
    set_block_addr_occupency(tvars->can1_addr, BLK_OCC_FREE, 0xFF, snone);
    tvars->can1_addr = tvars->can2_addr;
    tvars->c1_sblk = first_lsblk_with_canton(tvars->can2_addr, tvars->c1_sblk);
    tvars->can2_addr = 0xFF; // will be updated by update_c2
    tvars->tick_flags |= _TFLAG_C1_CHANGED | _TFLAG_C1LSB_CHANGED;
}



void ctrl2_evt_entered_c1(int tidx, train_ctrl_t *tvars, _UNUSED_ uint8_t from_bemf)
{
    itm_debug2(DBG_CTRL, "enter C1", tidx, tvars->_state);
}
void ctrl2_evt_leaved_c2(int tidx, train_ctrl_t *tvar)
{
    itm_debug2(DBG_CTRL, "leave C2", tidx, tvar->_state);
}

void ctrl2_evt_pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t ca_addr, uint8_t trignum)
{
    if (tvar->_state != train_running_c1) {
        itm_debug2(DBG_ERR|DBG_CTRL, "bad st/3",tidx, tvar->_state);
        if (1==trignum) tvar->pose2_set = 0;
        return;
    }
    if (ca_addr != tvar->can1_addr) {
        itm_debug3(DBG_ERR|DBG_POSE|DBG_CTRL, "ptrg bad", tidx, ca_addr, tvar->can1_addr);
        return;
    }
    switch (trignum) {
        case 0: {
            //tvar->tick_flags |= _TFLAG_POSE_TRIG1;
            lsblk_num_t ns = next_lsblk(tvar->c1_sblk, (tvar->_dir<0), NULL);
            if (ns.n<0) {
                itm_debug3(DBG_CTRL|DBG_ERR, "no next!", tidx, tvar->c1_sblk.n, tvar->_dir);
                break;
            }
            if (canton_for_lsblk(ns) != tvar->can1_addr) {
                itm_debug3(DBG_ERR|DBG_CTRL, "bad sblk", tidx, ns.n, tvar->can1_addr);
                break;
            }
            tvar->c1_sblk = ns;
            tvar->tick_flags |= _TFLAG_C1LSB_CHANGED;
            }
            break;
        case 1:
            tvar->pose2_set = 0;
            tvar->tick_flags |= _TFLAG_POSE_TRIG2;
            break;
        default:
            break;
    }
}
void ctrl2_evt_stop_detected(int tidx, train_ctrl_t *tvar, int32_t pose)
{
    // TODO
    tvar->tick_flags |= _TFLAG_STOP_DETECTED;
}

static const uint16_t perm_flags = (_TFLAG_STATE_CHANGED|_TFLAG_DIR_CHANGED|_TFLAG_TSPD_CHANGED|_TFLAG_C1C2_CHANGED);

int ctrl2_tick_process(int tidx, train_ctrl_t *tvars, const train_config_t *tconf)
{
    int nloop = 0;
    if (occupency_changed) tvars->tick_flags |= _TFLAG_OCC_CHANGED;
    if (0==tvars->tick_flags) return 0;
    
    uint16_t pflags = 0;
    int32_t pose0 = 0;
    int32_t pose1 = 0;
    while (tvars->tick_flags) {
        nloop++;
        uint16_t flags = tvars->tick_flags;
        pflags |= (flags & perm_flags);
        tvars->tick_flags = 0;

        if (flags & _TFLAG_DSPD_CHANGED) {
            ctrl2_check_alreadystopped(tidx, tvars);
            ctrl2_check_checkstart(tidx, tvars);
        }
        if (flags & _TFLAG_STOP_DETECTED) {
            ctrl2_stop_detected(tidx, tvars);
        }
        if (flags & _TFLAG_STATE_CHANGED) {
            ctrl2_check_stop(tidx, tvars);
        }
        if (flags & _TFLAG_POSE_TRIG2) {
            ctrl2_had_trig2(tidx, tvars);
        }
        if (flags & _TFLAG_C1_CHANGED) {
            if (! (flags & _TFLAG_C1LSB_CHANGED)) fatal();
        }
        if (flags & (_TFLAG_C1LSB_CHANGED|_TFLAG_DIR_CHANGED)) {
            ctrl2_update_c2(tidx, tvars, tconf, &pose0);
        }
        if (flags & (_TFLAG_C1LSB_CHANGED|_TFLAG_OCC_CHANGED)) {
            ctrl2_update_topo(tidx, tvars, tconf, &pose1);
        }
            
        if (flags & (_TFLAG_LIMIT_CHANGED|/*_TFLAG_TSPD_CHANGED|*/_TFLAG_DSPD_CHANGED)) {
            ctrl2_apply_speed_limit(tidx, tvars);
        }
        
    }
    if (pflags & (_TFLAG_C1C2_CHANGED|_TFLAG_DIR_CHANGED)) {
        ctrl2_sendlow_c1c2(tidx, tvars);
    }
    if (pflags & _TFLAG_STATE_CHANGED) {
        ctrl2_notify_state(tidx, tvars);
    }
    if (pose0) {
        ctrl_set_pose_trig(tidx, pose0, 0);
    }
    if (pose1) {
        ctrl_set_pose_trig(tidx, pose1, 1);
    }
    if (pflags & (_TFLAG_TSPD_CHANGED)) {
        ctrl2_sendlow_tspd(tidx, tvars);
    }
    return nloop;
}
