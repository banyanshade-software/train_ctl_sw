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
    uint8_t reverse_bemf:1;
} conf_canton_t;


int conf_canton_num_entries(void);
const conf_canton_t *conf_canton_get(int num);




#endif
