//
//  ctrlP.c
//  train_throttle
//
//  Created by Daniel BRAUN on 28/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#if 0 // obsolete

#include <stdint.h>


#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

//#include "../railconfig.h"
#include "../config/conf_train.h"

#include "ctrl.h"
#include "ctrlP.h"
#include "trig_tags.h"
#include "cautoP.h"



#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif


static lsblk_num_t snone = {-1};

#define SPD_LIMIT_EOT    70   //20
#define SPD_LIMIT_NOLIMIT 100


//uint8_t ctrl_flag_notify_speed = 1;






static inline int _traindir(int tidx, const train_oldctrl_t *tvar, const conf_train_t *tconf)
{
    if (!tconf) {
        tconf = conf_train_get(tidx);
    }
    if (tconf->reversed) return -tvar->_dir;
    return tvar->_dir;
}

// ------------------------------------------------------


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

static int32_t get_lsblk_len_cm_steep(lsblk_num_t lsbk, const conf_train_t *tconf, train_oldctrl_t *tvar)
{
    int8_t steep = 0;
	int cm = get_lsblk_len_cm(lsbk, &steep);
	itm_debug3(DBG_CTRL|DBG_POSEC, "steep?", steep, tvar->_dir, lsbk.n);
	if (steep*tvar->_dir > 0) {
        if (!tconf->slipping) FatalError("NSLP", "no slipping", Error_Slipping);
		int cmold = cm;
		cm = cm * tconf->slipping / 100;
		itm_debug3(DBG_CTRL|DBG_POSEC, "steep!", tvar->c1_sblk.n, cmold, cm);
	}
	return cm;
}

// ------------------------------------------------------

static int32_t ctrl_pose_percent_s1(const conf_train_t *tconf, train_oldctrl_t *tvar, int percent)
{
    int cm = get_lsblk_len_cm_steep(tvar->c1_sblk, tconf, tvar);
    int mm;
    int mm1 = cm * (100-percent) / 10; // 10%
    if (mm1<120) mm1 = 120; // min guard
    if (tvar->_dir>0) {
        // going right
        mm = tvar->beginposmm + (cm*10-mm1) - tconf->trainlen_right_cm*10;
        if (mm<=tvar->beginposmm) mm=tvar->beginposmm;
    } else {
        // going left
        mm = tvar->beginposmm + mm1 + tconf->trainlen_left_cm*10;
        if (mm>=tvar->beginposmm+cm*10) mm=tvar->beginposmm+cm*10;
    }
    int32_t p = pose_convert_from_mm(tconf, mm);
    itm_debug3(DBG_CTRL|DBG_POSEC, "pcent_c1 ", tvar->c1_sblk.n, cm, percent);
    itm_debug1(DBG_CTRL|DBG_POSEC, "pcent_c1.", p);
    if (!p) p=1;
    return p;
}


static int32_t ctrl_pose_middle_s1(const conf_train_t *tconf, train_oldctrl_t *tvar)
{
    return ctrl_pose_percent_s1(tconf, tvar, 50);
}

static int32_t ctrl_pose_limit_s1(const conf_train_t *tconf, train_oldctrl_t *tvar)
{
    return ctrl_pose_percent_s1(tconf, tvar, 90);
}



static int32_t ctrl_pose_end_s1(const conf_train_t *tconf, train_oldctrl_t *tvar)
{
    int cm = get_lsblk_len_cm_steep(tvar->c1_sblk, tconf, tvar);
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


static int32_t ctrl_pose_len_s1(const conf_train_t *tconf, train_oldctrl_t *tvar)
{
	int cm = get_lsblk_len_cm_steep(tvar->c1_sblk, tconf, tvar);
	int mm;
	if (tvar->_dir<0) {
		int lmm = (tconf->trainlen_right_cm + 5) * 10;
		if (lmm > cm*10) lmm = cm*10;
		mm = tvar->beginposmm + (cm*10-lmm);
	} else {
		int lmm = (tconf->trainlen_left_cm + 5) * 10;
		if (lmm > cm*10) lmm = cm*10;
		mm = tvar->beginposmm + cm*10 - lmm;
	}
	int32_t p = pose_convert_from_mm(tconf, mm);
	if (!p) p=1;
	return p;
}

void ctrl2_upcmd_settrigU1(int tidx, train_oldctrl_t *tvars, uint8_t t)
{
    if (!tvars->_dir) return;
    int32_t p;
    switch (t) {
        default:
            //FALLTHRU
        case 1:
            p = ctrl_pose_middle_s1(conf_train_get(tidx), tvars);
            break;
        case 2:
            p = ctrl_pose_end_s1(conf_train_get(tidx), tvars);
            break;
        case 3:
        	p = ctrl_pose_len_s1(conf_train_get(tidx), tvars);
            //p = ctrl_pose_percent_s1(conf_train_get(tidx), tvars, 10);
            break;
        case 4:
        	p = ctrl_pose_limit_s1(conf_train_get(tidx), tvars);
        	break;
    }

    ctrl_set_pose_trig(tidx, tvars, _traindir(tidx, tvars, NULL), tvars->can1_xaddr,  p, tag_auto_u1);
}


void ctrl_set_pose_trig(int numtrain, _UNUSED_ train_oldctrl_t *tvars, int8_t dir,  xblkaddr_t canaddr, int32_t pose, uint8_t tag)
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
    const conf_train_t *tconf = conf_train_get(numtrain);
    if (tconf->reversed)  m.va16 = -pose/10;
    else m.va16 = pose/10;
    m.vcu8 = tag;
    m.vb8 = dir;
    itm_debug3(DBG_CTRL|DBG_POSEC, "S_TRIG", numtrain, tag, dir);
    mqf_write_from_ctrl(&m);
}


