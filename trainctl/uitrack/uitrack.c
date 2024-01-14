//
//  uitrack.c
//  train_throttle
//
//  Created by Daniel BRAUN on 06/08/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <stdio.h>
#include "trainctl_config.h"
#include "uitrack.h"

#include "../msg/trainmsg.h"
#include "../misc.h"



#ifndef BOARD_HAS_UI_CTC
#error BOARD_HAS_UI_CTC not defined, remove this file from build
#endif


static void uitrack_reset(void);
static void uitrack_change_blk(uint8_t blk, uint8_t v, uint8_t trn, uint8_t sblk);
static void uitrack_change_tn(int tn, int v);
static void uitrack_change_tn_reserv(int tn, int train);
static void uitrack_change_pres(uint32_t bitfield);

void uitrack_run_tick(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
    for (;;) {
        msg_64_t m;
        int rc = mqf_read_to_ui_track(&m);
        if (rc) break;
        if (m.to != MA3_UI_CTC) continue;
        uint8_t blk, v, trn, sblk;
        switch (m.cmd) {
        default:
        	break;
        case CMD_BLK_CHG_NOTIF:
        	blk = m.vbytes[0];
        	v   = m.vbytes[1];
            trn =  m.vbytes[2];
        	sblk = m.vbytes[3];
        	uitrack_change_blk(blk, v, trn, sblk);
        	break;
        case CMD_TN_RESER_NOTIF:
            uitrack_change_tn_reserv(m.v1, m.v2);
            break;
        case CMD_TN_CHG_NOTIF:
            uitrack_change_tn(m.subc, m.v1);
            break;
        /*
        case CMD_TURNOUT_A:
        	uitrack_change_tn(m.subc, 0);
        	break;
        case CMD_TURNOUT_B:
        	uitrack_change_tn(m.subc, 1);
        	break;
         */
        case CMD_NOTIF_PRES:
            uitrack_change_pres(m.v32u);
            break;
        }
    }
}


static _UNUSED_ void uitrack_reset(void)
{
    
}
static void uitrack_change_blk(uint8_t blk, uint8_t v, uint8_t trn, uint8_t sblk)
{
    impl_uitrack_change_blk(blk, v, trn, sblk);
}

static void uitrack_change_tn(int tn, int v)
{
    impl_uitrack_change_tn(tn, v);
}

static void uitrack_change_tn_reserv(int tn, int train)
{
    impl_uitrack_change_tn_reserv(tn, train);
}

static void uitrack_change_pres(uint32_t bitfield)
{
    impl_uitrack_change_pres(bitfield);
}

void  __attribute__((weak))  impl_uitrack_change_pres(_UNUSED_ uint32_t bitfield)
{
    
}

void  __attribute__((weak))  impl_uitrack_change_blk(_UNUSED_ uint8_t blk, _UNUSED_ uint8_t v, _UNUSED_ uint8_t trn, _UNUSED_ uint8_t sblk)
{
    
}

void  __attribute__((weak))  impl_uitrack_change_tn(_UNUSED_ int tn, _UNUSED_ enum topo_turnout_state v)
{
    
}


void  __attribute__((weak))  impl_uitrack_change_tn_reserv(_UNUSED_ int tn, _UNUSED_ int v)
{
    
}
