//
//  longtrain.c
//  train_throttle
//
//  Created by Daniel Braun on 20/12/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#if 0

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
#include "c3autoP.h"

#include "trig_tags.h"
//#include "cautoP.h"


int32_t get_lsblk_len_cm_steep(lsblk_num_t lsbk, const conf_train_t *tconf, train_ctrl_t *tvar)
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

int32_t ctrl3_getcurpossmm(train_ctrl_t *tvars, const conf_train_t *tconf, int left)
{
    if (POSE_UNKNOWN == tvars->_curposmm) {
        if (left) return tvars->beginposmm;
        return tvars->beginposmm + 10*get_lsblk_len_cm_steep(tvars->c1_sblk, tconf, tvars);
    }
    return tvars->_curposmm;
}

/* replacement for next_lsblk to handle automatic mode and
   turnout reservation
 */

static lsblk_num_t next_lsblk_and_reserve(int tidx, train_ctrl_t *tvars, lsblk_num_t sblknum, uint8_t left, int8_t *palternate)
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




static inline int ctrl3_get_next_sblks__(_UNUSED_ int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left, lsblk_num_t *resp, int nsblk, int16_t *premainlenmm)
{
    if (premainlenmm) *premainlenmm = 0;
    int lidx = 0;
    int mm = 10*(left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm);
    lsblk_num_t cblk = tvars->c1_sblk;
    // curposmm
    int l0mm = ctrl3_getcurpossmm(tvars, tconf, left) - tvars->beginposmm;
    int first = 1;
    for (;;) {
        int lmm = 10*get_lsblk_len_cm(cblk, NULL);
        if (first /*l0mm*/) {
            first=0;
            if (left) {
                lmm = l0mm;
            } else {
                lmm = lmm-l0mm;
            }
            l0mm = 0;
        }
        if (lmm > mm) {
            // done
            if (premainlenmm) *premainlenmm = lmm-mm;
            return lidx;
        }
        mm -= lmm;
        //cblk = next_lsblk(cblk, left, NULL);
        int8_t a;
        cblk = next_lsblk_and_reserve(tidx, tvars, cblk, left, &a);
        resp[lidx] = cblk;
        lidx++;
        if (lidx>=nsblk) return lidx;
        if (cblk.n == -1) return lidx;
    }
}
int ctrl3_get_next_sblks_(_UNUSED_ int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left, lsblk_num_t *resp, int nsblk, int16_t *premainlenmm)
{
    int rc = ctrl3_get_next_sblks__(tidx, tvars, tconf, left, resp, nsblk, premainlenmm);
    if (!rc &&(resp[0].n != -1)) {
        // should not happen
        FatalError("SBlk", "bad sblk__", Error_NextSblk);
        resp[0].n = -1;
    }
    if (rc && (resp[0].n == -1)) {
        //rc = ctrl3_get_next_sblks__(tidx, tvars, tconf, left, resp, nsblk, premainlenmm);// for debug
        // will happen if train is positionned at begin of track, and
        // left side is too long (hence curposmm is impossible if left len is correct)
        // it happens specifically at init
        rc = 0;
    }
    return rc;
}


static lsblk_num_t _last(struct forwdsblk *f, lsblk_num_t cur)
{
    if (f->numlsblk) return f->r[f->numlsblk-1];
    return  cur;
}

