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

static void _add_trig(int tidx, train_ctrl_t *tvars, pose_trig_tag_t tag, int pos)
{
    
}


lsblk_num_t next_lsblk_and_reserve(int tidx, train_ctrl_t *tvars, lsblk_num_t sblknum, uint8_t left, int8_t *palternate)
{
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
            int rc = occupency_turnout_reserve(tn, tidx);
            if (rc) {
                // cant reserve
                a.n = -1;
                return a;
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



int lt4_get_trigs(int tidx, train_ctrl_t *tvars, const conf_train_t *tconf, int left,  rettrigs_t *rett)
{
    /*
     
     1/ determine max advance without trig
     Right:
         lmax = c1len-(curpos-beginpos)
         set trig tag_end_lsblk at beginpos+c1len
     Left :
         lmax = (curpos-beginpos)
         set trig tag_end_lsblk at beginpos

     */
    int lmax, tlen;
    lsblk_num_t c1 = tvars->c1_sblk;
    int c1len = 10*get_lsblk_len_cm(c1, NULL);
    if (!left) {
        tlen = tconf->trainlen_right_cm*10;
        lmax = c1len - (tvars->_curposmm - tvars->beginposmm);
        _add_trig(tidx, tvars, tag_end_lsblk, tvars->beginposmm+c1len);
    } else {
        // left
        tlen = tconf->trainlen_left_cm*10;
        lmax = tvars->_curposmm - tvars->beginposmm;
        _add_trig(tidx, tvars, tag_end_lsblk, tvars->beginposmm);
    }
    
    /*
     
     2/ determine max forward len
     Maxmargin = max (brake+margin_stop, margin_c2len, …)
     Right :
         tlen = rightlencm*10
     Left:
         tlen = leftlencm*10
     maxflen = tlen*10+maxmargin

     */
    int maxmargin = brake_len_mm+margin_stop_len_mm; // XXX to fix
    int tflen = (left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm) * 10;
    int maxflen = tflen+maxmargin;

    /*
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
    int flen = 0;
    int rlen = maxflen;
    int clen = lmax;
    int rc = 0;
    lsblk_num_t cs = tvars->c1_sblk;
    for (;;) {
        if (flen>maxflen) break;
        if (rlen<=0) break; // done, enough space
        //get and reserve next_lsblk
        int8_t a;
        lsblk_num_t ns = next_lsblk_and_reserve(tidx, tvars, cs, left, &a);
        if (ns.n == -1) {
            //set trig and return flags
            if (rlen>=margin_stop_len_mm) {
                _add_trig(tidx, tvars, a ? tag_stop_blk_wait:tag_stop_eot, flen+tvars->beginposmm); //XXX
                
                if (rlen>=brake_len_mm+margin_stop_len_mm) {
                    _add_trig(tidx, tvars, tag_brake, flen+tvars->beginposmm);//XXX
                } else {
                    rc = brake_len_mm+margin_stop_len_mm-rlen;
                }
            } else {
                rc = -1;
                if (a) rett->isocc = 1;
                else   rett->isoet = 1;
            }
            
            break;
        }
        rlen = rlen - clen;
        flen = flen + clen;
        clen = get_lsblk_len_cm(ns, NULL);
    }
    
    return 0;
}

int lt4_check_back(int tidx, train_ctrl_t *tvars, const conf_train_t *tconf, int left, rettrigs_t *rett)
{
    return 0;
}
