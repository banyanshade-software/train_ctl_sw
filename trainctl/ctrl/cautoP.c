//
//  cautoP.c
//  train_throttle
//
//  Created by Daniel BRAUN on 13/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif

#error obsolete

#if 0 // to be completely refactor with LT

#include "ctrl.h"
#include "ctrlP.h"
#include "cautoP.h"


static void route_error(int tidx, train_oldctrl_t *tvars)
{
    itm_debug1(DBG_ERR|DBG_AUTO, "CA.ERR", tidx);
    ctrl2_set_mode(tidx, tvars, train_manual);
}

static void route_end(int tidx, train_oldctrl_t *tvars)
{
    itm_debug1(DBG_ERR|DBG_AUTO, "CA.END", tidx);
    //tvars->routeidx = 0;
    ctrl2_set_mode(tidx, tvars, train_manual);
}

int cauto_update_turnouts(_UNUSED_ int tidx, lsblk_num_t cur, uint8_t left, uint8_t next)
{
    lsblk_num_t ta, tb;
    xtrnaddr_t tn;
    next_lsblk_nums(cur, left, &ta, &tb, &tn);
    if (ta.n == next) {
        int v = topology_get_turnout(tn);
        if (!v) return 0;
        return ctrl2_set_turnout(tn, topo_tn_straight, tidx);
    }
    if (tb.n == next) {
        int v = topology_get_turnout(tn);
        if (v) return 0;
        return ctrl2_set_turnout(tn, topo_tn_moving, tidx);
    }
    return -1;
}

lsblk_num_t cauto_peek_next_lsblk(int tidx, train_oldctrl_t *tvars, uint8_t left, int nstep)
{
    lsblk_num_t sret = {-1};
    lsblk_num_t cur = tvars->c1_sblk;
    if (tvars->_mode != train_auto) return sret;
    
    if (nstep) cur.n = tvars->route[tvars->routeidx+nstep-1];
    if (cur.n & 0xC0) {
    	itm_debug3(DBG_ERR|DBG_AUTO, "peek/C", tidx, cur.n, nstep);
    	route_error(tidx, tvars);
    	return sret;
    }
    uint8_t r = tvars->route[tvars->routeidx+nstep];
    if (r & 0xC0) {
        // control should have been consumed already
        //route_error(tidx, tvars);
        return sret;
    }
    if (_AR_WSTOP == r) return sret;
    sret.n = r;
    itm_debug3(DBG_AUTO, "ca.upd_to", tidx, nstep, r);
    int rc = cauto_update_turnouts(tidx, cur, left, r);
    if ((0) && rc) {
    	// still return possible next lsblk, it will just be detected as alternate
    	lsblk_num_t snone = {-1};
    	return snone;
    }
    return sret;
}

static uint8_t events[8] = {0};

//extern train_ctrl_t *tvars;

static void ar_ext(_UNUSED_ int tidx, _UNUSED_ train_oldctrl_t *tvars, uint8_t c1, _UNUSED_ uint8_t c2)
{
	switch (c1) {
	case _ARX_CLR_EVENT:
		memset(events, 0, sizeof(events));
		break;
	default:
		break;
	}
}