int ctrl3_get_next_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf)
{
    lsblk_num_t lastright = _last(&tvars->rightcars, tvars->c1_sblk);
    lsblk_num_t lastleft  = _last(&tvars->leftcars, tvars->c1_sblk);
    tvars->freelsblk.n = -1;
    int lastnright = tvars->rightcars.numlsblk;
    int lastnleft  = tvars->leftcars.numlsblk;
    
    memset(tvars->rightcars.r, 0xFF, sizeof(tvars->rightcars.r));
    memset(tvars->leftcars.r, 0xFF, sizeof(tvars->leftcars.r));
    tvars->rightcars.numlsblk = ctrl3_get_next_sblks_(tidx, tvars, tconf, 0, tvars->rightcars.r, MAX_LSBLK_CARS, &tvars->rightcars.rlen_mm);
    tvars->leftcars.numlsblk = ctrl3_get_next_sblks_(tidx, tvars, tconf, 1, tvars->leftcars.r, MAX_LSBLK_CARS, &tvars->leftcars.rlen_mm);
    
    if (tvars->_sdir>0) {
        if (lastnleft>0) {
            if (lastnleft >= tvars->leftcars.numlsblk+2) {
                itm_debug1(DBG_ERR|DBG_CTRL, "problem here", 0);
            	FatalError("ABRT", "ctrl3_get_next_sblks", Error_Abort);
            }
            lsblk_num_t l = _last(&tvars->leftcars, tvars->c1_sblk);
            if (l.n != lastleft.n) {
                if (lastleft.n == tvars->c1_sblk.n) {
                	 itm_debug1(DBG_ERR|DBG_CTRL, "problem here", 1);
                	 FatalError("ABRT", "badc1", Error_Abort);
                }
                tvars->freelsblk = lastleft;
            }
        }
    } else if (tvars->_sdir <0) {
        if (lastnright>0) {
            lsblk_num_t l = _last(&tvars->rightcars, tvars->c1_sblk);
            if (l.n != lastright.n) {
                tvars->freelsblk = lastright;
            }
        }
    }
    // XXX handle freelsblk here ?
    // no must be in ctrlLT.c
    /* call map :
     _train_check_dir()  <--- handled here
        _updated_while_running()
        ctrl3_occupency_updated()
        ctrl3_upcmd_set_desired_speed()
     ctrl3_update_front_sblks()
        ctrl3_pose_triggered()
     ctrl3_update_front_sblks_c1changed()
        -[TestLongTrain testProgressLeft]
        -[TestLongTrain testProgressRight]
     turn_train_on()
     */
    return 0; // XXX error handling here
}

static const int brake_len_mm = 160;
static const int margin_stop_len_mm = 120;
static const int margin_c2_len_mm = 200;


static int trigmm_for_frontdistmm(_UNUSED_ int tidx, train_ctrl_t *tvars,  _UNUSED_ const conf_train_t *tconf, int left, int distmm)
{
    struct forwdsblk _UNUSED_ *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    if (!left) {
        int lmm = tvars->_curposmm - tvars->beginposmm + distmm;
        if (lmm<10*get_lsblk_len_cm(tvars->c1_sblk, NULL)) {
            return lmm+tvars->beginposmm;
        }
    } else {
        // ex: curpos = 900, begin = 0, seg len = 90, dist 70
        int lmm = (tvars->_curposmm - tvars->beginposmm) - distmm;
        if (lmm>=0 && lmm<=10*get_lsblk_len_cm(tvars->c1_sblk, NULL)) {
            return lmm+tvars->beginposmm;
        }
    }
    return -999999;
}

