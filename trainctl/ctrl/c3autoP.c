//
//  c3autoP.c
//  train_throttle
//
//  Created by Daniel Braun on 11/03/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//



#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif


#include "ctrl.h"
#include "ctrlLT.h"

#include "c3autoP.h"

#include "../utils/ihm_messages.h"
#define IHM_MSG(_m, _s, _v) do { ihm_message(mqf_write_from_ctrl, _m, _s, _v); } while(0)

#ifndef __clang__
static_assert(sizeof(cauto_path_items_t) == 2);
#else
typedef char compile_assert_c3[(sizeof(cauto_path_items_t) == 2) ? 1 : -1];
#endif


typedef struct st_path {
    uint16_t cidx;
    int8_t spd;
    uint8_t delay;
    uint8_t brake_end:1;
    cauto_path_items_t path[C3AUTO_NUM_PATH_ITEM];
} cauto_vars_t;

cauto_vars_t c3avar[MAX_TRAINS];

static int is_eop(const cauto_path_items_t *p)
{
    if (p->t) return 0;
    if (p->val==0) return 1;
    return 0;
}

void c3auto_start(int tidx)
{
    IHM_MSG(MSG_AUTO_START, tidx, 0);
    c3avar[tidx].cidx = 0;
    c3avar[tidx].spd = 0;
}

static void _update_sblk(int tidx, int oldidx, lsblk_num_t prevlsb)
{
    int8_t spd2 = c3avar[tidx].path[oldidx].val;
    int8_t ospd = c3avar[tidx].spd;
    if (spd2 != ospd) {
        if (!spd2) {
            // this is eop
        	itm_debug2(DBG_AUTO, "Aeop", tidx, c3avar[tidx].path[oldidx].sblk.n);
            if (!is_eop(&c3avar[tidx].path[oldidx])) {
                itm_debug3(DBG_ERR|DBG_AUTO, "eop??", tidx, c3avar[tidx].spd, c3avar[tidx].path[oldidx].sblk.n);
            }
            itm_debug2(DBG_AUTO, "brkeop", tidx, oldidx);
            ctrl_set_brake_on_back(tidx, prevlsb.n);
            c3avar[tidx].brake_end = 1;
            return;
        }
        if (spd2 && ospd && (SIGNOF(spd2) != SIGNOF((ospd)))) {
            // change direction
        	itm_debug3(DBG_AUTO, "chgdir", tidx, ospd, spd2);
            c3avar[tidx].brake_end = 0;
            ctrl_delayed_set_desired_spd(tidx, 0);
        }
        ctrl_delayed_set_desired_spd(tidx, spd2*2);
        c3avar[tidx].spd = spd2;
    }
}
void c3auto_set_s1(int tidx, lsblk_num_t s1)
{
	itm_debug2(DBG_AUTO, "Ac1", tidx, s1.n);
    int idx = c3avar[tidx].cidx;
    lsblk_num_t prevlsb = { .n = -1 };
    for (;;idx++) {
        if (c3avar[tidx].path[idx].t) continue;
        if (-1 == prevlsb.n) {
            prevlsb = c3avar[tidx].path[idx].sblk;
        }
        if (c3avar[tidx].path[idx].sblk.n == s1.n) {
            c3avar[tidx].cidx = idx;
            _update_sblk(tidx, idx, prevlsb);
            return;
        }

        if (is_eop(&c3avar[tidx].path[idx])) {
            if ((0)) {
                FatalError("nos1", "S1 not found in path", Error_AutoNoS1);
                ctrl_set_mode(tidx, train_manual);
            } else {
                itm_debug2(DBG_ERR|DBG_AUTO, "S1 not in path", tidx, s1.n);
            }
            return;
        }
    }
	FatalError("Ac1", "c3auto_set_s1", Error_AutoSetS1);
}

