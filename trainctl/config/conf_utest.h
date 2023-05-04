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

#include "trainctl_config.h"



// error used only for unit test



typedef struct conf_utest {
    uint32_t fixed;
    uint32_t alpha;
    uint32_t beta;
} conf_utest_t;


unsigned int conf_utest_num_entries(void);
const conf_utest_t *conf_utest_get(int num);



#ifdef TRN_BOARD_G4SLV1
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_G4SLV1



#ifdef TRN_BOARD_UNIT_TEST
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_UNIT_TEST



#ifdef TRN_BOARD_UI
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_UI



#ifdef TRN_BOARD_MAINV04
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_MAINV04



#ifdef TRN_BOARD_MAINV0
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_MAINV0



#ifdef TRN_BOARD_DISPATCHER
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_MAIN_ZERO



#ifdef TRN_BOARD_SIMU
#define NUM_UTESTS 1 // 1 
#endif // TRN_BOARD_SIMU


#define MAX_UTESTS 1




#endif
