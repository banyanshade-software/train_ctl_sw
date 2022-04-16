// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_train_H_
#define _conf_train_H_

#include <stdint.h>

// this code goes in all .h files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

#ifndef TRAIN_SIMU
#include "trainctl_config.h"
#else
#include "train_simu.h"
#endif



typedef enum train_volt_policy /*: uint8_t*/ {
    vpolicy_normal = 0,
    vpolicy_pure_pwm,
    vpolicy_pure_volt,
} train_volt_policy_t;

#include "conf_pidctl.h"
#include "conf_inertia.h"


typedef struct conf_train {
    struct conf_pidctl pidctl;
    struct conf_inertia inertia;
    train_volt_policy_t volt_policy;
    uint8_t enabled:1;
    uint8_t enable_inertia:2;
    uint8_t enable_pid:1;
    uint8_t fix_bemf:1;
    uint8_t en_spd2pow:1;
    uint8_t reversed:1;
    uint8_t min_power;
    uint8_t notify_speed:1;
    uint8_t notify_pose:1;
    uint8_t bemfIIR;
    uint8_t postIIR;
    uint8_t slipping;
    uint16_t pose_per_cm;
    uint8_t trainlen_left;
    uint8_t trainlen_right;
} conf_train_t;


int conf_train_num_entries(void);
const conf_train_t *conf_train_get(int num);




#endif
