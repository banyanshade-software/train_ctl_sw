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

static int check_for_dist(_UNUSED_ int tidx, train_ctrl_t *tvars,  struct forwdsblk *fsblk, int left, int distcm, uint8_t *pa)
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

static int check_front(int tidx, train_ctrl_t *tvars,  struct forwdsblk *fsblk, int left, int16_t maxcm, int8_t *pa)
{
    lsblk_num_t ns = (fsblk->numlsblk>0) ? fsblk->r[fsblk->numlsblk-1] : tvars->c1_sblk;
    int cm0 = (ctrl3_getcurpossmm(tvars, conf_train_get(tidx), left)-tvars->beginposmm)/10;
    cm0 += fsblk->rlen_cm;
    int cm = 0;
    int slen = get_lsblk_len_cm(ns, NULL);
    for (;;) {
        ns = next_lsblk(ns, left, pa);
        if (ns.n == -1) {
            // EOT or BLKWAIT
            printf("ho");
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

#define ADD_TRIG_NOTHING 0x7FFF
int _add_trig(int left, rettrigs_t *ret, int rlencm, int c1lencm, int curcm, int k, pose_trig_tag_t tag, int dist, int maxcm)
{
    if (left) {
        // todo
        printf("hai");
    }
    int l = dist-k;
    int trg = curcm+rlencm-l;
    if (l>=rlencm) {
        int s = l-rlencm;
        return s;
    } else if (l<c1lencm) {
        if (trg>maxcm) {
            printf("hu");
        } else {
            ret->trigs[ret->ntrig].poscm = trg;
            ret->trigs[ret->ntrig].tag = tag;
            ret->ntrig++;
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
    int k = check_front(tidx, tvars, fsblk, left, c1lencm, &a);
    if (a != -1) {
        // lcccc|cc----------|-----||
        //                      k
        //             <--margin--->
        //             <lstp >
        //          < rlen   >
        // train can advance rlen-lstp
        int maxcm = tvars->beginposmm/10+c1lencm;
        int rc = _add_trig(left, ret, fsblk->rlen_cm, c1lencm, curcm, k, a ? tag_stop_blk_wait : tag_stop_eot, margin_stop_len_cm, maxcm);
        if (rc!=ADD_TRIG_NOTHING) {
            if (a) ret->isocc = 1;
            else ret->isoet = 1;
            return -1;
        }
        rc = _add_trig(left, ret, fsblk->rlen_cm, c1lencm, curcm, k, tag_brake, margin_stop_len_cm+brake_len_cm, maxcm);
        
        if (rc!=ADD_TRIG_NOTHING) {
            // braake
            return brake_len_cm - rc;
        }
#if 0
        int lstp = margin_stop_len_cm-k;
        int trg = curcm+fsblk->rlen_cm-lstp;
        printf("lstp=%d->trg=%d\n", lstp, trg);//+curcm
        if (lstp>fsblk->rlen_cm) {
            retc = lstp;
            if (a) ret->isocc = 1;
            else ret->isoet = 1;
            return retc;
        } else if (lstp<c1lencm) {
            ret->trigs[ret->ntrig].poscm = trg;
            ret->trigs[ret->ntrig].tag = a ? tag_stop_blk_wait : tag_stop_eot;
            ret->ntrig++;
        }
        int lbrk = lstp-brake_len_cm;
        printf("lbrk=%d\n", lbrk);
        if (lbrk<0) {
            retc = brake_len_cm+lbrk;
        } else if (lbrk>c1lencm){
            // ignore
        } else {
            ret->trigs[ret->ntrig].poscm = lbrk+bcm;
            ret->trigs[ret->ntrig].tag = tag_brake;
            ret->ntrig++;
        }
#endif

    }
    /*
    int l1 = check_for_dist(tidx, tvars, fsblk, left,  brake_len_cm+margin_stop_len_cm, &a);
    if ((1)) {
        if (l1==9999) {
            // rerun for gdb
            l1 = check_for_dist(tidx, tvars, fsblk, left,  brake_len_cm+margin_stop_len_cm, &a);
        }
    }
    if (l1<=0) {
        retc = brake_len_cm+l1;
        if (retc<=0) retc = 1;
    } else if (!left) {
        if ((l1>0) && (l1+curcm<maxcm)) {
            ret->trigs[trigidx].poscm = l1+curcm;
            ret->trigs[trigidx].tag = tag_brake;
            trigidx++;
        }
    } else {
        // left
        if ((l1>0) && (l1<maxcm)) {
            ret->trigs[trigidx].poscm = curcm-l1;
            ret->trigs[trigidx].tag = tag_brake;
            trigidx++;
        }
    }
    
    int stoplmm = 0;
    int l2 = check_for_dist(tidx, tvars, fsblk, left, margin_stop_len_cm, &a);
    //printf("l2/8=%d\n", l2);
    if (l2<=0) {
        retc = -1;
        if (a) {
            ret->isocc = 1;
        } else {
            ret->isoet = 1;
        }
    } else if (!left) {
        if ((l2>0) && (l2+curcm<maxcm)) {
            stoplmm = l2*10;
            ret->trigs[trigidx].poscm = l2+curcm;
            ret->trigs[trigidx].tag = a ? tag_stop_blk_wait : tag_stop_eot;
            trigidx++;
        }
    } else {
        // left
        if (l2 <= maxcm) {
            stoplmm = l2*10;
            ret->trigs[trigidx].poscm = curcm-l2;
            ret->trigs[trigidx].tag = a ? tag_stop_blk_wait : tag_stop_eot;
            trigidx++;
        }
    }

    if (retc<0) return retc;
    */
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
