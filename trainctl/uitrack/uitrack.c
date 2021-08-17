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
#include "misc.h"

static void uitrack_reset(void);
static void uitrack_change_blk(int blk, int v, int trn);
static void uitrack_change_tn(int tn, int v);

void uitrack_run_tick(_UNUSED_ uint32_t notif_flags, uint32_t tick, _UNUSED_ uint32_t dt)
{
    for (;;) {
        msg_64_t m;
        int rc = mqf_read_to_ui_track(&m);
        if (rc) break;
        if (m.to != MA_UI(UISUB_TRACK)) continue;
        int blk; int v; int trn; //int rst;
        switch (m.cmd) {
        default:
        	break;
        case CMD_BLK_CHANGE:
        	blk = m.vbytes[0];
        	v   = m.vbytes[1];
            trn = m.vbytes[2];
        	/*rst = m.vbytes[2];
        	if (rst) {
        		uitrack_reset();
        	}*/
        	uitrack_change_blk(blk, v, trn);
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


static void uitrack_reset(void)
{
    
}
static void uitrack_change_blk(int blk, int v, int trn)
{
    impl_uitrack_change_blk(blk, v, trn);
}

static void uitrack_change_tn(int tn, int v)
{
    impl_uitrack_change_tn(tn, v);
}

void  __attribute__((weak))  impl_uitrack_change_blk(_UNUSED_ int blk, _UNUSED_ int v, int trn)
{
    
}

void  __attribute__((weak))  impl_uitrack_change_tn(_UNUSED_ int tn, _UNUSED_ int v)
{
    
}
