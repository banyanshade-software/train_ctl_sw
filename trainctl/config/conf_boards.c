// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_boards.h"
#include "conf_boards.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"



#ifdef TRN_BOARD_MAIN

unsigned int conf_boards_num_entries(void)
{
    return 8; // 8 
}

static conf_boards_t conf_boards[8] = {
  {     // 0
     .uuid = 0,
     .board_type = 0,
     .disable = 0,
  },
  {     // 1
     .uuid = 0,
     .board_type = 0,
     .disable = 0,
  },
  {     // 2
     .uuid = 0,
     .board_type = 0,
     .disable = 0,
  },
  {     // 3
     .uuid = 0,
     .board_type = 0,
     .disable = 0,
  },
  {     // 4
     .uuid = 0,
     .board_type = 0,
     .disable = 0,
  },
  {     // 5
     .uuid = 0,
     .board_type = 0,
     .disable = 0,
  },
  {     // 6
     .uuid = 0,
     .board_type = 0,
     .disable = 0,
  },
  {     // 7
     .uuid = 0,
     .board_type = 0,
     .disable = 0,
  }
};

#endif // TRN_BOARD_MAIN




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_boards_num_entries(void)
{
    return 0; // 0 
}

static conf_boards_t conf_boards[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_boards_num_entries(void)
{
    return 0; // 0 
}

static conf_boards_t conf_boards[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_boards_num_entries(void)
{
    return 0; // 0 
}

static conf_boards_t conf_boards[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




#ifdef TRN_BOARD_SIMU

unsigned int conf_boards_num_entries(void)
{
    return 0; // 0 
}

static conf_boards_t conf_boards[0] = {
};

#endif // TRN_BOARD_SIMU




const conf_boards_t *conf_boards_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_boards_num_entries()) {
        return NULL;
    }
    return &conf_boards[num];
}



void *conf_boards_ptr(void)
{
    return &conf_boards[0];
}



int32_t conf_boards_local_get(unsigned int fieldnum, unsigned int instnum)
{
    const conf_boards_t *c = conf_boards_get(instnum);
    if (!c) return 0;
    switch (fieldnum) {
    default: break;
    case conf_numfield_boards_uuid:
        return c->uuid;
    case conf_numfield_boards_board_type:
        return c->board_type;
    case conf_numfield_boards_disable:
        return c->disable;
    }
    return 0;
}



void conf_boards_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v)
{
    conf_boards_t *ca = (conf_boards_t *) conf_boards_ptr();
    if (!ca) return;
    conf_boards_t *c = &ca[instnum];
    switch (fieldnum) {
    default: break;
    case conf_numfield_boards_uuid:
        c->uuid = v;
        break;
    case conf_numfield_boards_board_type:
        c->board_type = v;
        break;
    case conf_numfield_boards_disable:
        c->disable = v;
        break;
    }

}

// boards config store type 1 num 11



// end
