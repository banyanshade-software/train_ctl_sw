// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_utestloc_H_
#define _conf_utestloc_H_

#include <stdint.h>

// this code goes in all .h files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

#include "trainctl_config.h"



typedef struct conf_utestloc {
    uint32_t fixed;
    uint32_t alpha;
    uint32_t beta;
} conf_utestloc_t;


unsigned int conf_utestloc_num_entries(void);
const conf_utestloc_t *conf_utestloc_get(int num);



#ifdef TRN_BOARD_MAIN
#define NUM_UTESTLOCS 4 // 4 
#endif // TRN_BOARD_MAIN



#ifdef TRN_BOARD_DISPATCHER
#define NUM_UTESTLOCS 0 // 0 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_UTESTLOCS 0 // 0 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_UTESTLOCS 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO


#define MAX_UTESTLOCS 4




#endif
