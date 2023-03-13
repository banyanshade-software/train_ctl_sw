//
//  trace_train.c
//  train_throttle
//
//  Created by Daniel Braun on 11/01/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//


#include <stdint.h>


#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

#include "../config/conf_train.h"

#include "ctrl.h"

#include "trace_train.h"

#if trace_train_enable == 0
void _trace_train_postick(uint32_t tick, int tidx, train_ctrl_t *tvars)
{
    
}

void _trace_train_trig(uint32_t tick, int tidx, train_ctrl_t *tvars, pose_trig_tag_t tag, int32_t pos)
{
    
}

#else

typedef struct {
    lsblk_num_t c1lblk;
    int8_t sdir;
    uint32_t beginposmm;
    uint32_t curposmm;
    train_state_t state;
    int16_t         _desired_signed_speed;
    uint16_t        _target_unisgned_speed;
    uint8_t oldc1;
    uint8_t c1;
    uint8_t c2;
} train_trace_tick_record_t;

typedef struct {
    pose_trig_tag_t tag;
    int32_t oldpose;
    int32_t adjustedpose;
} train_trace_trig_record_t;

typedef struct {
    int8_t sblk;
    int8_t canton;
} train_trace_free_record_t;

typedef struct {
    int8_t sblk;
    int8_t canton;
} train_trace_simu_record_t;

typedef enum {
    trace_kind_tick = 1,
    trace_kind_trig,
    trace_kind_trig_set,
    trace_kind_free,
    trace_kind_simu,
} trace_rec_kind_t;

typedef struct {
    uint32_t tick;
    trace_rec_kind_t kind;
    union {
        train_trace_tick_record_t tickrec;
        train_trace_trig_record_t trigrec;
        train_trace_free_record_t freerec;
        train_trace_simu_record_t simurec;
    };
} train_trace_record_t;

#define NUM_TRACE_ITEM 100

typedef struct {
    int nextidx;
    int sidx;
    train_trace_record_t rec[NUM_TRACE_ITEM];
} train_trace_t;

#define NUM_TRAIN_TRACE 1

static train_trace_t trace[NUM_TRAIN_TRACE];

static train_trace_record_t *get_newrec(int tidx)
{
    if (tidx>=NUM_TRAIN_TRACE) return NULL;
    train_trace_record_t *rec = &trace[tidx].rec[trace[tidx].nextidx];
    trace[tidx].sidx = trace[tidx].nextidx;
    trace[tidx].nextidx++;
    trace[tidx].nextidx %= NUM_TRACE_ITEM;
    
    memset(rec, 0, sizeof(*rec));
    return rec;
}
static train_trace_record_t *get_lastrec(int tidx)
{
    if (tidx>=NUM_TRAIN_TRACE) return NULL;
    train_trace_t *t = &trace[tidx];
    if (t->sidx == t->nextidx) return NULL;
    train_trace_record_t *rec = &t->rec[t->sidx];
    return rec;
}

static void cancel_rec(int tidx)
{
    if (tidx>=NUM_TRAIN_TRACE) return;
    train_trace_t *t = &trace[tidx];
    t->rec[t->sidx].tick = 0;
    t->nextidx = t->sidx;
    t->sidx = (t->nextidx+NUM_TRACE_ITEM-1) % NUM_TRACE_ITEM;
}

void _trace_train_postick(uint32_t tick, int tidx, train_ctrl_t *tvars)
{
    train_trace_record_t *lrec = get_lastrec(tidx);
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_tick;
    rec->tickrec.c1lblk = tvars->c1_sblk;
    rec->tickrec.sdir = tvars->_sdir;
    rec->tickrec.beginposmm = tvars->beginposmm;
    rec->tickrec.curposmm = tvars->_curposmm;
    rec->tickrec.state = tvars->_state;
    rec->tickrec._desired_signed_speed = tvars->_desired_signed_speed;
    rec->tickrec._target_unisgned_speed = tvars->_target_unisgned_speed;
    rec->tickrec.c1 = tvars->can1_xaddr.v;
    rec->tickrec.c2 = tvars->can2_xaddr.v;
    rec->tickrec.oldc1 = tvars->canOld_xaddr.v;

    if (lrec && (lrec->kind == trace_kind_tick)) {
        if (!memcmp(&lrec->tickrec, &rec->tickrec, sizeof(train_trace_tick_record_t))) {
            cancel_rec(tidx);
        } else {
            itm_debug1(DBG_CTRL, "hop", 0);
        }
    }
    
}