void c3auto_freeback(int tidx, lsblk_num_t freelsblk)
{
	itm_debug2(DBG_AUTO, "Afreebk", tidx, freelsblk.n);
    int idx = c3avar[tidx].cidx;
    if (!c3avar[tidx].path[idx].t && !c3avar[tidx].path[idx+1].t) {
        int d1 = SIGNOF0(c3avar[tidx].path[idx].val);
        int t =  d1 * SIGNOF0(c3avar[tidx].path[idx+1].val);
        if (t<0) {
            // this is direction change
            lsblk_num_t prev = next_lsblk(c3avar[tidx].path[idx].sblk, (d1>0), NULL);
            itm_debug3(DBG_AUTO, "Achg", tidx, freelsblk.n, prev.n);
            if (prev.n == freelsblk.n) {
                // all train is now on current sblk (and next sblk)
                // stop to go to station mode
                if (c3avar[tidx].spd) IHM_MSG(MSG_AUTO_CHDIR, tidx, 0);
                c3avar[tidx].brake_end = 0;
                ctrl_delayed_set_desired_spd(tidx, 0);
                c3avar[tidx].spd = 0;
            }
        }
    }
}

void c3auto_station(int tidx)
{
    if (c3avar[tidx].brake_end) {
        IHM_MSG(MSG_AUTO_DONE, tidx, 0);
        itm_debug1(DBG_AUTO, "Adone", tidx);
        ctrl_set_mode(tidx, train_manual);
        return;
    }
    if (c3avar[tidx].spd) {
        // not triggered by us
        IHM_MSG(MSG_AUTO_UNEXPSTOP, tidx, 0);
        itm_debug2(DBG_AUTO|DBG_ERR, "station?", tidx, c3avar[tidx].spd);
        return;
    }
    c3avar[tidx].delay = 100;
}

static void _c3auto_station_delayexpired(int tidx)
{
    int idx = c3avar[tidx].cidx;
    if (!c3avar[tidx].path[idx].t && !c3avar[tidx].path[idx+1].t) {
        int d1 = SIGNOF0(c3avar[tidx].path[idx].val);
        int t =  d1 * SIGNOF0(c3avar[tidx].path[idx+1].val);
        if (t<0) {
            // this is direction change
            c3avar[tidx].cidx++;
            _update_sblk(tidx, idx+1, c3avar[tidx].path[idx].sblk);
        }
    }
}

void c3auto_tick(int tidx)
{
    if (!c3avar[tidx].delay) return;
    c3avar[tidx].delay --;
    if (!c3avar[tidx].delay) {
        _c3auto_station_delayexpired(tidx);
    }
}
void c3auto_set_turnout(int tidx, xtrnaddr_t tn)
{
    int idx = c3avar[tidx].cidx;
    for (;;idx++) {
        if (is_eop(&c3avar[tidx].path[idx])) {
            if ((0)) {
                FatalError("notn", "TN not found in path", Error_AutoNoTN);
                ctrl_set_mode(tidx, train_manual);
            }
            itm_debug2(DBG_ERR|DBG_AUTO, "TN not in path", tidx, tn.v);
            return;
        }

        if (0==c3avar[tidx].path[idx].t) continue;
        if (c3avar[tidx].path[idx].tn.v == tn.v) {
            enum topo_turnout_state cs = topology_get_turnout(tn);
            enum topo_turnout_state nv = c3avar[tidx].path[idx].val ? topo_tn_turn : topo_tn_straight;
            if (nv != cs) {
                ctrl_set_turnout(tn, c3avar[tidx].path[idx].val ? topo_tn_turn : topo_tn_straight, tidx);
            }
            return;
        }
    }
}

cauto_path_items_t *c3auto_get_path(int numtrain)
{
    return c3avar[numtrain].path;
}



#ifdef TRAIN_SIMU
void c3auto_dump(cauto_path_items_t *p)
{
    for (;;) {
        if (p->t) {
            printf("tn %d pos %d --", p->tn.v, p->val);
        } else {
            printf("sblk %d spd %d --", p->sblk.n, p->val*2);
            if (p->val ==0) break;
        }
        p++;
    }
    printf("\n");
}
#endif
