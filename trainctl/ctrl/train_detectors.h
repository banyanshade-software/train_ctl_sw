//
//  train_detectors.h
//  train_throttle
//
//  Created by Daniel Braun on 09/07/2023.
//  Copyright © 2023 Daniel BRAUN. All rights reserved.
//

#ifndef train_detectors_h
#define train_detectors_h

#include "misc.h"
#include "../msg/trainmsg.h"

/*
 we have several detectors, that will be run sequencially
 The first detector typically detect train presence (by applying a short
 pwm power, and measuring consumed current through ina3221.
 The next detectors will detect trains on lsblk that may not be moninoted by ina3221
 (no info on current consumtion) and try to recognize locomotive type,
 either by measuring frequency response or by measuring impulse response.
 
 For each canton, each detector will run a sequence of start steps, and,
 spcifically if detection is interrupted, a sequence of stop steps
 
 Each step returns normally 0, and will return -1 if steps should be interrupted
 for this canton.
 */

typedef struct st_detector_step {
    const struct st_detector_step *nextstep;
    int (*detect_start_canton)(xblkaddr_t);
    int (*detect_stop_canton)(xblkaddr_t);
} train_detector_step_t;

typedef struct st_detector {
    const struct st_detector *next;
    // init and deinit called when starting/ending detector
    void (*const detect_init)(uint8_t p); // parameter unused, 0
    void (*const detect_deinit)(void);
    // parse handles CMD_DETECTION_REPORT
    int (*const detect_parse)(msg_64_t *m);
    // ...
    const train_detector_step_t *steps;
    const char *name; // for debug
} train_detector_t;


#if 0
// now all private
/* start actions step */
int detect_step_check_canton_exist(xblkaddr_t);
int detect_step_notify_ui(xblkaddr_t);
int detect_step_start_pwm(xblkaddr_t);
int detect_step_wait(xblkaddr_t);
int detect_step_start_inameas(xblkaddr_t);
int detect_step_stop_pwm(xblkaddr_t);

/* stop actions steps */
int detect_step_stop_pwm(xblkaddr_t);
int detect_step_stop_inameas(xblkaddr_t);
int detect_step_stop_notify_ui(xblkaddr_t);
#endif

/* detectors */
extern const train_detector_t alldetectors;

#endif /* train_detector_h */
