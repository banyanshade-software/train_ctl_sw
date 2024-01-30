// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_train.h"
#include "conf_train.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"


    // this goes only in config_train.c


#ifdef TRN_BOARD_G4SLV1

unsigned int conf_train_num_entries(void)
{
    return 0; // 0 
}

static conf_train_t conf_train[0] = {
};

#endif // TRN_BOARD_G4SLV1




#ifdef TRN_BOARD_G4MASTER1

unsigned int conf_train_num_entries(void)
{
    return 8; // 8 
}

static conf_train_t conf_train[8] = {
  {     // 0
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 1,
     .trainlen_right_cm = 1,
  },
  {     // 1
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 15,
     .trainlen_right_cm = 2,
  },
  {     // 2
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 3
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 4
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 5
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 6
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 7
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  }
};

#endif // TRN_BOARD_G4MASTER1




#ifdef TRN_BOARD_UNIT_TEST

unsigned int conf_train_num_entries(void)
{
    return 4; // 4 
}

static conf_train_t conf_train[4] = {
  {     // 0
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 1,
     .trainlen_right_cm = 1,
  },
  {     // 1
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 15,
     .trainlen_right_cm = 2,
  },
  {     // 2
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 3
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  }
};

#endif // TRN_BOARD_UNIT_TEST




#ifdef TRN_BOARD_UI

unsigned int conf_train_num_entries(void)
{
    return 0; // 0 
}

static conf_train_t conf_train[0] = {
};

#endif // TRN_BOARD_UI




#ifdef TRN_BOARD_MAINV04

unsigned int conf_train_num_entries(void)
{
    return 8; // 8 
}

static conf_train_t conf_train[8] = {
  {     // 0
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 1,
     .trainlen_right_cm = 1,
  },
  {     // 1
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 15,
     .trainlen_right_cm = 2,
  },
  {     // 2
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 3
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 4
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 5
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 6
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 7
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  }
};

#endif // TRN_BOARD_MAINV04




#ifdef TRN_BOARD_MAINV0

unsigned int conf_train_num_entries(void)
{
    return 8; // 8 
}

static conf_train_t conf_train[8] = {
  {     // 0
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 1,
     .trainlen_right_cm = 1,
  },
  {     // 1
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 15,
     .trainlen_right_cm = 2,
  },
  {     // 2
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 3
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 4
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 5
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 6
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 7
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  }
};

#endif // TRN_BOARD_MAINV0




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_train_num_entries(void)
{
    return 0; // 0 
}

static conf_train_t conf_train[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_train_num_entries(void)
{
    return 0; // 0 
}

static conf_train_t conf_train[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_train_num_entries(void)
{
    return 0; // 0 
}

static conf_train_t conf_train[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




#ifdef TRN_BOARD_SIMU

unsigned int conf_train_num_entries(void)
{
    return 4; // 4 
}

static conf_train_t conf_train[4] = {
  {     // 0
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 1,
     .trainlen_right_cm = 1,
  },
  {     // 1
     .locotype = 0,
     .enabled = 1,
     .notify_pose = 0,
     .trainlen_left_cm = 15,
     .trainlen_right_cm = 2,
  },
  {     // 2
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  },
  {     // 3
     .locotype = 0,
     .enabled = 0,
     .notify_pose = 0,
     .trainlen_left_cm = 10,
     .trainlen_right_cm = 10,
  }
};

#endif // TRN_BOARD_SIMU




const conf_train_t *conf_train_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_train_num_entries()) {
        return NULL;
    }
    return &conf_train[num];
}



void *conf_train_ptr(void)
{
    return &conf_train[0];
}



int32_t conf_train_local_get(unsigned int fieldnum, unsigned int instnum)
{
    const conf_train_t *c = conf_train_get(instnum);
    if (!c) return 0;
    switch (fieldnum) {
    default: break;
    case conf_numfield_train_locotype:
        return c->locotype;
    case conf_numfield_train_enabled:
        return c->enabled;
    case conf_numfield_train_trainlen_left_cm:
        return c->trainlen_left_cm;
    case conf_numfield_train_trainlen_right_cm:
        return c->trainlen_right_cm;
    }
    return 0;
}



void conf_train_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v)
{
    conf_train_t *ca = (conf_train_t *) conf_train_ptr();
    if (!ca) return;
    conf_train_t *c = &ca[instnum];
    switch (fieldnum) {
    default: break;
    case conf_numfield_train_locotype:
        c->locotype = v;
        break;
    case conf_numfield_train_enabled:
        c->enabled = v;
        break;
    case conf_numfield_train_trainlen_left_cm:
        c->trainlen_left_cm = v;
        break;
    case conf_numfield_train_trainlen_right_cm:
        c->trainlen_right_cm = v;
        break;
    }

}

// train config store type 1 num 1



// end
