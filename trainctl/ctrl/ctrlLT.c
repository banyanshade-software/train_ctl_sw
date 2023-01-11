//
//  ctrlLT.c
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
#include "ctrlLT.h"
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

static int _train_check_dir(int tidx, train_ctrl_t *tvars, int sdir, rettrigs_t *rett);
static void _apply_trigs(int tidx, train_ctrl_t *tvars, const rettrigs_t *rett);

// -----------------------------------------------------------------

static void _set_dir(int tidx, train_ctrl_t *tvars, int sdir);
static void _update_spd_limit(int tidx, train_ctrl_t *tvars, int sdir);
static void _set_speed(int tidx, train_ctrl_t *tvars, int signed_speed, int apply);
static void _set_state(int tidx, train_ctrl_t *tvars, train_state_t newstate);
static void _apply_speed(int tidx, train_ctrl_t *tvars);
static void _check_c2(int tidx, train_ctrl_t *tvars, rettrigs_t *rett);
static void _updated_while_running(int tidx, train_ctrl_t *tvars);

// -----------------------------------------------------------------


static uint32_t pose_convert_from_mm(const conf_train_t *tconf, int32_t mm);
static uint32_t pose_convert_to_mm( const conf_train_t *tconf, int32_t poseval);

// -----------------------------------------------------------------


static int  _lock_train_occupency(int tidx, train_ctrl_t *tvars);
static void _release_all_blocks(int tidx, train_ctrl_t *tvars);

// -----------------------------------------------------------------


void ctrl3_init_train(int tidx, train_ctrl_t *tvars, lsblk_num_t sblk, int on)
{
    itm_debug1(DBG_CTRL, "INIT", tidx);
    memset(tvars, 0, sizeof(*tvars));
    tvars->_desired_signed_speed = 0;
    tvars->_mode = train_manual;
    tvars->_sdir = 0;
    tvars->_spd_limit = 100;
    tvars->_state = train_state_off;
    tvars->_target_unisgned_speed = 0;
    
    tvars->beginposmm = 0;
    tvars->_curposmm = POSE_UNKNOWN;
    tvars->c1_sblk = sblk;
    //TODO
    tvars->c1c2dir_changed = 1;
    tvars->can2_xaddr.v = 0xFF;
    tvars->can1_xaddr = canton_for_lsblk(sblk);
    
    if (on) turn_train_on(tidx, tvars);

}

void turn_train_off(int tidx, train_ctrl_t *tvars)
{
    switch (tvars->_state) {
        case train_state_off:
            return;
        case train_state_blkwait:
        case train_state_end_of_track:
            _set_dir(tidx, tvars, 0);
            // FALLTHRU
        case train_state_station:
            _release_all_blocks(tidx, tvars);
            _set_state(tidx, tvars, train_state_off);
            break;
        default:
            _set_speed(tidx, tvars, 0, 1);
            tvars->off_requested = 1;
            break;
    }
}
void turn_train_on(int tidx, train_ctrl_t *tvars)
{
    tvars->off_requested = 0;
    if (tvars->_state != train_state_off) {
        itm_debug2(DBG_ERR|DBG_CTRL, "not off", tidx, tvars->_state);
        return;
    }
    const conf_train_t *conf = conf_train_get(tidx);
    ctrl3_get_next_sblks(tidx, tvars, conf);
    int rc = _lock_train_occupency(tidx, tvars);
    if (!rc) {
        _set_state(tidx, tvars, train_state_station);
        return;
    }
    itm_debug1(DBG_ERR|DBG_CTRL, "cant reserve", tidx);
}




