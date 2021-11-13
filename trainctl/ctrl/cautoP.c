//
//  cautoP.c
//  train_throttle
//
//  Created by Daniel BRAUN on 13/11/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#include "misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

#include "cautoP.h"


int cauto_update_turnouts(int tidx, lsblk_num_t cur, int8_t dir, uint8_t next)
{
    lsblk_num_t ta, tb;
    int tn;
    next_lsblk_nums(cur, (dir<0) ? 1 : 0, &ta, &tb, &tn);
    if (ta.n == next) {
        int v = topology_get_turnout(tn);
        if (!v) return 0;
        ctrl2_set_turnout(tn, 0);
        return 1;
    }
    if (tb.n == next) {
        int v = topology_get_turnout(tn);
        if (v) return 0;
        ctrl2_set_turnout(tn, 1);
        return 1;
    }
    return -1;
}

