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

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#include <stdio.h>
#endif



#ifdef TRN_BOARD_MAIN

int conf_boards_num_entries(void)
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

int conf_boards_num_entries(void)
{
    return 0; // 0 
}

static conf_boards_t conf_boards[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

int conf_boards_num_entries(void)
{
    return 0; // 0 
}

static conf_boards_t conf_boards[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

int conf_boards_num_entries(void)
{
    return 0; // 0 
}

static conf_boards_t conf_boards[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




const conf_boards_t *conf_boards_get(int num)
{
  if (num<0) return NULL;
    if (num>=conf_boards_num_entries()) return NULL;
    return &conf_boards[num];
}

// boards config store type 1 num 1



// end
