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


#ifndef __clang__
static_assert(sizeof(cauto_path_items_t) == 2);
#else
typedef char compile_assert_c3[(sizeof(cauto_path_items_t) == 2) ? 1 : -1];
#endif


typedef struct st_path {
    uint16_t cidx;
    int8_t spd;
    uint8_t delay;
    cauto_path_items_t path[C3AUTO_NUM_PATH_ITEM];
} cauto_vars_t;

cauto_vars_t c3avar[MAX_TRAINS];

static int is_eop(cauto_path_items_t *p)
{
    if (p->t) return 0;
    if (p->val==0) return 1;
    return 0;
}

void c3auto_start(int tidx)
{
    c3avar[tidx].cidx = 0;
    c3avar[tidx].spd = 0;
}

static void _update_sblk(int tidx, int oldidx)
{
    int8_t spd2 = c3avar[tidx].path[oldidx].val;
    int8_t ospd = c3avar[tidx].spd;
    if (spd2 != ospd) {
        if (spd2 && ospd && (SIGNOF(spd2) != SIGNOF((ospd)))) {
            // change direction
            ctrl_delayed_set_desired_spd(tidx, 0);
        }
        ctrl_delayed_set_desired_spd(tidx, spd2*2);
        c3avar[tidx].spd = spd2;
    }
}
void c3auto_set_s1(int tidx, lsblk_num_t s1)
{
    int idx = c3avar[tidx].cidx;
    for (;;idx++) {
        if (c3avar[tidx].path[idx].t) continue;
        if (c3avar[tidx].path[idx].sblk.n == s1.n) {
            c3avar[tidx].cidx = idx;
            _update_sblk(tidx, idx);
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
	FatalError("Ac1", "c3auto_set_s1", Error_Abort);
}

void c3auto_freeback(int tidx, lsblk_num_t freelsblk)
{
    int idx = c3avar[tidx].cidx;
    if (!c3avar[tidx].path[idx].t && !c3avar[tidx].path[idx+1].t) {
        int d1 = SIGNOF0(c3avar[tidx].path[idx].val);
        int t =  d1 * SIGNOF0(c3avar[tidx].path[idx+1].val);
        if (t<0) {
            // this is direction change
            lsblk_num_t prev = next_lsblk(c3avar[tidx].path[idx].sblk, (d1>0), NULL);
            if (prev.n == freelsblk.n) {
                // all train is now on current sblk (and next sblk)
                // stop to go to station mode
                ctrl_delayed_set_desired_spd(tidx, 0);
                c3avar[tidx].spd = 0;
            }
        }
    }
}

void c3auto_station(int tidx)
{
    if (c3avar[tidx].spd) {
        // not triggered by us
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
            _update_sblk(tidx, idx+1);
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
