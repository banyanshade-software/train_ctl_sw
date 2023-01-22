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

int ctrl3_get_next_sblks_(_UNUSED_ int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left, lsblk_num_t *resp, int nsblk, int16_t *premainlenmm)
{
    if (premainlenmm) *premainlenmm = 0;
    int lidx = 0;
    int mm = 10*(left ? tconf->trainlen_left_cm : tconf->trainlen_right_cm);
    lsblk_num_t cblk = tvars->c1_sblk;
    // curposmm
    int l0mm = ctrl3_getcurpossmm(tvars, tconf, left) - tvars->beginposmm;
    for (;;) {
        int lmm = 10*get_lsblk_len_cm(cblk, NULL);
        if (l0mm) {
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
    tvars->rightcars.numlsblk = ctrl3_get_next_sblks_(tidx, tvars, tconf, 0, tvars->rightcars.r, MAX_LSBLK_CARS, &tvars->rightcars.rlen_mm);
    tvars->leftcars.numlsblk = ctrl3_get_next_sblks_(tidx, tvars, tconf, 1, tvars->leftcars.r, MAX_LSBLK_CARS, &tvars->leftcars.rlen_mm);
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

static int _check_front_condition_eot( train_ctrl_t *tvars, lsblk_num_t lastsblk, lsblk_num_t testsblk)
{
    if (testsblk.n == -1) return 1;
    return 0;
}

static int _check_front_condition_res_c2(train_ctrl_t *tvars, lsblk_num_t lastsblk, lsblk_num_t testsblk)
{
    xblkaddr_t c1 = canton_for_lsblk(lastsblk);
    xblkaddr_t c2 = canton_for_lsblk(testsblk);
    if ((c2.v !=0xff) && (c1.v != c2.v)) {
        tvars->can2_future = c2;
        return 1;
    }
    return 0;
}

int _check_front_condition_s1pose( train_ctrl_t *tvars, lsblk_num_t lastsblk, lsblk_num_t testsblk)
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


static int check_front(int tidx, train_ctrl_t *tvars,  struct forwdsblk *fsblk, int left, int16_t maxmm, int8_t *pa, check_condition_t cond)
{
    lsblk_num_t fs = (fsblk->numlsblk>0) ? fsblk->r[fsblk->numlsblk-1] : tvars->c1_sblk;
    lsblk_num_t ns = fs;
    int mm0 = ctrl3_getcurpossmm(tvars, conf_train_get(tidx), left)-tvars->beginposmm;
    if (left) {
        mm0 -= fsblk->rlen_mm;
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
        mm0 += fsblk->rlen_mm;
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

static int check_loco(int tidx, train_ctrl_t *tvars,  struct forwdsblk *fsblk, int left, int16_t maxmm, int8_t *pa, check_condition_t cond)
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
    int lmm = distmm-kmm;
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
        if (lmm >= rlenmm) {
            int smm = lmm-rlenmm;
            return smm;
        } else if (lmm < c1lenmm) {
            if (trg<curmm) {
                ret->trigs[ret->ntrig].posmm = trg;
                ret->trigs[ret->ntrig].tag = tag;
                ret->ntrig++;
            }
        }
    }
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
    memset(ret, 0, sizeof(rettrigs_t));
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
    
    kmm = check_front(tidx, tvars, fsblk, left, c1lenmm, &a, _check_front_condition_res_c2);
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
        kmm = check_loco(tidx, tvars, fsblk, left, c1lenmm, &a, _check_front_condition_res_c2);
        if (a != -1) {
            int rc = _add_trig_loco(left, ret, 0, c1lenmm, curmm, kmm, tag_need_c2, margin_c2_len_mm, minmm, maxmm, trlenmm);
            if (rc != ADD_TRIG_NOTHING) {
                ret->power_c2 = 1;
            }
        }
    }
    
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
            if (fsblk->r[0].n != tvars->c1_sblk.n) {
                // will actually occur with POS_UNKNOWN
                //return -1;
            }
        }
    }
    occupency_set_occupied(tvars->can1_xaddr, tidx, tvars->c1_sblk, tvars->_sdir);
    // this could be improved,
    // but for now let's be safe
    return ctrl3_get_next_sblks(tidx, tvars, tconf);
}
