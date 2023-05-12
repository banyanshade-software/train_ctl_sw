// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_servo_H_
#define _conf_servo_H_

#include <stdint.h>

// this code goes in all .h files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

#include "trainctl_config.h"




#ifndef TRAIN_SIMU

#if defined(STM32F4)
#include "stm32f4xx_hal.h"

#elif defined(STM32G4)
#include "stm32g4xx_hal.h"

#elif defined(STM32F1)
#include "stm32f1xx_hal.h"

#else
#error no board hal
#endif


#else
typedef void *GPIO_TypeDef;
#endif





typedef struct conf_servo {
    GPIO_TypeDef *port_power;
    int16_t pin_power;
    int8_t pwm_timer_num;
    int8_t pwm_timer_ch;
    int8_t direction;
    uint16_t min;
    uint16_t max;
    uint16_t spd;
    uint16_t pos0;
} conf_servo_t;


unsigned int conf_servo_num_entries(void);
const conf_servo_t *conf_servo_get(int num);



#ifdef TRN_BOARD_G4SLV1
#define NUM_SERVOS 1 // 1 
#endif // TRN_BOARD_G4SLV1



#ifdef TRN_BOARD_G4MASTER1
#define NUM_SERVOS 1 // 1 
#endif // TRN_BOARD_G4MASTER1



#ifdef TRN_BOARD_UNIT_TEST
#define NUM_SERVOS 4 // 4 
#endif // TRN_BOARD_UNIT_TEST



#ifdef TRN_BOARD_UI
#define NUM_SERVOS 0 // 0 
#endif // TRN_BOARD_UI



#ifdef TRN_BOARD_MAINV04
#define NUM_SERVOS 0 // 0 
#endif // TRN_BOARD_MAINV04



#ifdef TRN_BOARD_MAINV0
#define NUM_SERVOS 0 // 0 
#endif // TRN_BOARD_MAINV0



#ifdef TRN_BOARD_DISPATCHER
#define NUM_SERVOS 0 // 0 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_SERVOS 2 // 2 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_SERVOS 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO



#ifdef TRN_BOARD_SIMU
#define NUM_SERVOS 2 // 2 
#endif // TRN_BOARD_SIMU


#define MAX_SERVOS 4




#endif
