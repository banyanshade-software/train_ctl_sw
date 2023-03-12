//
//  c3autoP.c
//  train_throttle
//
//  Created by Daniel Braun on 11/03/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//



#include "../misc.h"
#include "../msg/trainmsg.h"

#include "../topology/topology.h"
#include "../topology/occupency.h"

#ifndef BOARD_HAS_CTRL
#error BOARD_HAS_CTRL not defined, remove this file from build
#endif


#include "ctrl.h"
#include "ctrlLT.h"

#include "c3autoP.h"


#ifndef __clang__
static_assert(sizeof(cauto_path_items_t) == 2);
#else
typedef char compile_assert_c3[(sizeof(cauto_path_items_t) == 2) ? 1 : -1];
#endif


typedef struct st_path {
    int cidx;
    cauto_path_items_t path[C3AUTO_NUM_PATH_ITEM];
} cauto_vars_t;

cauto_vars_t c3avar[MAX_TRAINS];

void c3auto_set_s1(int tidx, lsblk_num_t s1)
{
    abort();
}
void c3auto_set_turnout(int tidx, xtrnaddr_t tn)
{
    abort();
}
cauto_path_items_t *c3auto_get_path(int numtrain)
{
    return c3avar[numtrain].path;
}



#ifdef TRAIN_SIMU
void c3auto_dump(cauto_path_items_t *p)
{
    for (;;) {
        if (p->t) {
            printf("tn %d pos %d --", p->tn.v, p->val);
        } else {
            printf("sblk %d spd %d --", p->sblk.n, p->val*2);
            if (p->val ==0) break;
        }
        p++;
    }
    printf("\n");
}
#endif