static int trigmm_for_backdistmm(_UNUSED_ int tidx, train_ctrl_t *tvars,  _UNUSED_ const conf_train_t *tconf, int left, int distmm)
{
    struct forwdsblk _UNUSED_ *fsblk = left ? &tvars->rightcars : &tvars->leftcars;
    if (!left) {
        int lmm = tvars->_curposmm - tvars->beginposmm + distmm;
        if (lmm<10*get_lsblk_len_cm(tvars->c1_sblk, NULL)) {
            return lmm+tvars->beginposmm;
        }
    } else {
        // ex: curpos = 900, begin = 0, seg len = 90, dist 70
        int lmm = (tvars->_curposmm - tvars->beginposmm) - distmm;
        if (lmm>=0 && lmm<=10*get_lsblk_len_cm(tvars->c1_sblk, NULL)) {
            return lmm+tvars->beginposmm;
        }
    }
//#error todo
#if 0
    if (!left) {
        int lmm = tvars->_curposmm - tvars->beginposmm + distmm;
        if (lmm<10*get_lsblk_len_cm(tvars->c1_sblk, NULL)) {
            return lmm+tvars->beginposmm;
        }
    } else {
        // ex: curpos = 900, begin = 0, seg len = 90, dist 70
        int lmm = (tvars->_curposmm - tvars->beginposmm) - distmm;
        if (lmm>=0 && lmm<=10*get_lsblk_len_cm(tvars->c1_sblk, NULL)) {
            return lmm+tvars->beginposmm;
        }
    }
#endif
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

typedef int (*check_condition_t)( train_ctrl_t *tvars, lsblk_num_t lastsblk, lsblk_num_t testsblk);

static int _check_front_condition_eot(_UNUSED_ train_ctrl_t *tvars, _UNUSED_ lsblk_num_t lastsblk, lsblk_num_t testsblk)
{
    if (testsblk.n == -1) return 1;
    return 0;
}

static int _check_front_condition_res_c2(train_ctrl_t *tvars, lsblk_num_t lastsblk, lsblk_num_t testsblk)
{
    xblkaddr_t c1 = canton_for_lsblk(lastsblk);
    xblkaddr_t c2 = canton_for_lsblk(testsblk);
    if ((c2.v !=0xff) && (c1.v != c2.v)) {
        if (c2.v == tvars->can1_xaddr.v) {
            return 0;
        }
        if (c2.v == tvars->can2_xaddr.v) {
            return 0;
        }
        tvars->tmp_c2_future = c2;
        return 1;
    }
    return 0;
}

int _check_front_condition_s1pose(_UNUSED_ train_ctrl_t *tvars, lsblk_num_t lastsblk, lsblk_num_t testsblk)
{
    xblkaddr_t c1 = canton_for_lsblk(lastsblk);
    xblkaddr_t c2 = canton_for_lsblk(testsblk);
    if (c1.v != c2.v) {
        return 0;
    }
    int a1 = get_lsblk_ina3221(lastsblk);
    int a2 = get_lsblk_ina3221(testsblk);
    if (a1 == a2) return 1;
    return 0;
}

int ckdebug = 0;
#define CKFRONT_DEBUG(...) do {\
  if (ckdebug) printf("----" __VA_ARGS__);\
} while(0)

static int check_front(int tidx, train_ctrl_t *tvars,  struct forwdsblk *fsblk, int left, int16_t maxmm, int8_t *pa, check_condition_t cond)
{
    lsblk_num_t fs = (fsblk->numlsblk>0) ? fsblk->r[fsblk->numlsblk-1] : tvars->c1_sblk;
    lsblk_num_t ns = fs;
    CKFRONT_DEBUG("ns=%d, numlsdblk=%d rlen=%d\n", fs.n, fsblk->numlsblk, fsblk->rlen_mm);
    int mm0 = ctrl3_getcurpossmm(tvars, conf_train_get(tidx), left)-tvars->beginposmm;
    CKFRONT_DEBUG("mm0=%d\n", mm0);
    if (mm0<-150) {
        extern void trace_train_dump(int);
        trace_train_dump(0);
        CKFRONT_DEBUG("ho");
    }
    if (left) {
        mm0 -= fsblk->rlen_mm;
        CKFRONT_DEBUG("left, mm0 = %d\n", mm0);
        int mm = 0;
        for (;;) {
            //ns = next_lsblk(ns, left, pa);
            ns = next_lsblk_and_reserve(tidx, tvars, ns, left, pa);
            CKFRONT_DEBUG("    mm=%d ns=%d\n", mm, ns.n);
            if (cond(tvars, fs, ns)) {
                // EOT or BLKWAIT
                CKFRONT_DEBUG("   COND true, return %d\n", mm);
                return mm;
            } else if (ns.n == -1) {
                CKFRONT_DEBUG("   ns.n==-1, return 0\n");
                *pa = -1;
                return 0;
            } else {
                mm += 10*get_lsblk_len_cm(ns, NULL);
                CKFRONT_DEBUG("    mm=%d\n", mm);
                // xxx KO
                if (mm+mm0 >= maxmm+brake_len_mm+margin_stop_len_mm) {
                    CKFRONT_DEBUG("   mm=%d mm0=%d, mm+mm0=%d / maxmm=%d, -> %d, return 0\n", mm, mm0, mm+mm0, maxmm, maxmm+brake_len_mm+margin_stop_len_mm);
                    *pa = -1;
                    return 0;
                } else if (ns.n == -1) {
                    CKFRONT_DEBUG("   ns.n==-1 (b), return 0\n");
                    *pa = -1;
                    return 0;
                }
            }
        }

    } else {
        // right
        mm0 += fsblk->rlen_mm;
        int mm = 0;
        //int slen = get_lsblk_len_cm(ns, NULL);
        for (;;) {
            //ns = next_lsblk(ns, left, pa);
            ns = next_lsblk_and_reserve(tidx, tvars, ns, left, pa);
            if (cond(tvars, fs, ns)) {
                // EOT or BLKWAIT
                return mm;
            } else if (ns.n == -1) {
                *pa = -1;
                return 0;
            } else {
                mm += 10*get_lsblk_len_cm(ns, NULL);
                if (mm+mm0 >= maxmm+brake_len_mm+margin_stop_len_mm) {
                    *pa = -1;
                    return 0;
                } else if (ns.n == -1) {
                    *pa = -1;
                    return 0;
                }
            }
        }
    }
}

