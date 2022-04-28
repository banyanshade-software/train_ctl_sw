// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_utest.h"
#include "conf_utest.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"



// used only for unit test



#ifdef TRN_BOARD_MAINV04

unsigned int conf_utest_num_entries(void)
{
    return 1; // 1 
}

static conf_utest_t conf_utest[1] = {
  {     // 0
     .fixed = 42,
     .alpha = 1001,
     .beta = 1001,
  }
};

#endif // TRN_BOARD_MAINV04




#ifdef TRN_BOARD_MAINV0

unsigned int conf_utest_num_entries(void)
{
    return 1; // 1 
}

static conf_utest_t conf_utest[1] = {
  {     // 0
     .fixed = 42,
     .alpha = 1000,
     .beta = 1000,
  }
};

#endif // TRN_BOARD_MAINV0




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_utest_num_entries(void)
{
    return 1; // 1 
}

static conf_utest_t conf_utest[1] = {
  {     // 0
     .fixed = 42,
     .alpha = 1001,
     .beta = 1001,
  }
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_utest_num_entries(void)
{
    return 1; // 1 
}

static conf_utest_t conf_utest[1] = {
  {     // 0
     .fixed = 42,
     .alpha = 1001,
     .beta = 1001,
  }
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_utest_num_entries(void)
{
    return 1; // 1 
}

static conf_utest_t conf_utest[1] = {
  {     // 0
     .fixed = 42,
     .alpha = 1001,
     .beta = 1001,
  }
};

#endif // TRN_BOARD_MAIN_ZERO




#ifdef TRN_BOARD_SIMU

unsigned int conf_utest_num_entries(void)
{
    return 1; // 1 
}

static conf_utest_t conf_utest[1] = {
  {     // 0
     .fixed = 42,
     .alpha = 1001,
     .beta = 1001,
  }
};

#endif // TRN_BOARD_SIMU




const conf_utest_t *conf_utest_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_utest_num_entries()) {
        return NULL;
    }
    return &conf_utest[num];
}

// utest config store type 0 num 8
int conf_utest_propagate(unsigned int numinst, unsigned int numfield, int32_t value)
{
    if (numinst>=conf_utest_num_entries()) return -1;
    conf_utest_t *conf = &conf_utest[numinst];
    switch (numfield) {
    default: return -1;
    case conf_numfield_utest_alpha:
        conf->alpha = value;
        break;
    case conf_numfield_utest_beta:
        conf->beta = value;
        break;
    }
    return 0;
}


int32_t conf_utest_default_value(unsigned int numinst, unsigned int numfield, unsigned int boardnum)
{
    (void) boardnum;
    (void) numinst;
    //if (numinst>=conf_utest_num_entries()) return 0;
    switch (numfield) {
    default: return 0;
    case conf_numfield_utest_alpha:
        return 1001;
    case conf_numfield_utest_beta:
        return 1001;
    }
    return 0;
}





// end