// -----------------------------------------------------------------------------------



void ctrl_reset_timer(int tidx, train_oldctrl_t *tvar, int numtimer)
{
    itm_debug2(DBG_CTRL, "reset_timer", tidx, numtimer);
    if (numtimer<0 || numtimer>=NUM_TIMERS) FatalError("BTIM", "bad timer num", Error_CtrlTimerNum);
    tvar->timertick[numtimer] = 0;
}

void ctrl_set_timer(int tidx, train_oldctrl_t *tvar, int numtimer, uint32_t tval)
{
    itm_debug3(DBG_CTRL, "set_timer", tidx, numtimer, tval);
    if (numtimer<0 || numtimer>=NUM_TIMERS) FatalError("BTIM", "bad timer num", Error_CtrlTimerNum);
    tvar->timertick[numtimer] = HAL_GetTick() + tval;
}


void ctrl2_set_mode(int tidx, train_oldctrl_t *tvar, train_mode_t mode)
{
    itm_debug2(DBG_CTRL, "set mode", tidx, mode);
    if (tvar->_mode == mode) return;
    tvar->_mode = mode;
    tvar->tick_flags |= _TFLAG_MODE_CHANGED;
    
    // notif UI
    msg_64_t m;
    m.from = MA1_CTRL(tidx);
    m.to = MA3_UI_GEN; //(UISUB_TFT);
    m.cmd = CMD_TRMODE_NOTIF;
    m.v1u = mode;
    mqf_write_from_ctrl(&m);

    if (mode == train_notrunning) {
    	 tvar->_dir = 0;
         tvar->_target_speed = 0;
         tvar->desired_speed = 0;
         tvar->_ostate = train_off;
         tvar->c1c2 = 0;
    	ctrl2_sendlow_c1c2(tidx, tvar);
        if (tvar->can2_xaddr.v != 0xFF) { set_block_addr_occupency(tvar->can2_xaddr, BLK_OCC_FREE, tidx, snone);
        }
        if (tvar->can1_xaddr.v != 0xFF) { set_block_addr_occupency(tvar->can1_xaddr, BLK_OCC_FREE, tidx, tvar->c1_sblk);
        }
    }
}




// ----------------------

static void free_block_c1(_UNUSED_ int tidx, train_oldctrl_t *tvars)
{
	set_block_addr_occupency(tvars->can1_xaddr, BLK_OCC_FREE, tidx, snone);
}

static void free_block_c2(_UNUSED_ int tidx, train_oldctrl_t *tvars)
{
	set_block_addr_occupency(tvars->can2_xaddr, BLK_OCC_FREE, tidx, snone);
}

static void free_block_other(int tidx, _UNUSED_ train_oldctrl_t *tvars, xblkaddr_t ca)
{
	set_block_addr_occupency(ca, BLK_OCC_FREE, tidx, snone);
}

// ----------------------


static lsblk_num_t next_lsblk_free(int tidx, train_oldctrl_t *tvars,  uint8_t left, int8_t *palternate, xblkaddr_t *pcan)
{
    lsblk_num_t nsa = {-1};
    lsblk_num_t retns = {-1};
    lsblk_num_t curns = tvars->c1_sblk;
    for (int i=0;i<3;i++) {
        if (tvars->_mode == train_auto) {
            nsa = cauto_peek_next_lsblk(tidx, tvars, left, i);
        }
        lsblk_num_t ns = next_lsblk(curns, left, palternate);
        if (ns.n >= 0) {
            if ((nsa.n >= 0) && (nsa.n != ns.n)) {
                ns.n = -1;
                if (palternate) *palternate = 1;
            } else {
                //  check occupency
            	xblkaddr_t ca = canton_for_lsblk(ns);
                int f = occupency_block_is_free(ca, tidx);
                if (!f) {
                    // block not free
                    ns.n = -1;
                    if (palternate) *palternate = 1;
                }
            }
        }

        if (!i) retns = ns;
        if (!pcan) break;
        xblkaddr_t c = canton_for_lsblk(ns);
        *pcan = c;
        if (c.v != tvars->can1_xaddr.v) break;
        if (palternate && *palternate) break;
        curns = ns;
    }
    return retns;
}


static void set_speed_limit(train_oldctrl_t *tvar, uint16_t lim)
{
    if (lim == tvar->spd_limit) return;
    tvar->spd_limit = lim;
    tvar->tick_flags |= _TFLAG_LIMIT_CHANGED;
}

void ctrl2_reset_longtrain(_UNUSED_ int tidx, train_oldctrl_t *tvars)
{
    tvars->rightcars.numlsblk = 0;
    tvars->leftcars.numlsblk = 0;
    memset(tvars->rightcars.r, 0xFF, sizeof(tvars->rightcars.r));
    memset(tvars->leftcars.r, 0xFF, sizeof(tvars->leftcars.r));
}

void ctrl2_init_train(_UNUSED_ int tidx, train_oldctrl_t *tvars,
                      lsblk_num_t sblk)
{
	itm_debug1(DBG_CTRL, "INIT", tidx);
    tvars->c1_sblk = sblk;
    tvars->_dir = 0;
    tvars->_target_speed = 0;
    tvars->_ostate = train_station;
    tvars->can1_xaddr = canton_for_lsblk(sblk);
    tvars->can2_xaddr.v = 0xFF;
    tvars->desired_speed = 0;
    tvars->spd_limit = 100;
    tvars->c1c2 = 0;
    tvars->pose2_set = 0;
    //tvars->behaviour_flags = 0;
    tvars->tick_flags |=
        _TFLAG_C1LSB_CHANGED | _TFLAG_DIR_CHANGED |
        _TFLAG_DSPD_CHANGED | _TFLAG_TSPD_CHANGED |
        _TFLAG_STATE_CHANGED | _TFLAG_LIMIT_CHANGED;
    tvars->beginposmm = 0;
    tvars->_curposmm = POSE_UNKNOWN;
    tvars->route = NULL;
    tvars->routeidx = 0;
    ctrl2_reset_longtrain(tidx, tvars);
}

