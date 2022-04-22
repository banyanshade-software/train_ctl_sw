// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_led.h"
#include "conf_led.propag.h"



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

int conf_led_num_entries(void)
{
    return 3; // 3 
}

static conf_led_t conf_led[3] = {
  {     // 0
     .port_led = GPIOA,
     .pin_led = GPIO_PIN_7,
     .defprog = 0,
  },
  {     // 1
     .port_led = GPIOA,
     .pin_led = GPIO_PIN_7,
     .defprog = 0,
  },
  {     // 2
     .port_led = GPIOA,
     .pin_led = GPIO_PIN_7,
     .defprog = 0,
  }
};

#endif // TRN_BOARD_MAIN




#ifdef TRN_BOARD_DISPATCHER

int conf_led_num_entries(void)
{
    return 0; // 0 
}

static conf_led_t conf_led[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

int conf_led_num_entries(void)
{
    return 0; // 0 
}

static conf_led_t conf_led[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

int conf_led_num_entries(void)
{
    return 0; // 0 
}

static conf_led_t conf_led[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




const conf_led_t *conf_led_get(int num)
{
  if (num<0) return NULL;
    if (num>=conf_led_num_entries()) {
        return NULL;
    }
    return &conf_led[num];
}

// led config store type 0 num 4
int conf_led_propagate(unsigned int numinst, unsigned int numfield, int32_t value)
{
    if (numinst>=conf_led_num_entries()) return -1;
    conf_led_t *conf = &conf_led[numinst];
    switch (numfield) {
    default: return -1;
    case conf_numfield_led_defprog:
        conf->defprog = value;
        break;
    }
    return 0;
}


int32_t conf_led_default_value(unsigned int numinst, unsigned int numfield, unsigned int boardnum)
{
    //if (numinst>=conf_led_num_entries()) return 0;
    switch (numfield) {
    default: return 0;
    case conf_numfield_led_defprog:
        return 0;
    }
    return 0;
}





// end
