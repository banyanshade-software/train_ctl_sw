//
//  c3autoP.h
//  train_throttle
//
//  Created by Daniel Braun on 11/03/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#ifndef c3autoP_h
#define c3autoP_h

void c3auto_start(int tidx);
void c3auto_set_turnout(int tidx, xtrnaddr_t tn);
void c3auto_set_s1(int tidx, lsblk_num_t s1);
void c3auto_freeback(int tidx, lsblk_num_t freelsblk);

typedef struct {
    uint8_t t:1;        // t=0: lsblk, t=1: turnout
    int8_t val:7;       // lsblk: signed speed divided by 2, turnout: pos (0=straight, 1=turn)
    union {
        lsblk_num_t sblk;
        xtrnaddr_t tn;
    };
} cauto_path_items_t;

#define C3AUTO_NUM_PATH_ITEM 32

cauto_path_items_t *c3auto_get_path(int numtrain);

#ifdef TRAIN_SIMU
void c3auto_dump(cauto_path_items_t *p);
#endif

#endif /* c3autoP_h */