static void check_for_ctrl(int tidx, train_oldctrl_t *tvars)
{
    if (tvars->_mode != train_auto) return;
    if (!tvars->route) return;
    for (;;) {
        uint8_t r = tvars->route[tvars->routeidx];
        if (0==(r & 0xC0)) return;
        if (IS_AR_SPD(r)) {
            // speed
            int8_t v = ((tvars->route[tvars->routeidx] & 0x3F)<<2);
    		itm_debug2(DBG_AUTO, "ca.spd", tidx, v);
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
    		itm_debug3(DBG_AUTO, "ca.trig", tidx, n, events[n]);
            tvars->routeidx++;
            topology_updated(tidx);
            continue;
        }
        if ((r & 0xF8) == _AR_WEVENT(0)) {
            int n = r & 0x07;
            if (events[n]) {
        		itm_debug2(DBG_AUTO, "ca.evt ok", tidx, n);
                events[n]--;
                tvars->routeidx++;
                continue;
            }
    		itm_debug2(DBG_AUTO, "ca.evt w", tidx, n);
            return;
        }
        if (r == _AR_WTIMER) {
        	if (tvars->got_texp) {
        		itm_debug2(DBG_AUTO, "ca.tim ok", tidx, tvars->routeidx);
        		tvars->got_texp = 0;
        		tvars->routeidx++;
        		continue;
        	}
    		itm_debug2(DBG_AUTO, "ca.tim w", tidx, tvars->routeidx);
        	return;
        }
        if (r == _AR_WTRG_U1) {
            if (tvars->got_u1) {
                itm_debug2(DBG_AUTO, "ca.wu1 ok", tidx, tvars->routeidx);
                tvars->got_u1 = 0;
                tvars->routeidx++;
                continue;
            }
            itm_debug2(DBG_AUTO, "ca.wu1 w", tidx, tvars->routeidx);
            return;
        }
        if ((r & 0xF8) == _AR_TIMER(0)) {
        	int t = (1<<(r & 0x07)); // in seconds
        	itm_debug3(DBG_AUTO, "ca.timer", tidx, r & 0x07, t);
        	ctrl_set_timer(tidx, tvars, TAUTO, t*1000);
        	tvars->routeidx++;
        	continue;
        }
        if (r == _AR_LED) {
            tvars->routeidx++;
            uint8_t led_num = tvars->route[tvars->routeidx++];
            uint8_t prog_num = tvars->route[tvars->routeidx++];
            ctrl2_send_led(led_num, prog_num);
            continue;
        }
        if (r == _AR_TRG_LIM) {
        	// cannot call ctrl2_upcmd_settrigU1() now
        	// trig would b sent to spdctl before C1C2 - which reset POSE
        	// thus we set stpmiddle and handle it in cauto_end_tick()
    		itm_debug1(DBG_AUTO, "ca.trglim", tidx);
        	tvars->trigu1 = 4;
            tvars->routeidx++;
            continue;
        }
        if (r == _AR_TRG_END) {
            itm_debug1(DBG_AUTO, "ca.trgend", tidx);
            tvars->trigu1 = 2;
            tvars->routeidx++;
            continue;
        }
        if (r == _AR_TRG_TLEN) {
        	itm_debug1(DBG_AUTO, "ca.trglen", tidx);
        	tvars->trigu1 = 3;
        	tvars->routeidx++;
        	continue;
        }
        if (r == _AR_END) { // END
    		itm_debug1(DBG_AUTO, "ca.end", tidx);
            route_end(tidx, tvars);
            return;
        }
        if (r == _AR_LOOP) { // LOOP
    		itm_debug1(DBG_AUTO, "ca.loop", tidx);
            tvars->routeidx = 0;
            continue;
        }
        if (r == _AR_DBG) {
        	itm_debug1(DBG_AUTO, "heho", tidx);
            tvars->routeidx++;
            continue;
        }
        if (r == _AR_EXT) {
        	ar_ext(tidx, tvars, tvars->route[tvars->routeidx+1], tvars->route[tvars->routeidx+2]);
        	tvars->routeidx += 3;
        	continue;
        }
        itm_debug3(DBG_ERR|DBG_AUTO, "ca.unkn", tidx, tvars->routeidx, r);
        route_error(tidx, tvars);
        break;
    }
}


void cauto_had_stop(int tidx, train_oldctrl_t *tvars)
{
    if (tvars->_mode != train_auto) return;
    uint8_t r = tvars->route[tvars->routeidx];
    if (r & 0xC0) {
        // control should have been consumed already
        //route_error(tidx, tvars);
        return;
    }
    if (_AR_WSTOP != r) return;
    itm_debug3(DBG_ERR|DBG_AUTO, "ca.wstp ok", tidx, tvars->routeidx, r);

    tvars->routeidx++;
    check_for_ctrl(tidx, tvars);
}

void cauto_had_trigU1(int tidx, train_oldctrl_t *tvars)
{
	itm_debug1(DBG_AUTO, "ca.trigU1", tidx);
    //_ctrl2_upcmd_set_desired_speed(tidx, tvars, 0);
    tvars->got_u1 = 1;
    topology_updated(tidx);
}

void cauto_had_timer(_UNUSED_ int tidx, train_oldctrl_t *tvars)
{
	itm_debug1(DBG_AUTO, "ca.had_tim", tidx);
	tvars->got_texp = 1;
    topology_updated(tidx);
}


void cauto_check_start(int tidx, train_oldctrl_t *tvars)
{
	itm_debug1(DBG_AUTO, "ca.chst", tidx);
    check_for_ctrl(tidx, tvars);
}

void cauto_c1_updated(int tidx, train_oldctrl_t *tvars)
{
	itm_debug1(DBG_AUTO, "ca.c1", tidx);
    if (tvars->_mode != train_auto) return;
    if (!tvars->route) return;
    if (!tvars->_dir) return;
    if (0 != (tvars->route[tvars->routeidx] & 0xC0)) return;

    /*
    int nsb = tvars->route[tvars->routeidx] & 0x7F;
    if (tvars->c1_sblk.n == nsb) {
    	// normal case : new c1 is what we expected
    	itm_debug2(DBG_AUTO, "ca.c1", tidx, nsb);
    	tvars->routeidx++;
    	check_for_ctrl(tidx, tvars);
    	return;
    }*/
    for (int i=0; i<3; i++) {
    	if (0 != (tvars->route[tvars->routeidx+i] & 0xC0)) break;
    	int nsb = tvars->route[tvars->routeidx+i] & 0x7F;
    	if (tvars->c1_sblk.n == nsb) {
    		tvars->routeidx += i+1;
    		check_for_ctrl(tidx, tvars);
    		return;
    	}
    }
    itm_debug2(DBG_AUTO|DBG_ERR, "ca.c1 bad", tidx, tvars->c1_sblk.n);
    route_error(tidx, tvars);

}

void cauto_end_tick(int tidx, train_oldctrl_t *tvars)
{
	itm_debug2(DBG_AUTO, "ca.tick", tidx, tvars->trigu1);
	if (tvars->trigu1) {
        ctrl2_upcmd_settrigU1(tidx, tvars, tvars->trigu1);
        tvars->trigu1 = 0;
	}
	check_for_ctrl(tidx, tvars);
}


#endif
