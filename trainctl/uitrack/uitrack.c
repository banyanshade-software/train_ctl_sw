//
//  uitrack.c
//  train_throttle
//
//  Created by Daniel BRAUN on 06/08/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include <stdio.h>
#include "uitrack.h"

#include "../msg/trainmsg.h"
#include "../misc.h"



#ifndef BOARD_HAS_IHM
#error BOARD_HAS_IHM not defined, remove this file from build
#endif


static void uitrack_reset(void);
static void uitrack_change_blk(int blk, int v, int trn, int sblk);
static void uitrack_change_tn(int tn, int v);
static void uitrack_change_tn_reserv(int tn, int train);

void uitrack_run_tick(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
    for (;;) {
        msg_64_t m;
        int rc = mqf_read_to_ui_track(&m);
        if (rc) break;
        if (m.to != MA_UI(UISUB_TRACK)) continue;
        int blk; int v; int trn; int sblk; //int rst;
        switch (m.cmd) {
        default:
        	break;
        case CMD_BLK_CHG_NOTIF:
        	blk = m.vbytes[0];
        	v   = m.vbytes[1];
            trn = (int8_t) m.vbytes[2];
        	sblk = (int8_t) m.vbytes[3];
        	uitrack_change_blk(blk, v, trn, sblk);
        	break;
        case CMD_TN_RESER_NOTIF:
            uitrack_change_tn_reserv(m.v1, m.v2);
            break;
        case CMD_TURNOUT_A:
        	uitrack_change_tn(m.v2, 0);
        	break;
        case CMD_TURNOUT_B:
        	uitrack_change_tn(m.v2, 1);
        	break;
        }
    }
}


static _UNUSED_ void uitrack_reset(void)
{
    
}
static void uitrack_change_blk(int blk, int v, int trn, int sblk)
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


void  __attribute__((weak))  impl_uitrack_change_blk(_UNUSED_ int blk, _UNUSED_ int v, _UNUSED_ int trn, _UNUSED_ int sblk)
{
    
}

void  __attribute__((weak))  impl_uitrack_change_tn(_UNUSED_ int tn, _UNUSED_ int v)
{
    
}


void  __attribute__((weak))  impl_uitrack_change_tn_reserv(_UNUSED_ int tn, _UNUSED_ int v)
{
    
}
