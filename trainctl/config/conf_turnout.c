// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_turnout.h"
#include "conf_turnout.propag.h"



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

int conf_turnout_num_entries(void)
{
    return 3; // 3 
}

static conf_turnout_t conf_turnout[3] = {
  {     // 0
     .port = GPIOA,
     .pinA = GPIO_PIN_9,
     .pinB = GPIO_PIN_10,
     .reverse = 0,
  },
  {     // 1
     .port = GPIOE,
     .pinA = GPIO_PIN_0,
     .pinB = GPIO_PIN_1,
     .reverse = 0,
  },
  {     // 2
     .port = GPIOE,
     .pinA = GPIO_PIN_8,
     .pinB = GPIO_PIN_7,
     .reverse = 0,
  }
};

#endif // TRN_BOARD_MAIN




#ifdef TRN_BOARD_DISPATCHER

int conf_turnout_num_entries(void)
{
    return 0; // 0 
}

static conf_turnout_t conf_turnout[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

int conf_turnout_num_entries(void)
{
    return 0; // 0 
}

static conf_turnout_t conf_turnout[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

int conf_turnout_num_entries(void)
{
    return 0; // 0 
}

static conf_turnout_t conf_turnout[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




const conf_turnout_t *conf_turnout_get(int num)
{
  if (num<0) return NULL;
    if (num>=conf_turnout_num_entries()) return NULL;
    return &conf_turnout[num];
}

// turnout config store type 0 num 3
int conf_turnout_propagate(int numinst, int numfield, int32_t value)
{
    if (numinst>=conf_turnout_num_entries()) return -1;
    conf_turnout_t *conf = &conf_turnout[numinst];
    switch (numfield) {
    default: return -1;
    case conf_numfield_reverse:
        conf->reverse = value;
        break;
    }
    return 0;
}





// end