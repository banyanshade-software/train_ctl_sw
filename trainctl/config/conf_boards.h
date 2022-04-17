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



#ifdef TRN_BOARD_MAIN
#define NUM_BOARDSS 8 // 8 
#endif // TRN_BOARD_MAIN



#ifdef TRN_BOARD_DISPATCHER
#define NUM_BOARDSS 0 // 0 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_BOARDSS 0 // 0 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_BOARDSS 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO


#define MAX_BOARDSS 8




#endif
