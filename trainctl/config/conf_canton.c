// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_canton.h"
#include "conf_canton.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#include <stdio.h>
#endif



#ifdef TRN_BOARD_MAIN

int conf_canton_num_entries(void)
{
    return 6; // 6 
}

static conf_canton_t conf_canton[6] = {
  {     // 0
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_0,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_1,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_2,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse_bemf = 0,
  },
  {     // 1
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_3,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_4,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_5,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse_bemf = 0,
  },
  {     // 2
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_6,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_7,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_8,
     .pwm_timer_num = 2,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse_bemf = 0,
  },
  {     // 3
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOD,
     .volt_b0 = GPIO_PIN_9,
     .volt_port_b1 = GPIOD,
     .volt_b1 = GPIO_PIN_10,
     .volt_port_b2 = GPIOD,
     .volt_b2 = GPIO_PIN_11,
     .pwm_timer_num = 3,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_2,
     .notif_bemf = 0,
     .reverse_bemf = 0,
  },
  {     // 4
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = GPIOE,
     .volt_b0 = GPIO_PIN_2,
     .volt_port_b1 = GPIOE,
     .volt_b1 = GPIO_PIN_3,
     .volt_port_b2 = GPIOE,
     .volt_b2 = GPIO_PIN_5,
     .pwm_timer_num = 3,
     .ch0 = TIM_CHANNEL_3,
     .ch1 = TIM_CHANNEL_4,
     .notif_bemf = 0,
     .reverse_bemf = 0,
  },
  {     // 5
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = GPIO_PIN_0,
     .volt_port_b1 = NULL,
     .volt_b1 = GPIO_PIN_0,
     .volt_port_b2 = NULL,
     .volt_b2 = GPIO_PIN_0,
     .pwm_timer_num = 0,
     .ch0 = 0,
     .ch1 = 0,
     .notif_bemf = 0,
     .reverse_bemf = 0,
  }
};

#endif // TRN_BOARD_MAIN




#ifdef TRN_BOARD_DISPATCHER

int conf_canton_num_entries(void)
{
    return 1; // 1 
}

static conf_canton_t conf_canton[1] = {
  {     // 0
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_1,
     .notif_bemf = 0,
     .reverse_bemf = 0,
  }
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

int conf_canton_num_entries(void)
{
    return 0; // 0 
}

static conf_canton_t conf_canton[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

int conf_canton_num_entries(void)
{
    return 0; // 0 
}

static conf_canton_t conf_canton[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




const conf_canton_t *conf_canton_get(int num)
{
  if (num<0) return NULL;
    if (num>=conf_canton_num_entries()) {
        return NULL;
    }
    return &conf_canton[num];
}

static const conf_canton_t canton_template = {
     .volts_cv = { 1000, 770, 621,  538, 451, 413, 379, 355},
     .volt_port_b0 = NULL,
     .volt_b0 = 0,
     .volt_port_b1 = NULL,
     .volt_b1 = 0,
     .volt_port_b2 = NULL,
     .volt_b2 = 0,
     .pwm_timer_num = 1,
     .ch0 = TIM_CHANNEL_1,
     .ch1 = TIM_CHANNEL_1,
     .notif_bemf = 0,
     .reverse_bemf = 0,
  };
const conf_canton_t *conf_canton_template(void)
{
  return &canton_template;
}

// canton config store type 0 num 2
int conf_canton_propagate(int numinst, int numfield, int32_t value)
{
    if (numinst>=conf_canton_num_entries()) return -1;
    conf_canton_t *conf = &conf_canton[numinst];
    switch (numfield) {
    default: return -1;
    case conf_numfield_reverse_bemf:
        conf->reverse_bemf = value;
        break;
    }
    return 0;
}





// end
