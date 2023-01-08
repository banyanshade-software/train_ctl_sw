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


int32_t ctrl3_getcurpossmm(train_ctrl_t *tvars, const conf_train_t *tconf, int left)
{
    if (POSE_UNKNOWN == tvars->_curposmm) {
        if (left) return tvars->beginposmm;
        return tvars->beginposmm + 10*get_lsblk_len_cm_steep(tvars->c1_sblk, tconf, tvars);
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
    int l0 = (ctrl3_getcurpossmm(tvars, tconf, left) - tvars->beginposmm) / 10;
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
    tvars->rightcars.numlsblk = ctrl3_get_next_sblks_(tidx, tvars, tconf, 0, tvars->rightcars.r, MAX_LSBLK_CARS, &tvars->rightcars.rlen_cm);
    tvars->leftcars.numlsblk = ctrl3_get_next_sblks_(tidx, tvars, tconf, 1, tvars->leftcars.r, MAX_LSBLK_CARS, &tvars->leftcars.rlen_cm);
    return 0; // XXX error handling here
}

static const int brake_len_cm = 16;
static const int margin_stop_len_cm = 12;
static const int margin_c2_len_cm = 20;


static int trigmm_for_frontdistcm(_UNUSED_ int tidx, train_ctrl_t *tvars,  _UNUSED_ const conf_train_t *tconf, int left, int distcm)
{
    struct forwdsblk _UNUSED_ *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    if (!left) {
        int lmm = tvars->_curposmm - tvars->beginposmm + 10*distcm;
        if (lmm<10*get_lsblk_len_cm(tvars->c1_sblk, NULL)) {
            return lmm+tvars->beginposmm;
        }
    } else {
        // ex: curpos = 900, begin = 0, seg len = 90, dist 70
        int lmm = (tvars->_curposmm - tvars->beginposmm) - 10*distcm;
        if (lmm>=0 && lmm<=10*get_lsblk_len_cm(tvars->c1_sblk, NULL)) {
            return lmm+tvars->beginposmm;
        }
    }
    return -999999;
}

/*
static int check_for_dist(_UNUSED_ int tidx, train_ctrl_t *tvars,  struct forwdsblk *fsblk, int left, int distcm, int8_t *pa)
{
    lsblk_num_t ns = (fsblk->numlsblk>0) ? fsblk->r[fsblk->numlsblk-1] : tvars->c1_sblk;
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
 */

typedef int (*check_condition_t)(lsblk_num_t lastsblk, lsblk_num_t testsblk);

static int _check_front_condition_eot(lsblk_num_t lastsblk, lsblk_num_t testsblk)
{
    if (testsblk.n == -1) return 1;
    return 0;
}

static int _check_front_condition_res_c2(lsblk_num_t lastsblk, lsblk_num_t testsblk)
{
    xblkaddr_t c1 = canton_for_lsblk(lastsblk);
    xblkaddr_t c2 = canton_for_lsblk(testsblk);
    if (c1.v != c2.v) return 1;
    return 0;
}

static int check_front(int tidx, train_ctrl_t *tvars,  struct forwdsblk *fsblk, int left, int16_t maxcm, int8_t *pa, check_condition_t cond)
{
    lsblk_num_t fs = (fsblk->numlsblk>0) ? fsblk->r[fsblk->numlsblk-1] : tvars->c1_sblk;
    lsblk_num_t ns = fs;
    int cm0 = (ctrl3_getcurpossmm(tvars, conf_train_get(tidx), left)-tvars->beginposmm)/10;
    if (left) {
        cm0 -= fsblk->rlen_cm;
        int cm = 0;
        for (;;) {
            ns = next_lsblk(ns, left, pa);
            if (cond(fs, ns)) {
                // EOT or BLKWAIT
                return cm;
            } else {
                cm += get_lsblk_len_cm(ns, NULL);
                if (cm+cm0 >= maxcm+brake_len_cm+margin_stop_len_cm) {
                    *pa = -1;
                    return 0;
                }
            }
        }

    } else {
        cm0 += fsblk->rlen_cm;
        int cm = 0;
        //int slen = get_lsblk_len_cm(ns, NULL);
        for (;;) {
            ns = next_lsblk(ns, left, pa);
            if (cond(fs, ns)) {
                // EOT or BLKWAIT
                return cm;
            } else {
                cm += get_lsblk_len_cm(ns, NULL);
                if (cm+cm0 >= maxcm+brake_len_cm+margin_stop_len_cm) {
                    *pa = -1;
                    return 0;
                }
            }
        }
    }
}

#define ADD_TRIG_NOTHING 0x7FFF
int _add_trig(int left, rettrigs_t *ret, int rlencm, int c1lencm, int curcm, int k, pose_trig_tag_t tag, int dist, int mincm, int maxcm, int trlen)
{
    int l = dist-k;
    if (!left) {
        int trg = curcm+rlencm-l;
        if (l>=rlencm) {
            int s = l-rlencm;
            return s;
        } else if (l<c1lencm) {
            if (trg>maxcm) {
                //printf("hu");
            } else {
                ret->trigs[ret->ntrig].poscm = trg;
                ret->trigs[ret->ntrig].tag = tag;
                ret->ntrig++;
            }
        }
    } else {
        int trg = mincm+l+trlen;
        if (l>=rlencm) {
            int s = l-rlencm;
            return s;
        } else if (l<c1lencm) {
            if (trg<curcm) {
                ret->trigs[ret->ntrig].poscm = trg;
                ret->trigs[ret->ntrig].tag = tag;
                ret->ntrig++;
            }
        }
    }
    return ADD_TRIG_NOTHING;
}

int ctrl3_check_front_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left,  rettrigs_t *ret)
{
    memset(ret, 0, sizeof(rettrigs_t));
    ret->isoet = 0;
    ret->isocc = 0;
    struct forwdsblk *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    int retc = 0;
    int curcm = ctrl3_getcurpossmm(tvars, tconf, left)/10;
    int c1lencm = get_lsblk_len_cm(tvars->c1_sblk, NULL);

    // distance that will trigger a c1sblk change
    //int dc1mm =  10*get_lsblk_len_cm(tvars->c1_sblk, NULL) - (tvars->_curposmm - tvars->beginposmm) ;
    // trigger for end of seg
    
    int lmm = trigmm_for_frontdistcm(tidx, tvars, tconf, left, fsblk->rlen_cm);
    if ((0 <= lmm-tvars->beginposmm)  && (lmm-tvars->beginposmm <= c1lencm*10)) {
        ret->trigs[ret->ntrig].poscm = lmm/10;
        ret->trigs[ret->ntrig].tag = tag_chkocc;
        ret->ntrig++;
    }
    int8_t a;
    
    //int ltcm = left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm;
    //int bcm = tvars->beginposmm/10;
    int k = check_front(tidx, tvars, fsblk, left, c1lencm, &a, _check_front_condition_eot);
    if (a != -1) {
        // lcccc|cc----------|-----||
        //                      k
        //             <--margin--->
        //             <lstp >
        //          < rlen   >
        // train can advance rlen-lstp
        int maxcm = tvars->beginposmm/10+c1lencm;
        int mincm = tvars->beginposmm/10;
        int trlen = left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm;
        int rc = _add_trig(left, ret, fsblk->rlen_cm, c1lencm, curcm, k, a ? tag_stop_blk_wait : tag_stop_eot, margin_stop_len_cm, mincm, maxcm, trlen);
        if (rc!=ADD_TRIG_NOTHING) {
            if (a) ret->isocc = 1;
            else ret->isoet = 1;
            return -1;
        }
        rc = _add_trig(left, ret, fsblk->rlen_cm, c1lencm, curcm, k, tag_brake, margin_stop_len_cm+brake_len_cm, mincm, maxcm, trlen);
        
        if (rc!=ADD_TRIG_NOTHING) {
            // braake
            return brake_len_cm - rc;
        }
    }
  
#if 0
    lsblk_num_t ns = next_lsblk(tvars->c1_sblk, left, &a);
    if (ns.n != -1) {
        // if same canton and no ina3221
        if ((canton_for_lsblk(ns).v == tvars->can1_xaddr.v)
            && (ignore_ina_pres() || (get_lsblk_ina3221(tvars->c1_sblk) == get_lsblk_ina3221(ns)))) {
            int l = 0;
            if (!left) {
                l = tvars->beginposmm + get_lsblk_len_cm(tvars->c1_sblk, NULL);
                if (l>=stoplmm) goto chkc1;
            } else {
                l = tvars->beginposmm;
                if (l<=stoplmm) goto chkc1;
            }
            ret->trigs[trigidx].poscm = l/10;
            ret->trigs[trigidx].tag = tag_end_lsblk;
            trigidx++;
        }
    chkc1:
        if (canton_for_lsblk(ns).v != tvars->can1_xaddr.v) {
            //static int trig_for_frontdistcm(_UNUSED_ int tidx, train_ctrl_t *tvars,  _UNUSED_ const conf_train_t *tconf, int left, int distcm)

            int l1 = trigmm_for_frontdistcm(tidx, tvars, tconf, left,  margin_c2_len_cm)/10;
            
            if (!left) {
                if (l1>=stoplmm) goto done;
                if (l1<=0) {
                    goto done;
                }
            } else {
                if (l1<=stoplmm) goto done;
            }
            ret->trigs[trigidx].poscm = l1/10;
            ret->trigs[trigidx].tag = tag_need_c2;
            trigidx++;
        }
    done:
        itm_debug1(DBG_CTRL, "hop", tidx);
    }
#endif
    return retc;
}


int ctrl3_update_front_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left)
{
    struct forwdsblk *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    
    if ((1)) {
        // sanity check, c1sblk should not have change
        lsblk_num_t ns = next_lsblk(tvars->c1_sblk, left, NULL);
        if (fsblk->numlsblk) {
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
        if (fsblk->numlsblk) {
            if (fsblk->r[0].n != tvars->c1_sblk.n) return -1;
        }
    }
    // this could be improved,
    // but for now let's be safe
    return ctrl3_get_next_sblks(tidx, tvars, tconf);
}
