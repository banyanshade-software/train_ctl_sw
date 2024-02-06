// this file is generated automatically by config
// DO NOT EDIT


#include <stdint.h>
#include <stddef.h>
#include "conf_locomotive.h"
#include "conf_locomotive.propag.h"



// this code goes in all .c files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

// include board definitions

#include "trainctl_config.h"


    // this goes only in config_train.c


#ifdef TRN_BOARD_G4SLV1

unsigned int conf_locomotive_num_entries(void)
{
    return 0; // 0 
}

static conf_locomotive_t conf_locomotive[0] = {
};

#endif // TRN_BOARD_G4SLV1




#ifdef TRN_BOARD_G4MASTER1

unsigned int conf_locomotive_num_entries(void)
{
    return 8; // 8 
}

static conf_locomotive_t conf_locomotive[8] = {
  {     // 0
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 450,
  },
  {     // 1
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 2
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 3
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 4
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 5
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 6
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 7
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  }
};

#endif // TRN_BOARD_G4MASTER1




#ifdef TRN_BOARD_UNIT_TEST

unsigned int conf_locomotive_num_entries(void)
{
    return 4; // 4 
}

static conf_locomotive_t conf_locomotive[4] = {
  {     // 0
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 450,
  },
  {     // 1
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 2
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 3
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  }
};

#endif // TRN_BOARD_UNIT_TEST




#ifdef TRN_BOARD_UI

unsigned int conf_locomotive_num_entries(void)
{
    return 0; // 0 
}

static conf_locomotive_t conf_locomotive[0] = {
};

#endif // TRN_BOARD_UI




#ifdef TRN_BOARD_MAINV04

unsigned int conf_locomotive_num_entries(void)
{
    return 8; // 8 
}

static conf_locomotive_t conf_locomotive[8] = {
  {     // 0
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 450,
  },
  {     // 1
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 2
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 3
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 4
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 5
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 6
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 7
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  }
};

#endif // TRN_BOARD_MAINV04




#ifdef TRN_BOARD_MAINV0

unsigned int conf_locomotive_num_entries(void)
{
    return 8; // 8 
}

static conf_locomotive_t conf_locomotive[8] = {
  {     // 0
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 450,
  },
  {     // 1
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 2
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 3
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 4
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 5
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 6
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 7
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  }
};

#endif // TRN_BOARD_MAINV0




#ifdef TRN_BOARD_DISPATCHER

unsigned int conf_locomotive_num_entries(void)
{
    return 0; // 0 
}

static conf_locomotive_t conf_locomotive[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

unsigned int conf_locomotive_num_entries(void)
{
    return 0; // 0 
}

static conf_locomotive_t conf_locomotive[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

unsigned int conf_locomotive_num_entries(void)
{
    return 0; // 0 
}

static conf_locomotive_t conf_locomotive[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




#ifdef TRN_BOARD_SIMU

unsigned int conf_locomotive_num_entries(void)
{
    return 8; // 8 
}

static conf_locomotive_t conf_locomotive[8] = {
  {     // 0
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 450,
  },
  {     // 1
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 2
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 3
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 4
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 5
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 6
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  },
  {     // 7
     .pidctl = {
     .kP = 500,
     .kI = 100,
     .kD = 500,
     },
     .inertia = {
     .dec = 120,
     .acc = 120,
     },
     .volt_policy = vpolicy_normal,
     .enable_inertia = 0,
     .enable_pid = 1,
     .fix_bemf = 0,
     .en_spd2pow = 0,
     .reversed = 0,
     .min_power = 0,
     .notify_pose = 0,
     .bemfIIR = 0,
     .postIIR = 0,
     .slipping = 120,
     .pose_per_cm = 500,
  }
};

#endif // TRN_BOARD_SIMU




const conf_locomotive_t *conf_locomotive_get(int num)
{
  if (num<0) return NULL;
    if ((unsigned int)num>=conf_locomotive_num_entries()) {
        return NULL;
    }
    return &conf_locomotive[num];
}



void *conf_locomotive_ptr(void)
{
    return &conf_locomotive[0];
}



int32_t conf_locomotive_local_get(unsigned int fieldnum, unsigned int instnum)
{
    const conf_locomotive_t *c = conf_locomotive_get(instnum);
    if (!c) return 0;
    switch (fieldnum) {
    default: break;
    case conf_numfield_pidctl_kP:
        return c->pidctl.kP;
    case conf_numfield_pidctl_kI:
        return c->pidctl.kI;
    case conf_numfield_pidctl_kD:
        return c->pidctl.kD;
    case conf_numfield_inertia_dec:
        return c->inertia.dec;
    case conf_numfield_inertia_acc:
        return c->inertia.acc;
    case conf_numfield_locomotive_volt_policy:
        return c->volt_policy;
    case conf_numfield_locomotive_enable_inertia:
        return c->enable_inertia;
    case conf_numfield_locomotive_enable_pid:
        return c->enable_pid;
    case conf_numfield_locomotive_reversed:
        return c->reversed;
    case conf_numfield_locomotive_bemfIIR:
        return c->bemfIIR;
    case conf_numfield_locomotive_postIIR:
        return c->postIIR;
    case conf_numfield_locomotive_slipping:
        return c->slipping;
    case conf_numfield_locomotive_pose_per_cm:
        return c->pose_per_cm;
    }
    return 0;
}



void conf_locomotive_local_set(unsigned int fieldnum, unsigned int instnum, int32_t v)
{
    conf_locomotive_t *ca = (conf_locomotive_t *) conf_locomotive_ptr();
    if (!ca) return;
    conf_locomotive_t *c = &ca[instnum];
    switch (fieldnum) {
    default: break;
    case conf_numfield_pidctl_kP:
        c->pidctl.kP = v;
        break;
    case conf_numfield_pidctl_kI:
        c->pidctl.kI = v;
        break;
    case conf_numfield_pidctl_kD:
        c->pidctl.kD = v;
        break;
    case conf_numfield_inertia_dec:
        c->inertia.dec = v;
        break;
    case conf_numfield_inertia_acc:
        c->inertia.acc = v;
        break;
    case conf_numfield_locomotive_volt_policy:
        c->volt_policy = v;
        break;
    case conf_numfield_locomotive_enable_inertia:
        c->enable_inertia = v;
        break;
    case conf_numfield_locomotive_enable_pid:
        c->enable_pid = v;
        break;
    case conf_numfield_locomotive_reversed:
        c->reversed = v;
        break;
    case conf_numfield_locomotive_bemfIIR:
        c->bemfIIR = v;
        break;
    case conf_numfield_locomotive_postIIR:
        c->postIIR = v;
        break;
    case conf_numfield_locomotive_slipping:
        c->slipping = v;
        break;
    case conf_numfield_locomotive_pose_per_cm:
        c->pose_per_cm = v;
        break;
    }

}

// locomotive config store type 1 num 6



// end