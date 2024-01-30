//
//  train2loco.c
//  train_throttle
//
//  Created by Daniel Braun on 30/01/2024.
//  Copyright © 2024 Daniel BRAUN. All rights reserved.
//


#include "../config/conf_train.h"
#include "../config/conf_locomotive.h"
#include "train2loco.h"

typedef struct {
    const conf_locomotive_t *confloco;
    uint8_t _locotype;
} t2l_t;

static t2l_t trctl[NUM_TRAINS] = {0};

// -----------------------------------------------------------------

const conf_locomotive_t *getloco(int tidx)
{
    const conf_train_t *tconf = conf_train_get(tidx);
    t2l_t *tvars = &trctl[tidx];
    if ((!tvars->confloco) || (tconf->locotype != tvars->_locotype)) {
        tvars->confloco = conf_locomotive_get(tconf->locotype);
        tvars->_locotype = tconf->locotype;
    }
    return tvars->confloco;
}



// -----------------------------------------------------------------
