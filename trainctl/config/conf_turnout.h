// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_turnout_H_
#define _conf_turnout_H_

#include <stdint.h>

// this code goes in all .h files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

#include "trainctl_config.h"




#ifndef TRAIN_SIMU
#ifdef STM32_F4
#include "stm32f4xx_hal.h"
#else
#include "stm32f1xx_hal.h"
#endif
#else
typedef void *GPIO_TypeDef;
#endif



typedef struct conf_turnout {
    GPIO_TypeDef *cmd_portA;
    GPIO_TypeDef *cmd_portB;
    uint16_t pinA;
    uint16_t pinB;
    uint8_t reverse;
} conf_turnout_t;


unsigned int conf_turnout_num_entries(void);
const conf_turnout_t *conf_turnout_get(int num);



#ifdef TRN_BOARD_UI
#define NUM_TURNOUTS 0 // 0 
#endif // TRN_BOARD_UI



#ifdef TRN_BOARD_MAINV04
#define NUM_TURNOUTS 6 // 6 
#endif // TRN_BOARD_MAINV04



#ifdef TRN_BOARD_MAINV0
#define NUM_TURNOUTS 3 // 3 
#endif // TRN_BOARD_MAINV0



#ifdef TRN_BOARD_DISPATCHER
#define NUM_TURNOUTS 0 // 0 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_TURNOUTS 8 // 8 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_TURNOUTS 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO



#ifdef TRN_BOARD_SIMU
#define NUM_TURNOUTS 4 // 4 
#endif // TRN_BOARD_SIMU


#define MAX_TURNOUTS 8




#endif