void ctrl2_upcmd_set_desired_speed(_UNUSED_ int tidx, train_oldctrl_t *tvars, int16_t desired_speed)
{
    if (tvars->_mode == train_auto) {
        // switch back to manual ?
        ctrl2_set_mode(tidx, tvars, train_manual);
        //return;
    }
    _ctrl2_upcmd_set_desired_speed(tidx, tvars, desired_speed);
}

void _ctrl2_upcmd_set_desired_speed(_UNUSED_ int tidx, train_oldctrl_t *tvars, int16_t desired_speed)
{
	itm_debug2(DBG_CTRL, "DSPD", tidx, desired_speed);
    if (tvars->desired_speed != desired_speed) {
        tvars->tick_flags |= _TFLAG_DSPD_CHANGED;

        tvars->desired_speed = desired_speed;
        //tvars->desired_speed2 = 0;
    }
}

void ctrl2_set_state(_UNUSED_ int tidx, train_oldctrl_t *tvar, train_oldstate_t ns)
{
    if (ns == tvar->_ostate) {
        return;
    }
    itm_debug3(DBG_CTRL, "STATE", tidx, tvar->_ostate, ns);
    tvar->_ostate = ns;
    tvar->tick_flags |= _TFLAG_STATE_CHANGED;
}

void ctrl2_set_dir(_UNUSED_ int tidx, train_oldctrl_t *tvar, int8_t dir)
{
    if (tvar->_dir != dir) {
        tvar->tick_flags |= _TFLAG_DIR_CHANGED;
        tvar->_dir = dir;
    }
}

