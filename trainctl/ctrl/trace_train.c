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
#include "../utils/framing.h"


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
    uint8_t pow2future;
} __attribute__((packed)) train_trace_tick_record_t;

typedef struct {
    pose_trig_tag_t tag;
    int32_t oldpose;
    int32_t adjustedpose;
    uint8_t c1;
    uint8_t ignore;
    uint8_t sn;
    uint8_t futc2;
} __attribute__((packed)) train_trace_trig_record_t;

typedef struct {
    int8_t sblk;
    int8_t canton;
} __attribute__((packed)) train_trace_free_record_t;

typedef struct {
    int8_t sblk;
    int8_t canton;
} __attribute__((packed)) train_trace_simu_record_t;

typedef struct {
    int8_t inanum;
    int8_t inaon;
} __attribute__((packed)) train_trace_ina_record_t;


typedef struct {
    lsblk_num_t c1lblk;
    lsblk_num_t oldc1;
    uint32_t curposmm;
    uint8_t org;
} __attribute__((packed)) train_trace_setc1_record_t;


typedef struct {
    int on;
} __attribute__((packed)) train_trace_brake_record_t;


typedef enum {
    trace_kind_tick = 1,
    trace_kind_trig,
    trace_kind_trig_set,
    trace_kind_free,
    trace_kind_simu,
    trace_kind_ina,
    trace_kind_setc1,
    trace_kind_brake,
} __attribute__((packed)) trace_rec_kind_t;

typedef struct {
    uint32_t tick;
    trace_rec_kind_t kind;
    union {
        train_trace_tick_record_t tickrec;
        train_trace_trig_record_t trigrec;
        train_trace_free_record_t freerec;
        train_trace_simu_record_t simurec;
        train_trace_ina_record_t  inarec;
        train_trace_setc1_record_t setc1rec;
        train_trace_brake_record_t brake;
    } __attribute__((packed));
} __attribute__((packed)) train_trace_record_t;



#define NUM_TRACE_ITEM 100

typedef struct {
    int nextidx;
    int sidx;
    train_trace_record_t rec[NUM_TRACE_ITEM];
} train_trace_t;

#define NUM_TRAIN_TRACE 2

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
    rec->tickrec.pow2future = tvars->pow_c2_future.v;

    
    if (lrec && (lrec->kind == trace_kind_tick)) {
        if (!memcmp(&lrec->tickrec, &rec->tickrec, sizeof(train_trace_tick_record_t))) {
            cancel_rec(tidx);
        } else {
            //itm_debug1(DBG_CTRL, "hop", 0);
        }
    }
    
}

void _trace_train_setc1(uint32_t tick, int tidx, train_ctrl_t *tvars, lsblk_num_t newc1, int org)
{
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_setc1;
    rec->setc1rec.c1lblk = newc1;
    rec->setc1rec.oldc1 = tvars->c1_sblk;
    rec->setc1rec.curposmm = tvars->_curposmm;
    rec->setc1rec.org = org;
}

void _trace_train_trig(uint32_t tick, int tidx, _UNUSED_ train_ctrl_t *tvars, uint8_t sn, pose_trig_tag_t tag, int32_t oldpos, int32_t adjutedpos, int ignc)
{
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_trig;
    rec->trigrec.tag = tag;
    rec->trigrec.oldpose = oldpos;
    rec->trigrec.adjustedpose = adjutedpos;
    rec->trigrec.c1 = tvars->c1_sblk.n;
    rec->trigrec.ignore = ignc;
    rec->trigrec.sn = sn;
}


