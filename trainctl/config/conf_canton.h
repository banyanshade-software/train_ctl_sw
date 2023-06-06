// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_canton_H_
#define _conf_canton_H_

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





typedef struct conf_canton {
    uint16_t volts_cv[8];
    GPIO_TypeDef *volt_port_b0;
    uint16_t volt_b0;
    GPIO_TypeDef *volt_port_b1;
    uint16_t volt_b1;
    GPIO_TypeDef *volt_port_b2;
    uint16_t volt_b2;
    uint8_t pwm_timer_num;
    uint32_t ch0;
    uint32_t ch1;
    uint8_t notif_bemf:1;
    uint8_t reverse:1;
    uint8_t reverse_bemf:1;
} conf_canton_t;


unsigned int conf_canton_num_entries(void);
const conf_canton_t *conf_canton_get(int num);



#ifdef TRN_BOARD_G4SLV1
#define NUM_CANTONS 4 // 4 
#endif // TRN_BOARD_G4SLV1



#ifdef TRN_BOARD_G4MASTER1
#define NUM_CANTONS 4 // 4 
#endif // TRN_BOARD_G4MASTER1



#ifdef TRN_BOARD_UNIT_TEST
#define NUM_CANTONS 6 // 6 
#endif // TRN_BOARD_UNIT_TEST



#ifdef TRN_BOARD_UI
#define NUM_CANTONS 0 // 0 
#endif // TRN_BOARD_UI



#ifdef TRN_BOARD_MAINV04
#define NUM_CANTONS 6 // 6 
#endif // TRN_BOARD_MAINV04



#ifdef TRN_BOARD_MAINV0
#define NUM_CANTONS 6 // 6 
#endif // TRN_BOARD_MAINV0



#ifdef TRN_BOARD_DISPATCHER
#define NUM_CANTONS 1 // 1 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_CANTONS 0 // 0 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_CANTONS 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO



#ifdef TRN_BOARD_SIMU
#define NUM_CANTONS 6 // 6 
#endif // TRN_BOARD_SIMU


#define MAX_CANTONS 6


const conf_canton_t *conf_canton_template(void);




#endif
