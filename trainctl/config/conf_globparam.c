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

#include "trainctl_config.h"



#ifdef TRN_BOARD_UNIT_TEST

unsigned int conf_globparam_num_entries(void)
{
    return 0; // 0 
}

static conf_globparam_t conf_globparam[0] = {
};

#endif // TRN_BOARD_UNIT_TEST




#ifdef TRN_BOARD_UI

unsigned int conf_globparam_num_entries(void)
{
    return 0; // 0 
}

static conf_globparam_t conf_globparam[0] = {
};

#endif // TRN_BOARD_UI




#ifdef TRN_BOARD_MAINV04

unsigned int conf_globparam_num_entries(void)
{
    return 1; // 1 
}

static conf_globparam_t conf_globparam[1] = {
  {     // 0
     .pwmfreq = 100,
     .test_mode = 0,
     .oscillo = 0,
     .ignoreIna3221 = 0,
  }
};

#endif // TRN_BOARD_MAINV04




#ifdef TRN_BOARD_MAINV0

unsigned int conf_globparam_num_entries(void)
{
    return 1; // 1 
}

static conf_globparam_t conf_globparam[1] = {
  {     // 0
     .pwmfreq = 100,
     .test_mode = 0,
     .oscillo = 0,
     .ignoreIna3221 = 0,
  }
};

#endif // TRN_BOARD_MAINV0




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_globparam_num_entries(void)
{
    return 0; // 0 
}

static conf_globparam_t conf_globparam[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_globparam_num_entries(void)
{
    return 0; // 0 
}

static conf_globparam_t conf_globparam[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_globparam_num_entries(void)
{
    return 0; // 0 
}

static conf_globparam_t conf_globparam[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




#ifdef TRN_BOARD_SIMU

unsigned int conf_globparam_num_entries(void)
{
    return 1; // 1 
}

static conf_globparam_t conf_globparam[1] = {
  {     // 0
     .pwmfreq = 100,
     .test_mode = 0,
     .oscillo = 0,
     .ignoreIna3221 = 0,
  }
};

#endif // TRN_BOARD_SIMU




const conf_globparam_t *conf_globparam_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_globparam_num_entries()) {
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
    default: break;
    case conf_numfield_globparam_pwmfreq:
        return c->pwmfreq;
    case conf_numfield_globparam_test_mode:
        return c->test_mode;
    case conf_numfield_globparam_oscillo:
        return c->oscillo;
    case conf_numfield_globparam_ignoreIna3221:
        return c->ignoreIna3221;
    }
    return 0;
}



void conf_globparam_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v)
{
    conf_globparam_t *ca = (conf_globparam_t *) conf_globparam_ptr();
    if (!ca) return;
    conf_globparam_t *c = &ca[instnum];
    switch (fieldnum) {
    default: break;
    case conf_numfield_globparam_pwmfreq:
        c->pwmfreq = v;
        break;
    case conf_numfield_globparam_test_mode:
        c->test_mode = v;
        break;
    case conf_numfield_globparam_oscillo:
        c->oscillo = v;
        break;
    case conf_numfield_globparam_ignoreIna3221:
        c->ignoreIna3221 = v;
        break;
    }

}

// globparam config store type 1 num 10



// end
