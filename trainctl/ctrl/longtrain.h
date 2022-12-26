//
//  longtrain.h
//  train_throttle
//
//  Created by Daniel Braun on 20/12/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#ifndef longtrain_h
#define longtrain_h

#include <stdio.h>

/// return value for ctrl2_check_front_sblks
#define NUMTRIGS 3
struct sttrig {
    int16_t poscm;
    int8_t tag;
};
typedef struct {
    uint8_t isoet:1;
    uint8_t isocc:1;
    struct sttrig trigs[NUMTRIGS];
} rettrigs_t;

// -------------------------------------------------


int ctrl3_get_next_sblks_(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left, lsblk_num_t *resp, int nsblk, int16_t *premainlen);

int ctrl3_get_next_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf);

/// ctrl3_check_front_sblks
/// Find values for triggers for : next check, brake, and stop
/// It is assumed that leftcars and rightcars are up to date
/// (ie. that ctrl3_get_next_sblks() was called recently)
/// @param tidx train number
/// @param tvars train tvars structure
/// @param tconf triain tconf config pointer
/// @param left 1 if train is going left, 0 if it is going right
/// @param ret return triggers to be set
/// @return int             -1 if train should stop immediatly (or should not start),
///                 >0 value if train should brake
///                 0 otherwise
int ctrl3_check_front_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left,  rettrigs_t *ret);


/// ctrl3_update_front_sblks
/// called when tag_chkocc occurs, it will update tvars->leftcars  and tvars->rightcars
/// @param tidx train number
/// @param tvars train tvars structure
/// @param tconf triain tconf config pointer
/// @param left 1 if train is going left, 0 if it is going right
int ctrl3_update_front_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left);



/// called when c1sblk changed, it will update tvars->leftcars  and tvars->rightcars
/// @param tidx train number
/// @param tvars train tvars structure
/// @param tconf triain tconf config pointer
/// @param left 1 if train is going left, 0 if it is going right
int ctrl3_update_front_sblks_c1changed(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left);

#endif /* longtrain_h */