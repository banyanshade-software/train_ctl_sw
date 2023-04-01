//
//  longtrain4.c
//  train_throttle
//
//  Created by Daniel Braun on 27/03/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//


#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

//#include "../railconfig.h"
#include "../config/conf_train.h"

#include "ctrl.h"
//#include "ctrlP.h"
#include "ctrlLT.h"

//#include "longtrain.h"
#include "c3autoP.h"

#include "trig_tags.h"
#include "longtrain4.h"



static const int brake_len_mm = 160;
static const int margin_stop_len_mm = 120;
static const int margin_c2_len_mm = 200;


/*
 
 2/ determine max forward len
 Maxmargin = max (brake+margin_stop, margin_c2len, …)
 Right :
     tlen = rightlencm*10
 Left:
     tlen = leftlencm*10
 maxflen = tlen*10+maxmargin

 3/
 Flen = 0, rlen=maxflen
 Clen = lmax
 Repeat until flen <= maxflen
     if (rlen<=0)
         break // done, enough space
     get and reserve next_lsblk
     if (no next_lsblk)
         set trig and return flags
         break
     rlen = rlen - clen
             Flen = flen + clen
     clen = len(next_sblk)

 */

static void _add_trig(train_ctrl_t *tvars, int left, int c1len, rettrigs_t *rett, pose_trig_tag_t tag, int pos)
{
    if (rett->ntrig>=NUMTRIGS) {
        FatalError("ntrg", "too many trigs", Error_Abort);
        return;
    }
    if (!left) {
        rett->trigs[rett->ntrig].posmm = pos+tvars->beginposmm;
    } else {
        rett->trigs[rett->ntrig].posmm = c1len-pos+tvars->beginposmm;
    }
    rett->trigs[rett->ntrig].tag = tag;
    rett->ntrig++;
}

static void _add_endlsblk_trig(train_ctrl_t *tvars, int left, int c1len,  rettrigs_t *rett, lsblk_num_t c1, lsblk_num_t ns, int pos)
{
    if (ns.n == -1) {
        return;
    }
    if (canton_for_lsblk(c1).v != canton_for_lsblk(ns).v) {
        return;
    }
    int ina1 = get_lsblk_ina3221(c1);
    int ina2 = get_lsblk_ina3221(ns);
    if ((ina1 == ina2) || (ina2 == 0xFF)) {
        _add_trig(tvars, left, c1len, rett, tag_end_lsblk, pos);
    }
}

lsblk_num_t next_lsblk_and_reserve(int tidx, train_ctrl_t *tvars, lsblk_num_t sblknum, uint8_t left, int8_t *palternate, uint8_t doreserve)
{
    if (doreserve) {
        printf("debug");
    }
    if (palternate)  *palternate = 0;
    int back = 0;
    if ((tvars->_sdir<0) && !left) back = 1;
    else if ((tvars->_sdir>0) && left) back = 1;
    
    lsblk_num_t a, b;
    xtrnaddr_t tn;
    next_lsblk_nums(sblknum, left, &a, &b, &tn);
    if (tn.v == 0xFF) return a;
    
    if (palternate) *palternate = 1;
    if (!back) {
        int kt = occupency_turnout_reservedby(tn);
        if (kt == -1) {
            if (doreserve) {
                int rc = occupency_turnout_reserve(tn, tidx);
                if (rc) {
                    // cant reserve
                    a.n = -1;
                    return a;
                }
            }
        } else if (kt != tidx) {
            a.n = -1;
            return a;
        }
        // reserved by me
        if (palternate && (tvars->_mode==train_auto)) {
            c3auto_set_turnout(tidx, tn);
        }
    }
    a = (topology_get_turnout(tn) == topo_tn_turn) ? b : a;
    return a;
}



static int is_not_powered(int tidx, train_ctrl_t *tvars, xblkaddr_t ncanton)
{
    if (tvars->can1_xaddr.v == ncanton.v) return 0;
    if (tvars->can2_xaddr.v == ncanton.v) return 0;
    if (tvars->canOld_xaddr.v == ncanton.v) {
        return 0;
    }
    return 1;
}




