// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_globparam.h"
#include "conf_globparam.propag.h"



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

int conf_globparam_num_entries(void)
{
    return 1; // 1 
}

static conf_globparam_t conf_globparam[1] = {
  {     // 0
     .pwmfreq = 100,
     .test_mode = 0,
     .oscilo = 0,
  }
};

#endif // TRN_BOARD_MAIN




#ifdef TRN_BOARD_DISPATCHER

int conf_globparam_num_entries(void)
{
    return 0; // 0 
}

static conf_globparam_t conf_globparam[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

int conf_globparam_num_entries(void)
{
    return 0; // 0 
}

static conf_globparam_t conf_globparam[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

int conf_globparam_num_entries(void)
{
    return 0; // 0 
}

static conf_globparam_t conf_globparam[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




const conf_globparam_t *conf_globparam_get(int num)
{
  if (num<0) return NULL;
    if (num>=conf_globparam_num_entries()) {
        return NULL;
    }
    return &conf_globparam[num];
}

// globparam config store type 1 num 10



// end
