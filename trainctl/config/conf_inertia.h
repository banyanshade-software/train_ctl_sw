// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_inertia_H_
#define _conf_inertia_H_

#include <stdint.h>

// this code goes in all .h files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#endif



struct conf_inertia {
    int16_t dec;
    int16_t acc;
};




#endif