void _trace_train_trig(uint32_t tick, int tidx, _UNUSED_ train_ctrl_t *tvars, pose_trig_tag_t tag, int32_t oldpos, int32_t adjutedpos)
{
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_trig;
    rec->trigrec.tag = tag;
    rec->trigrec.oldpose = oldpos;
    rec->trigrec.adjustedpose = adjutedpos;
}


void _trace_train_trig_set(uint32_t tick, int tidx, _UNUSED_ train_ctrl_t *tvars, pose_trig_tag_t tag, int32_t pos)
{
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_trig_set;
    rec->trigrec.tag = tag;
    rec->trigrec.adjustedpose = pos;
}

void _trace_train_free(uint32_t tick, int tidx, int sblk, int canton)
{
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_free;
    rec->freerec.sblk = sblk;
    rec->freerec.canton = canton;
}

void _trace_train_simu(uint32_t tick, int tidx, int sblk, int canton)
{
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_simu;
    rec->simurec.sblk = sblk;
    rec->simurec.canton = canton;
}

#ifdef TRAIN_SIMU

static const char *state_name(train_state_t st)
{
    switch (st) {
        case train_state_off:           return "off";
        case train_state_running:       return "running";
        case train_state_station:       return "station";
        case train_state_blkwait:       return "blkwait";
        case train_state_blkwait0:      return "blkwait0";
        case train_state_end_of_track:  return "eot";
        case train_state_end_of_track0: return "eot0";
        default:
            return "???";
            break;
    }
}

static const char *trig_name(pose_trig_tag_t tag)
{
    switch (tag) {
        case tag_invalid :      return "INVALID";
        case tag_end_lsblk:     return "end_lsblk";
        case tag_need_c2:       return "need_c2";
        case tag_reserve_c2:    return "reserve_c2";
        case tag_stop_blk_wait: return "blk_wait";
        case tag_stop_eot:      return "stop_eot";
        case tag_chkocc:        return "chkocc";
        case tag_brake:         return "brake";
        case tag_free_back:     return "free_back";
        case tag_auto_u1:       return "u1";
        case tag_leave_canton:  return "leave";

        default: return "???";
    }
}
#endif


void trace_train_dump(int tidx)
{
#ifdef TRAIN_SIMU
    if (tidx>=NUM_TRAIN_TRACE) return;
    train_trace_t *t = &trace[tidx];

    int f = 1;
    for (int i=0; i<NUM_TRACE_ITEM; i++) {
        int idx = (i+t->nextidx) % NUM_TRACE_ITEM;
        train_trace_record_t *rec = &t->rec[idx];
        if (f && !rec->tick) continue;
        f = 0;
        switch (rec->kind) {
            case trace_kind_tick:
                printf("%2d %6.6d      state=%-9s dir=%d sblk=%d spd=%3d dspd=%3d pos=%d from %d -- pow %d %d %d\n",
                       idx, rec->tick,
                       state_name(rec->tickrec.state),
                       rec->tickrec.sdir,
                       rec->tickrec.c1lblk.n,
                       rec->tickrec._target_unisgned_speed,
                       rec->tickrec._desired_signed_speed,
                       rec->tickrec.curposmm,
                       rec->tickrec.beginposmm,
                       rec->tickrec.oldc1,
                       rec->tickrec.c1,
                       rec->tickrec.c2);
                break;
            case trace_kind_trig:
                printf("%2d %6.6d TRIG   %-9s pos=%d->%d\n",
                       idx, rec->tick,
                       trig_name(rec->trigrec.tag),
                       rec->trigrec.oldpose,
                       rec->trigrec.adjustedpose);
                break;
            case trace_kind_trig_set:
                printf("%2d %6.6d --set  %-9s pos=%d\n",
                       idx, rec->tick,
                       trig_name(rec->trigrec.tag),
                       rec->trigrec.adjustedpose);
                break;
            case trace_kind_free:
                printf("%2d %6.6d free sblk=%d canton=%d\n",
                       idx, rec->tick,
                       rec->freerec.sblk, rec->freerec.canton);
                break;
            case trace_kind_simu:
                printf("%2d %6.6d ======== simu: sblk=%d canton=%d\n",
                       idx, rec->tick,
                       rec->simurec.sblk, rec->simurec.canton);
                break;
            default:
                break;
        }
    }
#else
    // target trace dump, TODO
    (void)tidx; // unused for now
#endif
}

#endif
