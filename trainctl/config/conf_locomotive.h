// this file is generated automatically by config
// DO NOT EDIT


#ifndef _conf_locomotive_H_
#define _conf_locomotive_H_

#include <stdint.h>

// this code goes in all .h files
/*
 * Z-ATC configuration store
 * D. BRAUN 2022
 */

#include "trainctl_config.h"



typedef enum train_volt_policy /*: uint8_t*/ {
    vpolicy_normal = 0,
    vpolicy_pure_pwm,
    vpolicy_pure_volt,
} train_volt_policy_t;

#include "conf_pidctl.h"
#include "conf_inertia.h"


typedef struct conf_locomotive {
    struct conf_pidctl pidctl;
    struct conf_inertia inertia;
    train_volt_policy_t volt_policy;
    uint8_t enable_inertia:2;
    uint8_t enable_pid:1;
    uint8_t fix_bemf:1;
    uint8_t en_spd2pow:1;
    uint8_t reversed:1;
    uint8_t min_power;
    uint8_t notify_pose:1;
    uint8_t bemfIIR;
    uint8_t postIIR;
    uint8_t slipping;
    uint16_t pose_per_cm;
} conf_locomotive_t;


unsigned int conf_locomotive_num_entries(void);
const conf_locomotive_t *conf_locomotive_get(int num);



#ifdef TRN_BOARD_G4SLV1
#define NUM_LOCOMOTIVES 0 // 0 
#endif // TRN_BOARD_G4SLV1



#ifdef TRN_BOARD_G4MASTER1
#define NUM_LOCOMOTIVES 8 // 8 
#endif // TRN_BOARD_G4MASTER1



#ifdef TRN_BOARD_UNIT_TEST
#define NUM_LOCOMOTIVES 4 // 4 
#endif // TRN_BOARD_UNIT_TEST



#ifdef TRN_BOARD_UI
#define NUM_LOCOMOTIVES 0 // 0 
#endif // TRN_BOARD_UI



#ifdef TRN_BOARD_MAINV04
#define NUM_LOCOMOTIVES 8 // 8 
#endif // TRN_BOARD_MAINV04



#ifdef TRN_BOARD_MAINV0
#define NUM_LOCOMOTIVES 8 // 8 
#endif // TRN_BOARD_MAINV0



#ifdef TRN_BOARD_DISPATCHER
#define NUM_LOCOMOTIVES 0 // 0 
#endif // TRN_BOARD_DISPATCHER



#ifdef TRN_BOARD_SWITCHER
#define NUM_LOCOMOTIVES 0 // 0 
#endif // TRN_BOARD_SWITCHER



#ifdef TRN_BOARD_MAIN_ZERO
#define NUM_LOCOMOTIVES 0 // 0 
#endif // TRN_BOARD_MAIN_ZERO



#ifdef TRN_BOARD_SIMU
#define NUM_LOCOMOTIVES 8 // 8 
#endif // TRN_BOARD_SIMU


#define MAX_LOCOMOTIVES 8




#endif
