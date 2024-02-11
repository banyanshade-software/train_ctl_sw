//
//  uigen.c
//  train_throttle
//
//  Created by Daniel Braun on 27/01/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//


#include <stdio.h>
#include "trainctl_config.h"
#include "uigen.h"

#include "../msg/trainmsg.h"
#include "../misc.h"



#ifndef BOARD_HAS_UI_GEN
#error BOARD_HAS_UI_GEN not defined, remove this file from build
#endif

void __attribute__((weak)) received_ui_gen(msg_64_t *m)
{
}

void __attribute__((weak)) received_broadcast(msg_64_t *m)
{
}
void uigen_run_tick(_UNUSED_ uint32_t notif_flags, _UNUSED_ uint32_t tick, _UNUSED_ uint32_t dt)
{
    for (;;) {
        msg_64_t m;
        int rc = mqf_read_to_ui(&m);
        if (rc) break;
        if (m.to == MA3_BROADCAST) {
            received_broadcast(&m);
            continue;
        }
        if (m.to != MA3_UI_GEN) {
            itm_debug2(DBG_MSG, "notuigen", m.to, m.cmd);
            continue;
        }
        
        received_ui_gen(&m);
    }
}
