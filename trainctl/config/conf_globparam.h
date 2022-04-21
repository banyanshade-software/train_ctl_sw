// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_globparam_H_
#define _conf_globparam_H_

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



typedef struct conf_globparam {
    uint16_t pwmfreq;
    int test_mode;
    int oscilo;
} conf_globparam_t;


int conf_globparam_num_entries(void);
const conf_globparam_t *conf_globparam_get(int num);



#ifdef TRN_BOARD_MAIN
#define NUM_GLOBPARAMS 1 // 1 
#endif // TRN_BOARD_MAIN



#ifdef TRN_BOARD_DISPATCHER
#define NUM_GLOBPARAMS 0 // 0 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_GLOBPARAMS 0 // 0 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_GLOBPARAMS 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO


#define MAX_GLOBPARAMS 1




#endif