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

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#include <stdio.h>
#endif


    // this goes only in config_train.c


#ifdef TRN_BOARD_MAIN

int conf_train_num_entries(void)
{
    return 8; // 8 
}

static conf_train_t conf_train[8] = {
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
     .enabled = 1,
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
     .trainlen_left = 6,
     .trainlen_right = 18,
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
     .enabled = 1,
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
     .trainlen_left = 15,
     .trainlen_right = 2,
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
     .enabled = 0,
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
     .trainlen_left = 10,
     .trainlen_right = 10,
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
     .enabled = 0,
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
     .trainlen_left = 10,
     .trainlen_right = 10,
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
     .enabled = 0,
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
     .trainlen_left = 10,
     .trainlen_right = 10,
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
     .enabled = 0,
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
     .trainlen_left = 10,
     .trainlen_right = 10,
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
     .enabled = 0,
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
     .trainlen_left = 10,
     .trainlen_right = 10,
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
     .enabled = 0,
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
     .trainlen_left = 10,
     .trainlen_right = 10,
  }
};

#endif // TRN_BOARD_MAIN




#ifdef TRN_BOARD_DISPATCHER

int conf_train_num_entries(void)
{
    return 0; // 0 
}

static conf_train_t conf_train[0] = {
};

#endif // TRN_BOARD_DISPATCHER




#ifdef TRN_BOARD_SWITCHER

int conf_train_num_entries(void)
{
    return 0; // 0 
}

static conf_train_t conf_train[0] = {
};

#endif // TRN_BOARD_SWITCHER




#ifdef TRN_BOARD_MAIN_ZERO

int conf_train_num_entries(void)
{
    return 0; // 0 
}

static conf_train_t conf_train[0] = {
};

#endif // TRN_BOARD_MAIN_ZERO




const conf_train_t *conf_train_get(int num)
{
  if (num<0) return NULL;
    if (num>=conf_train_num_entries()) {
        return NULL;
    }
    return &conf_train[num];
}

// train config store type 0 num 1
int conf_train_propagate(int numinst, int numfield, int32_t value)
{
    if (numinst>=conf_train_num_entries()) return -1;
    conf_train_t *conf = &conf_train[numinst];
    switch (numfield) {
    default: return -1;
    case conf_numfield_kP:
        conf->pidctl.kP = value;
     break;
    case conf_numfield_kI:
        conf->pidctl.kI = value;
     break;
    case conf_numfield_kD:
        conf->pidctl.kD = value;
     break;
    case conf_numfield_dec:
        conf->inertia.dec = value;
     break;
    case conf_numfield_acc:
        conf->inertia.acc = value;
     break;
    case conf_numfield_volt_policy:
        conf->volt_policy = value;
        break;
    case conf_numfield_enabled:
        conf->enabled = value;
        break;
    case conf_numfield_enable_inertia:
        conf->enable_inertia = value;
        break;
    case conf_numfield_enable_pid:
        conf->enable_pid = value;
        break;
    case conf_numfield_reversed:
        conf->reversed = value;
        break;
    case conf_numfield_bemfIIR:
        conf->bemfIIR = value;
        break;
    case conf_numfield_postIIR:
        conf->postIIR = value;
        break;
    case conf_numfield_slipping:
        conf->slipping = value;
        break;
    case conf_numfield_pose_per_cm:
        conf->pose_per_cm = value;
        break;
    case conf_numfield_trainlen_left:
        conf->trainlen_left = value;
        break;
    case conf_numfield_trainlen_right:
        conf->trainlen_right = value;
        break;
    }
    return 0;
}





// end
