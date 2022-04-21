// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_utest_H_
#define _conf_utest_H_

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



#ifndef TRAIN_SIMU
#error used only for unit test
#endif



typedef struct conf_utest {
    uint32_t fixed;
    uint32_t alpha;
    uint32_t beta;
} conf_utest_t;


int conf_utest_num_entries(void);
const conf_utest_t *conf_utest_get(int num);



#ifdef TRN_BOARD_MAIN
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_MAIN



#ifdef TRN_BOARD_DISPATCHER
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_MAIN_ZERO


#define MAX_UTESTS 1




#endif
