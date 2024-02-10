//
//  oam_detect.c
//  train_throttle
//
//  Created by Daniel Braun on 10/02/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//

#include <stdint.h>

#include "oam_detect.h"
#include "../config/conf_train.h"
#include "locomotives.h"

static uint8_t sblk[NUM_TRAINS];

void oam_detect_reset(void)
{
    for (int i=0; i<NUM_TRAINS; i++) {
        sblk[i] = 0xFF;
    }
}
void oam_train_detected(uint8_t canton, uint8_t lsblk, uint8_t _loco, uint8_t reserved)
{
    //locomotive_t loco = (locomotive_t) _loco;
    int n = conf_train_num_entries();
    if (n>NUM_TRAINS) n = NUM_TRAINS;
    int idxavail = -1; // index of config slot that can be used to add a train
    for (int i=0; i<n; i++) {
        if (sblk[i] != 0xFF) continue;
        const conf_train_t *conf = conf_train_get(i);
        conf_train_t *wconf = (conf_train_t *)conf; // writable
        if (conf->locotype == _loco) {
            sblk[i] = lsblk;
            wconf->enabled = 1;
            continue;
        } else if (!conf->enabled && (conf->locotype == 0)) {
            idxavail = i;
        }
        // not found : add a train
        if (idxavail<0) {
            itm_debug3(DBG_ERR, "cantadd", canton, lsblk, _loco);
            return;
        }
        sblk[idxavail] = lsblk;
        wconf->enabled = 1;
        wconf->locotype = _loco;
        if (!conf->trainlen_right_cm) wconf->trainlen_right_cm = 10;
        if (!conf->trainlen_left_cm) wconf->trainlen_left_cm = 10;
    }

}

int oam_start_sblk_for_train(uint8_t tidx)
{
    if (tidx > NUM_TRAINS) return 0xFF;
    return sblk[tidx];
}
