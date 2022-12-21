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
struct sttrig {
    int16_t poscm;
    int8_t tag;
};
typedef struct sttrig rettrigs_t[3];

// -------------------------------------------------


int ctrl3_get_next_sblks_(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left, lsblk_num_t *resp, int nsblk, int16_t *premainlen);

int ctrl3_get_next_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf);

int ctrl3_check_front_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left,  rettrigs_t ret);

int ctrl3_update_front_sblks(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left);

int ctrl3_update_front_sblks_c1changed(int tidx, train_ctrl_t *tvars,  const conf_train_t *tconf, int left);

#endif /* longtrain_h */
