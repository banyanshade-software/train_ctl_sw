//
//  ctrlP.h
//  train_throttle
//
//  Created by Daniel BRAUN on 28/10/2021.
//  Copyright © 2021 Daniel BRAUN. All rights reserved.
//

#ifndef ctrlP_h
#define ctrlP_h

#include "railconfig.h"
// -----------------------------------------------------------
//per train stucture


// timers number
#define TLEAVE_C1      0
#define TAUTO          1
#define NUM_TIMERS     2

// timers values in tick (ms)
#define TLEAVE_C1_VALUE 20
#define TGUARD_C1_VALUE 1

typedef struct {
    train_mode_t   _mode;
    train_state_t  _state;

    uint16_t tick_flags;

    uint16_t _target_speed;
    int8_t   _dir;
    
    uint8_t  c1c2:1;
    uint8_t  pose2_set:1;
    uint8_t  pose2_is_blk_wait:1;
    uint8_t trig_eoseg:1;
    uint8_t measure_pose_percm:1;
    
    uint8_t     can1_addr;
    lsblk_num_t c1_sblk;
    uint8_t     can2_addr;

    uint16_t spd_limit;
    int16_t desired_speed;
    //int16_t desired_speed2;
    //uint8_t limited:1;

    //uint16_t behaviour_flags;
    uint32_t timertick[NUM_TIMERS];
    
    int32_t curposmm;
    int32_t beginposmm; // left side is 0, mm of right side
    
    const uint8_t *route;
    uint8_t routeidx;
    uint8_t trigu1:4;
    uint8_t got_texp:1;
    uint8_t got_u1:1;
} train_ctrl_t;

/*
 tick_flags values
 */
//#define _TFLAG_C1_CHANGED    (1<<0) obsolete
//#define _TFLAG_POSE_TRIG1  (1<<1)
#define _TFLAG_NEED_C2       (1<<1)
#define _TFLAG_C1LSB_CHANGED (1<<2)
#define _TFLAG_POSE_TRIG2    (1<<3)
#define _TFLAG_STOP_DETECTED (1<<4)
#define _TFLAG_OCC_CHANGED   (1<<5)
#define _TFLAG_STATE_CHANGED (1<<6)
#define _TFLAG_DIR_CHANGED   (1<<7)
#define _TFLAG_TSPD_CHANGED  (1<<8)
#define _TFLAG_LIMIT_CHANGED (1<<9)
#define _TFLAG_DSPD_CHANGED  (1<<10)
#define _TFLAG_C1_CHANGED    (1<<11)
#define _TFLAG_C2_CHANGED    (1<<11) //xx
#define _TFLAG_MODE_CHANGED  (1<<14)


int ctrl2_tick_process(int tidx, train_ctrl_t *tvars, const train_config_t *tconf, int8_t occupency_changed);
void ctrl2_init_train(int tidx, train_ctrl_t *tvars,
                      lsblk_num_t sblk);

void ctrl2_upcmd_set_desired_speed(int tidx, train_ctrl_t *tvars, int16_t desired_speed);
void _ctrl2_upcmd_set_desired_speed(int tidx, train_ctrl_t *tvars, int16_t desired_speed);
void ctrl2_upcmd_settrigU1(int tidx, train_ctrl_t *tvars, uint8_t t);

void ctrl2_set_mode(int tidx, train_ctrl_t *tvar, train_mode_t mode);

void ctrl2_evt_entered_c2(int tidx, train_ctrl_t *tvar, uint8_t from_bemf);
void ctrl2_evt_leaved_c1(int tidx, train_ctrl_t *tvars);
void ctrl2_evt_entered_c1(int tidx, train_ctrl_t *tvars, _UNUSED_ uint8_t from_bemf);
void ctrl2_evt_leaved_c2(int tidx, train_ctrl_t *tvar);


void ctrl2_set_state(int tidx, train_ctrl_t *tvar, train_state_t ns);
void ctrl2_stop_detected(int tidx, train_ctrl_t *tvars);
void ctrl2_set_dir(int tidx, train_ctrl_t *tvar, int8_t dir);
void ctrl2_set_tspeed(int tidx, train_ctrl_t *tvar, uint16_t tspeed);
void ctrl2_check_alreadystopped(int tidx, train_ctrl_t *tvar);
void ctrl2_check_checkstart(int tidx, train_ctrl_t *tvar);
void ctrl2_check_stop(int tidx, train_ctrl_t *tvar);
void ctrl2_apply_speed_limit(int tidx, train_ctrl_t *tvar);
void ctrl2_update_topo(int tidx, train_ctrl_t *tvar, const train_config_t *tconf, int32_t *ppose1);
void ctrl2_update_c2(int tidx, train_ctrl_t *tvar, const train_config_t *tconf, int32_t *ppose0);
void ctrl2_notify_state(int tidx, train_ctrl_t *tvar);
void ctrl2_sendlow_tspd(int tidx, train_ctrl_t *tvar);
void ctrl2_sendlow_c1c2(int tidx, train_ctrl_t *tvar);
int  ctrl2_evt_pose_triggered(int tidx, train_ctrl_t *tvar, uint8_t ca_addr, uint8_t trigbits, int16_t cposd10);
void ctrl2_evt_stop_detected(int tidx, train_ctrl_t *tvar, int32_t pose);

void ctrl_set_pose_trig(int numtrain, int32_t pose, int n);

#define ignore_bemf_presence 0
#define ignore_ina_presence  1


void ctrl_reset_timer(int tidx, train_ctrl_t *tvar, int numtimer);
void ctrl_set_timer(int tidx, train_ctrl_t *tvar, int numtimer, uint32_t tval);


/*
// behaviour_flags
#define BEHAVE_STOPPED        (1<<1)        // 2
#define BEHAVE_EOT1            (1<<2)        // 4
#define BEHAVE_EOT2            (1<<3)        // 8
#define BEHAVE_BLKW            (1<<4)        // 16
#define BEHAVE_PTRIG            (1<<5)        // 32
#define BEHAVE_RESTARTBLK     (1<<6)        // 64
#define BEHAVE_TBEHAVE        (1<<7)        // 128
#define BEHAVE_CHBKLK        (1<<8)        // 256
*/

extern uint8_t ctrl_flag_notify_speed ;


// provided by ctrl.c
extern int ctrl2_set_turnout(int tn, int v, int train_num); // ctrl.c
extern void ctrl2_send_led(uint8_t led_num, uint8_t prog_num);


#endif /* ctrlP_h */
