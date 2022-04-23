// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_pidctl_H_
#define _conf_pidctl_H_

#include <stdint.h>

// this code goes in all .h files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

#include "trainctl_config.h"



struct conf_pidctl {
    int32_t kP;
    int32_t kI;
    int32_t kD;
};




#endif
