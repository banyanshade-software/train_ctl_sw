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

#include "../topology/topology.h"
#include "../topology/occupency.h"

#include "misc.h"
#include "railconfig.h"

#include "ctrl.h"
#include "ctrlP.h"
#include "cautoP.h"

static lsblk_num_t snone = {-1};

#define SPD_LIMIT_EOT    70   //20
#define SPD_LIMIT_NOLIMIT 100

// for test/debug
//static uint8_t ignore_bemf_presence = 0;
//static uint8_t ignore_ina_presence = 1;

uint8_t ctrl_flag_notify_speed = 1;

static void fatal(void)
{
    itm_debug1(DBG_ERR, "fatal", 0);
#ifdef TRAIN_SIMU
    abort();
#else
    for (;;) osDelay(1000);
#endif
}

// ------------------------------------------------------

static uint32_t pose_convert_to_mm( const train_config_t *tconf, int32_t poseval)
{
    int32_t mm = poseval*10/tconf->pose_per_cm;
    return mm;
}

static uint32_t pose_convert_from_mm(const train_config_t *tconf, int32_t mm)
{
    int32_t pv = mm * tconf->pose_per_cm / 10;
    return pv;
}


static int32_t ctrl_pose_middle_c1(const train_config_t *tconf, train_ctrl_t *tvar)
{
    int cm = get_lsblk_len(tvar->c1_sblk);
    int mm;
    mm = tvar->beginposmm + cm*5;
    int32_t p = pose_convert_from_mm(tconf, mm);
    if (!p) p=1;
    return p;
}


static int32_t ctrl_pose_end_c1(const train_config_t *tconf, train_ctrl_t *tvar)
{
    int cm = get_lsblk_len(tvar->c1_sblk);
    int mm;
    if (tvar->_dir<0) {
        mm = tvar->beginposmm;
    } else {
        mm = tvar->beginposmm+cm*10;
    }
    int32_t p = pose_convert_from_mm(tconf, mm);
    if (!p) p=1;
    return p;
}

void ctrl_set_pose_trig(int numtrain, int32_t pose, int n)
{
    itm_debug2(DBG_CTRL, "set posetr", numtrain, pose);
    msg_64_t m = {0};
    m.from = MA_CONTROL_T(numtrain);
    m.to =  MA_TRAIN_SC(numtrain);
    m.cmd = n ? CMD_POSE_SET_TRIG_U1 :  CMD_POSE_SET_TRIG0; // XXXX
    const train_config_t *tconf = get_train_cnf(numtrain);
    if (tconf->reversed)  m.v32 = -pose;
    else m.v32 = pose;
    mqf_write_from_ctrl(&m);
}

