// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_utestloc.h"
#include "conf_utestloc.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"



#ifdef TRN_BOARD_MAIN

unsigned int conf_utestloc_num_entries(void)
{
    return 4; // 4 
}

static conf_utestloc_t conf_utestloc[4] = {
  {     // 0
     .fixed = 42,
     .alpha = 1000,
     .beta = 2000,
  },
  {     // 1
     .fixed = 42,
     .alpha = 1001,
     .beta = 2001,
  },
  {     // 2
     .fixed = 42,
     .alpha = 1002,
     .beta = 2002,
  },
  {     // 3
     .fixed = 42,
     .alpha = 1000,
     .beta = 2000,
  }
};

#endif // TRN_BOARD_MAIN




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_utestloc_num_entries(void)
{
    return 0; // 0 
}

static conf_utestloc_t conf_utestloc[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_utestloc_num_entries(void)
{
    return 0; // 0 
}

static conf_utestloc_t conf_utestloc[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_utestloc_num_entries(void)
{
    return 0; // 0 
}

static conf_utestloc_t conf_utestloc[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




#ifdef TRN_BOARD_SIMU

unsigned int conf_utestloc_num_entries(void)
{
    return 2; // 2 
}

static conf_utestloc_t conf_utestloc[2] = {
  {     // 0
     .fixed = 42,
     .alpha = 1000,
     .beta = 2000,
  },
  {     // 1
     .fixed = 42,
     .alpha = 1000,
     .beta = 2000,
  }
};

#endif // TRN_BOARD_SIMU




const conf_utestloc_t *conf_utestloc_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_utestloc_num_entries()) {
        return NULL;
    }
    return &conf_utestloc[num];
}



void *conf_utestloc_ptr(void)
{
    return &conf_utestloc[0];
}



int32_t conf_utestloc_local_get(unsigned int fieldnum, unsigned int instnum)
{
    const conf_utestloc_t *c = conf_utestloc_get(instnum);
    if (!c) return 0;
    switch (fieldnum) {
    default: break;
    case conf_numfield_utestloc_alpha:
        return c->alpha;
    case conf_numfield_utestloc_beta:
        return c->beta;
    }
    return 0;
}



void conf_utestloc_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v)
{
    conf_utestloc_t *ca = (conf_utestloc_t *) conf_utestloc_ptr();
    if (!ca) return;
    conf_utestloc_t *c = &ca[instnum];
    switch (fieldnum) {
    default: break;
    case conf_numfield_utestloc_alpha:
        c->alpha = v;
        break;
    case conf_numfield_utestloc_beta:
        c->beta = v;
        break;
    }

}

// utestloc config store type 1 num 9



// end