void ctrl3_upcmd_set_desired_speed(int tidx, train_ctrl_t *tvars, int16_t desired_speed)
{
    //int is_eot = 0;
    //int is_occ = 0;
    int rc = 0;
    if (!desired_speed) FatalError("DSpd", "FSM desspd",  Error_FSM_DSpd);
    int sdir = SIGNOF0(desired_speed);
    rettrigs_t rett = {0};
    switch (tvars->_state) {
        case train_state_off:
            return;
        case train_station:
station:
            rc = _train_check_dir(tidx, tvars, sdir, &rett);
            if (rett.isoet) {
                return;
            }
            if (rett.isocc) {
                _set_dir(tidx, tvars, sdir);
                _set_speed(tidx, tvars, desired_speed, 0);
                _set_state(tidx, tvars, train_state_blkwait);
                return;
            }
            if (rc<0) FatalError("FSMd", "setdir", Error_FSM_ChkNeg1);
            // otherwise, start train
            _check_c2(tidx, tvars, &rett);
            _set_dir(tidx, tvars, sdir);
            _update_spd_limit(tidx, tvars, sdir);
            _set_speed(tidx, tvars, desired_speed, 1);
            _apply_trigs(tidx, tvars, &rett);
            _set_state(tidx, tvars, train_state_running);
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
        case train_state_off:
            return;
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
        case train_state_off:
            return;
        case train_state_station:
        case train_state_blkwait:
        case train_state_end_of_track:
            //ignore
            return;
            break;
        
        case train_state_running:
            if (tvars->_target_unisgned_speed == 0) {
                _set_dir(tidx, tvars, 0);
                _set_state(tidx, tvars, train_state_station);
            }
            return;
            break;
            
        case train_state_blkwait0:
            if (tvars->_desired_signed_speed) {
                _set_state(tidx, tvars, train_state_blkwait);
            } else {
                _set_dir(tidx, tvars, 0);
                _set_state(tidx, tvars, train_state_station);
            }
            return;
            break;
            
        case train_state_end_of_track0:
            if (tvars->_desired_signed_speed) {
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


void ctrl3_pose_triggered(int tidx, train_ctrl_t *tvars, pose_trig_tag_t trigtag, xblkaddr_t ca_addr, int16_t cposd10)
{
    itm_debug3(DBG_CTRL|DBG_POSEC, "POSEtrg", tidx, ca_addr.v, cposd10);

    if (ca_addr.v != tvars->can1_xaddr.v) {
        itm_debug3(DBG_ERR|DBG_POSEC|DBG_CTRL, "ptrg bad", tidx, ca_addr.v, tvars->can1_xaddr.v);
        return;
    }
    
    const conf_train_t *tconf = conf_train_get(tidx);
    tvars->_curposmm = pose_convert_to_mm(tconf, cposd10*10);
    itm_debug3(DBG_POSE|DBG_CTRL, "curposmm", tidx, tvars->_curposmm, trigtag);

    
    switch (tvars->_state) {
        case train_state_off:
            return;
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
                    itm_debug2(DBG_CTRL, "end lsb", tidx, tvars->c1_sblk.n);
                    lsblk_num_t ns = next_lsblk(tvars->c1_sblk, (tvars->_sdir<0) ? 1 : 0, NULL);
                    if (ns.n == -1) {
                        itm_debug2(DBG_ERR|DBG_CTRL, "end/nxt", tidx, tvars->c1_sblk.n);
                        return;
                        break;
                    }
                    xblkaddr_t b = canton_for_lsblk(ns);
                    if (b.v != tvars->can1_xaddr.v) {
                        itm_debug2(DBG_ERR|DBG_CTRL, "end/badc1", tidx, tvars->c1_sblk.n);
                        break;
                    }
                    tvars->c1_sblk = ns;
                    tvars->beginposmm = tvars->_curposmm; // TODO adjust for trig delay
                    ctrl3_update_front_sblks_c1changed(tidx, tvars, conf_train_get(tidx), tvars->_sdir<0 ? 1 : 0);
                    _updated_while_running(tidx, tvars);
                    return;
                    break;
                case tag_chkocc:
                    // TODO
                    // update position, update trigs
                    ctrl3_update_front_sblks(tidx, tvars, conf_train_get(tidx), tvars->_sdir<0 ? 1 : 0);
                    _updated_while_running(tidx, tvars);
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
               
                    
                case tag_brake:
                    itm_debug2(DBG_ERR|DBG_CTRL, "trg brk", tidx, tvars->c1_sblk.n);
                    break;
                case tag_need_c2:
                    itm_debug2(DBG_ERR|DBG_CTRL, "trg nc2", tidx, tvars->c1_sblk.n);
                    break;
                case tag_reserve_c2:
                    itm_debug2(DBG_ERR|DBG_CTRL, "trg rc2", tidx, tvars->c1_sblk.n);
                    break;
                default:
                    itm_debug2(DBG_ERR|DBG_CTRL, "unh trg", tidx, tvars->c1_sblk.n);
                    break;
            }
        default:
            break;
    }
    FatalError("FSM_", "end fsm", Error_FSM_Nothandled);
}



void ctrl3_occupency_updated(int tidx, train_ctrl_t *tvars)
{
    int rc = 0;
    rettrigs_t rett = {0};
    switch (tvars->_state) {
        case train_state_off:
            return;
        case train_state_station:
        case train_state_end_of_track0:
        case train_state_end_of_track:
            //ignore
            return;
            break;
        
        case train_state_running:
            rc = _train_check_dir(tidx, tvars, tvars->_sdir, &rett);
            if (rett.isocc) {
                int spd = tvars->_desired_signed_speed;
                _set_speed(tidx, tvars, 0, 1);
                _set_speed(tidx, tvars, spd, 0);
                _set_state(tidx, tvars, train_state_blkwait);
                return;
            }
            if (rett.isoet) {
                FatalError("FSMe", "run to eot", Error_FSM_RunToEot);
                _set_speed(tidx, tvars, 0, 1);
                _set_state(tidx, tvars, train_state_end_of_track);
                return;
            }
            // otherwise ignore, update trigs (not needed)
            //_apply_trigs(tidx, tvars, &rett);
            return;
            break;
            
        case train_state_blkwait:
        case train_state_blkwait0:
            rc = _train_check_dir(tidx, tvars, tvars->_sdir, &rett);
            if (rett.isocc) {
                // still in blkwait, ignore
                return;
            }
            if (rett.isoet) {
                FatalError("FSMe", "blk to eot", Error_FSM_BlkToEot);
                if (tvars->_state == train_state_blkwait0) {
                    _set_state(tidx, tvars, train_state_end_of_track0);
                } else {
                    _set_state(tidx, tvars, train_state_end_of_track);
                }
                return;
            }
            if (rc<0) FatalError("FSMd", "setdir", Error_FSM_ChkNeg2);
            // train exit blkwait condition and can start
            _update_spd_limit(tidx, tvars, tvars->_sdir);
            _set_speed(tidx, tvars, tvars->_desired_signed_speed, 1);
            _apply_trigs(tidx, tvars, &rett);
            _set_state(tidx, tvars, train_state_running);
            return;
            break;
            
        default:
            break;
    }
    FatalError("FSMo", "end fsm", Error_FSM_Nothandled);
}


void ctrl3_evt_entered_c2(int tidx, train_ctrl_t *tvars, uint8_t from_bemf)
{
    if (from_bemf && ignore_bemf_pres()) return;
    itm_debug3(DBG_CTRL, "evt_ent_c2", tidx, tvars->can1_xaddr.v,  tvars->can2_xaddr.v);

    if (tvars->_state != train_state_running) {
        itm_debug3(DBG_CTRL|DBG_ERR, "ent C2/badst", tidx, tvars->_state, from_bemf);
        return;
    }
    if (tvars->c1c2) {
        itm_debug3(DBG_CTRL|DBG_ERR, "ent C2/c1c2", tidx, tvars->_state, from_bemf);
    }
    if (tvars->can2_xaddr.v == 0xFF) {
        itm_debug3(DBG_CTRL|DBG_ERR, "ent C2/noc2", tidx, tvars->_state, from_bemf);
        return;
    }
    tvars->c1c2 = 1;
    
    // swap can1 and can2
    xblkaddr_t t = tvars->can1_xaddr;
    tvars->can1_xaddr = tvars->can2_xaddr;
    tvars->can2_xaddr = t;
    
    if (1 == tvars->can1_xaddr.v) { // XXX Hardcoded for now
        tvars->measure_pose_percm = 1;
    }
    tvars->c1_sblk = first_lsblk_with_canton(tvars->can1_xaddr, tvars->c1_sblk);
    ctrl3_update_front_sblks_c1changed(tidx, tvars, conf_train_get(tidx), tvars->_sdir<0 ? 1 : 0);
    
    _updated_while_running(tidx,tvars);
    
    if (from_bemf && ignore_ina_pres()) {
        //ctrl_set_timer(tidx, tvars, TLEAVE_C1, TLEAVE_C1_VALUE);
    } else {
        //ctrl_set_timer(tidx, tvar, TLEAVE_C1, TGUARD_C1_VALUE);
    }
}

static void _updated_while_running(int tidx, train_ctrl_t *tvars)
{
    rettrigs_t rett = {0};
    int rc = _train_check_dir(tidx, tvars, tvars->_sdir, &rett);
    if (rett.isoet) {
        int rc = _train_check_dir(tidx, tvars, tvars->_sdir, &rett);
        _set_speed(tidx, tvars, 0, 1);
        _set_state(tidx, tvars, train_state_end_of_track);
        return;
    }
    if (rett.isocc) {
        int spd = tvars->_desired_signed_speed;
        _set_speed(tidx, tvars, 0, 1);
        _set_speed(tidx, tvars, spd, 0);
        _set_state(tidx, tvars, train_state_blkwait);
        return;
    }
    if (rc<0) FatalError("FSMd", "setdir", Error_FSM_ChkNeg1);
    _check_c2(tidx, tvars, &rett);
    _set_dir(tidx, tvars, tvars->_sdir);
    _update_spd_limit(tidx, tvars, tvars->_sdir);
    _set_speed(tidx, tvars, tvars->_desired_signed_speed, 1);
    _apply_trigs(tidx, tvars, &rett);
    _set_state(tidx, tvars, train_state_running);
    return;
}

void ctrl3_evt_leaved_c1(int tidx, train_ctrl_t *tvars)
{
    itm_debug3(DBG_CTRL|DBG_POSE, "evt_left_c1", tidx, tvars->_state, tvars->can1_xaddr.v);
    if (tvars->_state != train_running_c1) {
        itm_debug2(DBG_CTRL|DBG_ERR, "leav_c1/bs", tidx, tvars->_state);
        return;
    }
    if (!tvars->c1c2) {
        itm_debug2(DBG_CTRL|DBG_ERR, "leav_c1/nc1c2", tidx, tvars->_state);
        return;
    }
    tvars->c1c2 = 0;
#if 0
    ctrl_reset_timer(tidx, tvars, TLEAVE_C1);
    free_block_c1(tidx, tvars);
    //set_block_addr_occupency(tvars->can1_addr, BLK_OCC_FREE, 0xFF, snone);
    if (1 == tvars->can2_xaddr.v) { // XXX Hardcoded for now
        tvars->measure_pose_percm = 1;
    }
    itm_debug2(DBG_CTRL, "** C1", tidx, tvars->can1_xaddr.v);
    tvars->can1_xaddr = tvars->can2_xaddr;
    tvars->c1_sblk = first_lsblk_with_canton(tvars->can2_xaddr, tvars->c1_sblk);
    if (tvars->c1_sblk.n == -1) {
        FatalError("C1no", "No C1 in leaved C1", Error_CtrlNoC1l);
    }

    int len = get_lsblk_len_cm_steep(tvars->c1_sblk, conf_train_get(tidx), tvars);
    if (tvars->_dir<0) {
        tvars->beginposmm =  -len*10;
    } else {
        tvars->beginposmm = 0;
    }
    itm_debug2(DBG_CTRL|DBG_POSE, "beginpos", tidx, tvars->beginposmm);

    tvars->can2_xaddr.v = 0xFF; // will be updated by update_c2
    tvars->tick_flags |=  _TFLAG_C1_CHANGED|_TFLAG_C1_CHANGED | _TFLAG_C1LSB_CHANGED;
#endif
}
// -----------------------------------------------------------------

static int _train_check_dir(int tidx, train_ctrl_t *tvars, int sdir, rettrigs_t *rett)
{
    if (!sdir) {
        FatalError("FSMd", "FSM no dir", Error_FSM_NoDir);
        return 0;
    }
    const conf_train_t *conf = conf_train_get(tidx);
    ctrl3_get_next_sblks(tidx, tvars, conf);
    int rc2 = ctrl3_check_front_sblks(tidx, tvars, conf_train_get(tidx), (sdir<0) ? 1 : 0, rett);
    
    if (rc2 < 0) {
        if (!rett->isocc && !rett->isoet) {
            FatalError("FSMb", "rcneg unk", Error_FSM_ShouldEotOcc);
        }
    }
    itm_debug3(DBG_CTRL, "rc2", tidx, rc2, tvars->_state);
    if (rc2>0) {
        // set brake
        tvars->stopposmm = ctrl3_getcurpossmm(tvars, conf_train_get(tidx), (sdir<0)) + rc2*10*sdir;
        tvars->brake = 1;
    } else {
        tvars->brake = 0;
    }
    
    return rc2;

}


static void _set_one_trig(int numtrain, const conf_train_t *tconf, int8_t dir,  xblkaddr_t canaddr, int32_t pose, uint8_t tag)
{
    itm_debug3(DBG_CTRL, "set posetr", numtrain, tag, pose);
    if (!tag) {
        itm_debug2(DBG_ERR|DBG_POSEC, "no tag", numtrain, tag);
        FatalError("NOTG", "no tag", Error_CtrlBadPose);
    }
    if (!dir) {
        itm_debug2(DBG_ERR|DBG_POSEC, "no dir", numtrain, tag);
        FatalError("NODI", "no dir", Error_CtrlBadPose);
    }
    if (abs(pose)>32000*10) {
        itm_debug3(DBG_ERR|DBG_POSEC, "toobig", numtrain, tag, pose);
    }
    msg_64_t m = {0};
    m.from = MA1_CTRL(numtrain);
    //m.to =  MA_TRAIN_SC(numtrain);
    TO_CANTON(m, canaddr);
    m.cmd = CMD_POSE_SET_TRIG;
    //const conf_train_t *tconf = conf_train_get(numtrain);
    if (tconf->reversed)  m.va16 = -pose/10;
    else m.va16 = pose/10;
    m.vcu8 = tag;
    m.vb8 = dir;
    itm_debug3(DBG_CTRL|DBG_POSEC, "S_TRIG", numtrain, tag, dir);
    mqf_write_from_ctrl(&m);
}



static void _apply_trigs(int tidx, train_ctrl_t *tvars, const rettrigs_t *rett)
{
    const conf_train_t *conf = conf_train_get(tidx);
    for (int i=0; i<NUMTRIGS;i++) {
        if (!rett->trigs[i].tag) continue;
        _set_one_trig(tidx, conf,  tvars->_sdir, tvars->can1_xaddr ,
                      pose_convert_from_mm(conf, rett->trigs[i].poscm*10),
                      rett->trigs[i].tag);
    }
}

// -----------------------------------------------------------------
static void _sendlow_c1c2_dir(int tidx, train_ctrl_t *tvars);

static void _set_dir(int tidx, train_ctrl_t *tvars, int sdir)
{
    if (tvars->_sdir == sdir) return;
    tvars->_sdir = sdir;
    tvars->c1c2dir_changed = 1;
}
static void _update_spd_limit(int tidx, train_ctrl_t *tvars, int sdir)
{
    // TODO
    tvars->_spd_limit = 99;
}

static void _set_speed(int tidx, train_ctrl_t *tvars, int signed_speed, int applyspd)
{
    if (signed_speed && (tvars->_sdir != SIGNOF0(signed_speed))) {
        FatalError("DIRs", "bad spd sign", Error_FSM_SignDir);
    }
    tvars->_desired_signed_speed = signed_speed;
    if (!applyspd) return;
    _apply_speed(tidx, tvars);
}

static uint8_t brake_maxspd(int distmm);

static void _apply_speed(int tidx, train_ctrl_t *tvars)
{
    int spd = abs(tvars->_desired_signed_speed);
    // apply spd limit
    if (spd > tvars->_spd_limit) spd = SIGNOF0(spd)*tvars->_spd_limit;

    // apply brake
    int32_t pos = ctrl3_getcurpossmm(tvars, conf_train_get(tidx), (tvars->_sdir<0));
    if (tvars->brake) {
        int dist = abs(pos - tvars->stopposmm);
        int maxspd = brake_maxspd(dist);
        if (spd>maxspd) spd = maxspd;
    }
    
    if (tvars->_target_unisgned_speed == spd) {
        // dont send if same value
        return;
    }
    tvars->_target_unisgned_speed = spd;

    //  send to spdctl
    if (tvars->c1c2dir_changed) {
        _sendlow_c1c2_dir(tidx, tvars);
    }
    msg_64_t m = {0};
    m.from = MA1_CTRL(tidx);
    m.to =   MA1_SPDCTL(tidx);
    m.cmd = CMD_SET_TARGET_SPEED;
    // direction already given by SET_C1_C2
    //m.v1 = trctl[trnum]._dir*trctl[trnum]._target_speed;
    m.v1u = tvars->_target_unisgned_speed;
    m.v2 = 0;
    mqf_write_from_ctrl(&m);
    
    if (ctrl_flag_notify_speed) {
        msg_64_t m = {0};
        m.from = MA1_CTRL(tidx);
        m.to = MA3_UI_GEN; //(UISUB_TFT);
        m.cmd = CMD_TRTSPD_NOTIF;
        m.v1u = tvars->_target_unisgned_speed;
        m.v2 = tvars->_sdir;
        mqf_write_from_ctrl(&m);
    }
}


static void _sendlow_c1c2_dir(int tidx, train_ctrl_t *tvars)
{
    if (!tvars->c1c2dir_changed) {
        FatalError("FSMc", "c1c2dirchanged 0", Error_FSM_C1C2Zero);
        return;
    }
    tvars->c1c2dir_changed = 0;
    msg_64_t m = {0};
    m.from = MA1_CTRL(tidx);
    m.to =  MA1_SPDCTL(tidx);
    m.cmd = CMD_SET_C1_C2;
    int dir = tvars->_sdir;
    
    const conf_train_t *tconf = conf_train_get(tidx);
    if (tconf->reversed) dir = -dir;
    
    m.vbytes[0] = tvars->can1_xaddr.v;
    m.vbytes[1] = dir;
    m.vbytes[2] = tvars->can2_xaddr.v;
    m.vbytes[3] = dir; // TODO dir might be reversed
    mqf_write_from_ctrl(&m);
}

static void _set_state(int tidx, train_ctrl_t *tvars, train_state_t newstate)
{
    if (tvars->_state == newstate) return;
    train_state_t oldstate = tvars->_state;
    tvars->_state = newstate;
    //  sanity check
    switch (newstate) {
        case train_state_blkwait:
        case train_state_blkwait0:
            if (!tvars->_desired_signed_speed) {
                FatalError("FSMb", "FSM san check", Error_FSM_Sanity2);
            }
            // FALLTHRU
        case train_state_end_of_track0:
        case train_state_end_of_track:
            if (!tvars->_sdir) {
                FatalError("FSMb", "FSM san check", Error_FSM_Sanity1);
            }
            break;
        case train_state_station:
            if (tvars->_sdir) {
                FatalError("FSMb", "FSM san check", Error_FSM_Sanity3);
            }
            if (tvars->_desired_signed_speed) {
                FatalError("FSMb", "FSM san check", Error_FSM_Sanity4);
            }
            break;

        default:
            break;
    }
    // sanity check on c1
    if ((1)) {
        if (tvars->c1_sblk.n == -1) {
            FatalError("FSMb", "FSM san", Error_FSM_Sanity5);
        } else {
            xblkaddr_t b = canton_for_lsblk(tvars->c1_sblk);
            if (b.v == 0xFF) {
                FatalError("FSMb", "FSM san", Error_FSM_Sanity6);
            } else if (b.v != tvars->can1_xaddr.v) {
                FatalError("FSMb", "FSM san", Error_FSM_Sanity7);
            }
        }
        
        // sanity check on c1c2
        if (tvars->c1c2) {
            if (tvars->can2_xaddr.v == 0xFF) {
                FatalError("FSMb", "FSM san", Error_FSM_Sanity8);
            }
        }
        if (tvars->can2_xaddr.v != 0xFF) {
            if (tvars->can2_xaddr.v == tvars->can1_xaddr.v) {
                FatalError("FSMb", "FSM san", Error_FSM_Sanity9);
            }
        }
    }
    // notify UI
    msg_64_t m = {0};
    m.from = MA1_CTRL(tidx);
    m.to = MA3_UI_GEN;//(UISUB_TFT);
    m.cmd = CMD_TRSTATE_NOTIF;
    m.v1u = newstate;
    m.v2u = oldstate;
    mqf_write_from_ctrl(&m);
}

static void _set_and_power_c2(int tidx, train_ctrl_t *tvars)
{
    if (tvars->can2_xaddr.v != 0xFF) {
        if (tvars->can2_xaddr.v != tvars->can2_future.v) {
            FatalError("FUT2", "bad future c2", Error_FSM_BadFut2);
        }
        return;
    }
    tvars->can2_xaddr = tvars->can2_future;
}

static void _reserve_c2(int tidx, train_ctrl_t *tvars)
{
    lsblk_num_t ns = {-1};
    set_block_addr_occupency(tvars->can2_future, BLK_OCC_C2, tidx, ns);
}

static void _check_c2(int tidx, train_ctrl_t *tvars, rettrigs_t *rett)
{
    if (rett->res_c2) {
        _reserve_c2(tidx, tvars);
    }
    if (rett->power_c2) {
        _set_and_power_c2(tidx, tvars);
    }
}



static uint8_t brake_maxspd(int distmm)
{
    // TODO better brake
    return (distmm>100) ? 100:distmm;
}

static uint32_t pose_convert_from_mm(const conf_train_t *tconf, int32_t mm);
static uint32_t pose_convert_to_mm( const conf_train_t *tconf, int32_t poseval);



static uint32_t pose_convert_to_mm( const conf_train_t *tconf, int32_t poseval)
{
    int32_t mm = poseval*10/tconf->pose_per_cm;
    return mm;
}

static uint32_t pose_convert_from_mm(const conf_train_t *tconf, int32_t mm)
{
    int32_t pv = mm * tconf->pose_per_cm / 10;
    return pv;
}

// ---------------------------------------------

static int _car_occupied(int tidx, train_ctrl_t *tvars,  struct forwdsblk *fwdcars)
{
    for (int i=0; i<fwdcars->numlsblk; i++) {
        lsblk_num_t lsb = fwdcars->r[i];
        if (lsb.n == -1) continue;
        xblkaddr_t blk = canton_for_lsblk(lsb);
        int rc = occupency_set_occupied_car(blk, tidx, lsb);
        if (rc) return rc;
    }
    return 0;
}
static int _lock_train_occupency(int tidx, train_ctrl_t *tvars)
{
    int rc;
    rc = occupency_set_occupied(tvars->can1_xaddr, tidx, tvars->c1_sblk);
    if (rc) return rc;
    rc = _car_occupied(tidx, tvars, &tvars->rightcars);
    if (rc) return rc;
    rc = _car_occupied(tidx, tvars, &tvars->leftcars);
    if (rc) return rc;

    return 0;
}
static void _release_all_blocks(int tidx, train_ctrl_t *tvars)
{
    FatalError("NI", "not implemented", Error_FSM_NotImplemented);
}

