//
//  train_detectors.c
//  train_throttle
//
//  Created by Daniel Braun on 10/07/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#include <stdio.h>

#include "misc.h"
#include "../msg/trainmsg.h"
#include "train_detectors.h"


/* start actions step */
int detect_step_check_canton_exist(xblkaddr_t detect_canton)
{
    lsblk_num_t lsb = any_lsblk_with_canton(detect_canton);
    if (lsb.n<0) {
        return  -1;
    }
    return 0;
}

int detect_step_notify_ui(xblkaddr_t detect_canton)
{
    if ((1)) {
        msg_64_t m = {0};
        
        m.cmd = CMD_UI_DETECT;
        m.v1 = detect_canton.v;
        m.v2 = 0;
        m.to = MA3_UI_GEN; //(UISUB_TFT);
        mqf_write_from_ctrl(&m);
    }
    return 0;
}

static void _start_canton(xblkaddr_t detect_canton, uint8_t method, uint16_t param)
{
    msg_64_t m = {0};
    m.from = MA1_CONTROL();
    TO_CANTON(m, detect_canton);
    m.cmd = CMD_START_DETECT_TRAIN;
    m.v2u = method;
    m.v1u = param; //%pwm
    mqf_write_from_ctrl(&m);
}
int detect_step_start_pwm(xblkaddr_t  detect_canton)
{
    _start_canton(detect_canton, 0, 20);
    return 0;
}
int detect_step_wait(xblkaddr_t detect_canton)
{
    
    return 0;
}
int detect_step_start_inameas(xblkaddr_t detect_canton)
{
    return 0;
}



/* stop actions steps */
int detect_step_stop_pwm(xblkaddr_t detect_canton)
{
    msg_64_t m = {0};
    TO_CANTON(m, detect_canton);
    //m.to = detect_canton; //MA_CANTON(0, canton);
    m.from = MA1_CONTROL();
    m.cmd = CMD_STOP_DETECT_TRAIN;
    mqf_write_from_ctrl(&m);
    return 0;
}
int detect_step_stop_inameas(xblkaddr_t detect_canton);


int detect_step_stop_notify_ui(xblkaddr_t detect_canton)
{
    if ((1)) {
        msg_64_t m = {0};
        
        m.cmd = CMD_UI_DETECT;
        m.v1 = -1;//detect_canton.v;
        m.v2 = 0;
        m.to = MA3_UI_GEN; //(UISUB_TFT);
        mqf_write_from_ctrl(&m);
    }
    return 0;
}
