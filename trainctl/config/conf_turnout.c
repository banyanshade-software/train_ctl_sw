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

unsigned int conf_turnout_num_entries(void)
{
    return 3; // 3 
}

static conf_turnout_t conf_turnout[3] = {
  {     // 0
     .cmd_port = GPIOA,
     .pinA = GPIO_PIN_9,
     .pinB = GPIO_PIN_10,
     .reverse = 0,
  },
  {     // 1
     .cmd_port = GPIOE,
     .pinA = GPIO_PIN_0,
     .pinB = GPIO_PIN_1,
     .reverse = 0,
  },
  {     // 2
     .cmd_port = GPIOE,
     .pinA = GPIO_PIN_8,
     .pinB = GPIO_PIN_7,
     .reverse = 0,
  }
};

#endif // TRN_BOARD_MAIN




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_turnout_num_entries(void)
{
    return 0; // 0 
}

static conf_turnout_t conf_turnout[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_turnout_num_entries(void)
{
    return 0; // 0 
}

static conf_turnout_t conf_turnout[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_turnout_num_entries(void)
{
    return 0; // 0 
}

static conf_turnout_t conf_turnout[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




const conf_turnout_t *conf_turnout_get(int num)
{
  if (num<0) return NULL;
    if (num>=conf_turnout_num_entries()) {
        return NULL;
    }
    return &conf_turnout[num];
}

// turnout config store type 0 num 3
int conf_turnout_propagate(unsigned int numinst, unsigned int numfield, int32_t value)
{
    if (numinst>=conf_turnout_num_entries()) return -1;
    conf_turnout_t *conf = &conf_turnout[numinst];
    switch (numfield) {
    default: return -1;
    case conf_numfield_turnout_reverse:
        conf->reverse = value;
        break;
    }
    return 0;
}


int32_t conf_turnout_default_value(unsigned int numinst, unsigned int numfield, unsigned int boardnum)
{
    (void) boardnum;
    //if (numinst>=conf_turnout_num_entries()) return 0;
    switch (numfield) {
    default: return 0;
    case conf_numfield_turnout_reverse:
        return 0;
    }
    return 0;
}





// end
