// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_boards_H_
#define _conf_boards_H_

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



typedef struct conf_boards {
    uint32_t uuid;
    uint8_t board_type;
    uint8_t disable;
} conf_boards_t;


int conf_boards_num_entries(void);
const conf_boards_t *conf_boards_get(int num);




#endif