void _trace_train_trig_set(uint32_t tick, int tidx, _UNUSED_ train_ctrl_t *tvars, uint8_t sn, pose_trig_tag_t tag, int32_t pos, uint8_t fut, int ignore)
{
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_trig_set;
    rec->trigrec.tag = tag;
    rec->trigrec.adjustedpose = pos;
    rec->trigrec.sn = sn;
    rec->trigrec.ignore = ignore ? 1 : 0;
    rec->trigrec.futc2 = fut;
    if ((tag_reserve_c2 == tag) && (0xFF==fut)) {
        abort();
    }
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

void _trace_train_ina3221(uint32_t tick, int tidx, int lsegnum, int on)
{
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_ina;
    rec->inarec.inanum = lsegnum;
    rec->inarec.inaon = on;
}

void _trace_train_brake(uint32_t tick, int tidx,  train_ctrl_t *tvars, int on)
{
    train_trace_record_t *rec = get_newrec(tidx);
    if (!rec) return;
    rec->tick = tick;
    rec->kind = trace_kind_brake;
    rec->brake.on = on;
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
        case tag_brake_user:    return "brake_U";
        case tag_free_back:     return "free_back";
        case tag_auto_u1:       return "u1";
        case tag_leave_canton:  return "leave";

        default:
            return "???";
    }
}


static void _trace_train_dump(train_trace_record_t *t, int numitem, int startidx, int h);

void trace_train_dump(int tidx)
{
    if (tidx>=NUM_TRAIN_TRACE) return;
    train_trace_t *t = &trace[tidx];
    _trace_train_dump(t->rec, NUM_TRACE_ITEM, t->nextidx, 0);
}

void trace_train_dumphtml(int tidx)
{
    if (tidx>=NUM_TRAIN_TRACE) return;
    train_trace_t *t = &trace[tidx];
    _trace_train_dump(t->rec, NUM_TRACE_ITEM, t->nextidx, 1);
}

void trace_train_dumpbuf(void *buf, int numbytes)
{
    if (numbytes % sizeof(train_trace_record_t)) {
        printf("problem here\n");
    }
    _trace_train_dump((train_trace_record_t *)buf, numbytes/sizeof(train_trace_record_t), 0, 1);
}

static const char *_c1orgstr(int org)
{
    switch (org) {
        case 0: return "trig";
        case 1: return "ina";
        case 2: return "evtc2";
        default:
            return "???";
            break;
    }
}

