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



void *conf_globparam_ptr(void)
{
    return &conf_globparam[0];
}



int32_t conf_globparam_local_get(unsigned int fieldnum, unsigned int instnum)
{
    const conf_globparam_t *c = conf_globparam_get(instnum);
    if (!c) return 0;
    switch (fieldnum) {
    case conf_numfield_pwmfreq:
        return c->pwmfreq;
    case conf_numfield_test_mode:
        return c->test_mode;
    case conf_numfield_oscilo:
        return c->oscilo;
    }
    return 0;
}



void conf_globparam_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v)
{
    conf_globparam_t *ca = (conf_globparam_t *) conf_globparam_ptr();
    if (!ca) return;
    conf_globparam_t *c = &ca[instnum];
    switch (fieldnum) {
    case conf_numfield_pwmfreq:
        c->pwmfreq = v;
        break;
    case conf_numfield_test_mode:
        c->test_mode = v;
        break;
    case conf_numfield_oscilo:
        c->oscilo = v;
        break;
    }

}

// globparam config store type 1 num 10



// end