static int check_loco(int tidx, train_ctrl_t *tvars,  _UNUSED_ struct forwdsblk *fsblk, int left, int16_t maxmm, int8_t *pa, check_condition_t cond)
{
    lsblk_num_t fs = tvars->c1_sblk;
    lsblk_num_t ns = fs;
    int mm0 = ctrl3_getcurpossmm(tvars, conf_train_get(tidx), left)-tvars->beginposmm;
    if (left) {
        //cm0 -= fsblk->rlen_cm;
        int mm = 0;
        for (;;) {
            ns = next_lsblk(ns, left, pa);
            if (cond(tvars, fs, ns)) {
                // EOT or BLKWAIT
                return mm;
            } else if (ns.n == -1) {
                *pa = -1;
                return 0;
            } else {
                mm += 10*get_lsblk_len_cm(ns, NULL);
                if (mm+mm0 >= maxmm+brake_len_mm+margin_stop_len_mm) {
                    *pa = -1;
                    return 0;
                }
            }
        }

    } else {
        //cm0 += fsblk->rlen_cm;
        int mm = 0;
        //int slen = get_lsblk_len_cm(ns, NULL);
        for (;;) {
            ns = next_lsblk(ns, left, pa);
            if (cond(tvars, fs, ns)) {
                // EOT or BLKWAIT
                return mm;
            } else if (ns.n == -1) {
                *pa = -1;
                return 0;
            } else {
                mm += 10*get_lsblk_len_cm(ns, NULL);
                if (mm+mm0 >= maxmm+brake_len_mm+margin_stop_len_mm) {
                    *pa = -1;
                    return 0;
                } else if (ns.n == -1) {
                    *pa = -1;
                    return 0;
                }
            }
        }
    }
}


// _add_trig : all distances in mm
#define ADD_TRIG_NOTHING 0x7FFF
int _add_trig(int left, rettrigs_t *ret, int rlenmm, int c1lenmm, int curmm, int kmm, pose_trig_tag_t tag, int distmm, int minmm, int maxmm, int trlenmm)
{
    CKFRONT_DEBUG("add_trig tag%d, rlenmm=%d, c1lenmm=%d, curmm=%d, kmm=%d, distmm=%d, minmm=%d, maxmm=%d, trlenmm=%d",
                  tag, rlenmm, c1lenmm, curmm, kmm, distmm, minmm, maxmm, trlenmm);
    int lmm = distmm-kmm;
    CKFRONT_DEBUG("lmm=%d\n", lmm);
    if (!left) {
        int trg = curmm+rlenmm-lmm;
        if (lmm>=rlenmm) {
            int smm = lmm-rlenmm;
            return smm;
        } else if (lmm<c1lenmm) {
            if (trg>maxmm) {
                //printf("hu");
            } else {
                ret->trigs[ret->ntrig].posmm = trg;
                ret->trigs[ret->ntrig].tag = tag;
                ret->ntrig++;
            }
        }
    } else {
        int trg = minmm+lmm+trlenmm;
        CKFRONT_DEBUG("left trg=%d rlenmm=%d\n", trg, rlenmm);
        if (lmm >= rlenmm) {
            int smm = lmm-rlenmm;
            CKFRONT_DEBUG("smm=%d -> return smm\n", smm);
            return smm;
        } else if (lmm < c1lenmm) {
            CKFRONT_DEBUG("lmm %d < c1lenmm %d\n", lmm, c1lenmm);
            if (trg<curmm) {
                CKFRONT_DEBUG("trg %d < curmm %d, add trig\n", trg, curmm);
                ret->trigs[ret->ntrig].posmm = trg;
                ret->trigs[ret->ntrig].tag = tag;
                ret->ntrig++;
            } else {
                CKFRONT_DEBUG("trg %d NOT < curmm %d, ignore\n", trg, curmm);
            }
        }
    }
    CKFRONT_DEBUG("return NOTHING\n");
    return ADD_TRIG_NOTHING;
}

