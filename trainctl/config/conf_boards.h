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

#include "trainctl_config.h"


typedef enum {
    board_unknown = 0, // not used
    board_mainV04  = 1,
    board_switcher = 16,
    board_dispatcher = 32,
    board_ui = 48,

    // will be removed
    board_mainV0  = 250,
} board_type_t;


typedef struct conf_boards {
    uint32_t uuid;
    board_type_t btype;
    uint8_t disable:1;
    uint8_t master:1;
} conf_boards_t;


unsigned int conf_boards_num_entries(void);
const conf_boards_t *conf_boards_get(int num);



#ifdef TRN_BOARD_MAINV04
#define NUM_BOARDSS 16 // 16 
#endif // TRN_BOARD_MAINV04



#ifdef TRN_BOARD_MAINV0
#define NUM_BOARDSS 16 // 16 
#endif // TRN_BOARD_MAINV0



#ifdef TRN_BOARD_DISPATCHER
#define NUM_BOARDSS 0 // 0 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_BOARDSS 0 // 0 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_BOARDSS 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO



#ifdef TRN_BOARD_SIMU
#define NUM_BOARDSS 0 // 0 
#endif // TRN_BOARD_SIMU


#define MAX_BOARDSS 16




#endif
