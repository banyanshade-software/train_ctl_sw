//
//  occupency.c
//  train_throttle
//
//  Created by Daniel BRAUN on 27/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

//#include <stdio.h>

#include "misc.h"
#include "../msg/trainmsg.h"
#include "topology.h"
#include "occupency.h"
#include "railconfig.h"



typedef struct {
    uint8_t occ;
    uint8_t trnum;
    lsblk_num_t lsblk;
} canton_occ_t;

static  canton_occ_t canton_occ[0x40] = {0};


uint8_t occupency_changed = 0;

static inline uint8_t addr_to_num(uint8_t addr)
{
    // MA_CANTON(<#_board#>, <#_c#>)
    // // M2:  0 0 : (6bits) bbb xxx        CANTON (00) and TURNOUT (01)
    return addr & 0x3F;
}

static uint8_t notif_blk_reset = 1;

static void notif_blk_occup_chg(int blknum, uint8_t val, uint8_t trnum)
{
    msg_64_t m;
    m.from = MA_CONTROL();
    m.to = MA_UI(UISUB_TRACK);
    m.cmd = CMD_BLK_CHANGE;
    m.vbytes[0] = blknum;
    m.vbytes[1] = val;
    m.vbytes[2] = trnum;
    m.vbytes[3] = 0;
    notif_blk_reset = 0;
    mqf_write_from_ctrl(&m);
}

void set_block_addr_occupency(uint8_t blkaddr, uint8_t v, uint8_t trnum, lsblk_num_t lsb)
{
    if (0xFF == blkaddr) Error_Handler();
    canton_occ_t *co = &canton_occ[addr_to_num(blkaddr)];
    if (co->occ != v) {
        if (USE_BLOCK_DELAY_FREE && (v==BLK_OCC_FREE)) {
            if (co->occ >= BLK_OCC_DELAY1) Error_Handler();
            co->occ = BLK_OCC_DELAYM;
            itm_debug1(DBG_CTRL, "delay free", blkaddr);
        } else {
            co->occ = v;
            occupency_changed = 1;
        }
    }
    co->trnum = trnum;
    if (co->lsblk.n != lsb.n) {
        co->lsblk = lsb;
        occupency_changed = 1;
    }
    if (occupency_changed) {    notif_blk_occup_chg(blkaddr, co->occ, trnum);
    }
}



uint8_t get_block_addr_occupency(uint8_t blkaddr)
{
    if (0xFF == blkaddr) Error_Handler();
    return canton_occ[addr_to_num(blkaddr)].occ;
}




void check_block_delayed(_UNUSED_ uint32_t tick)
{
    if (!USE_BLOCK_DELAY_FREE) return;

    static uint32_t lastcheck = 0;
    if (tick<lastcheck+100) return;
    lastcheck = tick;

    for (int i=0; i<0x40; i++) {
        if (canton_occ[i].occ == BLK_OCC_DELAY1) {
            itm_debug1(DBG_CTRL, "FREE(d)", i);
            canton_occ[i].occ = BLK_OCC_FREE;
            occupency_changed = 1;
            notif_blk_occup_chg(i, canton_occ[i].occ, 0xFF);
        } else if (canton_occ[i].occ > BLK_OCC_DELAY1) {
            canton_occ[i].occ --;
        }
    }
}
