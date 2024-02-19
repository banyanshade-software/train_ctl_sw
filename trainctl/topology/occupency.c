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



static inline uint8_t occupied(int dir)
{
    if (dir<0) return BLK_OCC_LOCO_LEFT;
    if (dir>0) return BLK_OCC_LOCO_RIGHT;
    return BLK_OCC_LOCO_STOP;
}


typedef struct {
    uint8_t occ;
    uint8_t trnum;
    lsblk_num_t lsblk;
} canton_occ_t;

#define NUM_OCC_CANTON 0x40
static  canton_occ_t canton_occ[NUM_OCC_CANTON] = {0};



static uint32_t lastcheck = 0;

void occupency_clear(void)
{
	itm_debug1(DBG_OCCUP, "occ clear", 0);
    lastcheck = 0;
    memset(canton_occ, 0, sizeof(canton_occ));
    for (int i=0; i<NUM_OCC_CANTON; i++) {
        canton_occ[i].trnum = 0xFF;
    }
    occupency_clear_turnouts();
}

void occupency_remove_train(int trnum)
{
	itm_debug1(DBG_OCCUP, "occ rm tr", trnum);
    for (int i=0; i<NUM_OCC_CANTON; i++) {
        if (canton_occ[i].trnum == trnum) {
            canton_occ[i].trnum = 0xFF;
            canton_occ[i].occ = BLK_OCC_FREE;
            canton_occ[i].lsblk.n = -1;
        }
    }
    occupency_turnout_release_for_train(trnum);
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

// set_block_addr_occupency is obsolete
#if 0
void set_block_addr_occupency(xblkaddr_t blkaddr, uint8_t v, uint8_t trnum, lsblk_num_t lsb)
{
    int chg = 0;
    if (0xFF == blkaddr.v) {
        FatalError("OccFF", "bad occupency", Error_Occupency);
    }
    //int blknum = addr_to_num(blkaddr);
    canton_occ_t *co = &canton_occ[blkaddr.v];
    if (co->occ != v) {
        if (USE_BLOCK_DELAY_FREE && (v==BLK_OCC_FREE)) {
            if (co->occ > BLK_OCC_DELAYM) FatalError("OccD1", "bad occupency", Error_OccDelay);
            co->occ = BLK_OCC_DELAYM;
            itm_debug1(DBG_OCCUP, "delay free", blkaddr.v);
        } else {
            co->occ = v;
            chg = 1;
            if (BLK_OCC_FREE == co->occ) {
            	itm_debug2(DBG_OCCUP, "occ-free", blkaddr.v, trnum);
            	_block_freed(blkaddr, co);
                // non delayed free, untested
                trnum = 0xFF;
                lsb.n = -1;
            } else {
            	itm_debug3(DBG_OCCUP, "occ-occ", blkaddr.v, trnum, v);
            }
        }
    }
    if ((co->trnum != 0xFF) && (trnum != 0xFF) && (co->trnum != trnum)) {
    	itm_debug3(DBG_ERR|DBG_OCCUP, "occ-TCH", blkaddr.v, co->trnum, trnum);
    }
    co->trnum = trnum;
    if (co->lsblk.n != lsb.n) {
        co->lsblk = lsb;
        chg = 1;
    }
    if (chg) {
        topology_updated(trnum);
        notif_blk_occup_chg(blkaddr, co);
    }
}
#endif

static int _set_occupied(xblkaddr_t blkaddr, uint8_t trnum, lsblk_num_t lsb, int car, int sdir)
{
    int chg = 0;
    if (0xFF == blkaddr.v) FatalError("OccFF", "bad occupency", Error_Occupency);

    itm_debug3(DBG_OCCUP, "occ set", trnum, blkaddr.v, car);

    canton_occ_t *co = &canton_occ[blkaddr.v];
    if (co->occ != BLK_OCC_FREE) {
    	itm_debug3(DBG_OCCUP, "occ-upd", trnum, blkaddr.v, car);
        if (co->trnum != trnum) {
            itm_debug3(DBG_ERR|DBG_OCCUP, "blk occ", blkaddr.v, co->trnum, trnum);
            return -1;
        }
    }
    
    uint8_t occ;
    switch (car) {
        case 0: occ = occupied(sdir);   break;
        case 1: occ = BLK_OCC_CARS;     break;
        default: occ = BLK_OCC_C2;      break;
    }
    if ((occ != co->occ) || (trnum != co->trnum) || (lsb.n != co->lsblk.n)) {
        co->occ = occupied(sdir);
        co->trnum = trnum;
        co->lsblk = lsb;
        topology_updated(trnum);
        chg = 1;
    }

    if (chg) {
        notif_blk_occup_chg(blkaddr, co);
    }
    return 0;
}

int occupency_set_occupied(xblkaddr_t blkaddr, uint8_t trnum, lsblk_num_t lsb, int sdir)
{
    return _set_occupied(blkaddr, trnum, lsb, 0, sdir);
}


int occupency_set_occupied_car(xblkaddr_t blkaddr, uint8_t trnum, lsblk_num_t lsb, int sdir)
{
    return _set_occupied(blkaddr, trnum, lsb, 1, sdir);
}

int occupency_set_occupied_c2(xblkaddr_t blkaddr, uint8_t trnum, lsblk_num_t lsb, int sdir)
{
    return _set_occupied(blkaddr, trnum, lsb, -1, sdir);
}


void occupency_set_free(xblkaddr_t blkaddr, uint8_t trnum)
{
    int chg = 0;
	itm_debug2(DBG_OCCUP, "occ-free", trnum, blkaddr.v);
    if (0xFF == blkaddr.v) FatalError("OccFF", "bad occupency", Error_Occupency);
    canton_occ_t *co = &canton_occ[blkaddr.v];
    if (co->occ > BLK_OCC_DELAYM) FatalError("OccD1", "bad occupency", Error_OccDelay);
    if (BLK_OCC_FREE == co->occ) return;
    if (co->occ>=BLK_OCC_DELAY1) return;
    
    if (co->trnum != trnum) {
    	/*
    	 * do not free a block occupied by another train !
    	 * this could happen on freeback, when train is stopped (freeback would free were the canton the train
    	 * may have reserved, but in cas of blk_wait, it is used by another train)
    	 */
    	itm_debug3(DBG_ERR|DBG_OCCUP, "bad trnum", blkaddr.v, co->trnum, trnum);
    	return;
    }
    chg = 1;
    if (USE_BLOCK_DELAY_FREE) {
        co->occ = BLK_OCC_DELAYM;
        itm_debug1(DBG_CTRL, "delay free", blkaddr.v);
    } else {
        co->occ = BLK_OCC_FREE;
        chg = 1;
        _block_freed(blkaddr, co);
        // non delayed free, untested
        co->trnum = -1;
        co->lsblk.n = -1;
        topology_updated(trnum);
    }
    if (chg) {
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
            topology_updated(canton_occ[i].trnum);
            canton_occ[i].occ = BLK_OCC_FREE;
            canton_occ[i].trnum = 0xFF;
            canton_occ[i].lsblk.n = -1;
            notif_blk_occup_chg(bi, &canton_occ[i]);
        } else if (canton_occ[i].occ > BLK_OCC_DELAY1) {
            canton_occ[i].occ --;
        }
    }
}





