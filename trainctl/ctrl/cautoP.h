//
//  cautoP.h
//  train_throttle
//
//  Created by Daniel BRAUN on 13/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef cautoP_h
#define cautoP_h

int cauto_update_turnouts(int tidx, lsblk_num_t cur, int8_t dir, uint8_t next);

extern void ctrl2_set_turnout(int tn, int v); // ctrl.c TODO move proto

#endif /* cautoP_h */