int lt4_get_trigs(int tidx, train_ctrl_t *tvars, const conf_train_t *tconf, int left,  rettrigs_t *rett)
{
    int stopped = 0;
    if (tvars->_sdir) {
        if ((left &&(tvars->_sdir>0)) || (!left && (tvars->_sdir<0))) {
            FatalError("BadDir", "bad dir lt4_get_trigs", Error_Abort);
        }
    } else {
        stopped = 1;
    }
    
    int maxadvancefortrig;
    lsblk_num_t c1 = tvars->c1_sblk;
    int c1len = 10*get_lsblk_len_cm(c1, NULL);
    if (!left) {
        maxadvancefortrig = c1len - (tvars->_curposmm - tvars->beginposmm);
    } else {
        // left
        maxadvancefortrig = tvars->_curposmm - tvars->beginposmm;
    }
    
    
    int maxmargin = brake_len_mm+margin_stop_len_mm; // XXX to fix
    if (margin_c2_len_mm>maxmargin) maxmargin = margin_c2_len_mm;
    
    int train_fwd_len = (left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm) * 10;
    //int maxflen = train_fwd_len+maxmargin;

    occupency_set_occupied(canton_for_lsblk(c1), tidx, c1, tvars->_sdir);

    
    int clen = maxadvancefortrig;
    int alen = c1len;
    int totallen = 0;//c1len;
    int rc = 0;
    const int posloco = left ? (c1len - tvars->_curposmm + tvars->beginposmm) : ( tvars->_curposmm - tvars->beginposmm); // curposmm, without beginposmm
    //const int poshead = left ? posloco-train_fwd_len : posloco+train_fwd_len;
    //const int poshead = _pose_sub(posloco, -train_fwd_len, left);
    const int poshead = posloco + train_fwd_len; // _pose_sub(posloco, -train_fwd_len, left);
    int advancemm = 0;
    lsblk_num_t cs = tvars->c1_sblk;
    int first = 1;
    lsblk_num_t nextc1 = {.n=-1};
    int needchok = 1;
    int done = 0;
    
#define _AFTER_LOCO(_trg)       ((_trg)>posloco)
#define _BEFORE_C1END(_trg)     ((_trg)<=c1len)

#define _END_SBLK               (totallen+alen)
#define _BEFORE_END_SBLK(_m)    (_END_SBLK-(_m))
    
    for (;;) {
        // see what happens between advancemm and advancemm+clen
        int8_t a;
        int r;
        if (tvars->_sdir) {
            r = poshead+maxmargin > totallen+alen;
            if ((left &&(tvars->_sdir>0)) || (!left && (tvars->_sdir<0))) {
                FatalError("BadDir", "bad dir lt4_get_trigs", Error_Abort);
            }
        } else {
            r = poshead > totallen+alen;
        }
        lsblk_num_t ns = next_lsblk_and_reserve(tidx, tvars, cs, left, &a, r);
        
        //do we reach eot or blk wait ?
        if (ns.n == -1) {
            //set trig and return flags
            int trgbase = _BEFORE_END_SBLK(train_fwd_len); //totallen + alen - train_fwd_len;
            int trg = trgbase - margin_stop_len_mm; // trgbase-margin_stop_len_mm;
            if (_AFTER_LOCO(trg)) { // (trg > posloco) {
                if (_BEFORE_C1END(trg)) { //trg <= c1len) {
                    _add_trig(tvars, left, c1len, rett, a ? tag_stop_blk_wait:tag_stop_eot, trg);
                }
                trg = trgbase-margin_stop_len_mm - brake_len_mm;
                if (_AFTER_LOCO(trg)) { //(trg > posloco) {
                    if (_BEFORE_C1END(trg)) { //trg <= c1len) {
                        _add_trig(tvars, left, c1len, rett, tag_brake, trg);
                    }
                } else {
                    // pos+margin_stop_len_mm <= totallen
                    rc = brake_len_mm+trg-posloco;
                }
            } else {
                rc = -1;
                if (a) rett->isocc = 1;
                else   rett->isoet = 1;
            }
            done = 1;
            //break;
        } else {
            xblkaddr_t ncanton = canton_for_lsblk(ns);
            // reserve ns if train is on it
            if (poshead>totallen+alen) {
                occupency_set_occupied_car(ncanton, tidx, ns, tvars->_sdir);
                if (!left) {
                    //tvars->last_right = ns;
                }
            } else {
                
            }
            if (canton_for_lsblk(cs).v != ncanton.v) {
                // ...------------||----
                //   x
                int trgbase = _BEFORE_END_SBLK(train_fwd_len);  //totallen + alen - train_fwd_len;
                int trg = trgbase-margin_c2_len_mm; //trgbase-margin_c2_len_mm;
                if (_AFTER_LOCO(trg)) { //(trg > posloco) {
                    if (_BEFORE_C1END(trg)) { //trg <= c1len) {
                        _add_trig(tvars, left, c1len, rett, tag_reserve_c2,  trg);
                        if (tvars->res_c2_future.v == 0xFF) {
                            tvars->res_c2_future = ncanton;
                        }
                    }
                } else {
                    if (_AFTER_LOCO(trgbase)) { //(trgbase>posloco) {
                        rett->res_c2 = 1;
                        if (tvars->res_c2_future.v == 0xFF) {
                            tvars->res_c2_future = ncanton;
                        }
                    }
                }
                // loco advance for power_c2
                if (is_not_powered(tidx, tvars, ncanton)) {
                    trgbase = _BEFORE_END_SBLK(0); // totallen + alen;
                    trg = trgbase-margin_c2_len_mm; //trg = trgbase-margin_c2_len_mm;
                    if (_AFTER_LOCO(trg)) { //trg > posloco) {
                        if (_BEFORE_C1END(trg)) { //trg <= c1len) {
                            _add_trig(tvars, left, c1len, rett, tag_need_c2,  trg);
                            if (tvars->pow_c2_future.v == 0xFF) {
                                tvars->pow_c2_future = ncanton;
                            }
                        }
                    } else {
                        if (_AFTER_LOCO(trgbase)) { //trgbase> posloco) {
                            rett->power_c2 = 1;
                            if (tvars->pow_c2_future.v == 0xFF) {
                                tvars->pow_c2_future = ncanton;
                            }
                        }
                    }
                }
            }
        }
    
        
        int trg = _BEFORE_END_SBLK(train_fwd_len); // totallen+alen-train_fwd_len;
        //if (needchok && trg>posloco && trg<=c1len) {
        if (needchok && _AFTER_LOCO(trg) && _BEFORE_C1END(trg)) {
            // ----L xxxxxxx|xx-----|
            //                ^poshead
            _add_trig(tvars, left, c1len, rett, tag_chkocc, trg);
            needchok=0;
        }
        if (first) {
            nextc1 = ns;
            first = 0;
        }
        if (done) {
            break;
        }
        
        if (advancemm>maxadvancefortrig+maxmargin) {
            break;
        }
        advancemm += clen;
        totallen += alen;
        clen = 10*get_lsblk_len_cm(ns, NULL);
        alen = clen;
        cs = ns;
    }
    
    _add_endlsblk_trig(tvars, left, c1len, rett, tvars->c1_sblk, nextc1, c1len);
    
    return rc;
}

int lt4_check_back(int tidx, train_ctrl_t *tvars, const conf_train_t *tconf, int left, rettrigs_t *rett)
{
    return 0;
}
