//
//  cautoP.c
//  train_throttle
//
//  Created by Daniel BRAUN on 13/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include "misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

#include "ctrl.h"
#include "ctrlP.h"
#include "cautoP.h"


static void route_error(int tidx, train_ctrl_t *tvars)
{
    itm_debug1(DBG_ERR|DBG_AUTO, "R/ERR", tidx);
    ctrl2_set_mode(tidx, tvars, train_manual);
}

static void route_end(int tidx, train_ctrl_t *tvars)
{
    itm_debug1(DBG_ERR|DBG_AUTO, "R/END", tidx);
    //tvars->routeidx = 0;
    ctrl2_set_mode(tidx, tvars, train_manual);
}

int cauto_update_turnouts(int tidx, lsblk_num_t cur, int8_t dir, uint8_t next)
{
    lsblk_num_t ta, tb;
    int tn;
    next_lsblk_nums(cur, (dir<0) ? 1 : 0, &ta, &tb, &tn);
    if (ta.n == next) {
        int v = topology_get_turnout(tn);
        if (!v) return 0;
        ctrl2_set_turnout(tn, 0);
        return 1;
    }
    if (tb.n == next) {
        int v = topology_get_turnout(tn);
        if (v) return 0;
        ctrl2_set_turnout(tn, 1);
        return 1;
    }
    return -1;
}

lsblk_num_t cauto_peek_next_lsblk(int tidx, train_ctrl_t *tvars)
{
    lsblk_num_t sret = {-1};
    if (tvars->_mode != train_auto) return sret;
    
    uint8_t r = tvars->route[tvars->routeidx];
    if (r & 0xC0) {
        // control should have been consumed already
        route_error(tidx, tvars);
        return sret;
    }
    if (_AR_WSTOP == r) return sret;
    sret.n = r;
    cauto_update_turnouts(tidx, tvars->c1_sblk, tvars->_dir, r);

    return sret;
}

static uint8_t events[8] = {0};

static void check_for_ctrl(int tidx, train_ctrl_t *tvars)
{
    if (tvars->_mode != train_auto) return;
    if (!tvars->route) return;
    for (;;) {
        uint8_t r = tvars->route[tvars->routeidx];
        if (0==(r & 0xC0)) return;
        if ((r & 0xC0)==0x80) {
            // speed
            int8_t v = ((tvars->route[tvars->routeidx] & 0x3F)<<2);
            if (v>100) v=100;
            if (v<-100) v=-100;
            _ctrl2_upcmd_set_desired_speed(tidx, tvars, v);
            tvars->routeidx++;
            continue;
        }
        // 1 1 x x x x x x x control
        if ((r & 0xF8) == _AR_TRGEVENT(0)) {
            int n = r & 0x07;
            events[n]++;
            tvars->routeidx++;
            continue;
        }
        if ((r & 0xF8) == _AR_WEVENT(0)) {
            int n = r & 0x07;
            if (events[n]) {
                events[n]--;
                tvars->routeidx++;
                continue;
            }
            return;
        }
        if (r == _AR_STPHALF) {
        	// KO : trig is sent to spdctl before C1C2 - which reset POSE
            ctrl2_upcmd_settrigU1_half(tidx, tvars);
            tvars->routeidx++;
            continue;
        }
        if (r == _AR_END) { // END
            route_end(tidx, tvars);
            return;
        }
        if (r == _AR_LOOP) { // LOOP
            tvars->routeidx = 0;
            continue;
        }
        itm_debug3(DBG_ERR|DBG_AUTO, "unkn route", tidx, tvars->routeidx, r);
        route_error(tidx, tvars);
    }
}


void cauto_had_stop(int tidx, train_ctrl_t *tvars)
{
    if (tvars->_mode != train_auto) return;
    uint8_t r = tvars->route[tvars->routeidx];
    if (r & 0xC0) {
        // control should have been consumed already
        route_error(tidx, tvars);
        return;
    }
    if (_AR_WSTOP != r) return;
    
    tvars->routeidx++;
    check_for_ctrl(tidx, tvars);
}

void cauto_had_trigU1(int tidx, train_ctrl_t *tvars)
{
	itm_debug1(DBG_AUTO, "TrigU1", tidx);
    _ctrl2_upcmd_set_desired_speed(tidx, tvars, 0);
}



void cauto_check_start(int tidx, train_ctrl_t *tvars)
{
    check_for_ctrl(tidx, tvars);
}

void cauto_c1_updated(int tidx, train_ctrl_t *tvars)
{
    if (tvars->_mode != train_auto) return;
    if ((tvars->route) &&  (tvars->_dir) && (0 == (tvars->route[tvars->routeidx] & 0xC0))) {
        int nsb = tvars->route[tvars->routeidx] & 0x7F;
        if (tvars->c1_sblk.n == nsb) {
            tvars->routeidx++;
            check_for_ctrl(tidx, tvars);
        } else {
            itm_debug3(DBG_AUTO|DBG_ERR, "bad c1", tidx, nsb, tvars->c1_sblk.n);
            route_error(tidx, tvars);
        }
    }
}