static const char *trig_ignore_cause(int ignc)
{
    switch (ignc) {
        case 1: return "bad postag";
        case 2: return "wrong sblk";
        case 3: return "already done";
        case 4: return "not found";
            
        default:
            return "???";
            break;
    }
}
static void _trace_train_dump(train_trace_record_t *records, int numitem, int startidx, int h)
{
    int f = 1;
    const char *_ds = h ? "<i>" : "";
    const char *_eds = h ? "</i>" : "";
    const char *_ss = h ? "<b>" : "";
    const char *_ess = h ? "</b>" : "";
    const char *_cr = h ? "<br/>" : "\n";
    char line[256];
    line[0]='\0';
    printf("offset : %lu\n", offsetof(train_trace_record_t, tickrec.c1lblk));
    printf("offset : %lu\n", offsetof(train_trace_record_t, tickrec.sdir));
    printf("offset : %lu\n", offsetof(train_trace_record_t, tickrec.beginposmm));
    for (int i=0; i<numitem; i++) {
        int idx = (i+startidx) % numitem;
        train_trace_record_t *rec = &records[idx];
        if (f && !rec->tick) continue;
        f = 0;
        switch (rec->kind) {
            case trace_kind_tick:
                sprintf(line, "%s%2d %6.6d%s      state=%s%-9s%s dir=%d sblk=%d spd=%3d dspd=%3d pos=%d from %d -- pow (%d) %d %d (fut %d)%s",
                       _ds, idx, rec->tick, _eds,
                       _ss, state_name(rec->tickrec.state), _ess,
                       rec->tickrec.sdir,
                       rec->tickrec.c1lblk.n,
                       rec->tickrec._target_unisgned_speed,
                       rec->tickrec._desired_signed_speed,
                       rec->tickrec.curposmm,
                       rec->tickrec.beginposmm,
                       rec->tickrec.oldc1,
                       rec->tickrec.c1,
                       rec->tickrec.c2,
                       rec->tickrec.pow2future,
					   _cr);
                break;
            case trace_kind_trig:
                if (rec->trigrec.ignore) {
                    sprintf(line, "%2d %6.6d ign trig sn=%d   %-9s pos=%d->%d (%s)%s",
                            idx, rec->tick,
                            rec->trigrec.sn,
                            trig_name(rec->trigrec.tag),
                            rec->trigrec.oldpose,
                            rec->trigrec.adjustedpose,
                            trig_ignore_cause(rec->trigrec.ignore),
                            _cr);
                } else {
                    sprintf(line, "%2d %6.6d TRIG   sn=%d %-9s pos=%d->%d%s",
                            idx, rec->tick,
                            rec->trigrec.sn,
                            trig_name(rec->trigrec.tag),
                            rec->trigrec.oldpose,
                            rec->trigrec.adjustedpose,
                            _cr);
                }
                break;
            case trace_kind_trig_set:
                sprintf(line, "%2d %6.6d %sset  sn=%d %-9s pos=%d (sblk %d) (fut %d)%s",
                        idx, rec->tick,
                        rec->trigrec.ignore ? "--" : "++",
                        rec->trigrec.sn,
                        trig_name(rec->trigrec.tag),
                        rec->trigrec.adjustedpose,
                        rec->trigrec.c1,
                        rec->trigrec.futc2,
					   _cr);
                break;
            case trace_kind_free:
                sprintf(line, "%2d %6.6d free sblk=%d canton=%d%s",
                       idx, rec->tick,
                       rec->freerec.sblk, rec->freerec.canton,
					   _cr);
                break;
            case trace_kind_simu:
                sprintf(line, "%2d %6.6d ======== simu: sblk=%d canton=%d%s",
                       idx, rec->tick,
                       rec->simurec.sblk, rec->simurec.canton,
					   _cr);
                break;
            case trace_kind_ina:
                sprintf(line, "%2d %6.6d ------- %sina %d %s%s%s",
                        idx, rec->tick,
                        _ds,
                        rec->inarec.inanum,
                        rec->inarec.inaon ? "ON" : "off",
                        _eds,
                        _cr);
                break;
            case trace_kind_setc1:
                sprintf(line, "%2d %6.6d setc1 %d->%d pos=%d org=%s%s",
                        idx, rec->tick,
                        rec->setc1rec.oldc1.n, rec->setc1rec.c1lblk.n,
                        rec->setc1rec.curposmm, _c1orgstr(rec->setc1rec.org),
                        _cr);
                break;
            case trace_kind_brake:
                sprintf(line, "%2d %6.6d %s%s",
                        idx, rec->tick,
                        rec->brake.on ? "BRAKE on" : "brake off",
                        _cr);
                break;
            default:
                break;
        }
        if (h) __trace_train_append_line(line);
        else   fputs(line, stdout);
    }

}


#endif // TRAIN_SIMU

#ifndef TRAIN_SIMU
void frame_send_trace(_UNUSED_ void(*cb)(const uint8_t *d, int l), _UNUSED_ int train)
{
	if (train >= NUM_TRAIN_TRACE) {
		return;
	}
    train_trace_t *t = &trace[train];
    int f = 1;
    int tni = t->nextidx;
    for (int i=0; i<NUM_TRACE_ITEM; i++) {
        int idx = (i+tni) % NUM_TRACE_ITEM;
        train_trace_record_t *rec = &t->rec[idx];
        if (f && !rec->tick) continue;
        f = 0;
		uint8_t buf[32];
		int l = txrx_frm_escape2(buf, (uint8_t *)rec, sizeof(train_trace_record_t), sizeof(buf));
        cb(buf, l);
    }
}
#endif // !TRAIN_SIMU
#endif // trace_enable
