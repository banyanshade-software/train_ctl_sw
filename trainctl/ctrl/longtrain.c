//
//  longtrain.c
//  train_throttle
//
//  Created by Daniel Braun on 20/12/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
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

#include "longtrain.h"


#include "trig_tags.h"
//#include "cautoP.h"


static int32_t get_lsblk_len_cm_steep(lsblk_num_t lsbk, const conf_train_t *tconf, train_ctrl_t *tvar)
{
    int8_t steep = 0;
    int cm = get_lsblk_len_cm(lsbk, &steep);
    itm_debug3(DBG_CTRL|DBG_POSEC, "steep?", steep, tvar->_sdir, lsbk.n);
    if (steep*tvar->_sdir > 0) {
        if (!tconf->slipping) FatalError("NSLP", "no slipping", Error_Slipping);
        int cmold = cm;
        cm = cm * tconf->slipping / 100;
        itm_debug3(DBG_CTRL|DBG_POSEC, "steep!", lsbk.n, cmold, cm);
    }
    return cm;
}


static int32_t getcurpossmm(train_ctrl_t *tvars, const conf_train_t *tconf, int left)
{
    if (POSE_UNKNOWN == tvars->_curposmm) {
        if (left) return tvars->beginposmm;
        return tvars->beginposmm + get_lsblk_len_cm_steep(tvars->c1_sblk, tconf, tvars);
    }
    return tvars->_curposmm;
}

int ctrl3_get_next_sblks_(_UNUSED_ int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left, lsblk_num_t *resp, int nsblk, int16_t *premainlen)
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

int ctrl3_get_next_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf)
{
    memset(tvars->rightcars.r, 0xFF, sizeof(tvars->rightcars.r));
    memset(tvars->leftcars.r, 0xFF, sizeof(tvars->leftcars.r));
    tvars->rightcars.nr = ctrl3_get_next_sblks_(tidx, tvars, tconf, 0, tvars->rightcars.r, MAX_LSBLK_CARS, &tvars->rightcars.rlen_cm);
    tvars->leftcars.nr = ctrl3_get_next_sblks_(tidx, tvars, tconf, 1, tvars->leftcars.r, MAX_LSBLK_CARS, &tvars->leftcars.rlen_cm);
    return 0; // XXX error handling here
}

static const int brake_len_cm = 16;
static const int margin_len_cm = 12;


static int trig_for_frontdistcm(_UNUSED_ int tidx, train_ctrl_t *tvars,  _UNUSED_ const conf_train_t *tconf, int left, int distcm)
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

static int check_for_dist(_UNUSED_ int tidx, train_ctrl_t *tvars,  struct forwdsblk *fsblk, int left, int distcm, uint8_t *pa)
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

int ctrl3_check_front_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left,  rettrigs_t ret)
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


int ctrl3_update_front_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left)
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
    return ctrl3_get_next_sblks(tidx, tvars, tconf);
}

int ctrl3_update_front_sblks_c1changed(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left)
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
    return ctrl3_get_next_sblks(tidx, tvars, tconf);
}
