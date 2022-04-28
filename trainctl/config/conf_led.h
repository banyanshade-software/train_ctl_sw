// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_led_H_
#define _conf_led_H_

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



typedef struct conf_led {
    GPIO_TypeDef *port_led;
    uint16_t pin_led;
    uint8_t defprog;
} conf_led_t;


unsigned int conf_led_num_entries(void);
const conf_led_t *conf_led_get(int num);



#ifdef TRN_BOARD_MAINV04
#define NUM_LEDS 0 // 0 
#endif // TRN_BOARD_MAINV04



#ifdef TRN_BOARD_MAINV0
#define NUM_LEDS 3 // 3 
#endif // TRN_BOARD_MAINV0



#ifdef TRN_BOARD_DISPATCHER
#define NUM_LEDS 0 // 0 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_LEDS 0 // 0 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_LEDS 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO



#ifdef TRN_BOARD_SIMU
#define NUM_LEDS 3 // 3 
#endif // TRN_BOARD_SIMU


#define MAX_LEDS 3




#endif
