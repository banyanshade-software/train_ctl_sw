//
//  uitrack.h
//  train_throttle
//
//  Created by Daniel BRAUN on 06/08/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef uitrack_h
#define uitrack_h

#include "topology.h"

void uitrack_run_tick(uint32_t notif_flags, uint32_t tick, uint32_t dt);

void impl_uitrack_change_blk(int blk, int v, int trn, int sblk);
void impl_uitrack_change_tn(int tn, enum topo_turnout_state v);
void impl_uitrack_change_tn_reserv(int tn, int train);
void impl_uitrack_change_pres(uint32_t bitfield);

#endif /* uitrack_h */
