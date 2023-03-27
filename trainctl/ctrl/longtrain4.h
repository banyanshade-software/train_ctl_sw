//
//  longtrain4.h
//  train_throttle
//
//  Created by Daniel Braun on 27/03/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#ifndef longtrain4_h
#define longtrain4_h


/// return value for ctrl2_check_front_sblks
#define NUMTRIGS 6
struct sttrig {
    int16_t posmm;
    int8_t tag;
};
typedef struct {
    uint8_t isoet:1;
    uint8_t isocc:1;
    uint8_t res_c2:1;
    uint8_t power_c2:1;
    uint8_t ntrig;
    struct sttrig trigs[NUMTRIGS];
} rettrigs_t;

int lt4_get_trigs(int tidx, train_ctrl_t *tvars, const conf_train_t *tconf, int left,  rettrigs_t *rett);


int lt4_check_back(int tidx, train_ctrl_t *tvars, const conf_train_t *tconf, int left, rettrigs_t *rett);

#endif /* longtrain4_h */