int _add_trig_loco(int left, rettrigs_t *ret, int rlenmm, int c1lenmm, int curmm, int kmm, pose_trig_tag_t tag, int dist, int minmm, int maxmm, int trlen)
{
    // ---------|------||
    //          < -k-->
    //      <---dist-->
    int lmm = dist-kmm;
    if (!left) {
        int trg = minmm+c1lenmm-lmm;
        if (trg <= curmm) {
            int smm = minmm+lmm-curmm;
            return smm;
        } else if (lmm < c1lenmm) {
            if (trg > maxmm) {
                //printf("hu");
            } else {
                ret->trigs[ret->ntrig].posmm = trg;
                ret->trigs[ret->ntrig].tag = tag;
                ret->ntrig++;
            }
        }
    } else {
        int trg = minmm+lmm+trlen;
        if (lmm >= rlenmm) {
            int smm = lmm-rlenmm;
            return smm;
        } else if (lmm < c1lenmm) {
            if (trg < curmm) {
                ret->trigs[ret->ntrig].posmm = trg;
                ret->trigs[ret->ntrig].tag = tag;
                ret->ntrig++;
            }
        }
    }
    return ADD_TRIG_NOTHING;
}




int ctrl3_check_front_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left,  rettrigs_t *ret)
{
    //memset(ret, 0, sizeof(rettrigs_t));
    ret->isoet = 0;
    ret->isocc = 0;
    struct forwdsblk *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    int retc = 0;
    int curmm = ctrl3_getcurpossmm(tvars, tconf, left);
    int c1lenmm = 10*get_lsblk_len_cm(tvars->c1_sblk, NULL);
    int bpmm = tvars->beginposmm;

    // distance that will trigger a c1sblk change
    //int dc1mm =  10*get_lsblk_len_cm(tvars->c1_sblk, NULL) - (tvars->_curposmm - tvars->beginposmm) ;
    // trigger for end of seg
    
    int lmm = trigmm_for_frontdistmm(tidx, tvars, tconf, left, fsblk->rlen_mm);
    if ((0 <= lmm-tvars->beginposmm)  && (lmm-tvars->beginposmm <= c1lenmm)) {
        ret->trigs[ret->ntrig].posmm = lmm;
        ret->trigs[ret->ntrig].tag = tag_chkocc;
        ret->ntrig++;
    }
    int8_t a;
    
    //int ltcm = left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm;
    //int bcm = tvars->beginposmm/10;
    int maxmm = tvars->beginposmm+c1lenmm;
    int minmm = tvars->beginposmm;
    int trlenmm = 10*(left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm);
    
    
    tvars->tmp_c2_future.v = 0xFF;
    int kmm = check_front(tidx, tvars, fsblk, left, c1lenmm, &a, _check_front_condition_res_c2);
    if (tvars->res_c2_future.v == 0xFF) {
        tvars->res_c2_future = tvars->tmp_c2_future;
    }
    if (a != -1) {
        int rc = _add_trig(left, ret, fsblk->rlen_mm, c1lenmm, curmm, kmm, tag_reserve_c2, margin_c2_len_mm, minmm, maxmm, trlenmm);
        if (rc != ADD_TRIG_NOTHING) {
            ret->res_c2 = 1;
        }
    }
    
    // c1lemm ko for check_front() going left
    ckdebug=1;
    kmm = check_front(tidx, tvars, fsblk, left, c1lenmm, &a, _check_front_condition_eot);
    if (a != -1) {
        // lcccc|cc----------|-----||
        //                      k
        //             <--margin--->
        //             <lstp >
        //          < rlen   >
        // train can advance rlen-lstp
        
        int rc = _add_trig(left, ret, fsblk->rlen_mm, c1lenmm, curmm, kmm, a ? tag_stop_blk_wait : tag_stop_eot, margin_stop_len_mm, minmm, maxmm, trlenmm);
        if (rc!=ADD_TRIG_NOTHING) {
            if (a) ret->isocc = 1;
            else ret->isoet = 1;
            retc = -1;
        } else {
            if ((0)) {
                extern void trace_train_dump(int tidx);
                trace_train_dump(tidx);
            }
            // dont check for brake if already in stop condition
            rc = _add_trig(left, ret, fsblk->rlen_mm, c1lenmm, curmm, kmm, tag_brake, margin_stop_len_mm+brake_len_mm, minmm, maxmm, trlenmm);
            if (rc!=ADD_TRIG_NOTHING) {
                // brake
                retc = brake_len_mm - rc;
            }
        }
    }
    ckdebug=0;

    
    
    // s1 end
    if ((1)) {
        lsblk_num_t ns = next_lsblk(tvars->c1_sblk, left, NULL);
        xblkaddr_t c1 = canton_for_lsblk(tvars->c1_sblk);
        xblkaddr_t c2 = canton_for_lsblk(ns);
        if (c1.v == c2.v) {
            int a1 = get_lsblk_ina3221(tvars->c1_sblk);
            int a2 = get_lsblk_ina3221(ns);
            if (ignore_ina_pres() || (a1 == a2)) {
                if (!left) {
                    ret->trigs[ret->ntrig].posmm = bpmm + c1lenmm;
                } else {
                    ret->trigs[ret->ntrig].posmm = bpmm;
                }
                ret->trigs[ret->ntrig].tag = tag_end_lsblk;
                ret->ntrig++;
            }
        }
    }
    //  power c2
    if ((1)) {
        tvars->tmp_c2_future.v = 0xFF;
        kmm = check_loco(tidx, tvars, fsblk, left, c1lenmm, &a, _check_front_condition_res_c2);
        if (tvars->pow_c2_future.v == 0xFF) {
            tvars->pow_c2_future = tvars->tmp_c2_future;
        }
        if (a != -1) {
            int rc = _add_trig_loco(left, ret, 0, c1lenmm, curmm, kmm, tag_need_c2, margin_c2_len_mm, minmm, maxmm, trlenmm);
            if (rc != ADD_TRIG_NOTHING) {
                ret->power_c2 = 1;
            }
        }
    }
    
    return retc;
}