void ctrl2_stop_detected(int tidx, train_oldctrl_t *tvars)
{
	itm_debug1(DBG_CTRL, "c2 stp", tidx);
    switch (tvars->_ostate) {
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
    tvars->measure_pose_percm = 0;
}



void ctrl2_set_tspeed(_UNUSED_ int tidx, train_oldctrl_t *tvar, uint16_t tspeed)
{
    if (tvar->_target_speed != tspeed) {
        tvar->tick_flags |= _TFLAG_TSPD_CHANGED;
        if (tspeed >= 0x7FFF) {
        	FatalError("SPD", "bad tpeed", Error_CtrlTSpeed);
        }
        tvar->_target_speed = tspeed;
    }
}

void ctrl2_check_alreadystopped(int tidx, train_oldctrl_t *tvar)
{
    if (tvar->desired_speed) return;
    if (tvar->_ostate == train_running_c1) {
        if ((tvar->_dir == 0) | !tvar->_target_speed) {
            ctrl2_set_state(tidx, tvar, train_station);
        }
    }
}

/*

 */
void ctrl2_check_checkstart(int tidx, train_oldctrl_t *tvars)
{
    lsblk_num_t ns;
    
    itm_debug3(DBG_CTRL, "checkstart", tidx, tvars->_dir, tvars->desired_speed);

    if (!tvars->desired_speed) {
        if (tvars->_mode== train_auto) {
        	itm_debug1(DBG_CTRL, "cs cauto", tidx);
            cauto_check_start(tidx, tvars);
        }
    }
    if (!tvars->desired_speed) return;
    
    if (tvars->desired_speed == -36) {
    	itm_debug1(DBG_AUTO, "brk here", tidx);
    }
    switch (tvars->_ostate) {
        case train_station:
            ctrl2_set_dir(tidx, tvars, SIGNOF0(tvars->desired_speed));
            ctrl2_set_state(tidx, tvars, train_running_c1);
            //ctrl2_set_tspeed(tidx, tvars, SIGNOF0(tvars->desired_speed));
            break;
        
        case train_blk_wait:
        	itm_debug1(DBG_CTRL, "cs blk", tidx);
        	if (SIGNOF0(tvars->_dir) * SIGNOF0(tvars->desired_speed) < 0) {
        		tvars->_dir = 0;
        	}
        	// FALLTHRU
        case train_end_of_track:
            if (tvars->_target_speed) break; // already started
            if (tvars->_dir) {
            	// this may occurs if stop detection fails
            	if (tvars->_dir == SIGNOF(tvars->desired_speed)) {
            		break;
            	}
            	itm_debug3(DBG_ERR|DBG_CTRL, "miss stp?", tidx, tvars->_dir, tvars->desired_speed);
            }
            int8_t alternate;
            ns = next_lsblk_free(tidx, tvars,  (tvars->desired_speed<0),  &alternate, NULL);
            itm_debug3(DBG_CTRL, "cs next", tidx, ns.n, alternate);
            if (ns.n>=0) {
                ctrl2_set_state(tidx, tvars, train_running_c1);
                ctrl2_set_dir(tidx, tvars, SIGNOF0(tvars->desired_speed));
                //ctrl2_set_tspeed(tidx, tvar, SIGNOF0(tvar->desired_speed));
            } else {
                if (alternate) {
                    itm_debug1(DBG_CTRL, "cs g blk", tidx);
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

void ctrl2_check_stop(int tidx, train_oldctrl_t *tvar)
{
    switch (tvar->_ostate) {
        case train_off:
        case train_station:
            ctrl2_set_dir(tidx, tvar, 0);
            //FALLTHRU
        case train_blk_wait:
        case train_end_of_track:
            if (tvar->can2_xaddr.v != 0xFF) {
            	// dir might be 0 here
            	free_block_c2(tidx, tvar);
            	//set_block_addr_occupency(tvar->can2_addr, BLK_OCC_FREE, 0xFF, snone);
            }
            ctrl2_set_tspeed(tidx, tvar, 0);
            tvar->can2_xaddr.v = 0xFF;
            //set_block_addr_occupency(tvar->can1_xaddr, occupied(tvar->_dir), tidx, tvar->c1_sblk);
            break;
            
            
        default:
            break;
    }
}

void ctrl2_apply_speed_limit(int tidx, train_oldctrl_t *tvar)
{
    int16_t v;
     switch (tvar->_ostate) {
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
static void ctrl2_had_trig2(int tidx, train_oldctrl_t *tvar, uint8_t posetag)
{
	itm_debug1(DBG_CTRL, "had trig2", tidx);
    switch (tvar->_ostate) {
        //case train_running_c1c2:
        case train_running_c1:
            switch (posetag) {
                case tag_stop_blk_wait:
                    ctrl2_set_state(tidx, tvar, train_blk_wait);
                    break;
                case tag_stop_eot:
                    ctrl2_set_state(tidx, tvar, train_end_of_track);
                    break;
                default:
                    itm_debug2(DBG_ERR|DBG_CTRL|DBG_POSEC, "bad tag", tidx, posetag);
                    break;
            }
            break;
            
        default:
            break;
    }
}


void ctrl2_update_topo(int tidx, train_oldctrl_t *tvar, const conf_train_t *tconf, int32_t *ppose1, uint8_t *pposetag)
{
	itm_debug1(DBG_CTRL, "upd topo", tidx);
    switch (tvar->_ostate) {
        case train_off:
        case train_station:
            return;
        default:
            break;
    }
    
    int d = (tvar->_dir) ? tvar->_dir : SIGNOF0(tvar->desired_speed);
    if (!d) return;
    
    int8_t alternate = 0;
    lsblk_num_t ns = next_lsblk_free(tidx, tvar, (d < 0), &alternate, NULL);
    
    if (ns.n < 0) {
        switch (tvar->_ostate) {
            //case train_running_c1c2:
            case train_running_c1:
                if (tvar->pose2_set) {
                    //ctrl2_set_state(tidx, tvar, alternate ? train_blk_wait : train_end_of_track);
                } else {
                    if (!tvar->_dir) FatalError("NDIR", "no dir", Error_CtrlNoDir);
                    tvar->pose2_set = 1;
                    //tvar->pose2_is_blk_wait = alternate ? 1 : 0;
                    set_speed_limit(tvar, SPD_LIMIT_EOT);
                    int32_t posetval;
                    if ((0)) {
                        posetval = ctrl_pose_middle_s1(tconf, tvar);
                    } else {
                        posetval = ctrl_pose_limit_s1(tconf, tvar);
                    }
                    *ppose1 = posetval;
                    *pposetag = alternate ? tag_stop_blk_wait : tag_stop_eot;
                }
                break;
                
            default:
                break;
        }
        return;
    }
    tvar->pose2_set = 0;
    
    switch (tvar->_ostate) {
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
    if ((ns.n >= 0) && (tvar->can2_xaddr.v != canton_for_lsblk(ns).v)) {
        tvar->tick_flags |= _TFLAG_NEED_C2 ; // _TFLAG_C1LSB_CHANGED will trigger update_c2
    }
}
    

void ctrl2_update_c2(int tidx, train_oldctrl_t *tvar, const conf_train_t *tconf, int32_t *ppose0, uint8_t *pposetag)
{
	itm_debug3(DBG_CTRL, "updc2", tidx, tvar->c1_sblk.n, tvar->can1_xaddr.v);
    if (tvar->can1_xaddr.v == 0xFF) FatalError("U2C1", "Upd2 no C1", Error_CtrlNoC1);
    if (canton_for_lsblk(tvar->c1_sblk).v != tvar->can1_xaddr.v) FatalError("W.C1", "Wrong C1", Error_WrongC1);
    
    set_block_addr_occupency(tvar->can1_xaddr, occupied(tvar->_dir), tidx, tvar->c1_sblk);
    
    xblkaddr_t old_c2 = tvar->can2_xaddr;
    tvar->can2_xaddr.v = 0xFF;
    lsblk_num_t ns = snone;
    if (tvar->_dir) {
        int8_t alternate = 0;
        xblkaddr_t c2r;
        ns = next_lsblk_free(tidx, tvar, tvar->_dir<0, &alternate, &c2r);
        xblkaddr_t c2n = canton_for_lsblk(ns);
        itm_debug3(DBG_CTRL, "c2addr", tidx, tvar->can2_xaddr.v, c2r.v);
        //if ((tidx==1) && (c2addr.v = 0xFF) && (tvar->can1_xaddr.v == 0)) {
        // 	bh();
        //}
        if (c2n.v == tvar->can1_xaddr.v) {
            // ns is same canton as c1_sblk. if ina can't distinguish them, set trig
        	uint8_t ls2 = get_lsblk_ina3221(ns);
        	uint8_t ls1 = get_lsblk_ina3221(tvar->c1_sblk);
        	if (ignore_ina_pres() || (ls1 == ls2) || (ls1==0xFF)) {
                itm_debug3(DBG_POSEC, "setEOS ", ls1, ls2, ignore_ina_pres());
                itm_debug2(DBG_POSEC, "setEOS.", tvar->c1_sblk.n, ns.n);
        		int32_t posetval = ctrl_pose_end_s1(tconf, tvar);
        		*ppose0 = posetval;
                *pposetag = tag_end_lsblk;
        	}
        }
        if (c2r.v != tvar->can1_xaddr.v) {
            tvar->can2_xaddr = c2r;
        }
    } else {
    	itm_debug1(DBG_CTRL, "updc2 dir0", tidx);
        tvar->can2_xaddr.v = 0xFF;
    }
    if (tvar->can2_xaddr.v != old_c2.v) {
    	itm_debug2(DBG_CTRL, "updc2 c2", tidx, tvar->can2_xaddr.v);
        tvar->tick_flags |= _TFLAG_C2_CHANGED;
        if (old_c2.v != 0xFF) {
        	free_block_other(tidx, tvar, old_c2);
            //set_block_addr_occupency(old_c2, BLK_OCC_FREE, tidx, snone);
        }
        if (tvar->can2_xaddr.v != 0xFF) {
            set_block_addr_occupency(tvar->can2_xaddr, BLK_OCC_C2, tidx, ns);
        }
    }
}



void ctrl2_notify_state(int tidx, train_oldctrl_t *tvar)
{
    msg_64_t m = {0};
    m.from = MA1_CTRL(tidx);
    m.to = MA3_UI_GEN;//(UISUB_TFT);
    m.cmd = CMD_TRSTATE_NOTIF;
    m.v1u = tvar->_ostate;
    mqf_write_from_ctrl(&m);
}

void ctrl2_sendlow_tspd(int tidx, train_oldctrl_t *tvar)
{
    msg_64_t m = {0};
    m.from = MA1_CTRL(tidx);
    m.to =   MA1_SPDCTL(tidx);
    m.cmd = CMD_SET_TARGET_SPEED;
    // direction already given by SET_C1_C2
    //m.v1 = trctl[trnum]._dir*trctl[trnum]._target_speed;
    m.v1u = tvar->_target_speed;
    m.v2 = 0;
    mqf_write_from_ctrl(&m);
    
    if (ctrl_flag_notify_speed) {
        msg_64_t m = {0};
        m.from = MA1_CTRL(tidx);
        m.to = MA3_UI_GEN; //(UISUB_TFT);
        m.cmd = CMD_TRTSPD_NOTIF;
        m.v1u = tvar->_target_speed;
        m.v2 = tvar->_dir;
        mqf_write_from_ctrl(&m);
    }
}

void ctrl2_sendlow_c1c2(int tidx, train_oldctrl_t *tvar)
{
    msg_64_t m = {0};
    m.from = MA1_CTRL(tidx);
    m.to =  MA1_SPDCTL(tidx);
    m.cmd = CMD_SET_C1_C2old;
    int dir = tvar->_dir;
    
    const conf_train_t *tconf = conf_train_get(tidx);
    if (tconf->reversed) dir = -dir;
    
    m.vbytes[0] = tvar->can1_xaddr.v;
    m.vbytes[1] = dir;
    m.vbytes[2] = tvar->can2_xaddr.v;
    m.vbytes[3] = dir; // 0;
    //mqf_write_from_ctrl(&m);
}

void ctrl2_evt_entered_c2(int tidx, train_oldctrl_t *tvar, uint8_t from_bemf)
{
    if (from_bemf && ignore_bemf_presence) return;
    itm_debug3(DBG_CTRL, "evt_ent_c2", tidx, tvar->can1_xaddr.v,  tvar->can2_xaddr.v);

    if (tvar->_ostate != train_running_c1) {
        itm_debug3(DBG_CTRL|DBG_ERR, "ent C2/badst", tidx, tvar->_ostate, from_bemf);
        return;
    }
    if (tvar->c1c2) {
        itm_debug3(DBG_CTRL|DBG_ERR, "ent C2/c1c2", tidx, tvar->_ostate, from_bemf);
    }
    tvar->c1c2 = 1;
    
    if (from_bemf && ignore_ina_pres()) {
        ctrl_set_timer(tidx, tvar, TLEAVE_C1, TLEAVE_C1_VALUE);
    } else {
        ctrl_set_timer(tidx, tvar, TLEAVE_C1, TGUARD_C1_VALUE);
    }
    // update occupency status
    set_block_addr_occupency(tvar->can2_xaddr, occupied(tvar->_dir), tidx,
                             first_lsblk_with_canton(tvar->can2_xaddr, tvar->c1_sblk));
           
}


void ctrl2_evt_leaved_c1(int tidx, train_oldctrl_t *tvars)
{
    itm_debug3(DBG_CTRL|DBG_POSE, "evt_left_c1", tidx, tvars->_ostate, tvars->can1_xaddr.v);
    if (tvars->_ostate != train_running_c1) {
        itm_debug2(DBG_CTRL|DBG_ERR, "leav_c2/bs", tidx, tvars->_ostate);
        return;
    }
    if (!tvars->c1c2) {
        itm_debug2(DBG_CTRL|DBG_ERR, "leav_c2/nc1c2", tidx, tvars->_ostate);
        return;
    }
    
    ctrl_reset_timer(tidx, tvars, TLEAVE_C1);
    tvars->c1c2 = 0;
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
}



void ctrl2_evt_entered_c1(int tidx, train_oldctrl_t *tvars, _UNUSED_ uint8_t from_bemf)
{
    itm_debug2(DBG_CTRL, "enter C1", tidx, tvars->_ostate);
}
void ctrl2_evt_leaved_c2(int tidx, train_oldctrl_t *tvar)
{
    itm_debug2(DBG_CTRL, "leave C2", tidx, tvar->_ostate);
}

static int ctrl2_set_next_c1_lsblk(int tidx, train_oldctrl_t *tvar, lsblk_num_t ns, int fromtrig);

void ctrl2_evt_entered_s2(int tidx, train_oldctrl_t *tvars)
{
	lsblk_num_t ns = next_lsblk(tvars->c1_sblk, tvars->_dir<0, NULL);
	itm_debug3(DBG_CTRL|DBG_POSEC, "enter S2", tidx, tvars->_ostate, ns.n);

	ctrl2_set_next_c1_lsblk(tidx, tvars, ns, 0);
}

static int ctrl2_set_next_c1_lsblk(int tidx, train_oldctrl_t *tvar, lsblk_num_t ns, int fromtrig)
{
	int retcode = 0;
    int len1 = get_lsblk_len_cm_steep(tvar->c1_sblk, conf_train_get(tidx), tvar);
    int len2 = get_lsblk_len_cm_steep(ns, conf_train_get(tidx), tvar);
    int exppose;
    // XXXX
    if (!fromtrig) {
        // from ina
        tvar->beginposmm = 0;
        tvar->pose_reset = 1;
    }
    if (tvar->_dir<0) {
        exppose = tvar->beginposmm;
        tvar->beginposmm = tvar->beginposmm - len2*10;
    } else {
        tvar->beginposmm = tvar->beginposmm + len1*10;
        exppose = tvar->beginposmm;
    }
    if (abs(tvar->_curposmm - exppose)>30) {
        itm_debug3(DBG_ERR, "large p", tidx, exppose, tvar->_curposmm);
        retcode = 2;
    }
    if ((1) || (5==ns.n)) { // debug
    	itm_debug3(DBG_CTRL|DBG_POSEC, "enterS2 ", tidx, fromtrig, tvar->c1_sblk.n);
    	itm_debug3(DBG_CTRL|DBG_POSEC, "enterS2.", exppose, tvar->beginposmm, tvar->_curposmm);
    }
    tvar->c1_sblk = ns;
    tvar->tick_flags |= _TFLAG_C1LSB_CHANGED;
    return retcode;
}

int ctrl2_evt_pose_triggered(int tidx, train_oldctrl_t *tvar, const conf_train_t *tconf, xblkaddr_t ca_addr, uint8_t tag, int16_t cposd10)
{
    int retcode = 0;
	itm_debug3(DBG_CTRL|DBG_POSEC, "POSEtrg", tidx, ca_addr.v, cposd10);

    if (tvar->_ostate != train_running_c1) {
        itm_debug2(DBG_ERR|DBG_CTRL, "bad st/3",tidx, tvar->_ostate);
        if ((tag == tag_stop_blk_wait) || (tag==tag_stop_eot)) tvar->pose2_set = 0;
        return -1;
    }
    if (ca_addr.v != tvar->can1_xaddr.v) {
        itm_debug3(DBG_ERR|DBG_POSEC|DBG_CTRL, "ptrg bad", tidx, ca_addr.v, tvar->can1_xaddr.v);
        return -1;
    }
    if (!tconf) tconf = conf_train_get(tidx);
    tvar->_curposmm = pose_convert_to_mm(tconf, cposd10*10);
    tvar->pose_reset = 0;
    itm_debug3(DBG_POSE|DBG_CTRL, "curposmm", tidx, tvar->_curposmm, tag);
    switch (tag) {
        case tag_end_lsblk:
            // POSE Trig 1
            itm_debug1(DBG_CTRL|DBG_POSEC, "end lsblk", tidx);
            lsblk_num_t ns = next_lsblk_free(tidx, tvar,  (tvar->_dir<0), NULL, NULL);
            if (ns.n<0) {
                itm_debug3(DBG_CTRL|DBG_ERR, "no next!", tidx, tvar->c1_sblk.n, tvar->_dir);
                break;
            }
            if (canton_for_lsblk(ns).v != tvar->can1_xaddr.v) {
                itm_debug3(DBG_ERR|DBG_CTRL, "bad sblk", tidx, ns.n, tvar->can1_xaddr.v);
                break;
            }
        
            int len1 = get_lsblk_len_cm_steep(tvar->c1_sblk,tconf, tvar);
            int len2 = get_lsblk_len_cm_steep(ns, tconf, tvar);
            int exppose;
            tvar->c1_sblk = ns;
            if (tvar->_dir<0) {
                exppose = tvar->beginposmm;
                tvar->beginposmm = tvar->beginposmm - len2*10;
            } else {
                tvar->beginposmm = tvar->beginposmm + len1*10;
                exppose = tvar->beginposmm;
            }
            if (abs(tvar->_curposmm - exppose)>30) {
                itm_debug3(DBG_ERR, "large p", tidx, exppose, tvar->_curposmm);
                retcode = 2;
            }
            tvar->tick_flags |= _TFLAG_C1LSB_CHANGED;
            //tvar->beginposmm = (tvar->_dir >= 0) ? tvar->curposmm : tvar->curposmm + get_lsblk_len(ns)*10;
            break;
            
        case tag_stop_eot:
            if (!tvar->pose2_set) {
                itm_debug2(DBG_ERR|DBG_POSEC|DBG_CTRL, "unexp tag", tidx, tag);
                return 3;
            }
			tvar->pose2_set = 0;
			tvar->tick_flags |= _TFLAG_POSE_TRIG_EOT;
            break;
        case tag_stop_blk_wait:
            if (!tvar->pose2_set) {
                itm_debug2(DBG_ERR|DBG_POSEC|DBG_CTRL, "unexp tag", tidx, tag);
                return 3;
            }
            tvar->pose2_set = 0;
            tvar->tick_flags |= _TFLAG_POSE_TRIG_BLKW;
            break;
        case tag_auto_u1:
            itm_debug2(DBG_POSEC|DBG_CTRL|DBG_AUTO, "trig U1", tidx, tag);
            cauto_had_trigU1(tidx, tvar);
            break;
        default:
            itm_debug2(DBG_ERR|DBG_POSEC|DBG_CTRL, "unexp tag", tidx, tag);
            break;

    }
    return retcode;
}

void ctrl2_evt_stop_detected(_UNUSED_ int tidx, train_oldctrl_t *tvar, const conf_train_t *tconf,  _UNUSED_ int32_t pose)
{
    // TODO
    tvar->tick_flags |= _TFLAG_STOP_DETECTED;
    if (!tconf) tconf = conf_train_get(tidx);
    tvar->_curposmm = pose_convert_to_mm(tconf, pose);
    tvar->pose_reset = 0;
    itm_debug1(DBG_POSEC|DBG_CTRL, "curposmm/s", tvar->_curposmm);
}

static const uint16_t perm_flags = (_TFLAG_STATE_CHANGED|_TFLAG_DIR_CHANGED|_TFLAG_TSPD_CHANGED|_TFLAG_C1_CHANGED|_TFLAG_C2_CHANGED);

int ctrl2_tick_process(int tidx, train_oldctrl_t *tvars, const conf_train_t *tconf, int8_t occupency_changed)
{
    int nloop = 0;
    if (occupency_changed) tvars->tick_flags |= _TFLAG_OCC_CHANGED;
    if (0==tvars->tick_flags) return 0;
    
    uint16_t pflags = 0;
    int32_t pose_s1eoseg = 0; // pose for end of lsblk
    int32_t pose_topo = 0;
    uint8_t posetag_c2 = 0;
    uint8_t posetag_topo = 0;
    while (tvars->tick_flags) {
        nloop++;
        if (nloop>16) {
            itm_debug1(DBG_ERR|DBG_CTRL, "infinite", 0);
        }
        uint16_t flags = tvars->tick_flags;
        pflags |= (flags & perm_flags);
        tvars->tick_flags = 0;

        if (pflags & _TFLAG_C1_CHANGED) {
        	tvars->_curposmm = 0;
            tvars->pose_reset = 0;
        	itm_debug1(DBG_CTRL, "curposmm0", tidx);
        }

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
        if (flags & _TFLAG_POSE_TRIG_EOT) {
            ctrl2_had_trig2(tidx, tvars, tag_stop_eot);
        }
        if (flags & _TFLAG_POSE_TRIG_BLKW) {
            ctrl2_had_trig2(tidx, tvars, tag_stop_blk_wait);
        }
        
        if ((tvars->_mode == train_auto) && (flags & _TFLAG_C1LSB_CHANGED)) {
            cauto_c1_updated(tidx, tvars);
            
        }


        if (flags & (_TFLAG_C1LSB_CHANGED|_TFLAG_DIR_CHANGED|_TFLAG_NEED_C2)) {
            ctrl2_update_c2(tidx, tvars, tconf, &pose_s1eoseg, &posetag_c2);
        }
        if (flags & (_TFLAG_C1LSB_CHANGED|_TFLAG_OCC_CHANGED|_TFLAG_DIR_CHANGED)) {
            ctrl2_update_topo(tidx, tvars, tconf, &pose_topo, &posetag_topo);
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
		/*
    if (pose_s0eoseg && pose_s0middle) {
        itm_debug3(DBG_ERR|DBG_CTRL, "BAD POSE", tidx, pose_s0eoseg, pose_s0middle);
        FatalError("BPSE", "bad pose", Error_CtrlBadPose);
    }
    if (pose_s0eoseg) {
    	itm_debug3(DBG_CTRL, "TrEos", tidx, tvars->curposmm, pose_s0eoseg);
        ctrl_set_pose_trig(tidx, tvars, pose_s0eoseg, 0);
        tvars->trig_eoseg = 1;
    } else if (pose_s0middle) {
    	itm_debug3(DBG_CTRL, "TrMid", tidx, tvars->curposmm, pose_s0middle);
        ctrl_set_pose_trig(tidx, tvars, pose_s0middle, 0);
        tvars->trig_eoseg = 0;
        */
    if (pose_s1eoseg && pose_topo) {
        itm_debug3(DBG_ERR|DBG_CTRL, "BAD POSE", tidx, pose_s1eoseg, pose_topo);
        FatalError("BPSE", "bad pose bits", Error_CtrlBadPose);
    }
    if (posetag_c2) {
        ctrl_set_pose_trig(tidx, tvars,  _traindir(tidx, tvars, tconf), tvars->can1_xaddr, pose_s1eoseg, posetag_c2);
        //tvars->trig_eoseg = 1;
    } else if (posetag_topo) {
        ctrl_set_pose_trig(tidx, tvars, _traindir(tidx, tvars, tconf), tvars->can1_xaddr,  pose_topo, posetag_topo);
        //tvars->trig_eoseg = 0;
    }
    if (pflags & (_TFLAG_TSPD_CHANGED)) {
        ctrl2_sendlow_tspd(tidx, tvars);
    }
    if (tvars->_mode == train_auto) cauto_end_tick(tidx, tvars);
    return nloop;
}


// #longtrain
// moved to longtrain.c
#if 0
static int32_t getcurpossmm(train_oldctrl_t *tvars, const conf_train_t *tconf, int left)
{
    if (POSE_UNKNOWN == tvars->_curposmm) {
        if (left) return tvars->beginposmm;
        return tvars->beginposmm + get_lsblk_len_cm_steep(tvars->c1_sblk, tconf, tvars);
    }
    return tvars->_curposmm;
}

int ctrl2_get_next_sblks_(_UNUSED_ int tidx, train_oldctrl_t *tvars,  const conf_train_t *tconf, int left, lsblk_num_t *resp, int nsblk, int16_t *premainlen)
{
    if (premainlen) *premainlen = 0;
    int lidx = 0;
    int cm = left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm;
    lsblk_num_t cblk = tvars->c1_sblk;
    // curposmm
    int l0 = getcurpossmm(tvars, tconf, left) / 10;
    for (;;) {
        int l = get_lsblk_len_cm(cblk, NULL);
        if (l0) {
            if (left) {
                l = l0;
            } else {
                l = l-l0;
            }
            l0 = 0;
        }
        if (l > cm) {
            // done
            if (premainlen) *premainlen = l-cm;
            return lidx;
        }
        cm -= l;
        cblk = next_lsblk(cblk, left, NULL);
        resp[lidx] = cblk;
        lidx++;
        if (lidx>=nsblk) return lidx;
        if (cblk.n == -1) return lidx;
    }
}

int ctrl2_get_next_sblks(int tidx, train_oldctrl_t *tvars,  const conf_train_t *tconf)
{
    memset(tvars->rightcars.r, 0xFF, sizeof(tvars->rightcars.r));
    memset(tvars->leftcars.r, 0xFF, sizeof(tvars->leftcars.r));
    tvars->rightcars.nr = ctrl2_get_next_sblks_(tidx, tvars, tconf, 0, tvars->rightcars.r, MAX_LSBLK_CARS, &tvars->rightcars.rlen_cm);
    tvars->leftcars.nr = ctrl2_get_next_sblks_(tidx, tvars, tconf, 1, tvars->leftcars.r, MAX_LSBLK_CARS, &tvars->leftcars.rlen_cm);
    return 0; // XXX error handling here
}

static const int brake_len_cm = 16;
static const int margin_len_cm = 12;


static int trig_for_frontdistcm(_UNUSED_ int tidx, train_oldctrl_t *tvars,  _UNUSED_ const conf_train_t *tconf, int left, int distcm)
{
    struct forwdsblk _UNUSED_ *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    if (!left) {
        int lmm = tvars->_curposmm - tvars->beginposmm + 10*distcm;
        if (lmm<10*get_lsblk_len_cm(tvars->c1_sblk, NULL)) {
            return lmm+tvars->beginposmm;
        }
    } else {
        int lmm = (tvars->_curposmm - tvars->beginposmm) - 10*distcm;
        if (lmm>0) {
            return lmm+tvars->beginposmm;
        }
    }
    return -1;
}

static int check_for_dist(_UNUSED_ int tidx, train_oldctrl_t *tvars,  struct forwdsblk *fsblk, int left, int distcm, uint8_t *pa)
{
    lsblk_num_t ns = (fsblk->nr>0) ? fsblk->r[fsblk->nr-1] : tvars->c1_sblk;
    int slen = get_lsblk_len_cm(ns, NULL);
    int cklen = fsblk->rlen_cm-distcm;

    while (cklen<0) {
        ns = next_lsblk(ns, left, pa);
        if (ns.n == -1) {
            // block occupied (a=1) or eot (a=0)
            return cklen;
            break;
        }
        slen = get_lsblk_len_cm(ns, NULL);
        cklen += slen;
    }
    lsblk_num_t fs = next_lsblk(ns, left, pa);
    if (fs.n == -1) {
        return cklen;
    }
    return 9999;
}

int ctrl2_check_front_sblks(int tidx, train_oldctrl_t *tvars,  const conf_train_t *tconf, int left,  rettrigs_t ret)
{
    struct forwdsblk *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    int retc = 0;
    int curcm = tvars->_curposmm/10;
    int maxcm = get_lsblk_len_cm(tvars->c1_sblk, NULL);
     memset(ret, 0, sizeof(rettrigs_t));
    // distance that will trigger a c1sblk change
    //int dc1mm =  10*get_lsblk_len_cm(tvars->c1_sblk, NULL) - (tvars->_curposmm - tvars->beginposmm) ;
    // trigger for end of seg
    int lmm = trig_for_frontdistcm(tidx, tvars, tconf, left, fsblk->rlen_cm);
    if (lmm>=0) {
        ret[0].poscm = lmm/10;
        ret[0].tag = tag_chkocc;
    }
    uint8_t a;
    int l1 = check_for_dist(tidx, tvars, fsblk, left,  brake_len_cm+margin_len_cm, &a);
    if (l1<=0) {
        retc = brake_len_cm+l1;
        if (retc<=0) retc = 1;
    } else if ((l1>0) && (l1+curcm<maxcm)) {
        ret[1].poscm = l1+curcm;
        ret[1].tag = tag_brake;
    }
    int l2 = check_for_dist(tidx, tvars, fsblk, left, margin_len_cm, &a);
    //printf("l2/8=%d\n", l2);
    if (l2<=0) {
        retc = -1;
    } else if ((l2>0) && (l2+curcm<maxcm)) {
        ret[2].poscm = l2+curcm;
        ret[2].tag = a ? tag_stop_blk_wait : tag_stop_eot;
    }
   
    return retc;
}


int ctrl2_update_front_sblks(int tidx, train_oldctrl_t *tvars,  const conf_train_t *tconf, int left)
{
    struct forwdsblk *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    
    if ((1)) {
        // sanity check, c1sblk should not have change
        lsblk_num_t ns = next_lsblk(tvars->c1_sblk, left, NULL);
        if (fsblk->nr) {
            if (fsblk->r[0].n != ns.n) return -1;
        }
    }
    // this could be improved, as only last sblk (and rlen) are to be updated.
    // but for now let's be safe
    return ctrl2_get_next_sblks(tidx, tvars, tconf);
}

int ctrl2_update_front_sblks_c1changed(int tidx, train_oldctrl_t *tvars,  const conf_train_t *tconf, int left)
{
    struct forwdsblk *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    
    if ((1)) {
        // sanity check, c1sblk should be first item
        if (fsblk->nr) {
            if (fsblk->r[0].n != tvars->c1_sblk.n) return -1;
        }
    }
    // this could be improved,
    // but for now let's be safe
    return ctrl2_get_next_sblks(tidx, tvars, tconf);
}
#endif


#endif // obsolete
