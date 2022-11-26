//
//  occupency.c
//  train_throttle
//
//  Created by Daniel BRAUN on 27/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

//#include <stdio.h>

#include "../misc.h"

#ifndef TRACKPLAN_TESTPGM
#include "../msg/trainmsg.h"
//#include "../railconfig.h"
#include "../config/conf_turnout.h"
#include "../config/conf_canton.h"
#endif

#include "topology.h"
#include "topologyP.h"
#include "occupency.h"


#ifndef BOARD_HAS_TOPOLOGY
#error BOARD_HAS_TOPOLOGY not defined, remove this file from build
#endif


typedef struct {
    uint8_t occ;
    uint8_t trnum;
    lsblk_num_t lsblk;
} canton_occ_t;

static  canton_occ_t canton_occ[0x40] = {0};



//uint8_t occupency_changed = 0; replaced by topology_or_occupency_changed
static uint32_t lastcheck = 0;

void occupency_clear(void)
{
    lastcheck = 0;
    memset(canton_occ, 0, sizeof(canton_occ));
    occupency_clear_turnouts();
}

static void _block_freed(xblkaddr_t cnum, canton_occ_t *co)
{
	occupency_turnout_release_for_train_canton(co->trnum, cnum);
}

/*

static inline uint8_t addr_to_num(xblkaddr_t addr)
{
    return addr.v; // TODO : compact it as actually on 6 canton per board and << 8 boards
}
*/

static uint8_t notif_blk_reset = 1;
uint8_t notify_occupency_change = 1;

static void notif_blk_occup_chg(xblkaddr_t blk, canton_occ_t *co)
{
    if (!notify_occupency_change) return;
    msg_64_t m = {0};
    m.from = MA1_CONTROL();
    m.to = MA3_UI_CTC;
    m.cmd = CMD_BLK_CHG_NOTIF;
    m.vbytes[0] = blk.v;
    m.vbytes[1] = co->occ;
    m.vbytes[2] = co->trnum;
    m.vbytes[3] = co->lsblk.n;
    notif_blk_reset = 0;
    mqf_write_from_ctrl(&m);
}


void set_block_addr_occupency(xblkaddr_t blkaddr, uint8_t v, uint8_t trnum, lsblk_num_t lsb)
{
    int chg = 0;
    if (0xFF == blkaddr.v) FatalError("OccFF", "bad occupency", Error_Occupency);
    //int blknum = addr_to_num(blkaddr);
    canton_occ_t *co = &canton_occ[blkaddr.v];
    if (co->occ != v) {
        if (USE_BLOCK_DELAY_FREE && (v==BLK_OCC_FREE)) {
            if (co->occ > BLK_OCC_DELAYM) FatalError("OccD1", "bad occupency", Error_OccDelay);
            co->occ = BLK_OCC_DELAYM;
            itm_debug1(DBG_CTRL, "delay free", blkaddr.v);
        } else {
            co->occ = v;
            chg = 1;
            if (BLK_OCC_FREE == co->occ) {
            	_block_freed(blkaddr, co);
                // non delayed free, untested
                trnum = -1;
                lsb.n = -1;
            }
        }
    }
    co->trnum = trnum;
    if (co->lsblk.n != lsb.n) {
        co->lsblk = lsb;
        chg = 1;
    }
    if (chg) {
        topology_or_occupency_changed = 1;
        notif_blk_occup_chg(blkaddr, co);
    }
}



uint8_t get_block_addr_occupency(xblkaddr_t blkaddr)
{
    if (0xFF == blkaddr.v) FatalError("OccFF", "bad occupency", Error_Occupency);
    return canton_occ[blkaddr.v].occ;
}
uint8_t occupency_block_addr_info(xblkaddr_t blkaddr, uint8_t *ptrn, uint8_t *psblk)
{
    canton_occ_t *occ = &canton_occ[blkaddr.v];
    if (ptrn) *ptrn = occ->trnum;
    if (psblk) *psblk = occ->lsblk.n;
    return occ->occ;
}


uint8_t occupency_block_is_free(xblkaddr_t blkaddr, uint8_t trnum)
{
    canton_occ_t *oc = &canton_occ[blkaddr.v];
    if (BLK_OCC_FREE == oc->occ) return 1;
    if (trnum == oc->trnum) return 1;
    return 0;
}


void check_block_delayed(_UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
    if (!USE_BLOCK_DELAY_FREE) return;

    if (tick<lastcheck+100) return;
    lastcheck = tick;

    for (int i=0; i<0x40; i++) {
        if (canton_occ[i].occ == BLK_OCC_DELAY1) {
            itm_debug1(DBG_CTRL, "FREE(d)", i);
            xblkaddr_t bi = {.v= i};
        	_block_freed(bi, &canton_occ[i]);
            canton_occ[i].occ = BLK_OCC_FREE;
            canton_occ[i].trnum = 0xFF;
            canton_occ[i].lsblk.n = -1;
            topology_or_occupency_changed = 1;
            notif_blk_occup_chg(bi, &canton_occ[i]);
        } else if (canton_occ[i].occ > BLK_OCC_DELAY1) {
            canton_occ[i].occ --;
        }
    }
}