int ctrl3_check_back_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left,  rettrigs_t *ret)
{
    struct forwdsblk *fsblk = left ? &tvars->rightcars : &tvars->leftcars;
    /* if no back sblk don't even try to find a block to be freed */
    if (!fsblk->numlsblk) return 0;
    int retc = 0;
    lsblk_num_t leftblk = fsblk->numlsblk ? fsblk->r[fsblk->numlsblk-1] : tvars->c1_sblk;
    int lastlmm = 10*get_lsblk_len_cm(leftblk, NULL);
    //int curmm = ctrl3_getcurpossmm(tvars, tconf, left);
    int llen = lastlmm - fsblk->rlen_mm;
    if (llen<0) {
        return 0;
    }
    int c1lenmm = 10*get_lsblk_len_cm(tvars->c1_sblk, NULL);
    //int bpmm = tvars->beginposmm;

    // distance that will trigger a c1sblk change
    //int dc1mm =  10*get_lsblk_len_cm(tvars->c1_sblk, NULL) - (tvars->_curposmm - tvars->beginposmm) ;
    // trigger for end of seg
    int lmm = trigmm_for_backdistmm(tidx, tvars, tconf, left, llen);
    if ((0 <= lmm-tvars->beginposmm)  && (lmm-tvars->beginposmm <= c1lenmm)) {
        ret->trigs[ret->ntrig].posmm = lmm;
        ret->trigs[ret->ntrig].tag = tag_free_back;
        ret->ntrig++;
    }
#if 0
    int lmm = trigmm_for_frontdistmm(tidx, tvars, tconf, left, fsblk->rlen_mm);
    if ((0 <= lmm-tvars->beginposmm)  && (lmm-tvars->beginposmm <= c1lenmm)) {
        ret->trigs[ret->ntrig].posmm = lmm;
        ret->trigs[ret->ntrig].tag = tag_chkocc;
        ret->ntrig++;
    }
    int8_t a;
    
    //int ltcm = left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm;
    //int bcm = tvars->beginposmm/10;
    int maxmm = tvars->beginposmm+c1lenmm;
    int minmm = tvars->beginposmm;
    int trlenmm = 10*(left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm);
    
    int kmm = check_front(tidx, tvars, fsblk, left, c1lenmm, &a, _check_front_condition_eot);
    if (a != -1) {
        // lcccc|cc----------|-----||
        //                      k
        //             <--margin--->
        //             <lstp >
        //          < rlen   >
        // train can advance rlen-lstp
        
        int rc = _add_trig(left, ret, fsblk->rlen_mm, c1lenmm, curmm, kmm, a ? tag_stop_blk_wait : tag_stop_eot, margin_stop_len_mm, minmm, maxmm, trlenmm);
        if (rc!=ADD_TRIG_NOTHING) {
            if (a) ret->isocc = 1;
            else ret->isoet = 1;
            retc = -1;
        } else {
            // dont check for brake if already in stop condition
            rc = _add_trig(left, ret, fsblk->rlen_mm, c1lenmm, curmm, kmm, tag_brake, margin_stop_len_mm+brake_len_mm, minmm, maxmm, trlenmm);
            if (rc!=ADD_TRIG_NOTHING) {
                // brake
                retc = brake_len_mm - rc;
            }
        }
    }
    
    tvars->tmp_c2_future.v = 0xFF;
    kmm = check_front(tidx, tvars, fsblk, left, c1lenmm, &a, _check_front_condition_res_c2);
    if (tvars->res_c2_future.v == 0xFF) {
        tvars->res_c2_future = tvars->tmp_c2_future;
    }
    if (a != -1) {
        int rc = _add_trig(left, ret, fsblk->rlen_mm, c1lenmm, curmm, kmm, tag_reserve_c2, margin_c2_len_mm, minmm, maxmm, trlenmm);
        if (rc != ADD_TRIG_NOTHING) {
            ret->res_c2 = 1;
        }
    }
    
    // s1 end
    if ((1)) {
        lsblk_num_t ns = next_lsblk(tvars->c1_sblk, left, NULL);
        xblkaddr_t c1 = canton_for_lsblk(tvars->c1_sblk);
        xblkaddr_t c2 = canton_for_lsblk(ns);
        if (c1.v == c2.v) {
            int a1 = get_lsblk_ina3221(tvars->c1_sblk);
            int a2 = get_lsblk_ina3221(ns);
            if (ignore_ina_pres() || (a1 == a2)) {
                if (!left) {
                    ret->trigs[ret->ntrig].posmm = bpmm + c1lenmm;
                } else {
                    ret->trigs[ret->ntrig].posmm = bpmm;
                }
                ret->trigs[ret->ntrig].tag = tag_end_lsblk;
                ret->ntrig++;
            }
        }
    }
    //  power c2
    if ((1)) {
        tvars->tmp_c2_future.v = 0xFF;
        kmm = check_loco(tidx, tvars, fsblk, left, c1lenmm, &a, _check_front_condition_res_c2);
        if (tvars->pow_c2_future.v == 0xFF) {
            tvars->pow_c2_future = tvars->tmp_c2_future;
        }
        if (a != -1) {
            int rc = _add_trig_loco(left, ret, 0, c1lenmm, curmm, kmm, tag_need_c2, margin_c2_len_mm, minmm, maxmm, trlenmm);
            if (rc != ADD_TRIG_NOTHING) {
                ret->power_c2 = 1;
            }
        }
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

void ctrl3_update_c1changed(int tidx, train_ctrl_t *tvars,  _UNUSED_ const conf_train_t *tconf, int left)
{
    struct forwdsblk *fsblk = left ? &tvars->leftcars : &tvars->rightcars;
    
    if ((1)) {
        // sanity check, c1sblk should be first item
        if (fsblk->numlsblk) {
            if (fsblk->r[0].n != tvars->c1_sblk.n) {
                // will actually occur with POS_UNKNOWN
                //return -1;
            }
        }
    }
    occupency_set_occupied(tvars->can1_xaddr, tidx, tvars->c1_sblk, tvars->_sdir);
    if (tvars->_mode == train_auto) {
        c3auto_set_s1(tidx, tvars->c1_sblk);
    }
    // this could be improved,
    // but for now let's be safe
    //return ctrl3_get_next_sblks(tidx, tvars, tconf); moved out
}

#endif
