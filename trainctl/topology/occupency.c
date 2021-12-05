//
//  occupency.c
//  train_throttle
//
//  Created by Daniel BRAUN on 27/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

//#include <stdio.h>

#include "misc.h"
#ifndef TRACKPLAN_TESTPGM
#include "../msg/trainmsg.h"
#include "railconfig.h"
#endif

#include "topology.h"
#include "occupency.h"


typedef struct {
    uint8_t occ;
    uint8_t trnum;
    lsblk_num_t lsblk;
} canton_occ_t;

static  canton_occ_t canton_occ[0x40] = {0};

static void occupency_turnout_release_for_train_canton(int8_t train, uint8_t canton);


//uint8_t occupency_changed = 0; replaced by topology_or_occupency_changed
static uint32_t lastcheck = 0;

void occupency_clear(void)
{
    lastcheck = 0;
    memset(canton_occ, 0, sizeof(canton_occ));
}

static void _block_freed(int cnum, canton_occ_t *co)
{
	occupency_turnout_release_for_train_canton(co->trnum, cnum);
}


static inline uint8_t addr_to_num(uint8_t addr)
{
    // MA_CANTON(board, c)
    // // M2:  0 0 : (6bits) bbb xxx        CANTON (00) and TURNOUT (01)
    return addr & 0x3F;
}

static uint8_t notif_blk_reset = 1;
uint8_t notify_occupency_change = 1;

static void notif_blk_occup_chg(int blknum, canton_occ_t *co)
{
    if (!notify_occupency_change) return;
    msg_64_t m = {0};
    m.from = MA_CONTROL();
    m.to = MA_UI(UISUB_TRACK);
    m.cmd = CMD_BLK_CHG_NOTIF;
    m.vbytes[0] = blknum;
    m.vbytes[1] = co->occ;
    m.vbytes[2] = co->trnum;
    m.vbytes[3] = co->lsblk.n;
    notif_blk_reset = 0;
    mqf_write_from_ctrl(&m);
}


void set_block_addr_occupency(uint8_t blkaddr, uint8_t v, uint8_t trnum, lsblk_num_t lsb)
{
    int chg = 0;
    if (0xFF == blkaddr) Error_Handler();
    int blknum = addr_to_num(blkaddr);
    canton_occ_t *co = &canton_occ[addr_to_num(blkaddr)];
    if (co->occ != v) {
        if (USE_BLOCK_DELAY_FREE && (v==BLK_OCC_FREE)) {
            if (co->occ >= BLK_OCC_DELAY1) Error_Handler();
            co->occ = BLK_OCC_DELAYM;
            itm_debug1(DBG_CTRL, "delay free", blkaddr);
        } else {
            co->occ = v;
            chg = 1;
            if (BLK_OCC_FREE == co->occ) {
            	_block_freed(blknum, co);
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



uint8_t get_block_addr_occupency(uint8_t blkaddr)
{
    if (0xFF == blkaddr) Error_Handler();
    return canton_occ[addr_to_num(blkaddr)].occ;
}
uint8_t occupency_block_addr_info(uint8_t blkaddr, uint8_t *ptrn, uint8_t *psblk)
{
    canton_occ_t *occ = &canton_occ[addr_to_num(blkaddr)];
    if (ptrn) *ptrn = occ->trnum;
    if (psblk) *psblk = occ->lsblk.n;
    return occ->occ;
}


uint8_t occupency_block_is_free(uint8_t blkaddr, uint8_t trnum)
{
    canton_occ_t *oc = &canton_occ[addr_to_num(blkaddr)];
    if (BLK_OCC_FREE == oc->occ) return 1;
    if (trnum == oc->trnum) return 1;
    return 0;
}


void check_block_delayed(_UNUSED_ uint32_t tick)
{
    if (!USE_BLOCK_DELAY_FREE) return;

    if (tick<lastcheck+100) return;
    lastcheck = tick;

    for (int i=0; i<0x40; i++) {
        if (canton_occ[i].occ == BLK_OCC_DELAY1) {
            itm_debug1(DBG_CTRL, "FREE(d)", i);
        	_block_freed(i, &canton_occ[i]);
            canton_occ[i].occ = BLK_OCC_FREE;
            canton_occ[i].trnum = 0xFF;
            canton_occ[i].lsblk.n = -1;
            topology_or_occupency_changed = 1;
            notif_blk_occup_chg(i, &canton_occ[i]);
        } else if (canton_occ[i].occ > BLK_OCC_DELAY1) {
            canton_occ[i].occ --;
        }
    }
}


static volatile uint8_t  lockedby[NUM_TURNOUTS] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


static  void _notify_chg_owner(uint8_t turnout, int8_t numtrain)
{
    msg_64_t m = {0};
    m.cmd = CMD_TN_RESER_NOTIF;
    m.v1 = turnout;
    m.v2 = numtrain;
    m.to = MA_UI(UISUB_TRACK);
    m.from = MA_CONTROL();
    mqf_write_from_ctrl(&m);
}
int occupency_turnout_reserve(uint8_t turnout, int8_t numtrain)
{
	if (turnout >= NUM_TURNOUTS) return -1;
	//if (turnout<0) return -1;
	if (turnout>31) return -1;

	if (numtrain>=0) {
		uint8_t expected = numtrain;
		int ok = __atomic_compare_exchange_n(&lockedby[turnout], &expected, numtrain, 0 /*weak*/, __ATOMIC_ACQUIRE/*success memorder*/, __ATOMIC_ACQUIRE/*fail memorder*/ );
		if (!ok) {
			expected = 0xFF;
			ok = __atomic_compare_exchange_n(&lockedby[turnout], &expected, numtrain, 0 /*weak*/, __ATOMIC_ACQUIRE/*success memorder*/, __ATOMIC_ACQUIRE/*fail memorder*/ );
            _notify_chg_owner(turnout, numtrain);
		}
		if (!ok) {
			return -1;
		}
	}
	return 0;
}


void occupency_turnout_release(uint8_t turnout, _UNUSED_ int8_t train)
{
    int l = lockedby[turnout];
	lockedby[turnout] = 0xFF;
    if (l != 0xFF) {
        _notify_chg_owner(turnout, -1);
    }
}

static void occupency_turnout_release_for_train_canton(int8_t train, uint8_t canton)
{
	if (train<0) Error_Handler();
	for (int tn = 0; tn<NUM_TURNOUTS; tn++) {
		if (lockedby[tn] != train) continue;
		uint8_t ca1, ca2, ca3;
		topology_get_cantons_for_turnout(tn, &ca1, &ca2, &ca3);
		if ((ca1 == canton) || (ca2 == canton) || (ca3 == canton)) {
			occupency_turnout_release(tn, train);
		}
	}
}

