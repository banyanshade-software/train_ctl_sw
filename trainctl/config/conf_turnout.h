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

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#endif




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
    GPIO_TypeDef *port;
    uint16_t pinA;
    uint16_t pinB;
    uint8_t reverse;
} conf_turnout_t;


int conf_turnout_num_entries(void);
const conf_turnout_t *conf_turnout_get(int num);




#endif