void ctrl2_upcmd_settrigU1_half(int tidx, train_ctrl_t *tvars)
{
    if (!tvars->_dir) return;
    int32_t p = ctrl_pose_middle_c1(get_train_cnf(tidx), tvars);
    ctrl_set_pose_trig(tidx, p, 1);
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


void ctrl2_set_mode(int tidx, train_ctrl_t *tvar, train_mode_t mode)
{
    itm_debug2(DBG_CTRL, "set mode", tidx, mode);
    if (tvar->_mode == mode) return;
    tvar->_mode = mode;
    tvar->tick_flags |= _TFLAG_MODE_CHANGED;
    
    // notif UI
    msg_64_t m;
    m.from = MA_CONTROL_T(tidx);
    m.to = MA_UI(UISUB_TFT);
    m.cmd = CMD_TRMODE_NOTIF;
    m.v1u = mode;
    mqf_write_from_ctrl(&m);
}




// ----------------------


static lsblk_num_t next_lsblk_free(int tidx, train_ctrl_t *tvars,  uint8_t left, uint8_t *palternate)
{
    lsblk_num_t nsa = {-1};
    if (tvars->_mode == train_auto) {
        nsa = cauto_peek_next_lsblk(tidx, tvars);
    }
    lsblk_num_t ns = next_lsblk(tvars->c1_sblk, left, palternate);
    if (ns.n >= 0) {
        if ((nsa.n >= 0) && (nsa.n != ns.n)) {
            ns.n = -1;
            if (palternate) *palternate = 1;
        } else {
            //  check occupency
            uint8_t ca = canton_for_lsblk(ns);
            int f = occupency_block_is_free(ca, tidx);
            if (!f) {
                // block not free
                ns.n = -1;
                if (palternate) *palternate = 1;
            }
        }
    }
    return ns;
}


static void set_speed_limit(train_ctrl_t *tvar, uint16_t lim)
{
    if (lim == tvar->spd_limit) return;
    tvar->spd_limit = lim;
    tvar->tick_flags |= _TFLAG_LIMIT_CHANGED;
}


void ctrl2_init_train(_UNUSED_ int tidx, train_ctrl_t *tvars,
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
    //tvars->behaviour_flags = 0;
    tvars->tick_flags |=
        _TFLAG_C1LSB_CHANGED | _TFLAG_DIR_CHANGED |
        _TFLAG_DSPD_CHANGED | _TFLAG_TSPD_CHANGED |
        _TFLAG_STATE_CHANGED | _TFLAG_LIMIT_CHANGED;
    tvars->route = NULL;
    tvars->routeidx = 0;
}

void ctrl2_upcmd_set_desired_speed(_UNUSED_ int tidx, train_ctrl_t *tvars, int16_t desired_speed)
{
    if (tvars->_mode == train_auto) {
        // switch back to manual ?
        ctrl2_set_mode(tidx, tvars, train_manual);
        //return;
    }
    _ctrl2_upcmd_set_desired_speed(tidx, tvars, desired_speed);
}

void _ctrl2_upcmd_set_desired_speed(_UNUSED_ int tidx, train_ctrl_t *tvars, int16_t desired_speed)
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

void ctrl2_set_state(_UNUSED_ int tidx, train_ctrl_t *tvar, train_state_t ns)
{
    if (ns == tvar->_state) {
        return;
    }
    tvar->_state = ns;
    tvar->tick_flags |= _TFLAG_STATE_CHANGED;
}

void ctrl2_set_dir(_UNUSED_ int tidx, train_ctrl_t *tvar, int8_t dir)
{
    if (tvar->_dir != dir) {
        tvar->tick_flags |= _TFLAG_DIR_CHANGED;
        tvar->_dir = dir;
    }
}

void ctrl2_stop_detected(int tidx, train_ctrl_t *tvars)
{
	itm_debug1(DBG_CTRL, "c2 stp", tidx);
    switch (tvars->_state) {
        //case train_end_of_track:
        case train_running_c1:
            ctrl2_set_state(tidx, tvars, train_station);
            break;
        case train_end_of_track:
        case train_blk_wait:
            if ((tvars->_dir) && (SIGNOF(tvars->_dir) != SIGNOF(tvars->desired_speed))) {
                ctrl2_set_dir(tidx, tvars, SIGNOF(tvars->desired_speed));
                ctrl2_set_state(tidx, tvars, train_station);
            }
            break;
        default:
            break;
    }
    if (tvars->_mode == train_auto) cauto_had_stop(tidx, tvars);
    ctrl2_set_dir(tidx, tvars, 0);
}



void ctrl2_set_tspeed(_UNUSED_ int tidx, train_ctrl_t *tvar, uint16_t tspeed)
{
    if (tvar->_target_speed != tspeed) {
        tvar->tick_flags |= _TFLAG_TSPD_CHANGED;
        if (tspeed >= 0x7FFF) {
            fatal();
        }
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
void ctrl2_check_checkstart(int tidx, train_ctrl_t *tvars)
{
    lsblk_num_t ns;
    
    if (!tvars->desired_speed) {
        if (tvars->_mode== train_auto) {
            cauto_check_start(tidx, tvars);
        }
    }
    
    if (!tvars->desired_speed) return;
    
    switch (tvars->_state) {
        case train_station:
            ctrl2_set_dir(tidx, tvars, SIGNOF0(tvars->desired_speed));
            ctrl2_set_state(tidx, tvars, train_running_c1);
            //ctrl2_set_tspeed(tidx, tvars, SIGNOF0(tvars->desired_speed));
            break;
        
        case train_end_of_track:
        case train_blk_wait:
            if (tvars->_target_speed) break; // already started
            if (tvars->_dir) break;
            uint8_t alternate;
            ns = next_lsblk_free(tidx, tvars,  (tvars->desired_speed<0),  &alternate);
            if (ns.n>=0) {
                ctrl2_set_state(tidx, tvars, train_running_c1);
                ctrl2_set_dir(tidx, tvars, SIGNOF0(tvars->desired_speed));
                //ctrl2_set_tspeed(tidx, tvar, SIGNOF0(tvar->desired_speed));
            } else {
                //tvar->desired_speed = 0;
                if (alternate) {
                    ctrl2_set_state(tidx, tvars, train_blk_wait);
                    tvars->_dir = SIGNOF0(tvars->desired_speed);
                } else {
                    ctrl2_set_state(tidx, tvars,  train_end_of_track);
                }
            }
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
	itm_debug1(DBG_CTRL, "had trig2", tidx);
    switch (tvar->_state) {
        //case train_running_c1c2:
        case train_running_c1:
            ctrl2_set_state(tidx, tvar, tvar->pose2_is_blk_wait ? train_blk_wait : train_end_of_track);
            break;
            
        default:
            break;
    }
}


void ctrl2_update_topo(int tidx, train_ctrl_t *tvar, const train_config_t *tconf, int32_t *ppose1)
{
	itm_debug1(DBG_CTRL, "upd topo", tidx);
    switch (tvar->_state) {
        case train_off:
        case train_station:
            return;
        default:
            break;
    }
    
    int d = (tvar->_dir) ? tvar->_dir : SIGNOF0(tvar->desired_speed);
    if (!d) return;
    
    uint8_t alternate = 0;
    lsblk_num_t ns = next_lsblk_free(tidx, tvar, (d < 0), &alternate);
    
    if (ns.n < 0) {
        switch (tvar->_state) {
            //case train_running_c1c2:
            case train_running_c1:
                if (tvar->pose2_set) {
                    //ctrl2_set_state(tidx, tvar, alternate ? train_blk_wait : train_end_of_track);
                } else {
                    if (!tvar->_dir) fatal();
                    tvar->pose2_set = 1;
                    tvar->pose2_is_blk_wait = alternate ? 1 : 0;
                    set_speed_limit(tvar, SPD_LIMIT_EOT);
                    //xxxx
                    int32_t posetval = ctrl_pose_middle_c1(tconf, tvar);
                    //ctrl_set_pose_trig(tidx, posetval, 1);
                    *ppose1 = posetval;
                }
                break;
                
            default:
                break;
        }
        return;
    }
    tvar->pose2_set = 0;
    
    switch (tvar->_state) {
        case train_blk_wait:
            tvar->tick_flags |= _TFLAG_LIMIT_CHANGED ; // _TFLAG_DIR_CHANGED will trigger update_c2
            ctrl2_set_state(tidx, tvar, train_running_c1);
            ctrl2_set_dir(tidx, tvar, d);
            set_speed_limit(tvar, SPD_LIMIT_NOLIMIT);
            break;
        case train_running_c1:
            set_speed_limit(tvar, SPD_LIMIT_NOLIMIT);
            break;
        default:
            break;
    }
    if ((ns.n >= 0) && (tvar->can2_addr != canton_for_lsblk(ns))) {
        tvar->tick_flags |= _TFLAG_NEED_C2 ; // _TFLAG_C1LSB_CHANGED will trigger update_c2
    }
}
    

void ctrl2_update_c2(int tidx, train_ctrl_t *tvar, const train_config_t *tconf, int32_t *ppose0)
{
	itm_debug3(DBG_CTRL, "updc2", tidx, tvar->c1_sblk.n, tvar->can1_addr);
    if (tvar->can1_addr == 0xFF) fatal();
    if (canton_for_lsblk(tvar->c1_sblk) != tvar->can1_addr) fatal();
    
    set_block_addr_occupency(tvar->can1_addr, occupied(tvar->_dir), tidx, tvar->c1_sblk);
    
    uint8_t old_c2 = tvar->can2_addr;
    tvar->can2_addr = 0xFF;
    lsblk_num_t ns = snone;
    if (tvar->_dir) {
        uint8_t alternate = 0;
        ns = next_lsblk_free(tidx, tvar, tvar->_dir<0, &alternate);
        uint8_t c2 = canton_for_lsblk(ns);
        itm_debug3(DBG_CTRL, "c1c2addr", tidx, tvar->can2_addr, c2);
        if (c2 == tvar->can1_addr) {
            //set trig
            int32_t posetval = ctrl_pose_end_c1(tconf, tvar);
            //ctrl_set_pose_trig(tidx, posetval, 0);
            *ppose0 = posetval;
        } else {
            tvar->can2_addr = c2;
        }
    } else {
        tvar->can2_addr = 0xFF;
    }
    if (tvar->can2_addr != old_c2) {
        tvar->tick_flags |= _TFLAG_C2_CHANGED;
        if (old_c2 != 0xFF) {
            set_block_addr_occupency(old_c2, BLK_OCC_FREE, tidx, snone);
        }
        if (tvar->can2_addr != 0xFF) {
            set_block_addr_occupency(tvar->can2_addr, BLK_OCC_C2, tidx, ns);
        }
    }
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
    
    if (ctrl_flag_notify_speed) {
        msg_64_t m = {0};
        m.from = MA_CONTROL_T(tidx);
        m.to = MA_UI(UISUB_TFT);
        m.cmd = CMD_TRTSPD_NOTIF;
        m.v1u = tvar->_target_speed;
        m.v2 = tvar->_dir;
        mqf_write_from_ctrl(&m);
    }
}

void ctrl2_sendlow_c1c2(int tidx, train_ctrl_t *tvar)
{
    msg_64_t m = {0};
    m.from = MA_CONTROL_T(tidx);
    m.to =  MA_TRAIN_SC(tidx);
    m.cmd = CMD_SET_C1_C2;
    int dir = tvar->_dir;
    
    const train_config_t *tconf = get_train_cnf(tidx);
    if (tconf->reversed) dir = -dir;
    
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
    
    if (from_bemf && ignore_ina_presence) {
        ctrl_set_timer(tidx, tvar, TLEAVE_C1, TLEAVE_C1_VALUE);
    } else {
        ctrl_set_timer(tidx, tvar, TLEAVE_C1, TGUARD_C1_VALUE);
    }
    // update occupency status
    set_block_addr_occupency(tvar->can2_addr, occupied(tvar->_dir), tidx,
                             first_lsblk_with_canton(tvar->can2_addr, tvar->c1_sblk));
           
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

    int len = get_lsblk_len(tvars->c1_sblk);
    if (tvars->_dir<0) {
    	tvars->beginposmm =  -len*10;
    } else {
    	tvars->beginposmm = 0;
    }

    tvars->can2_addr = 0xFF; // will be updated by update_c2
    tvars->tick_flags |=  _TFLAG_C1_CHANGED|_TFLAG_C1_CHANGED | _TFLAG_C1LSB_CHANGED;
}



void ctrl2_evt_entered_c1(int tidx, train_ctrl_t *tvars, _UNUSED_ uint8_t from_bemf)
{
    itm_debug2(DBG_CTRL, "enter C1", tidx, tvars->_state);
}
void ctrl2_evt_leaved_c2(int tidx, train_ctrl_t *tvar)
{
    itm_debug2(DBG_CTRL, "leave C2", tidx, tvar->_state);
}

int ctrl2_evt_pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t ca_addr, uint8_t trigbits, int16_t cposd10)
{
    int retcode = 0;
	itm_debug3(DBG_CTRL, "POSEtrg", tidx, ca_addr, cposd10);

    if (tvar->_state != train_running_c1) {
        itm_debug2(DBG_ERR|DBG_CTRL, "bad st/3",tidx, tvar->_state);
        if (trigbits & (1<<1)) tvar->pose2_set = 0;
        return -1;
    }
    if (ca_addr != tvar->can1_addr) {
        itm_debug3(DBG_ERR|DBG_POSEC|DBG_CTRL, "ptrg bad", tidx, ca_addr, tvar->can1_addr);
        return -1;
    }
    const train_config_t *tconf = get_train_cnf(tidx);
    tvar->curposmm = pose_convert_to_mm(tconf, cposd10*10);
    itm_debug1(DBG_POSE|DBG_CTRL, "curposmm", tvar->curposmm);
    if (trigbits & (1<<0)) {
        if (tvar->trig_eoseg) {
            // POSE Trig 1
            //tvar->tick_flags |= _TFLAG_POSE_TRIG1;
            lsblk_num_t ns = next_lsblk_free(tidx, tvar,  (tvar->_dir<0), NULL);
            if (ns.n<0) {
                itm_debug3(DBG_CTRL|DBG_ERR, "no next!", tidx, tvar->c1_sblk.n, tvar->_dir);
                goto t2;
            }
            if (canton_for_lsblk(ns) != tvar->can1_addr) {
                itm_debug3(DBG_ERR|DBG_CTRL, "bad sblk", tidx, ns.n, tvar->can1_addr);
                goto t2;
            }
            
            
            int len1 = get_lsblk_len(tvar->c1_sblk);
            int len2 = get_lsblk_len(ns);
            int exppose;
            tvar->c1_sblk = ns;
            if (tvar->_dir<0) {
                exppose = tvar->beginposmm;
                tvar->beginposmm = tvar->beginposmm - len2*10;
            } else {
                tvar->beginposmm = tvar->beginposmm + len1*10;
                exppose = tvar->beginposmm;
            }
            if (abs(tvar->curposmm - exppose)>30) {
                itm_debug3(DBG_ERR, "large p", tidx, exppose, tvar->curposmm);
                retcode = 2;
            }
            tvar->tick_flags |= _TFLAG_C1LSB_CHANGED;
            //tvar->beginposmm = (tvar->_dir >= 0) ? tvar->curposmm : tvar->curposmm + get_lsblk_len(ns)*10;
        }
    }
t2:
    if (!tvar->trig_eoseg) {
    	// POSE Trig 2

    	if (!tvar->pose2_set) return 3;
    	tvar->pose2_set = 0;
    	tvar->tick_flags |= _TFLAG_POSE_TRIG2;
    }
    if (trigbits & (1<<1)) {
        itm_debug2(DBG_POSEC|DBG_CTRL|DBG_AUTO, "trig 1U", tidx, trigbits);
        cauto_had_trigU1(tidx, tvar);
        fatal();
    }
    return retcode;
}
void ctrl2_evt_stop_detected(_UNUSED_ int tidx, train_ctrl_t *tvar, _UNUSED_ int32_t pose)
{
    // TODO
    tvar->tick_flags |= _TFLAG_STOP_DETECTED;
    const train_config_t *tconf = get_train_cnf(tidx);
    tvar->curposmm = pose_convert_to_mm(tconf, pose);
    itm_debug1(DBG_POSE|DBG_CTRL, "curposmm/s", tvar->curposmm);
}

static const uint16_t perm_flags = (_TFLAG_STATE_CHANGED|_TFLAG_DIR_CHANGED|_TFLAG_TSPD_CHANGED|_TFLAG_C1_CHANGED|_TFLAG_C2_CHANGED);

int ctrl2_tick_process(int tidx, train_ctrl_t *tvars, const train_config_t *tconf, int8_t occupency_changed)
{
    int nloop = 0;
    if (occupency_changed) tvars->tick_flags |= _TFLAG_OCC_CHANGED;
    if (0==tvars->tick_flags) return 0;
    
    uint16_t pflags = 0;
    int32_t pose_s0eoseg = 0;
    int32_t pose_s0middle = 0;
    while (tvars->tick_flags) {
        nloop++;
        if (nloop>16) {
            itm_debug1(DBG_ERR|DBG_CTRL, "infinite", 0);
        }
        uint16_t flags = tvars->tick_flags;
        pflags |= (flags & perm_flags);
        tvars->tick_flags = 0;

        if (pflags & _TFLAG_C1_CHANGED) tvars->curposmm = 0;

        if (flags & (_TFLAG_DSPD_CHANGED|_TFLAG_MODE_CHANGED)) {
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
        
        if ((tvars->_mode == train_auto) && (flags & _TFLAG_C1LSB_CHANGED)) {
            cauto_c1_updated(tidx, tvars);
            
        }
        /*
        if ((tvars->_mode == train_auto) && (flags & (_TFLAG_DIR_CHANGED|_TFLAG_C1LSB_CHANGED))) {
            if ((tvars->route) &&  (tvars->_dir) && (0 == (tvars->route[tvars->routeidx] & 0x80))) {
                int nsb = tvars->route[tvars->routeidx] & 0x7F;
                int t = cauto_update_turnouts(tidx, tvars->c1_sblk, tvars->_dir, nsb);
                if (t>0) {
                    flags |= _TFLAG_OCC_CHANGED;
                } else if (t<0) {
                    ctrl2_set_state(tidx, tvars, train_blk_wait);
                }
            } else if ((tvars->route) &&  (!tvars->_dir) && (0x80 == (tvars->route[tvars->routeidx] & 0x80))) {
                if (tvars->route[tvars->routeidx]==0xFF) {
                    ctrl2_set_mode(tidx, tvars, train_manual);
                } else {
                	int8_t v = ((tvars->route[tvars->routeidx] & 0x7F)<<1);
                	tvars->desired_speed = v>>2;
                	tvars->desired_speed = SIGNOF0(tvars->desired_speed)*70;
                	tvars->routeidx++;
                	tvars->tick_flags |= _TFLAG_DSPD_CHANGED;
                }
            }
        }*/
        if (flags & (_TFLAG_C1LSB_CHANGED|_TFLAG_DIR_CHANGED|_TFLAG_NEED_C2)) {
            ctrl2_update_c2(tidx, tvars, tconf, &pose_s0eoseg);
        }
        if (flags & (_TFLAG_C1LSB_CHANGED|_TFLAG_OCC_CHANGED|_TFLAG_DIR_CHANGED)) {
            ctrl2_update_topo(tidx, tvars, tconf, &pose_s0middle);
        }
            
        if (flags & (_TFLAG_LIMIT_CHANGED|/*_TFLAG_TSPD_CHANGED|*/_TFLAG_DSPD_CHANGED)) {
            ctrl2_apply_speed_limit(tidx, tvars);
        }
        
    }
    if (pflags & (_TFLAG_C1_CHANGED | _TFLAG_C2_CHANGED |_TFLAG_DIR_CHANGED)) {
        ctrl2_sendlow_c1c2(tidx, tvars);
    }
    if (pflags & _TFLAG_STATE_CHANGED) {
        ctrl2_notify_state(tidx, tvars);
    }
    if (pose_s0eoseg && pose_s0middle) {
        itm_debug3(DBG_ERR|DBG_CTRL, "BAD POSE", tidx, pose_s0eoseg, pose_s0middle);
        fatal();
    }
    if (pose_s0eoseg) {
        ctrl_set_pose_trig(tidx, pose_s0eoseg, 0);
        tvars->trig_eoseg = 1;
    } else if (pose_s0middle) {
        ctrl_set_pose_trig(tidx, pose_s0middle, 0);
        tvars->trig_eoseg = 0;
    }
    if (pflags & (_TFLAG_TSPD_CHANGED)) {
        ctrl2_sendlow_tspd(tidx, tvars);
    }
    return nloop;
}
